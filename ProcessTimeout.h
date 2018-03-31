/* 
 * File:   ProcessTimeout.h
 * Author: abhishek.m
 *
 * Created on 18 May, 2017, 11:21 AM
 */

#ifndef PROCESSTIMEOUT_H
#define PROCESSTIMEOUT_H

#include <Poco/Runnable.h>
#include <Poco/Logger.h>
#include <ap/Iso8583JSON.h>

#include "Config.h"
#include "TranMgrDBHandler.h"

using namespace std;

class ProcessTimeout : public Poco::Runnable {
public:

    ProcessTimeout(Logger& alogger) : m_logger(alogger) {
    }

    bool processMsg(string data);
    ////////////////////////////////////////////////////////

    virtual void run() {
        string data;

        while (1) {
            try {
                if (Config::msg.size() > 0) {
                    data = Config::msg.front();

                    processMsg(data);

                    Config::msg.pop();
                }

                sleep(10);
            } catch (exception &e) {
                m_logger.error("Error processing transaction #ProcessTimeout");
                m_logger.error(e.what());
                
                m_logger.error("Aborting transaction #");
                Config::msg.pop();
            }
        }
    }

private:
    Logger& m_logger;
};
///////////////////////////////////////////////////////////////////////////

bool ProcessTimeout::processMsg(string data) {
    TranMgrDBHandler tmdbh(m_logger);
    bool fetchdata;
    bool status = false;

    m_logger.information("Data - " + data);

    Iso8583JSON msg;
    msg.parseMsg(data);

    if ((msg.getMsgType().compare("0100") == 0) || ((msg.getMsgType().compare("0200") == 0))) {
        msg.setMsgType("0420");
    } else if (msg.getMsgType().compare("0420") == 0) {
        msg.setMsgType("0421");
    }

    if (msg.getMsgType().compare("0420") == 0) {
        // add the 0420 to onl_trans
        long tran_nr = tmdbh.getNewTranNr();

        if (tran_nr == 0) {
            msg.setRspMsgType();

            msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);

            m_logger.critical("Could not fetch new transaction number");
            m_logger.critical("Response\n" + msg.dumpMsg());

            return false;
        }

        msg.setExtendedField(_009_PREV_TRAN_NR, msg.getExtendedField(_007_TRAN_NR));
        msg.setExtendedField(_002_ORG_ACQ_NODE_KEY, msg.getExtendedField(_001_ACQ_NODE_KEY));

        string tran_nr_s = NumberFormatter::format0(tran_nr, 12);
        msg.setExtendedField(_007_TRAN_NR, tran_nr_s);
        msg.setExtendedField(_001_ACQ_NODE_KEY, tran_nr_s);

        tmdbh.addToTrans(msg);
    }

    NodeInfo ni(msg.getField(_122_NODE_INFO));
    Route rt;
    fetchdata = tmdbh.getrouteinfo(ni.getIssuerNode(), rt);

    if (fetchdata == true) {
        int revs = rt.getRepeatReversal();

        while (revs > 0) {
            string rsp;

            rsp = Utility::ofPostRequest(rt.getUrl(), msg.toMsg(), rt.getTimeout());

            if ((rsp.compare("NOK") == 0) || (rsp.compare("TMO") == 0)) {
                m_logger.error("Response from remote : " + rsp);

                msg.setField(_039_RSP_CODE, _91_ISSUER_OR_SWITCH_INOPERATIVE);

                status = false;
                revs--;
            } else {
                Iso8583JSON msg_from_issuer;
                msg_from_issuer.parseMsg(rsp);

                msg.setField(_005_AMOUNT_SETTLE, msg_from_issuer.getField(_005_AMOUNT_SETTLE));
                msg.setField(_006_AMOUNT_CARDHOLDER_BILL, msg_from_issuer.getField(_006_AMOUNT_CARDHOLDER_BILL));
                msg.setField(_009_CONV_RATE_SETTLE, msg_from_issuer.getField(_009_CONV_RATE_SETTLE));
                msg.setField(_010_CONV_RATE_CARDHOLDER_BILL, msg_from_issuer.getField(_010_CONV_RATE_CARDHOLDER_BILL));
                msg.setField(_028_AMOUNT_TRAN_FEE, msg_from_issuer.getField(_028_AMOUNT_TRAN_FEE));
                msg.setField(_037_RETRIEVAL_REF_NR, msg_from_issuer.getField(_037_RETRIEVAL_REF_NR));
                msg.setField(_038_AUTH_ID_RSP, msg_from_issuer.getField(_038_AUTH_ID_RSP));
                msg.setField(_039_RSP_CODE, msg_from_issuer.getField(_039_RSP_CODE));
                msg.setField(_044_ADDITIONAL_RSP_DATA, msg_from_issuer.getField(_044_ADDITIONAL_RSP_DATA));
                msg.setField(_055_EMV_DATA, msg_from_issuer.getField(_055_EMV_DATA));
                msg.setField(_102_ACCOUNT_ID_1, msg_from_issuer.getField(_102_ACCOUNT_ID_1));
                msg.setField(_103_ACCOUNT_ID_2, msg_from_issuer.getField(_103_ACCOUNT_ID_2));
                msg.setField(_121_TRAN_DATA_RSP, msg_from_issuer.getField(_121_TRAN_DATA_RSP));

                status = true;
                revs = 0;
            }
        }
    } else {
        m_logger.error("Could not fetch Issuer node data");
        m_logger.error(msg.dumpMsg());

        msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);

        status = false;
    }

    msg.setRspMsgType();

    tmdbh.updateTrans(msg);
    tmdbh.updatereversal(msg.getExtendedField(_009_PREV_TRAN_NR));


    return status;
}
#endif /* PROCESSTIMEOUT_H */

