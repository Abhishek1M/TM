/* 
 * File:   Worker.h
 * Author: Abhy
 *
 * Created on May 27, 2018, 8:15 PM
 */

#ifndef WORKER_H
#define WORKER_H

#include <Poco/Runnable.h>
#include <Iso8583JSON.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Logger.h>
#include <iostream>

#include "TranMgrDBHandler.h"

using namespace std;

using Poco::Logger;
using Poco::Runnable;

class Worker : public Runnable {
public:

    Worker(string data, HTTPServerResponse &resp, Logger& logger) : _data(data), _resp(resp),
    m_logger(logger), tmdbh(logger) {
    }

    ~Worker() {
    }

    string processMsg(string request);
    bool isValidMsg(Iso8583JSON& msg);

    virtual void run() {
        string rsp;

        rsp = processMsg(_data);

        if (!rsp.empty()) {
            //_resp.setStatus(HTTPResponse::HTTP_OK);
            _resp.setContentType("application/json");

            ostream& out = _resp.send();
            out << rsp;

            out.flush();
        } else {
            _resp.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
            _resp.setContentType("text/plain");

            ostream& out = _resp.send();
            out << "Cannot process message";

            out.flush();
            
            m_logger.error("Worker#run -- Could not process message");
        }
    }


private:
    string _data;
    HTTPServerResponse &_resp;
    TranMgrDBHandler tmdbh;
    Logger& m_logger;
};
/////////////////////////////////////////////////

string Worker::processMsg(string request) {
    Iso8583JSON msg;

    //Parser parser;
    string resp("NOK");

    try {
        msg.parseMsg(request);

        m_logger.information(msg.dumpMsg());

        string val = msg.getMsgType();

        long tran_nr = tmdbh.getNewTranNr();

        if (tran_nr == 0) {
            msg.setRspMsgType();

            msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);

            m_logger.critical("Could not fetch new transaction number");
            m_logger.critical("Response\n" + msg.dumpMsg());

            resp = msg.toMsg();

            return resp;
        }

        string rrn = NumberFormatter::format0(tran_nr, 12);
        msg.setExtendedField(_007_TRAN_NR, rrn);
        if (msg.isFieldSet(_037_RETRIEVAL_REF_NR) == false) {
            msg.setField(_037_RETRIEVAL_REF_NR, rrn);
        }

        MsgHandler mh(m_logger);

        if (isValidMsg(msg) == false) {
            msg.setRspMsgType();
            msg.setField(39, "30");
        } else {
            if (val == "0100") {
                mh.processFinMsg(msg, tmdbh);
            } else if (val == "0200") {
                mh.processFinMsg(msg, tmdbh);
            } else if (val == "0320") {
                mh.process0320Msg(msg, tmdbh);
            } else if (val == "0420") {
                mh.process0420Msg(msg, tmdbh);
            } else if (val == "0520") {
                mh.process0520Msg(msg, tmdbh);
            } else if (val == "0620") {
                mh.process0620Msg(msg, tmdbh);
            } else if (val == "0220") {
                mh.process0220Msg(msg, tmdbh);
            }
        }

        m_logger.information(msg.dumpMsg());

        resp = msg.toMsg();
    } catch (Poco::Exception &e) {
        m_logger.error(Poco::format("Exception in TMRequestHandler::processMsg : (%s)", e.what()));

        msg.setRspMsgType();
        msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);

        resp = msg.toMsg();
    }

    //resp.append("\r\n");

    //    m_logger.information("Message To Acquirer Node\n" + resp);

    return resp;
}

//////////////////////////////////////////////////////

bool Worker::isValidMsg(Iso8583JSON& msg) {
    bool data_for_routing = false;

    if ((msg.getMsgType().compare(_0100_AUTH_REQ) == 0) ||
            (msg.getMsgType().compare(_0200_TRAN_REQ) == 0)) {
        // Field 002
        if (msg.isFieldSet(_002_PAN)) {
            int pan_len = msg.getField(_002_PAN).length();
            if (pan_len > 19 || pan_len < 12) {
                msg.setField(_044_ADDITIONAL_RSP_DATA,
                        "002");

                m_logger.error("Incorrect Request - Field 2 length is incorrect");
                return false;
            }

            data_for_routing = true;
        }

        // Field 035
        if (msg.isFieldSet(_035_TRACK_2_DATA)) {
            int track2_len = msg.getField(
                    _035_TRACK_2_DATA).
                    length();
            if (track2_len > 37 || track2_len < 12) {
                msg.setField(_044_ADDITIONAL_RSP_DATA,
                        "035");

                m_logger.error("Incorrect Request - Field 35 length is incorrect");
                return false;
            }

            data_for_routing = true;
        }

        // Field 003
        if (!msg.isFieldSet(_003_PROCESSING_CODE)) {
            msg.setField(_044_ADDITIONAL_RSP_DATA,
                    "003");

            m_logger.error("Incorrect Request - Field 3 is absent");
            return false;
        } else if (msg.getField(_003_PROCESSING_CODE).
                length() != 6) {
            msg.setField(_044_ADDITIONAL_RSP_DATA, "003");

            m_logger.error("Incorrect Request - Field 003 length is incorrect");
            return false;
        }

        // Field 004
        if (!msg.isFieldSet(_004_AMOUNT_TRANSACTION)) {
            m_logger.error("Incorrect Request - Field 4 is absent");
            msg.setField(_044_ADDITIONAL_RSP_DATA, "004");
            return false;
        }

        // Field 007
        if (!msg.isFieldSet(_007_TRANSMISSION_DATE_TIME)) {
            m_logger.error("Incorrect Request - Field 7 is absent");
            return false;
        } else if (msg.getField(_007_TRANSMISSION_DATE_TIME).
                length() != 10) {
            msg.setField(_044_ADDITIONAL_RSP_DATA,
                    "007");

            m_logger.error("Incorrect Request - Field 007 length is incorrect");
            return false;
        }

        // Field 011
        if (!msg.isFieldSet(_011_SYSTEMS_TRACE_AUDIT_NR)) {
            m_logger.error("Field 11 is absent");

            return false;
        } else if (msg.getField(_011_SYSTEMS_TRACE_AUDIT_NR).
                length() != 6) {
            msg.setField(_044_ADDITIONAL_RSP_DATA,
                    "011");

            m_logger.error("Field 011 length is incorrect");
            return false;
        }

        // Field 012
        if (!msg.isFieldSet(_012_TIME_LOCAL)) {
            m_logger.error("Field 12 is absent");
            return false;
        }

        if (msg.getField(_012_TIME_LOCAL).
                length() != 6) {
            msg.setField(_044_ADDITIONAL_RSP_DATA,
                    "012");

            m_logger.error("Field 012 length is incorrect");
            return false;
        }

        // Field 013
        if (!msg.isFieldSet(_013_DATE_LOCAL)) {
            m_logger.error("Field 13 is absent");
            return false;
        }

        if (msg.getField(_013_DATE_LOCAL).length() != 4) {
            msg.setField(_044_ADDITIONAL_RSP_DATA,
                    "013");

            m_logger.error("Field 013 length is incorrect");
            return false;
        }

        // Field 014
        if (msg.isFieldSet(_014_DATE_EXPIRATION)) {
            string expdt = msg.getField(
                    _014_DATE_EXPIRATION);
            if (expdt.length() != 4) {
                m_logger.error("Field 14 is invalid");
                msg.setField(_044_ADDITIONAL_RSP_DATA, "014");
                return false;
            }
        }

        // Field 018
        if (msg.isFieldSet(_018_MERCHANT_TYPE)) {
            string mcc = msg.getField(
                    _018_MERCHANT_TYPE);
            if (mcc.length() != 4) {
                msg.setField(_044_ADDITIONAL_RSP_DATA, "018");
                m_logger.error("Field 18 is invalid");
                return false;
            }
        }

        // Field 022
        if (!msg.isFieldSet(_022_POS_ENTRY_MODE)) {
            m_logger.error("Field 22 is absent");

            return false;
        } else if (msg.getField(_022_POS_ENTRY_MODE).
                length() != 3) {
            msg.setField(_044_ADDITIONAL_RSP_DATA, "022");

            m_logger.error("Field 022 length is incorrect");
            return false;
        }

        // Field 25
        if (!msg.isFieldSet(_025_POS_CONDITION_CODE)) {
            m_logger.error("Field 25 is absent");
            msg.setField(_044_ADDITIONAL_RSP_DATA,
                    "025");
            return false;
        } else if (msg.getField(_025_POS_CONDITION_CODE).
                length() != 2) {
            msg.setField(_044_ADDITIONAL_RSP_DATA, "025");

            m_logger.error("Field 025 length is incorrect");
            return false;
        }

        if (!msg.isFieldSet(_041_CARD_ACCEPTOR_TERM_ID)) {
            m_logger.error("Field 41 is absent");
            msg.setField(_044_ADDITIONAL_RSP_DATA,
                    "041");
            return false;
        } else if (msg.getField(_041_CARD_ACCEPTOR_TERM_ID).
                length() != 8) {
            msg.setField(_044_ADDITIONAL_RSP_DATA,
                    "041");

            m_logger.error("Field 041 length is incorrect");
            return false;
        }

        if (!msg.isFieldSet(_042_CARD_ACCEPTOR_ID_CODE)) {
            m_logger.error("Field 42 is absent");
            return false;
        }

        if (!msg.isFieldSet(_043_CARD_ACCEPTOR_NAME_LOC)) {
            m_logger.error("Field 43 is absent");
            return false;
        }

        if (msg.getField(_043_CARD_ACCEPTOR_NAME_LOC).length() > 40) {
            m_logger.error("Field 43 length is greater than 40");
            return false;
        }

        if (!msg.isFieldSet(_049_CURRENCY_CODE_TRAN)) {
            m_logger.error("Field 49 is absent");
            return false;
        }

        if (!msg.isFieldSet(_062_TRANS_ID)) {
            m_logger.error("Field 62 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _001_ACQ_NODE_KEY)) {
            m_logger.error("Extended Field 123_001 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _003_STORE_ID)) {
            m_logger.error("Extended Field 123_003 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _004_DEVICE_ID)) {
            m_logger.error("Extended Field 123_004 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _005_ACQ_PART_NAME)) {
            m_logger.error("Extended Field 123_005 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _006_TERM_BATCH_NR)) {
            m_logger.error("Extended Field 123_006 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _008_RETAILER_ID)) {
            m_logger.error("Extended Field 123_008 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _011_ORIGINATOR_TYPE)) {
            m_logger.error("Extended Field 123_011 is absent");
            return false;
        }
    } else

        if (msg.getMsgType().compare(_0220_TRAN_ADV) == 0) {
        // Check for MTI = 0220
        if (!msg.isFieldSet(_003_PROCESSING_CODE)) {
            m_logger.error("Field 3 is absent");
            return false;
        }

        if (!msg.isFieldSet(_004_AMOUNT_TRANSACTION)) {
            m_logger.error("Field 4 is absent");
            return false;
        }

        if (!msg.isFieldSet(_007_TRANSMISSION_DATE_TIME)) {
            m_logger.error("Field 7 is absent");
            return false;
        }

        if (!msg.isFieldSet(_011_SYSTEMS_TRACE_AUDIT_NR)) {
            m_logger.error("Field 11 is absent");

            return false;
        }

        if (!msg.isFieldSet(_022_POS_ENTRY_MODE)) {
            m_logger.error("Field 22 is absent");

            return false;
        }

        if (!msg.isFieldSet(_025_POS_CONDITION_CODE)) {
            m_logger.error("Field 25 is absent");
            return false;
        }

        if (!msg.isFieldSet(_041_CARD_ACCEPTOR_TERM_ID)) {
            m_logger.error("Field 41 is absent");
            return false;
        }

        if (!msg.isFieldSet(_042_CARD_ACCEPTOR_ID_CODE)) {
            m_logger.error("Field 42 is absent");
            return false;
        }

        if (!msg.isFieldSet(_049_CURRENCY_CODE_TRAN)) {
            m_logger.error("Field 49 is absent");
            return false;
        }

        if (!msg.isFieldSet(_062_TRANS_ID)) {
            m_logger.error("Field 62 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _001_ACQ_NODE_KEY)) {
            m_logger.error("Extended Field 123_001 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _003_STORE_ID)) {
            m_logger.error("Extended Field 123_003 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _004_DEVICE_ID)) {
            m_logger.error("Extended Field 123_004 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _005_ACQ_PART_NAME)) {
            m_logger.error("Extended Field 123_005 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _006_TERM_BATCH_NR)) {
            m_logger.error("Extended Field 123_006 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _008_RETAILER_ID)) {
            m_logger.error("Extended Field 123_008 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _011_ORIGINATOR_TYPE)) {
            m_logger.error("Extended Field 123_011 is absent");
            return false;
        }
    } else
        // 0520 MTI
        if (msg.getMsgType().compare(_0220_TRAN_ADV) == 0) {
        if (!msg.isFieldSet(_011_SYSTEMS_TRACE_AUDIT_NR)) {
            m_logger.error("Field 11 is absent");

            return false;
        }

        if (!msg.isFieldSet(_041_CARD_ACCEPTOR_TERM_ID)) {
            m_logger.error("Field 41 is absent");
            return false;
        }

        if (!msg.isFieldSet(_042_CARD_ACCEPTOR_ID_CODE)) {
            m_logger.error("Field 42 is absent");
            return false;
        }

        if (!msg.isFieldSet(_049_CURRENCY_CODE_TRAN)) {
            m_logger.error("Field 49 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(_001_ACQ_NODE_KEY)) {
            m_logger.error("Extended Field 123_001 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(_003_STORE_ID)) {
            m_logger.error("Extended Field 123_003 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(_004_DEVICE_ID)) {
            m_logger.error("Extended Field 123_004 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(_005_ACQ_PART_NAME)) {
            m_logger.error("Extended Field 123_005 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(_006_TERM_BATCH_NR)) {
            m_logger.error("Extended Field 123_006 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(_008_RETAILER_ID)) {
            m_logger.error("Extended Field 123_008 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(_011_ORIGINATOR_TYPE)) {
            m_logger.error("Extended Field 123_011 is absent");
            return false;
        }
    }

    return true;
}
//////////////////////////////////////////////////////


#endif /* WORKER_H */