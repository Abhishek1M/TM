/* 
 * File:   MsgHandler.h
 * Author: Abhishek M
 *
 * Created on 13 January, 2017, 3:46 PM
 */

#include <Poco/String.h>
#include <string>
#include <iostream>

#include <NodeInfo.h>
#include <Iso8583JSON.h>
#include "TranMgrDBHandler.h"
#include "Route.h"
#include "Config.h"
#include "HSMKeyMgmt.h"
#include "Track2.h"
#include "constants.h"
#include "WriteToDBEntity.h"

using namespace std;

#ifndef MSGHANDLER_H
#define MSGHANDLER_H

class MsgHandler
{
public:
    MsgHandler(Logger &logger) : td_logger(logger)
    {
    }

    virtual ~MsgHandler();

    bool processFinMsg(Iso8583JSON &msg, TranMgrDBHandler tmdbh);
    bool process0220Msg(Iso8583JSON &msg, TranMgrDBHandler tmdbh);
    void process0320Msg(Iso8583JSON &msg, TranMgrDBHandler tmdbh);
    bool process0420Msg(Iso8583JSON &msg, TranMgrDBHandler tmdbh);
    void process0520Msg(Iso8583JSON &msg, TranMgrDBHandler tmdbh);
    void process0620Msg(Iso8583JSON &msg, TranMgrDBHandler tmdbh);
    void process0800Msg(Iso8583JSON &msg, TranMgrDBHandler tmdbh);
    //

private:
    Logger &td_logger;

    bool constructPINBlock(Iso8583JSON &msg, string iss_key_name);
    bool getNewKeys(Iso8583JSON &msg);
    void clearRespFields(Iso8583JSON &msg);
    bool allowFallback(Iso8583JSON &msg);
    void updateDateTime(Iso8583JSON &msg);
    bool checkICAFlag(Iso8583JSON &msg);
};

/////////////////////////////////////////////////////////////////////////////////////

MsgHandler::~MsgHandler()
{
}

/////////////////////////////////////////////////////////////////////////////////////

bool MsgHandler::processFinMsg(Iso8583JSON &msg, TranMgrDBHandler tmdbh)
{
    bool route = false;
    Route rt;
    bool dbstatus;

    LocalDateTime now;
    string req_in = Poco::DateTimeFormatter::format(now, "%Y-%m-%d %H:%M:%s");

    msg.setExtendedField(_009_PREV_TRAN_NR, "0");

    if (msg.isFieldSet(_035_TRACK_2_DATA))
    {
        Track2 trk2(msg.getField(_035_TRACK_2_DATA));

        msg.setField(_002_PAN, trk2.getCardnumber());
        msg.setField(_014_DATE_EXPIRATION, trk2.getExpirydate());
        msg.setField(_040_SERVICE_RESTRICTION_CODE, trk2.getService_rest_code());
    }

    NodeInfo ni(msg.getNodeInfo());

    if (msg.isFieldSet(_067_EXTENDED_PAYMENT_CODE))
    {
        route = tmdbh.getExtdIssuerNode(msg, rt);
    }

    if (route == false)
    {
        route = tmdbh.getIssuerNode(msg, rt);

        if (route == false)
        {
            msg.setField(_039_RSP_CODE, _92_ROUTING_ERROR);
            dbstatus = tmdbh.addTMDeclineToTrans(msg);
            msg.setRspMsgType();

            td_logger.warning("No route found \n" + msg.dumpMsg());

            return false;
        }
    }

    // Update date/time in F073
    updateDateTime(msg);

    // Check Fallback
    if (allowFallback(msg) == false)
    {
        msg.setField(_039_RSP_CODE, _58_TRAN_NOT_PERMITTED_TERMINAL);
        dbstatus = tmdbh.addTMDeclineToTrans(msg);
        msg.setRspMsgType();

        clearRespFields(msg);

        td_logger.error("Fallback transaction not allowed\n" + msg.dumpMsg());

        return false;
    }

    // Perform ICA Check
    if (checkICAFlag(msg) == false)
    {
        msg.setField(_039_RSP_CODE, _L3_ICA_DISABLED);
        dbstatus = tmdbh.addTMDeclineToTrans(msg);
        msg.setRspMsgType();

        clearRespFields(msg);

        td_logger.error("Internation Transaction Not Allowed\n" + msg.dumpMsg());

        return false;
    }

    // Perform Merchant Limit Check

    // Perform PIN translation here
    if (msg.isFieldSet(_052_PIN_DATA) && (rt.getisskeyname().compare("N") != 0))
    {
        bool translate_status = constructPINBlock(msg, rt.getisskeyname());
        if (translate_status == false)
        {
            msg.setField(_039_RSP_CODE, _81_CRYPTO_ERROR);
            dbstatus = tmdbh.addTMDeclineToTrans(msg);
            msg.setRspMsgType();

            clearRespFields(msg);

            td_logger.error("Could not perform PIN translation\n" + msg.dumpMsg());

            return false;
        }
    }

    WriteToDBEntity w(msg.toMsg(), req_in, req_in, req_in, req_in, Config::_INSERT_TRANS);
    Config::commit_db.push(w);
    /*
    dbstatus = tmdbh.addToTrans(msg, req_in);
    if (dbstatus == false)
    {
        msg.setRspMsgType();
        msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);

        clearRespFields(msg);

        td_logger.error("DB entry unsuccessful \n" + msg.dumpMsg());

        return false;
    }
*/

    td_logger.debug("URL - " + rt.getUrl());
    td_logger.debug("Issuer Key Name - " + rt.getisskeyname());
    td_logger.debug("Process Name - " + rt.getPname());
    td_logger.debug("Timeout (seconds) - " + NumberFormatter::format(rt.getTimeout()));

    td_logger.trace("Request\n" + msg.toMsg());

    // Send request to Issuer Node
    string rsp;
    rsp = Utility::ofPostRequest(rt.getUrl(), msg.toMsg(), rt.getTimeout());

    td_logger.trace("Response\n" + rsp);

    string rsp_code = rsp.substr(0, 3);
    if (rsp_code.compare("NOK") == 0)
    {
        msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);

        td_logger.warning(rsp);
        td_logger.warning("Issuer/Process not available (NOK) - " + rt.getPname() + "(" + rt.getUrl() + ")");

        WriteToDBEntity wunok(msg.toMsg(), req_in, req_in, req_in, req_in, Config::_UPDATE_TRANS);
        Config::commit_db.push(wunok);

        msg.setRspMsgType();

        return false;
    }
    else if (rsp_code.compare("TMO") == 0)
    {
        // set the message in the timeout queue
        Config::msg.push(msg.toMsg());

        // after that form 91 response code and send back
        msg.setField(_039_RSP_CODE, _91_ISSUER_OR_SWITCH_INOPERATIVE);

        td_logger.warning(rsp);
        td_logger.warning("Issuer/Process not available (TMO) - " + rt.getPname() + "(" + rt.getUrl() + ")");

        WriteToDBEntity wutmo(msg.toMsg(), req_in, req_in, req_in, req_in, Config::_UPDATE_TRANS);
        Config::commit_db.push(wutmo);

        msg.setRspMsgType();

        return false;
    }

    Iso8583JSON msg_from_issuer;

    try
    {
        msg_from_issuer.parseMsg(rsp);
    }
    catch (Poco::Exception e)
    {
        td_logger.error(Poco::format("Exception in MsgHandler::processFinMsg : (%s)\nRsp: (%s)", e.what(), rsp));
        msg.setField(_039_RSP_CODE, _30_FORMAT_ERROR);

        dbstatus = tmdbh.updateTrans(msg);
        if (dbstatus == false)
        {
            msg.setRspMsgType();
            msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);
            clearRespFields(msg);
            td_logger.error("DB entry unsuccessful \n" + msg.dumpMsg());

            return false;
        }

        msg.setRspMsgType();
        return false;
    }

    // clear messages first
    clearRespFields(msg);

    // update with response from issuer
    msg.setMsgType(msg_from_issuer.getMsgType());
    msg.setField(_005_AMOUNT_SETTLE, msg_from_issuer.getField(_005_AMOUNT_SETTLE));
    msg.setField(_006_AMOUNT_CARDHOLDER_BILL, msg_from_issuer.getField(_006_AMOUNT_CARDHOLDER_BILL));
    msg.setField(_009_CONV_RATE_SETTLE, msg_from_issuer.getField(_009_CONV_RATE_SETTLE));
    msg.setField(_010_CONV_RATE_CARDHOLDER_BILL, msg_from_issuer.getField(_010_CONV_RATE_CARDHOLDER_BILL));
    msg.setField(_028_AMOUNT_TRAN_FEE, msg_from_issuer.getField(_028_AMOUNT_TRAN_FEE));
    msg.setField(_037_RETRIEVAL_REF_NR, msg_from_issuer.getField(_037_RETRIEVAL_REF_NR));
    msg.setField(_038_AUTH_ID_RSP, msg_from_issuer.getField(_038_AUTH_ID_RSP));
    msg.setField(_039_RSP_CODE, msg_from_issuer.getField(_039_RSP_CODE));
    msg.setField(_044_ADDITIONAL_RSP_DATA, msg_from_issuer.getField(_044_ADDITIONAL_RSP_DATA));
    msg.setField(_054_ADDITIONAL_AMOUNTS, msg_from_issuer.getField(_054_ADDITIONAL_AMOUNTS));
    msg.setField(_055_EMV_DATA, msg_from_issuer.getField(_055_EMV_DATA));
    msg.setField(_102_ACCOUNT_ID_1, msg_from_issuer.getField(_102_ACCOUNT_ID_1));
    msg.setField(_103_ACCOUNT_ID_2, msg_from_issuer.getField(_103_ACCOUNT_ID_2));
    msg.setField(_121_TRAN_DATA_RSP, msg_from_issuer.getField(_121_TRAN_DATA_RSP));

    WriteToDBEntity wu(msg.toMsg(), req_in, req_in, req_in, req_in, Config::_UPDATE_TRANS);
    Config::commit_db.push(wu);
    /*
    dbstatus = tmdbh.updateTrans(msg);
    if (dbstatus == false)
    {
        msg.setRspMsgType();
        msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);

        td_logger.warning("DB entry unsuccessful \n" + msg.dumpMsg());

        return false;
    }*/

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////

bool MsgHandler::process0220Msg(Iso8583JSON &msg, TranMgrDBHandler tmdbh)
{
    bool fetchdata;

    LocalDateTime now;
    string req_in = Poco::DateTimeFormatter::format(now, "%Y-%m-%d %H:%M:%s");

    // check if valid transaction request, ACQNODEKEY
    if (!msg.isExtendedFieldSet(_002_ORG_ACQ_NODE_KEY))
    {
        return processFinMsg(msg, tmdbh);
    }

    // Check for repeat transaction
    Tran tran_rev;

    fetchdata = tmdbh.getTranInfo(msg.getExtendedField(_001_ACQ_NODE_KEY), tran_rev);
    if (fetchdata == true)
    {
        // this is a completion request
        tran_rev.mapIntoIso(msg);
        msg.setRspMsgType();
        msg.setField(_039_RSP_CODE, tran_rev.rsp_code);
        td_logger.warning("processCompletionRequest - Repeat Reversal - Tran Nr # " + NumberFormatter::format(tran_rev.tran_nr));

        return false;
    }

    // if not we carry ahead
    // fetch the original record
    Tran tran_org;
    fetchdata = tmdbh.getTranInfo(msg.getExtendedField(_002_ORG_ACQ_NODE_KEY), tran_org);

    // check if original is present
    if (fetchdata == false)
    {
        msg.setRspMsgType();
        msg.setField(_039_RSP_CODE, _25_UNABLE_TO_LOCATE_RECORD);

        td_logger.warning("processCompletionRequest - Original Tran Not Found - Acq Node Key # " + msg.getExtendedField(
                                                                                                       _002_ORG_ACQ_NODE_KEY));

        return false;
    }

    // add the reversal in transactions
    string acq_node_key = msg.getExtendedField(_001_ACQ_NODE_KEY);
    string prev_acq_node_key = msg.getExtendedField(_002_ORG_ACQ_NODE_KEY);

    Iso8583JSON msg_to_iss_node;
    tran_org.mapIntoIso(msg_to_iss_node);
    string org_mti = msg_to_iss_node.getMsgType();

    string org_datetime;
    org_datetime = msg_to_iss_node.getField(_007_TRANSMISSION_DATE_TIME);

    msg_to_iss_node.setMsgType(msg.getMsgType());

    // Process for encrypted PAN
    msg_to_iss_node.setField(_004_AMOUNT_TRANSACTION, msg.getField(_004_AMOUNT_TRANSACTION));
    msg_to_iss_node.setField(_007_TRANSMISSION_DATE_TIME, msg.getField(_007_TRANSMISSION_DATE_TIME));
    msg_to_iss_node.setField(_011_SYSTEMS_TRACE_AUDIT_NR, msg.getField(_011_SYSTEMS_TRACE_AUDIT_NR));
    msg_to_iss_node.setField(_014_DATE_EXPIRATION, msg.getField(_014_DATE_EXPIRATION));
    msg_to_iss_node.setField(_023_CARD_SEQ_NR, msg.getField(_023_CARD_SEQ_NR));
    msg_to_iss_node.setField(_039_RSP_CODE, msg.getField(_039_RSP_CODE));

    msg_to_iss_node.setField(_061_POS_DATA, msg.getField(_061_POS_DATA));

    msg_to_iss_node.setField090(org_mti,
                                msg_to_iss_node.getField(_011_SYSTEMS_TRACE_AUDIT_NR),
                                org_datetime, msg_to_iss_node.getField(_032_ACQUIRING_INST_ID_CODE),
                                msg_to_iss_node.getField(_033_FORWARDING_INST_ID_CODE));

    msg_to_iss_node.setExtendedField(_001_ACQ_NODE_KEY, acq_node_key);
    msg_to_iss_node.setExtendedField(_002_ORG_ACQ_NODE_KEY, prev_acq_node_key);
    msg_to_iss_node.setExtendedField(_009_PREV_TRAN_NR, NumberFormatter::format(tran_org.tran_nr));

    msg_to_iss_node.setExtendedField(_007_TRAN_NR, msg.getExtendedField(_007_TRAN_NR));

    bool dbstatus = tmdbh.addToTrans(msg_to_iss_node, req_in);

    if (dbstatus == false)
    {
        td_logger.fatal("Cannot insert into DB\n" + msg.dumpMsg());

        msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);
    }

    //    msg.setRspMsgType();
    msg.setField(_002_PAN, msg_to_iss_node.getField(_002_PAN));
    msg.setField(_037_RETRIEVAL_REF_NR, msg_to_iss_node.getField(_037_RETRIEVAL_REF_NR));
    msg.setField(_038_AUTH_ID_RSP, msg_to_iss_node.getField(_038_AUTH_ID_RSP));
    msg.setField(_039_RSP_CODE, _00_SUCCESSFUL);
    msg.setExtendedField(_005_ACQ_PART_NAME, msg_to_iss_node.getExtendedField(_005_ACQ_PART_NAME));
    msg.setExtendedField(_009_PREV_TRAN_NR, NumberFormatter::format(tran_org.tran_nr));

    // Here our message (msg) to be sent to Acquirer is done
    // Introduce Poco::NotificationQueue to write the reversal message to be sent to the Issuer
    tmdbh.updateadjustment(msg_to_iss_node.getExtendedField(_009_PREV_TRAN_NR));

    // Send request to Issuer Node
    NodeInfo ni(msg_to_iss_node.getField(_122_NODE_INFO));
    Route rt;
    fetchdata = tmdbh.getrouteinfo(ni.getIssuerNode(), rt);

    string rsp;

    if (fetchdata)
    {
        rsp = Utility::ofPostRequest(rt.getUrl(), msg_to_iss_node.toMsg(), rt.getTimeout());
    }
    else
    {
        rsp = "NOK";
        td_logger.error("Could not fetch Issuer node data");
    }

    if (rsp.compare("NOK") == 0)
    {
        msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);

        td_logger.error("Issuer/Process not available (NOK) - " + rt.getPname() + "(" + rt.getUrl() + ")");

        msg.setRspMsgType();

        return false;
    }
    else if (rsp.compare("TMO") == 0)
    {
        msg.setField(_039_RSP_CODE, _91_ISSUER_OR_SWITCH_INOPERATIVE);

        td_logger.error("Issuer/Process not available (TMO) - " + rt.getPname() + "(" + rt.getUrl() + ")");

        msg.setRspMsgType();

        return false;
    }

    Iso8583JSON msg_from_issuer;

    try
    {
        msg_from_issuer.parseMsg(rsp);
    }
    catch (Poco::Exception e)
    {
        td_logger.error(Poco::format("Exception in MsgHandler::process0220Msg : (%s)\nRsp: (%s)", e.what(), rsp));
        msg.setField(_039_RSP_CODE, _30_FORMAT_ERROR);

        msg.setRspMsgType();
        return false;
    }

    // clear messages first
    clearRespFields(msg);

    // update with response from issuer
    msg.setMsgType(msg_from_issuer.getMsgType());
    msg.setField(_005_AMOUNT_SETTLE, msg_from_issuer.getField(_005_AMOUNT_SETTLE));
    msg.setField(_006_AMOUNT_CARDHOLDER_BILL, msg_from_issuer.getField(_006_AMOUNT_CARDHOLDER_BILL));
    msg.setField(_009_CONV_RATE_SETTLE, msg_from_issuer.getField(_009_CONV_RATE_SETTLE));
    msg.setField(_010_CONV_RATE_CARDHOLDER_BILL, msg_from_issuer.getField(_010_CONV_RATE_CARDHOLDER_BILL));
    msg.setField(_012_TIME_LOCAL, msg_from_issuer.getField(_012_TIME_LOCAL));
    msg.setField(_013_DATE_LOCAL, msg_from_issuer.getField(_013_DATE_LOCAL));
    msg.setField(_018_MERCHANT_TYPE, msg_from_issuer.getField(_018_MERCHANT_TYPE));
    msg.setField(_028_AMOUNT_TRAN_FEE, msg_from_issuer.getField(_028_AMOUNT_TRAN_FEE));
    msg.setField(_037_RETRIEVAL_REF_NR, msg_from_issuer.getField(_037_RETRIEVAL_REF_NR));
    msg.setField(_038_AUTH_ID_RSP, msg_from_issuer.getField(_038_AUTH_ID_RSP));
    msg.setField(_039_RSP_CODE, msg_from_issuer.getField(_039_RSP_CODE));
    msg.setField(_044_ADDITIONAL_RSP_DATA, msg_from_issuer.getField(_044_ADDITIONAL_RSP_DATA));
    msg.setField(_055_EMV_DATA, msg_from_issuer.getField(_055_EMV_DATA));
    msg.setField(_102_ACCOUNT_ID_1, msg_from_issuer.getField(_102_ACCOUNT_ID_1));
    msg.setField(_103_ACCOUNT_ID_2, msg_from_issuer.getField(_103_ACCOUNT_ID_2));
    msg.setField(_120_TRAN_DATA_REQ, msg_from_issuer.getField(_120_TRAN_DATA_REQ));
    msg.setField(_121_TRAN_DATA_RSP, msg_from_issuer.getField(_121_TRAN_DATA_RSP));
    msg.setField(_122_NODE_INFO, msg_from_issuer.getField(_122_NODE_INFO));

    dbstatus = tmdbh.updateTrans(msg);
    if (dbstatus == false)
    {
        msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);

        td_logger.warning("DB entry unsuccessful \n" + msg.dumpMsg());

        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////////////

void MsgHandler::process0320Msg(Iso8583JSON &msg, TranMgrDBHandler tmdbh)
{
    bool route = false;
    Route rt;
    bool dbstatus;

    msg.setExtendedField(_009_PREV_TRAN_NR, "0");

    NodeInfo ni(msg.getNodeInfo());

    if (msg.isFieldSet(_067_EXTENDED_PAYMENT_CODE))
    {
        route = tmdbh.getExtdIssuerNode(msg, rt);
    }

    if (route == false)
    {
        route = tmdbh.getIssuerNode(msg, rt);

        if (route == false)
        {
            td_logger.error("No route found \n" + msg.dumpMsg());

            msg.setField(_039_RSP_CODE, _92_ROUTING_ERROR);
        }
        else
        {
            msg.setField(_039_RSP_CODE, "00");
        }
    }
    else
    {
        msg.setField(_039_RSP_CODE, "00");
    }

    dbstatus = tmdbh.addTMDeclineToTrans(msg);
    msg.setRspMsgType();

    if (dbstatus == false)
    {
        msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);
        td_logger.error("DB entry unsuccessful \n" + msg.dumpMsg());
    }
}

/////////////////////////////////////////////////////////////////////////////////////

bool MsgHandler::process0420Msg(Iso8583JSON &msg, TranMgrDBHandler tmdbh)
{
    bool fetchdata;
    LocalDateTime now;
    string req_in = Poco::DateTimeFormatter::format(now, "%Y-%m-%d %H:%M:%s");

    // check if valid Reversal request, ACQNODEKEY
    if (!msg.isExtendedFieldSet(_002_ORG_ACQ_NODE_KEY))
    {
        msg.setRspMsgType();
        msg.setField(_039_RSP_CODE, _30_FORMAT_ERROR);

        return false;
    }

    // Check for repeat reversal
    Tran tran_rev;

    fetchdata = tmdbh.getTranInfo(msg.getExtendedField(_001_ACQ_NODE_KEY), tran_rev);
    if (fetchdata == true)
    {
        // this is a repeat reversal
        tran_rev.mapIntoIso(msg);
        msg.setRspMsgType();
        msg.setField(_039_RSP_CODE, tran_rev.rsp_code);
        td_logger.warning("processReversalRequest - Repeat Reversal - Tran Nr # " + NumberFormatter::format(tran_rev.tran_nr));

        return false;
    }

    // if not we carry ahead
    // fetch the original record
    Tran tran_org;
    fetchdata = tmdbh.getTranInfo(msg.getExtendedField(_002_ORG_ACQ_NODE_KEY), tran_org);

    // check if original is present
    if (fetchdata == false)
    {
        msg.setRspMsgType();
        msg.setField(_039_RSP_CODE, _25_UNABLE_TO_LOCATE_RECORD);

        td_logger.warning("processReversalRequest - Original Tran Not Found - Acq Node Key # " + msg.getExtendedField(
                                                                                                     _002_ORG_ACQ_NODE_KEY));

        return false;
    }

    // add the reversal in transactions
    string acq_node_key = msg.getExtendedField(_001_ACQ_NODE_KEY);
    string prev_acq_node_key = msg.getExtendedField(_002_ORG_ACQ_NODE_KEY);

    Iso8583JSON msg_to_iss_node;
    tran_org.mapIntoIso(msg_to_iss_node);
    string org_mti = msg_to_iss_node.getMsgType();

    //Iso8583JSON msg_to_acq_node;
    //tran_org.mapIntoIso(msg_to_acq_node);

    string org_datetime;
    org_datetime = msg_to_iss_node.getField(_007_TRANSMISSION_DATE_TIME);

    msg_to_iss_node.setMsgType(msg.getMsgType());

    // Process for encrypted PAN
    msg_to_iss_node.setField(_004_AMOUNT_TRANSACTION, msg.getField(_004_AMOUNT_TRANSACTION));
    msg_to_iss_node.setField(_007_TRANSMISSION_DATE_TIME, msg.getField(_007_TRANSMISSION_DATE_TIME));
    msg_to_iss_node.setField(_011_SYSTEMS_TRACE_AUDIT_NR, msg.getField(_011_SYSTEMS_TRACE_AUDIT_NR));
    msg_to_iss_node.setField(_014_DATE_EXPIRATION, msg.getField(_014_DATE_EXPIRATION));
    msg_to_iss_node.setField(_023_CARD_SEQ_NR, msg.getField(_023_CARD_SEQ_NR));
    msg_to_iss_node.setField(_039_RSP_CODE, msg.getField(_039_RSP_CODE));

    if ((msg.getField(_039_RSP_CODE).compare("E2") == 0) && !(msg.isFieldSet(_055_EMV_DATA)))
    {
        msg_to_iss_node.setField(_055_EMV_DATA, msg.getField(_055_EMV_DATA));
    }
    else
    {
        msg_to_iss_node.setField(_055_EMV_DATA, msg.getField(_055_EMV_DATA));
    }

    msg_to_iss_node.setField(_061_POS_DATA, msg.getField(_061_POS_DATA));

    msg_to_iss_node.setField090(org_mti, tran_org.stan,
                                org_datetime, msg_to_iss_node.getField(_032_ACQUIRING_INST_ID_CODE),
                                msg_to_iss_node.getField(_033_FORWARDING_INST_ID_CODE));

    msg_to_iss_node.setExtendedField(_001_ACQ_NODE_KEY, acq_node_key);
    msg_to_iss_node.setExtendedField(_002_ORG_ACQ_NODE_KEY, prev_acq_node_key);
    msg_to_iss_node.setExtendedField(_009_PREV_TRAN_NR, NumberFormatter::format(tran_org.tran_nr));

    msg_to_iss_node.setExtendedField(_007_TRAN_NR, msg.getExtendedField(_007_TRAN_NR));

    bool dbstatus = tmdbh.addToTrans(msg_to_iss_node, req_in);

    if (dbstatus == false)
    {
        td_logger.fatal("Cannot insert into DB\n" + msg.dumpMsg());

        msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);
    }

    //    msg.setRspMsgType();
    msg.setField(_002_PAN, msg_to_iss_node.getField(_002_PAN));
    msg.setField(_037_RETRIEVAL_REF_NR, msg_to_iss_node.getField(_037_RETRIEVAL_REF_NR));
    msg.setField(_038_AUTH_ID_RSP, msg_to_iss_node.getField(_038_AUTH_ID_RSP));
    msg.setField(_039_RSP_CODE, _00_SUCCESSFUL);
    msg.setExtendedField(_005_ACQ_PART_NAME, msg_to_iss_node.getExtendedField(_005_ACQ_PART_NAME));
    msg.setExtendedField(_009_PREV_TRAN_NR, NumberFormatter::format(tran_org.tran_nr));

    // Here our message (msg) to be sent to Acquirer is done
    // Introduce Poco::NotificationQueue to write the reversal message to be sent to the Issuer
    tmdbh.updatereversal(msg_to_iss_node.getExtendedField(_009_PREV_TRAN_NR));
    tmdbh.updatereversal(msg_to_iss_node.getExtendedField(_007_TRAN_NR));

    // Send request to Issuer Node
    NodeInfo ni(msg_to_iss_node.getField(_122_NODE_INFO));
    Route rt;
    fetchdata = tmdbh.getrouteinfo(ni.getIssuerNode(), rt);

    string rsp;

    if (fetchdata)
    {
        rsp = Utility::ofPostRequest(rt.getUrl(), msg_to_iss_node.toMsg(), rt.getTimeout());
    }
    else
    {
        rsp = "NOK";
        td_logger.error("Could not fetch Issuer node data");
    }

    if (rsp.compare("NOK") == 0)
    {
        msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);

        td_logger.error("Issuer/Process not available (NOK) - " + rt.getPname() + "(" + rt.getUrl() + ")");

        msg.setRspMsgType();

        return false;
    }
    else if (rsp.compare("TMO") == 0)
    {
        msg.setField(_039_RSP_CODE, _91_ISSUER_OR_SWITCH_INOPERATIVE);

        td_logger.error("Issuer/Process not available (TMO) - " + rt.getPname() + "(" + rt.getUrl() + ")");

        msg.setRspMsgType();

        return false;
    }

    Iso8583JSON msg_from_issuer;

    try
    {
        msg_from_issuer.parseMsg(rsp);
    }
    catch (Poco::Exception e)
    {
        td_logger.error(Poco::format("Exception in MsgHandler::processFinMsg : (%s)\nRsp: (%s)", e.what(), rsp));
        msg.setField(_039_RSP_CODE, _30_FORMAT_ERROR);

        msg.setRspMsgType();
        return false;
    }

    // clear messages first
    clearRespFields(msg);

    // update with response from issuer
    msg.setMsgType(msg_from_issuer.getMsgType());
    msg.setField(_005_AMOUNT_SETTLE, msg_from_issuer.getField(_005_AMOUNT_SETTLE));
    msg.setField(_006_AMOUNT_CARDHOLDER_BILL, msg_from_issuer.getField(_006_AMOUNT_CARDHOLDER_BILL));
    msg.setField(_009_CONV_RATE_SETTLE, msg_from_issuer.getField(_009_CONV_RATE_SETTLE));
    msg.setField(_010_CONV_RATE_CARDHOLDER_BILL, msg_from_issuer.getField(_010_CONV_RATE_CARDHOLDER_BILL));
    msg.setField(_012_TIME_LOCAL, msg_from_issuer.getField(_012_TIME_LOCAL));
    msg.setField(_013_DATE_LOCAL, msg_from_issuer.getField(_013_DATE_LOCAL));
    msg.setField(_018_MERCHANT_TYPE, msg_from_issuer.getField(_018_MERCHANT_TYPE));
    msg.setField(_028_AMOUNT_TRAN_FEE, msg_from_issuer.getField(_028_AMOUNT_TRAN_FEE));
    msg.setField(_037_RETRIEVAL_REF_NR, msg_from_issuer.getField(_037_RETRIEVAL_REF_NR));
    msg.setField(_038_AUTH_ID_RSP, msg_from_issuer.getField(_038_AUTH_ID_RSP));
    msg.setField(_039_RSP_CODE, msg_from_issuer.getField(_039_RSP_CODE));
    msg.setField(_044_ADDITIONAL_RSP_DATA, msg_from_issuer.getField(_044_ADDITIONAL_RSP_DATA));
    msg.setField(_055_EMV_DATA, msg_from_issuer.getField(_055_EMV_DATA));
    msg.setField(_102_ACCOUNT_ID_1, msg_from_issuer.getField(_102_ACCOUNT_ID_1));
    msg.setField(_103_ACCOUNT_ID_2, msg_from_issuer.getField(_103_ACCOUNT_ID_2));
    msg.setField(_120_TRAN_DATA_REQ, msg_from_issuer.getField(_120_TRAN_DATA_REQ));
    msg.setField(_121_TRAN_DATA_RSP, msg_from_issuer.getField(_121_TRAN_DATA_RSP));
    msg.setField(_122_NODE_INFO, msg_from_issuer.getField(_122_NODE_INFO));

    dbstatus = tmdbh.updateTrans(msg);
    if (dbstatus == false)
    {
        msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);

        td_logger.warning("DB entry unsuccessful \n" + msg.dumpMsg());

        return false;
    }

    return true;
}
/////////////////////////////////////////////////////////////////////////////////////

void MsgHandler::process0520Msg(Iso8583JSON &msg, TranMgrDBHandler tmdbh)
{
    msg.setField(_004_AMOUNT_TRANSACTION, "000000000000");
    msg.setField(_039_RSP_CODE, _00_SUCCESSFUL);
    msg.setField(_062_TRANS_ID, "000000");

    msg.setExtendedField(_009_PREV_TRAN_NR, "0000000000");

    td_logger.information(msg.dumpMsg());

    LocalDateTime now;
    string req_in = Poco::DateTimeFormatter::format(now, "%Y-%m-%d %H:%M:%s");

    bool dbstatus = tmdbh.addToTrans(msg, req_in);

    if (dbstatus == false)
    {
        td_logger.fatal("Cannot insert into DB\n" + msg.dumpMsg());

        msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);
    }

    msg.setRspMsgType();
}
/////////////////////////////////////////////////////////////////////////////////////

void MsgHandler::process0620Msg(Iso8583JSON &msg, TranMgrDBHandler tmdbh)
{

    bool route = false;
    Route rt;
    bool dbstatus;

    msg.setExtendedField(_009_PREV_TRAN_NR, "0");

    NodeInfo ni(msg.getNodeInfo());

    if (msg.isFieldSet(_067_EXTENDED_PAYMENT_CODE))
    {
        route = tmdbh.getExtdIssuerNode(msg, rt);
    }

    if (route == false)
    {
        route = tmdbh.getIssuerNode(msg, rt);

        if (route == false)
        {
            td_logger.error("No route found \n" + msg.dumpMsg());

            msg.setField(_039_RSP_CODE, _92_ROUTING_ERROR);
        }
        else
        {
            msg.setField(_038_AUTH_ID_RSP, "000000");
            msg.setField(_039_RSP_CODE, "00");
        }
    }
    else
    {
        msg.setField(_038_AUTH_ID_RSP, "000000");
        msg.setField(_039_RSP_CODE, "00");
    }

    dbstatus = tmdbh.addTMDeclineToTrans(msg);
    msg.setRspMsgType();

    if (dbstatus == false)
    {
        msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);
        td_logger.error("DB entry unsuccessful \n" + msg.dumpMsg());
    }
}

/////////////////////////////////////////////////////////////////////////////////////

void MsgHandler::process0800Msg(Iso8583JSON &msg, TranMgrDBHandler tmdbh)
{

    bool route = false;
    Route rt;
    bool dbstatus;

    string pcode;

    if (msg.isFieldSet(_003_PROCESSING_CODE))
    {
        pcode = msg.getField(_003_PROCESSING_CODE);

        if (Utility::startsWith(pcode, "92"))
        {
            if (!getNewKeys(msg))
            {
                msg.setRspMsgType();
                msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);
            }
        }
    }
    else
    {
        msg.setRspMsgType();
        msg.setField(_039_RSP_CODE, _00_SUCCESSFUL);
    }
}
///////////////////////////////////////////////////////////////////////////////

bool MsgHandler::getNewKeys(Iso8583JSON &msg)
{
    if (!msg.isExtendedFieldSet(_016_PIN_KEY))
    {
        td_logger.error("getNewKeys - Extd Field 16 absent");
        return false;
    }

    if (!msg.isExtendedFieldSet(_017_DATA_KEY))
    {
        td_logger.error("getNewKeys - Extd Field 17 absent");
        return false;
    }

    if (!msg.isFieldSet(_046_ADDITIONAL_DATA_ISO))
    {
        td_logger.error("getNewKeys - Field 46 absent");
        return false;
    }

    string pin_key_name = msg.getExtendedField(_016_PIN_KEY);
    string data_key_name = msg.getExtendedField(_017_DATA_KEY);
    string master_key = msg.getField(_046_ADDITIONAL_DATA_ISO);

    return true;
}
///////////////////////////////////////////////////////////////////////////////

bool MsgHandler::constructPINBlock(Iso8583JSON &msg, string iss_key_name)
{
    string acq_working_key = msg.getExtendedField(_016_PIN_KEY);
    HSMKeyMgmt hkm(Config::dburl);
    string newpinblock;

    if (!msg.isExtendedFieldSet(_016_PIN_KEY))
    {
        msg.setField(_039_RSP_CODE, _63_SECURITY_VIOLATION);
        td_logger.error("constructPINBlock - PIN Key Absent" + msg.dumpMsg());
        return false;
    }

    if (acq_working_key.find("TPK") != std::string::npos)
    {
        newpinblock = hkm.translateTPKtoZPK(
            acq_working_key, iss_key_name,
            msg.getField(_052_PIN_DATA),
            msg.getField(_002_PAN));
    }
    else if (acq_working_key.find("ZPK") != std::string::npos)
    {
        newpinblock = hkm.translateZPKtoZPK(
            acq_working_key, iss_key_name,
            msg.getField(_052_PIN_DATA),
            msg.getField(_002_PAN));
    }
    else if (acq_working_key.find("BDK") != std::string::npos)
    {
        if (!msg.isFieldSet(_053_SECURITY_INFO))
        {
            msg.setField(_039_RSP_CODE, _81_CRYPTO_ERROR);
            return false;
        }
        string ksn = msg.getField(_053_SECURITY_INFO);
        newpinblock = hkm.translateBDKtoZPKTDES(
            acq_working_key, iss_key_name,
            msg.getField(_052_PIN_DATA),
            msg.getField(_002_PAN),
            ksn);
    }

    if (newpinblock.empty() || (newpinblock.compare("NOK") == 0))
    {
        td_logger.error("constructPINBlock - PIN Block is empty" + msg.dumpMsg());
        msg.setField(_039_RSP_CODE, _81_CRYPTO_ERROR);

        return false;
    }
    else
    {
        msg.setField(_052_PIN_DATA,
                     newpinblock);
    }

    return true;
}
////////////////////////////////////////////////////////////////////////////////

void MsgHandler::clearRespFields(Iso8583JSON &msg)
{
    msg.removeField(_035_TRACK_2_DATA);
    msg.removeField(_045_TRACK_1_DATA);
    msg.removeField(_052_PIN_DATA);
    msg.removeField(_055_EMV_DATA);
}

////////////////////////////////////////////////////////////////////////////////

bool MsgHandler::allowFallback(Iso8583JSON &msg)
{
    if (Config::isFallbackallowed)
    {
        return true;
    }

    string serviceCode;
    string routingInfo;

    routingInfo = ";" + trim(msg.getNodeInfo().getRoutingInfo()) + ";";

    if (Config::fallbackExclusionList.find(routingInfo) != std::string::npos)
    {
        return true;
    }

    string acq_part_name = msg.getExtendedField(_005_ACQ_PART_NAME);
    if (Config::fallbackExclusionList.find(acq_part_name) != std::string::npos)
    {
        return true;
    }

    if (msg.isFieldSet(_040_SERVICE_RESTRICTION_CODE))
    {
        serviceCode = msg.getField(_040_SERVICE_RESTRICTION_CODE);
    }
    else
    {
        serviceCode = "000";
    }

    if (Utility::startsWith(serviceCode, "2") || Utility::startsWith(serviceCode, "6"))
    {
        if (!msg.isFieldSet(_055_EMV_DATA))
        {
            return false;
        }
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////

void MsgHandler::updateDateTime(Iso8583JSON &msg)
{
    string name_loc = msg.getField(_043_CARD_ACCEPTOR_NAME_LOC);
    string country = name_loc.substr(38, 40);

    //    if (Config.timeOffset.containsKey(country)) {
    //        int toffset = Config.timeOffset.get(country);
    //
    //        if (toffset != 0) {
    //            msg.setField(7, Utility.getDateTime(toffset));
    //            msg.setField(12, Utility.getTime(toffset));
    //            msg.setField(13, Utility.getDate(toffset).substring(4));
    //            msg.setField(_073_DATE_ACTION, Utility.getDate(toffset));
    //        }
    //    } else {
    //        Config.lgr.warn("TimeOffset setting not found in countries");
    //        msg.setField(Iso8583JSON.Bit._073_DATE_ACTION, Utility.getDate());
    //    }

    int offset = Config::timeOffset.find(country)->second;
    int tzd = offset * 60;

    DateTime now;

    now.makeLocal(tzd);
    string tnow;

    tnow = NumberFormatter::format0(now.year(), 4) + NumberFormatter::format0(now.month(), 2) + NumberFormatter::format0(now.day(), 2);

    msg.setField(_073_DATE_ACTION, tnow);
}

////////////////////////////////////////////////////////////////////////////////

bool MsgHandler::checkICAFlag(Iso8583JSON &msg)
{
    string icaflag;
    string cardcountry;

    string f120 = msg.getField(_120_TRAN_DATA_REQ);

    MapKeyValue mkv;
    mkv.fromMsg(f120);

    if (mkv.isKeyPresent("ICA"))
    {
        icaflag = mkv.get("ICA");
    }
    else
    {
        icaflag = "N";
    }

    if(icaflag.compare("Y")==0)
    {
        return true;
    }

    if (mkv.isKeyPresent("card_country"))
    {
        cardcountry = mkv.get("card_country");
        cardcountry = trim(cardcountry);
    }
    else
    {
        cardcountry = "XXX";
    }

    if (cardcountry.compare("IN") == 0 || cardcountry.compare("IND") == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

#endif /* MSGHANDLER_H */