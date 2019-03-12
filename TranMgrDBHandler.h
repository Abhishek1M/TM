#ifndef TRANMGRDBHANDLER_H
#define TRANMGRDBHANDLER_H

#include <iostream>
#include <string>

#include <Poco/Logger.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeFormatter.h>

#include <constants.h>
#include <Utility.h>
#include <Iso8583JSON.h>
#include <pqxx/field.hxx>
#include "DBManager.h"
#include "Route.h"
#include <Encrypt.h>
#include "Config.h"
#include "Tran.h"
#include "MapKeyValue.h"

using namespace std;

using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::Logger;
using Poco::trim;

class TranMgrDBHandler
{
  public:
    TranMgrDBHandler(Logger &logger) : td_logger(logger)
    {
    }

    long getNewTranNr();
    string appendQueryParams(string value);
    string appendQueryParams(long value);
    bool addToTrans(Iso8583JSON &msg, string req_in);
    bool updateTrans(Iso8583JSON &msg);
    bool addTMDeclineToTrans(Iso8583JSON &msg);
    bool getIssuerNode(Iso8583JSON &msg, Route &rt);
    bool getExtdIssuerNode(Iso8583JSON &msg, Route &rt);
    bool getTranInfo(string acq_node_key, Tran &tran);
    bool updatereversal(string tran_nr);
    bool updateadjustment(string tran_nr);
    bool getrouteinfo(string issuer_node_name, Route &rt);

  private:
    Logger &td_logger;
};

//////////////////////////////////////////////////////////////////////////

long TranMgrDBHandler::getNewTranNr()
{
    pqxx::connection c(Config::dburl);

    string query = "select nextval('trans_trannr');";

    if (!c.is_open())
    {
        td_logger.fatal("getNewTranNr - Could not connect to database");
        return 0;
    }

    // Create a non-transactional object
    pqxx::work txn(c);

    // Execute query
    pqxx::result r = txn.exec(query);

    if (r.size() != 1)
    {
        return 0;
    }

    long tran_nr = r[0][0].as<long>();

    c.disconnect();

    return tran_nr;
}

//////////////////////////////////////////////////////////////////////////

bool TranMgrDBHandler::addToTrans(Iso8583JSON &msg, string req_in)
{
    try
    {
        pqxx::connection c(Config::dburl);

        if (!c.is_open())
        {
            td_logger.fatal("addToTrans - Could not connect to database");
            return false;
        }

        pqxx::work txn(c);

        string enc_pan;

        if (msg.isFieldSet(_002_PAN))
        {
            enc_pan = Config::e.encryptdata(msg.getField(_002_PAN));
        }
        else
        {
            enc_pan = "NA";
        }

        td_logger.trace("Encrypted PAN");
        td_logger.trace(enc_pan);

        string query = "insert into transactions (tran_nr, mti, pan, tran_type, from_account, to_account, amount_tran, amount_stl, amount_chb, datetime_trans, fee_amount_chb, convrate_stl, convrate_chb, stan, time_local, date_local, date_exp, date_stl, date_conv, date_capture, merchant_type, countrycode_acqinstt, countrycode_panextd, countrycode_fwsinstt, pos_entry_mode, app_pan_nr, nii, pos_cond_code, pos_capture_code, auth_id_rsp_len, fee_amount_tran, fee_amount_stl, proc_fee_amount_tran, proc_fee_amount_stl, acq_instt_code, fwd_instt_id, extd_pan, retr_ref_nr, auth_id_rsp, rsp_code, service_rest_code, ca_term_id, ca_id_code, ca_name_loc, addl_rsp_data, addl_data_private, currencycode_tran, currencycode_stl, currencycode_chb, amount_cash, emv_request, emv_response, advice_reason_code, pos_data, settlement_code, extd_payment_code, countrycode_rcvinstt, countrycode_stlinstt, payee, stl_instt_id_code, rcv_instt_id_code, account_id_1, account_id_2, trans_data_req, trans_data_rsp, acquirer_node, issuer_node, super_merchant_id, routing_info, tran_state, pan_encrypted, req_in, req_out, rsp_in, rsp_out, acq_node_key, prev_trannr, transaction_id, term_batch_nr, acq_node_batch_nr, prev_acq_node_key,acq_part_name,retailer_id, store_id, device_id, org_type) values (";

        td_logger.debug(query);

        query.append(appendQueryParams(msg.getExtendedField(7)));
        query.append(appendQueryParams(msg.getMsgType()));
        query.append(appendQueryParams(Utility::protect(msg.getField(2))));
        query.append(appendQueryParams(msg.getField(3).substr(0, 2)));
        query.append(appendQueryParams(msg.getField(3).substr(2, 2)));
        query.append(appendQueryParams(msg.getField(3).substr(4, 2)));
        query.append(appendQueryParams(Utility::convertolong(msg.getField(4))));
        query.append(appendQueryParams("0"));
        query.append(appendQueryParams("0"));
        query.append(appendQueryParams(msg.getField(7)));
        query.append(appendQueryParams("0"));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(msg.getField(11)));
        query.append(appendQueryParams(msg.getField(12)));
        query.append(appendQueryParams(msg.getField(13)));
        query.append(appendQueryParams(msg.getField(_014_DATE_EXPIRATION)));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(msg.getField(18)));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(msg.getField(22)));
        query.append(appendQueryParams(msg.getField(23)));
        query.append(appendQueryParams(msg.getField(24)));
        query.append(appendQueryParams("00"));
        query.append(appendQueryParams("00"));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams("0"));
        query.append(appendQueryParams("0"));
        query.append(appendQueryParams("0"));
        query.append(appendQueryParams("0"));
        query.append(appendQueryParams(msg.getField(32)));
        query.append(appendQueryParams(msg.getField(33)));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(msg.getField(37)));
        query.append(appendQueryParams(msg.getField(38)));
        query.append(appendQueryParams(msg.getField(39)));
        query.append(appendQueryParams(msg.getField(40)));
        query.append(appendQueryParams(msg.getField(41)));
        query.append(appendQueryParams(msg.getField(42)));
        query.append(appendQueryParams(msg.getField(43)));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(msg.getField(49)));
        query.append(appendQueryParams(msg.getField(50)));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams("0"));
        query.append(appendQueryParams(msg.getField(55)));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(msg.getField(_120_TRAN_DATA_REQ)));
        query.append(appendQueryParams(msg.getField(_121_TRAN_DATA_RSP)));
        query.append(appendQueryParams(msg.getNodeInfo().getAcquirerNode()));
        query.append(appendQueryParams(msg.getNodeInfo().getIssuerNode()));
        query.append(appendQueryParams(msg.getNodeInfo().getSuperMID()));
        query.append(appendQueryParams(msg.getNodeInfo().getRoutingInfo()));
        query.append(appendQueryParams(_001_TRAN_COMPLETE));
        query.append(appendQueryParams(enc_pan));

        req_in = "'" + req_in + "',";
        query.append(req_in);

        td_logger.debug(query);

        LocalDateTime now;
        string req_out = Poco::DateTimeFormatter::format(now, "'%Y-%m-%d %H:%M:%s',");
        query.append(req_out);
        query.append("now(),");
        query.append("now(),");
        query.append(appendQueryParams(msg.getExtendedField(_001_ACQ_NODE_KEY)));
        query.append(appendQueryParams(
            NumberFormatter::format(Utility::convertolong(msg.getExtendedField(_009_PREV_TRAN_NR)))));
        query.append(appendQueryParams(msg.getField(_062_TRANS_ID)));
        query.append(appendQueryParams(msg.getExtendedField(_006_TERM_BATCH_NR)));
        query.append(appendQueryParams("000"));
        query.append(appendQueryParams(msg.getExtendedField(_002_ORG_ACQ_NODE_KEY)));
        query.append(appendQueryParams(msg.getExtendedField(_005_ACQ_PART_NAME)));
        query.append(appendQueryParams(msg.getExtendedField(_008_RETAILER_ID)));
        query.append(appendQueryParams(msg.getExtendedField(_003_STORE_ID)));
        query.append(appendQueryParams(msg.getExtendedField(_004_DEVICE_ID)));
        query.append("'" + (msg.getExtendedField(_011_ORIGINATOR_TYPE)) + "'");
        query.append(")");

        td_logger.debug(query);

        txn.exec(query);
        txn.commit();

        c.disconnect();
    }
    catch (...)
    {
        //td_logger.error(e.what());
        return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////

bool TranMgrDBHandler::updateTrans(Iso8583JSON &msg)
{
    try
    {
        pqxx::connection c(Config::dburl);

        if (!c.is_open())
        {
            td_logger.fatal("updateTrans - Could not connect to database");
            return false;
        }

        pqxx::work txn(c);

        string query = "update transactions set "; //retr_ref_nr=xretr_ref_nr  where tran_nr = xtran_nr;";

        query.append("amount_stl=" + NumberFormatter::format(Utility::convertolong(msg.getField(_005_AMOUNT_SETTLE))) + ",");
        query.append("amount_chb=" + NumberFormatter::format(Utility::convertolong(msg.getField(_006_AMOUNT_CARDHOLDER_BILL))) + ",");
        query.append("fee_amount_chb=" + NumberFormatter::format(Utility::convertolong(msg.getField(_028_AMOUNT_TRAN_FEE))) + ",");
        query.append("convrate_stl=" + NumberFormatter::format(Utility::convertolong(msg.getField(_009_CONV_RATE_SETTLE))) + ",");
        query.append("convrate_chb=" + NumberFormatter::format(Utility::convertolong(msg.getField(_010_CONV_RATE_CARDHOLDER_BILL))) + ",");
        query.append("auth_id_rsp_len='',");
        query.append("auth_id_rsp='" + msg.getField(_038_AUTH_ID_RSP) + "',");
        query.append("rsp_code='" + msg.getField(_039_RSP_CODE) + "',");
        query.append("addl_rsp_data='" + msg.getField(_044_ADDITIONAL_RSP_DATA) + "',");
        query.append("addl_data_private='',");
        query.append("emv_response='" + msg.getField(_055_EMV_DATA) + "',");
        query.append("settlement_code='" + msg.getField(_066_SETTLEMENT_CODE) + "',");
        query.append("account_id_1='" + msg.getField(_102_ACCOUNT_ID_1) + "',");
        query.append("account_id_2='" + msg.getField(_103_ACCOUNT_ID_2) + "',");
        query.append("trans_data_rsp='" + msg.getField(_121_TRAN_DATA_RSP) + "',");
        query.append("tran_state=1,");
        query.append("rsp_in=now(),");
        query.append("rsp_out=now(),");
        query.append("retr_ref_nr='" + msg.getField(_037_RETRIEVAL_REF_NR) + "' ");
        query.append("where tran_nr=" + msg.getExtendedField(_007_TRAN_NR));

        td_logger.debug(query);

        txn.exec(query);
        txn.commit();

        c.disconnect();
    }
    catch (exception &e)
    {
        td_logger.error(e.what());
        return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////////

bool TranMgrDBHandler::addTMDeclineToTrans(Iso8583JSON &msg)
{
    try
    {
        pqxx::connection c(Config::dburl);

        if (!c.is_open())
        {
            td_logger.fatal("addTMDeclineToTrans - Could not connect to database");
            return false;
        }
        pqxx::work txn(c);

        string enc_pan;

        if (msg.isFieldSet(_002_PAN))
        {
            Encrypt e;
            enc_pan = e.encryptdata(msg.getField(_002_PAN));
        }

        string query = "insert into transactions (tran_nr, mti, pan, tran_type, from_account, to_account, amount_tran, amount_stl, amount_chb, datetime_trans, fee_amount_chb, convrate_stl, convrate_chb, stan, time_local, date_local, date_exp, date_stl, date_conv, date_capture, merchant_type, countrycode_acqinstt, countrycode_panextd, countrycode_fwsinstt, pos_entry_mode, app_pan_nr, nii, pos_cond_code, pos_capture_code, auth_id_rsp_len, fee_amount_tran, fee_amount_stl, proc_fee_amount_tran, proc_fee_amount_stl, acq_instt_code, fwd_instt_id, extd_pan, retr_ref_nr, auth_id_rsp, rsp_code, service_rest_code, ca_term_id, ca_id_code, ca_name_loc, addl_rsp_data, addl_data_private, currencycode_tran, currencycode_stl, currencycode_chb, amount_cash, emv_request, emv_response, advice_reason_code, pos_data, settlement_code, extd_payment_code, countrycode_rcvinstt, countrycode_stlinstt, payee, stl_instt_id_code, rcv_instt_id_code, account_id_1, account_id_2, trans_data_req, trans_data_rsp, acquirer_node, issuer_node, super_merchant_id, routing_info, tran_state, pan_encrypted, req_in, req_out, rsp_in, rsp_out, acq_node_key, prev_trannr, transaction_id, term_batch_nr, acq_node_batch_nr, prev_acq_node_key,acq_part_name,retailer_id, store_id, device_id, org_type) values (";

        query.append(appendQueryParams(msg.getExtendedField(7)));
        query.append(appendQueryParams(msg.getMsgType()));
        query.append(appendQueryParams(Utility::protect(msg.getField(2))));
        query.append(appendQueryParams(msg.getField(3).substr(0, 2)));
        query.append(appendQueryParams(msg.getField(3).substr(2, 2)));
        query.append(appendQueryParams(msg.getField(3).substr(4, 2)));
        query.append(appendQueryParams(NumberParser::parse(msg.getField(4))));
        query.append(appendQueryParams("0"));
        query.append(appendQueryParams("0"));
        query.append(appendQueryParams(msg.getField(7)));
        query.append(appendQueryParams("0"));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(msg.getField(11)));
        query.append(appendQueryParams(msg.getField(12)));
        query.append(appendQueryParams(msg.getField(13)));
        query.append(appendQueryParams(msg.getField(14)));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(msg.getField(18)));
        query.append(appendQueryParams(" "));
        query.append(appendQueryParams(" "));
        query.append(appendQueryParams(" "));
        query.append(appendQueryParams(msg.getField(22)));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams("00"));
        query.append(appendQueryParams("00"));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams("0"));
        query.append(appendQueryParams("0"));
        query.append(appendQueryParams("0"));
        query.append(appendQueryParams("0"));
        query.append(appendQueryParams(msg.getField(32)));
        query.append(appendQueryParams(msg.getField(33)));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(msg.getField(37)));
        query.append(appendQueryParams(msg.getField(38)));
        query.append(appendQueryParams(msg.getField(39)));
        query.append(appendQueryParams(msg.getField(40)));
        query.append(appendQueryParams(msg.getField(41)));
        query.append(appendQueryParams(msg.getField(42)));
        query.append(appendQueryParams(msg.getField(43)));
        query.append(appendQueryParams(" "));
        query.append(appendQueryParams(" "));
        query.append(appendQueryParams(msg.getField(49)));
        query.append(appendQueryParams(msg.getField(50)));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams("0"));
        query.append(appendQueryParams(msg.getField(55)));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(""));
        query.append(appendQueryParams(msg.getField(120)));
        query.append(appendQueryParams(msg.getField(121)));
        query.append(appendQueryParams(msg.getNodeInfo().getAcquirerNode()));
        query.append(appendQueryParams(msg.getNodeInfo().getIssuerNode()));
        query.append(appendQueryParams(msg.getNodeInfo().getSuperMID()));
        query.append(appendQueryParams(msg.getNodeInfo().getRoutingInfo()));
        query.append(appendQueryParams(_001_TRAN_COMPLETE));
        query.append(appendQueryParams(enc_pan));
        query.append("now(),");
        query.append("now(),");
        query.append("now(),");
        query.append("now(),");
        query.append(appendQueryParams(msg.getExtendedField(1)));
        query.append(appendQueryParams(
            NumberFormatter::format(Utility::convertolong(msg.getExtendedField(_009_PREV_TRAN_NR)))));
        query.append(appendQueryParams(msg.getField(_062_TRANS_ID)));
        query.append(appendQueryParams(msg.getExtendedField(6)));
        query.append(appendQueryParams("0"));
        query.append(appendQueryParams(msg.getExtendedField(_002_ORG_ACQ_NODE_KEY)));
        query.append(appendQueryParams(msg.getExtendedField(_005_ACQ_PART_NAME)));
        query.append(appendQueryParams(msg.getExtendedField(8)));
        query.append(appendQueryParams(msg.getExtendedField(3)));
        query.append(appendQueryParams(msg.getExtendedField(4)));
        query.append("'" + (msg.getExtendedField(11)) + "'");
        query.append(")");

        td_logger.debug(query);

        txn.exec(query);
        txn.commit();

        c.disconnect();
    }
    catch (exception &e)
    {
        td_logger.error(e.what());
        return false;
    }
    return true;
}
//////////////////////////////////////////////////////////////////////////////////////////////////

string TranMgrDBHandler::appendQueryParams(string value)
{
    value = "'" + value + "',";
    return value;
}
//////////////////////////////////////////////////////////////////////////////////////////////////

string TranMgrDBHandler::appendQueryParams(long value)
{
    string strValue = NumberFormatter::format(value);
    strValue = "'" + strValue + "',";
    return strValue;
}

//////////////////////////////////////////////////////////////////////////

bool TranMgrDBHandler::getIssuerNode(Iso8583JSON &msg, Route &rt)
{
    pqxx::connection c(Config::dburl);

    if (!c.is_open())
    {
        td_logger.fatal("getIssuerNode - Could not connect to database");

        msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);

        return false;
    }

    NodeInfo ni(msg.getNodeInfo());
    string acquirer_node(ni.getAcquirerNode());

    if (acquirer_node == NULL)
    {
        msg.setField(_039_RSP_CODE, _92_ROUTING_ERROR);
        c.disconnect();
        return false;
    }
    else if (acquirer_node.length() == 0)
    {
        msg.setField(_039_RSP_CODE, _92_ROUTING_ERROR);
        c.disconnect();
        return false;
    }

    acquirer_node = trim(acquirer_node);
    string pan = msg.getField(_002_PAN).substr(0, 12);

    //    string query = "select rt_issuer_node, bin_owner, crdbflag, card_country, c.pname, "
    //            "timeout, acq_instt_id, fwd_instt_id, iss_keyname, connurl,repeat_reversal "
    //            "from getissnode('" + acquirer_node + "','" + pan + "')  a "
    //            "left join issuer_nodes b on a.rt_issuer_node=b.iss_node_name "
    //            "left join interface c on b.pname=c.pname;";

    string query = "select rt.rt_issuer_node, "
                   "bin.bin_owner,"
                   "bin.crdbflag, "
                   "bin.card_country, "
                   "c.pname, "
                   "timeout, "
                   "acq_instt_id, "
                   "fwd_instt_id, "
                   "iss_keyname, "
                   "connurl,"
                   "repeat_reversal, "
                   "bin.card_type, "
                   "bin.description, "
                   "bin.bank_name "
                   "from bins bin inner join routes rt on bin.bin_owner=rt.rt_bin_owner "
                   "inner join issuer_nodes b on rt_issuer_node=b.iss_node_name "
                   "inner join interface c on b.pname=c.pname "
                   "where "
                   "bin_range_low <= '" +
                   pan + "' and "
                         "bin_range_high >= '" +
                   pan + "' and "
                         "rt.rt_acq_node_name='" +
                   acquirer_node + "' "
                                   "order by rt_bl_priority limit 1;";

    // Create a non-transactional object
    pqxx::work txn(c);

    // Execute query
    pqxx::result r = txn.exec(query);

    if (r.size() == 0)
    {
        msg.setField(_039_RSP_CODE, _92_ROUTING_ERROR);
        c.disconnect();
        return false;
    }

    ni.setIssuerNode(r[0][0].as<string>());
    ni.setRoutingInfo(r[0][1].as<string>());

    rt.setPname(r[0][4].as<string>());
    rt.setTimeout(r[0][5].as<long>());
    rt.setUrl(r[0][9].as<string>());
    rt.setisskeyname(r[0][8].as<string>());
    rt.setRepeatReversal(r[0][10].as<int>());

    msg.setField(_032_ACQUIRING_INST_ID_CODE, r[0][6].as<string>());
    msg.setField(_033_FORWARDING_INST_ID_CODE, r[0][7].as<string>());

    msg.setField(_122_NODE_INFO, ni.getNodeInfo());

    MapKeyValue mkv;
    mkv.put("bin_owner", r[0][1].as<string>());
    mkv.put("card_country", r[0][3].as<string>());
    mkv.put("bank_name", r[0][13].as<string>());
    mkv.put("description", r[0][12].as<string>());
    mkv.put("card_type", r[0][11].as<string>());
    mkv.put("crdbflag", r[0][2].as<string>());

    string tran_req = msg.getField(_120_TRAN_DATA_REQ) + mkv.toMsg();
    msg.setField(_120_TRAN_DATA_REQ, tran_req);

    c.disconnect();

    return true;
}
//////////////////////////////////////////////////////////////////////////

bool TranMgrDBHandler::getExtdIssuerNode(Iso8583JSON &msg, Route &rt)
{

    pqxx::connection c(Config::dburl);
    if (!c.is_open())
    {
        td_logger.fatal("getExtdIssuerNode - Could not connect to database");
        msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);

        return false;
    }

    NodeInfo ni(msg.getNodeInfo());
    string acquirer_node(ni.getAcquirerNode());
    string extd_code(msg.getField(_067_EXTENDED_PAYMENT_CODE));

    if (acquirer_node == NULL)
    {
        return false;
    }
    else if (acquirer_node.length() == 0)
    {
        return false;
    }

    if (extd_code == NULL)
    {
        return false;
    }
    else if (extd_code.length() == 0)
    {
        return false;
    }

    acquirer_node = trim(acquirer_node);
    extd_code = trim(extd_code);

    string query = "select issuer_node, bin_owner, c.pname, timeout, acq_instt_id, fwd_instt_id, iss_keyname, connurl from routes_extd a left join issuer_nodes b on a.issuer_node=b.iss_node_name left join interface c on b.pname=c.pname where acquirer_node='" + acquirer_node + "' and extd_payment_code='" + extd_code + "'  limit 1";
    // Create a non-transactional object
    pqxx::work txn(c);

    // Execute query
    pqxx::result r = txn.exec(query);

    if (r.size() == 0)
    {
        c.disconnect();
        return false;
    }

    ni.setIssuerNode(r[0][0].as<string>());
    ni.setRoutingInfo(r[0][1].as<string>());

    rt.setPname(r[0][2].as<string>());
    rt.setTimeout(r[0][3].as<long>());
    rt.setUrl(r[0][7].as<string>());
    rt.setisskeyname(r[0][6].as<string>());

    msg.setField(_032_ACQUIRING_INST_ID_CODE, r[0][4].as<string>());
    msg.setField(_033_FORWARDING_INST_ID_CODE, r[0][5].as<string>());

    msg.setField(_122_NODE_INFO, ni.getNodeInfo());

    c.disconnect();

    return true;
}
///////////////////////////////////////////////////////////////////////////////

bool TranMgrDBHandler::getTranInfo(string acq_node_key, Tran &tran)
{
    try
    {
        pqxx::connection c(Config::dburl);
        if (!c.is_open())
        {
            td_logger.fatal("getTranInfo - Could not connect to database");
            return false;
        }
        string sql = "select * from FetchTranByAcqNodeKey('" + acq_node_key + "');";

        // Create a non-transactional object
        pqxx::work txn(c);

        // Execute query
        pqxx::result r = txn.exec(sql);

        if (r.size() == 0)
        {
            c.disconnect();
            tran.tran_nr = 0;
            return false;
        }

        tran.mapFromResultSet(r);

        c.disconnect();

        return true;
    }
    catch (exception &e)
    {
        td_logger.error(e.what());
    }

    return false;
}
////////////////////////////////////////////////////////////////////////////////

bool TranMgrDBHandler::updatereversal(string tran_nr)
{
    try
    {
        pqxx::connection c(Config::dburl);

        if (!c.is_open())
        {
            td_logger.fatal("updatereversal - Could not connect to database");
            return false;
        }
        string sql = "update transactions set isreversed='Y' where tran_nr=" + tran_nr + ";";

        // Create a non-transactional object
        pqxx::work txn(c);

        // Execute query
        txn.exec(sql);
        txn.commit();

        c.disconnect();

        return true;
    }
    catch (exception &e)
    {
        td_logger.error(e.what());
    }

    return false;
}
////////////////////////////////////////////////////////////////////////////////

bool TranMgrDBHandler::updateadjustment(string tran_nr)
{
    try
    {
        pqxx::connection c(Config::dburl);

        if (!c.is_open())
        {
            td_logger.fatal("updateadjustment - Could not connect to database");
            return false;
        }
        string sql = "update transactions set isadjusted='Y' where tran_nr=" + tran_nr + ";";

        // Create a non-transactional object
        pqxx::work txn(c);

        // Execute query
        txn.exec(sql);
        txn.commit();

        c.disconnect();

        return true;
    }
    catch (exception &e)
    {
        td_logger.error(e.what());
    }

    return false;
}
////////////////////////////////////////////////////////////////////////////////

bool TranMgrDBHandler::getrouteinfo(string issuer_node_name, Route &rt)
{
    string query = "select a.pname, a.timeout, b.connurl, '',repeat_reversal from issuer_nodes a left join interface b on a.pname=b.pname where a.iss_node_name='" + issuer_node_name + "';";

    pqxx::connection c(Config::dburl);
    if (!c.is_open())
    {
        td_logger.fatal("getrouteinfo - Could not connect to database");
        return false;
    }
    // Create a non-transactional object
    pqxx::work txn(c);

    // Execute query
    pqxx::result r = txn.exec(query);

    if (r.size() == 0)
    {
        c.disconnect();
        return false;
    }

    rt.setPname(r[0][0].as<string>());
    rt.setTimeout(r[0][1].as<long>());
    rt.setUrl(r[0][2].as<string>());
    rt.setRepeatReversal(r[0][4].as<int>());

    c.disconnect();

    return true;
}
#endif