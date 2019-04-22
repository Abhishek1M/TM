/*
 * TranMgr.cpp
 *
 */
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Util/ServerApplication.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/PartHandler.h>
#include <Poco/CountingStream.h>
#include <Poco/NullStream.h>
#include <Poco/StreamCopier.h>

#include <Poco/Logger.h>
#include <Poco/FileChannel.h>
#include <Poco/AutoPtr.h>
#include <Poco/FormattingChannel.h>
#include <Poco/PatternFormatter.h>

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <Poco/NumberFormatter.h>

#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/HelpFormatter.h>

#include <Poco/NumberParser.h>

#include <Poco/Timespan.h>

#include <iostream>
#include <string>
#include <vector>

#include <pqxx/pqxx>

#include <Iso8583JSON.h>
#include <Utility.h>
#include "TranMgrDBHandler.h"
#include <constants.h>
#include "MsgHandler.h"
#include "Config.h"
#include "ProcessTimeout.h"
#include "Worker.h"
#include "WorkerThreadPool.h"
#include "UpdateTransactionsInDB.h"

using Poco::Timespan;

using namespace Poco::Net;
using namespace Poco::Util;
using Poco::Net::NameValueCollection;
using namespace std;
using Poco::CountingInputStream;
using Poco::NullOutputStream;
using Poco::StreamCopier;

using Poco::ThreadPool;

using Poco::AutoPtr;
using Poco::FileChannel;
using Poco::FormattingChannel;
using Poco::Logger;
using Poco::PatternFormatter;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;

using Poco::NumberFormatter;
using Poco::NumberParser;

using namespace Poco::JSON;
using namespace Poco::Dynamic;

////////////////////////////////////////////////////////////////////////////////

string Config::dburl;
string Config::hsmurl;
string Config::mq;
string Config::moduleName;
bool Config::isFallbackallowed;
string Config::fallbackExclusionList;
int Config::maxCashAtPOS;
map<string, int> Config::timeOffset;
string Config::keypath;

//
queue<string> Config::msg;
queue<WriteToDBEntity> Config::commit_db;
//
Encrypt Config::e;
///////////////////////////////////////////////////////////////////////////////

class DBUpdate : public Poco::Runnable
{
public:
    DBUpdate(string modulename, Logger &alogger) : _modulename(modulename), m_logger(alogger)
    {
    }

    virtual void run()
    {
        while (1)
        {
            try
            {
                pqxx::connection c(Config::dburl);
                if (c.is_open())
                {
                    // Create a non-transactional object
                    pqxx::work txn(c);
                    string query =
                        "UPDATE interface set last_update_datetime=now() where pname='" + _modulename + "';";
                    // Execute query
                    txn.exec(query);
                    txn.commit();
                    m_logger.debug("UPDATE query :" + query);

                    c.disconnect();
                }
                else
                {
                    m_logger.error("Could not open connection. Error while updating status in interface");
                }
            }
            catch (Poco::Exception &e)
            {
                m_logger.error("Error while updating status in interface");
                m_logger.error(e.displayText());
                m_logger.error(e.message());
            }
            sleep(30);
        }
    }

private:
    string _modulename;
    Logger &m_logger;
};

////////////////////////////////////////////////////////////////////////////////

class TMGetStatusHandler : public HTTPRequestHandler
{
public:
    TMGetStatusHandler(Logger &alogger) : m_logger(alogger)
    {
    }

    ~TMGetStatusHandler()
    {
    }

    virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp)
    {
        LocalDateTime now;
        string responseStr = Poco::DateTimeFormatter::format(now, "%Y-%m-%d %H:%M:%s");
        responseStr = responseStr + "\nCommit DB Pending - " + NumberFormatter::format(Config::commit_db.size());
        responseStr = responseStr + "\nPending Timeout Reversals - " + NumberFormatter::format(Config::msg.size()) + "\n";
        responseStr = responseStr + "Ok\n";

        resp.setStatus(HTTPResponse::HTTP_OK);

        ostream &out = resp.send();
        out << responseStr;

        out.flush();

        m_logger.notice(responseStr);
    }

private:
    Logger &m_logger;
};
////////////////////////////////////////////////////////////////////////////////

class TMReloadHandler : public HTTPRequestHandler
{
public:
    TMReloadHandler()
    {
    }

    ~TMReloadHandler()
    {
    }

    virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp)
    {
        Application &app = Application::instance();
        string responseStr = "Reload - OK";

        app.loadConfiguration(Application::PRIO_APPLICATION);

        Logger &lgr = app.logger().get(Config::moduleName);
        lgr.information("Reloading values.");

        Config::hsmurl = app.config().getString("HSMURL");
        Config::maxCashAtPOS = app.config().getInt("CashAtPOSLimit", 0);
        Config::isFallbackallowed = app.config().getBool("AllowFallback", false);
        Config::fallbackExclusionList = app.config().getString("FallbackExclusionList", "");

        lgr.information("New HSM URL - " + Config::hsmurl);
        lgr.information("CashAtPOSLimit - " + NumberFormatter::format(Config::maxCashAtPOS));
        if (Config::isFallbackallowed == true)
        {
            lgr.information("Fallback is allowed");
        }
        else
        {
            lgr.information("Fallback is not allowed");
        }

        lgr.information("Loading TimeOffset values");
        Config::loadTimeOffset();

        resp.setStatus(HTTPResponse::HTTP_OK);
        //resp.setContentType("application/json; charset=UTF-8");

        ostream &out = resp.send();
        out << responseStr;

        out.flush();

        lgr.information("Reloading values - complete");
        lgr.notice("Responded with OK");
    }
};
////////////////////////////////////////////////////////////////////////////////

class TMRequestHandler : public HTTPRequestHandler
{
public:
    TMRequestHandler(Logger &logger) : m_logger(logger), tmdbh(logger)
    {
    }

    ~TMRequestHandler()
    {
    }

    virtual void handleRequest(HTTPServerRequest &req,
                               HTTPServerResponse &resp)
    {
        string responseStr;
        string requestStr;
        istream &istr = req.stream();

        StreamCopier::copyToString(istr, requestStr);

        try
        {
            if (!requestStr.empty() && requestStr.length() > 0)
            {
                //                Worker* worker = new Worker(requestStr, resp, m_logger);
                //                WorkerThreadPool::getInstance().tp->start(*worker);

                //                resp.setStatus(HTTPResponse::HTTP_CONTINUE);
                //
                //                ostream& out = resp.send();
                //                out.flush();
                m_logger.trace(requestStr);

                responseStr = processMsg(requestStr);

                resp.setStatus(HTTPResponse::HTTP_OK);
                resp.setContentType("application/json");
                resp.setContentLength64(responseStr.length());
                ostream &out = resp.send();
                out << responseStr;

                out.flush();
            }
            else
            {
                resp.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
                ostream &out = resp.send();
                out << "No data received in the request";

                out.flush();

                m_logger.error("No data received in the request");
            }
        }
        catch (Poco::Exception &e)
        {
            m_logger.error(Poco::format("Exception in TMRequestHandler::handleRequest : (%s)", e.what()));
        }
    }

private:
    Logger &m_logger;
    TranMgrDBHandler tmdbh;

    string processMsg(string request);

    bool isValidMsg(Iso8583JSON &msg);
};

//////////////////////////////////////////////////////

bool TMRequestHandler::isValidMsg(Iso8583JSON &msg)
{
    bool data_for_routing = false;

    if ((msg.getMsgType().compare(_0100_AUTH_REQ) == 0) ||
        (msg.getMsgType().compare(_0200_TRAN_REQ) == 0))
    {
        // Field 002
        if (msg.isFieldSet(_002_PAN))
        {
            int pan_len = msg.getField(_002_PAN).length();
            if (pan_len > 19 || pan_len < 12)
            {
                msg.setField(_044_ADDITIONAL_RSP_DATA, "002");

                m_logger.error("Incorrect Request - Field 2 length is incorrect");
                return false;
            }

            data_for_routing = true;
        }

        // Field 035
        if (msg.isFieldSet(_035_TRACK_2_DATA))
        {
            int track2_len = msg.getField(
                                    _035_TRACK_2_DATA)
                                 .length();
            if (track2_len > 37 || track2_len < 12)
            {
                msg.setField(_044_ADDITIONAL_RSP_DATA,
                             "035");

                m_logger.error("Incorrect Request - Field 35 length is incorrect");
                return false;
            }

            data_for_routing = true;
        }

        // Field 003
        if (!msg.isFieldSet(_003_PROCESSING_CODE))
        {
            msg.setField(_044_ADDITIONAL_RSP_DATA,
                         "003");

            m_logger.error("Incorrect Request - Field 3 is absent");
            return false;
        }
        else if (msg.getField(_003_PROCESSING_CODE).length() != 6)
        {
            msg.setField(_044_ADDITIONAL_RSP_DATA, "003");

            m_logger.error("Incorrect Request - Field 003 length is incorrect");
            return false;
        }

        // Field 004
        if (!msg.isFieldSet(_004_AMOUNT_TRANSACTION))
        {
            m_logger.error("Incorrect Request - Field 4 is absent");
            msg.setField(_044_ADDITIONAL_RSP_DATA, "004");
            return false;
        }

        // Field 007
        if (!msg.isFieldSet(_007_TRANSMISSION_DATE_TIME))
        {
            m_logger.error("Incorrect Request - Field 7 is absent");
            return false;
        }
        else if (msg.getField(_007_TRANSMISSION_DATE_TIME).length() != 10)
        {
            msg.setField(_044_ADDITIONAL_RSP_DATA,
                         "007");

            m_logger.error("Incorrect Request - Field 007 length is incorrect");
            return false;
        }

        // Field 011
        if (!msg.isFieldSet(_011_SYSTEMS_TRACE_AUDIT_NR))
        {
            m_logger.error("Field 11 is absent");

            return false;
        }
        else if (msg.getField(_011_SYSTEMS_TRACE_AUDIT_NR).length() != 6)
        {
            msg.setField(_044_ADDITIONAL_RSP_DATA,
                         "011");

            m_logger.error("Field 011 length is incorrect");
            return false;
        }

        // Field 012
        if (!msg.isFieldSet(_012_TIME_LOCAL))
        {
            m_logger.error("Field 12 is absent");
            return false;
        }

        if (msg.getField(_012_TIME_LOCAL).length() != 6)
        {
            msg.setField(_044_ADDITIONAL_RSP_DATA,
                         "012");

            m_logger.error("Field 012 length is incorrect");
            return false;
        }

        // Field 013
        if (!msg.isFieldSet(_013_DATE_LOCAL))
        {
            m_logger.error("Field 13 is absent");
            return false;
        }

        if (msg.getField(_013_DATE_LOCAL).length() != 4)
        {
            msg.setField(_044_ADDITIONAL_RSP_DATA,
                         "013");

            m_logger.error("Field 013 length is incorrect");
            return false;
        }

        // Field 014
        if (msg.isFieldSet(_014_DATE_EXPIRATION))
        {
            string expdt = msg.getField(
                _014_DATE_EXPIRATION);
            if (expdt.length() != 4)
            {
                m_logger.error("Field 14 is invalid");
                msg.setField(_044_ADDITIONAL_RSP_DATA, "014");
                return false;
            }
        }

        // Field 018
        if (msg.isFieldSet(_018_MERCHANT_TYPE))
        {
            string mcc = msg.getField(
                _018_MERCHANT_TYPE);
            if (mcc.length() != 4)
            {
                msg.setField(_044_ADDITIONAL_RSP_DATA, "018");
                m_logger.error("Field 18 is invalid");
                return false;
            }
        }

        // Field 022
        if (!msg.isFieldSet(_022_POS_ENTRY_MODE))
        {
            m_logger.error("Field 22 is absent");

            return false;
        }
        else if (msg.getField(_022_POS_ENTRY_MODE).length() != 3)
        {
            msg.setField(_044_ADDITIONAL_RSP_DATA, "022");

            m_logger.error("Field 022 length is incorrect");
            return false;
        }

        // Field 25
        if (!msg.isFieldSet(_025_POS_CONDITION_CODE))
        {
            m_logger.error("Field 25 is absent");
            msg.setField(_044_ADDITIONAL_RSP_DATA,
                         "025");
            return false;
        }
        else if (msg.getField(_025_POS_CONDITION_CODE).length() != 2)
        {
            msg.setField(_044_ADDITIONAL_RSP_DATA, "025");

            m_logger.error("Field 025 length is incorrect");
            return false;
        }

        if (!msg.isFieldSet(_041_CARD_ACCEPTOR_TERM_ID))
        {
            m_logger.error("Field 41 is absent");
            msg.setField(_044_ADDITIONAL_RSP_DATA,
                         "041");
            return false;
        }
        else if (msg.getField(_041_CARD_ACCEPTOR_TERM_ID).length() != 8)
        {
            msg.setField(_044_ADDITIONAL_RSP_DATA,
                         "041");

            m_logger.error("Field 041 length is incorrect");
            return false;
        }

        if (!msg.isFieldSet(_042_CARD_ACCEPTOR_ID_CODE))
        {
            m_logger.error("Field 42 is absent");
            return false;
        }

        if (!msg.isFieldSet(_043_CARD_ACCEPTOR_NAME_LOC))
        {
            m_logger.error("Field 43 is absent");
            return false;
        }

        if (msg.getField(_043_CARD_ACCEPTOR_NAME_LOC).length() > 40)
        {
            m_logger.error("Field 43 length is greater than 40");
            return false;
        }

        if (!msg.isFieldSet(_049_CURRENCY_CODE_TRAN))
        {
            m_logger.error("Field 49 is absent");
            return false;
        }

        if (!msg.isFieldSet(_062_TRANS_ID))
        {
            m_logger.error("Field 62 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _001_ACQ_NODE_KEY))
        {
            m_logger.error("Extended Field 123_001 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _003_STORE_ID))
        {
            m_logger.error("Extended Field 123_003 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _004_DEVICE_ID))
        {
            m_logger.error("Extended Field 123_004 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _005_ACQ_PART_NAME))
        {
            m_logger.error("Extended Field 123_005 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(_006_TERM_BATCH_NR))
        {
            m_logger.error("Extended Field 123_006 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(_008_RETAILER_ID))
        {
            m_logger.error("Extended Field 123_008 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(_011_ORIGINATOR_TYPE))
        {
            m_logger.error("Extended Field 123_011 is absent");
            return false;
        }
    }
    else if (msg.getMsgType().compare(_0220_TRAN_ADV) == 0)
    {
        // Check for MTI = 0220
        if (!msg.isFieldSet(_003_PROCESSING_CODE))
        {
            m_logger.error("Field 3 is absent");
            return false;
        }

        if (!msg.isFieldSet(_004_AMOUNT_TRANSACTION))
        {
            m_logger.error("Field 4 is absent");
            return false;
        }

        if (!msg.isFieldSet(_007_TRANSMISSION_DATE_TIME))
        {
            m_logger.error("Field 7 is absent");
            return false;
        }

        if (!msg.isFieldSet(_011_SYSTEMS_TRACE_AUDIT_NR))
        {
            m_logger.error("Field 11 is absent");

            return false;
        }

        if (!msg.isFieldSet(_022_POS_ENTRY_MODE))
        {
            m_logger.error("Field 22 is absent");

            return false;
        }

        if (!msg.isFieldSet(_025_POS_CONDITION_CODE))
        {
            m_logger.error("Field 25 is absent");
            return false;
        }

        if (!msg.isFieldSet(_041_CARD_ACCEPTOR_TERM_ID))
        {
            m_logger.error("Field 41 is absent");
            return false;
        }

        if (!msg.isFieldSet(_042_CARD_ACCEPTOR_ID_CODE))
        {
            m_logger.error("Field 42 is absent");
            return false;
        }

        if (!msg.isFieldSet(_049_CURRENCY_CODE_TRAN))
        {
            m_logger.error("Field 49 is absent");
            return false;
        }

        if (!msg.isFieldSet(_062_TRANS_ID))
        {
            m_logger.error("Field 62 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _001_ACQ_NODE_KEY))
        {
            m_logger.error("Extended Field 123_001 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _003_STORE_ID))
        {
            m_logger.error("Extended Field 123_003 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _004_DEVICE_ID))
        {
            m_logger.error("Extended Field 123_004 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _005_ACQ_PART_NAME))
        {
            m_logger.error("Extended Field 123_005 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _006_TERM_BATCH_NR))
        {
            m_logger.error("Extended Field 123_006 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _008_RETAILER_ID))
        {
            m_logger.error("Extended Field 123_008 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(
                _011_ORIGINATOR_TYPE))
        {
            m_logger.error("Extended Field 123_011 is absent");
            return false;
        }
    }
    else
        // 0520 MTI
        if (msg.getMsgType().compare(_0220_TRAN_ADV) == 0)
    {
        if (!msg.isFieldSet(_011_SYSTEMS_TRACE_AUDIT_NR))
        {
            m_logger.error("Field 11 is absent");

            return false;
        }

        if (!msg.isFieldSet(_041_CARD_ACCEPTOR_TERM_ID))
        {
            m_logger.error("Field 41 is absent");
            return false;
        }

        if (!msg.isFieldSet(_042_CARD_ACCEPTOR_ID_CODE))
        {
            m_logger.error("Field 42 is absent");
            return false;
        }

        if (!msg.isFieldSet(_049_CURRENCY_CODE_TRAN))
        {
            m_logger.error("Field 49 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(_001_ACQ_NODE_KEY))
        {
            m_logger.error("Extended Field 123_001 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(_003_STORE_ID))
        {
            m_logger.error("Extended Field 123_003 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(_004_DEVICE_ID))
        {
            m_logger.error("Extended Field 123_004 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(_005_ACQ_PART_NAME))
        {
            m_logger.error("Extended Field 123_005 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(_006_TERM_BATCH_NR))
        {
            m_logger.error("Extended Field 123_006 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(_008_RETAILER_ID))
        {
            m_logger.error("Extended Field 123_008 is absent");
            return false;
        }

        if (!msg.isExtendedFieldSet(_011_ORIGINATOR_TYPE))
        {
            m_logger.error("Extended Field 123_011 is absent");
            return false;
        }
    }

    return true;
}
//////////////////////////////////////////////////////

string TMRequestHandler::processMsg(string request)
{
    Iso8583JSON msg;

    //Parser parser;
    string resp("NOK");
    string rrn("000000000000");

    try
    {
        msg.parseMsg(request);

        m_logger.information(msg.dumpMsg());

        string val = msg.getMsgType();

        long tran_nr = tmdbh.getNewTranNr(msg.getExtendedField(_001_ACQ_NODE_KEY));

        if (tran_nr == 0)
        {
            msg.setRspMsgType();

            msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);

            m_logger.critical("Could not fetch new transaction number");
            m_logger.critical("Response\n" + msg.dumpMsg());

            resp = msg.toMsg();

            return resp;
        }

        msg.setExtendedField(_007_TRAN_NR, NumberFormatter::format(tran_nr));

        if (!msg.isFieldSet(_037_RETRIEVAL_REF_NR))
        {
            rrn = NumberFormatter::format0(tran_nr, 12);
            msg.setField(_037_RETRIEVAL_REF_NR, rrn);
        }
        else
        {
            rrn = msg.getField(_037_RETRIEVAL_REF_NR);
        }

        m_logger.trace("Processing transaction with #" + rrn);

        MsgHandler mh(m_logger);

        if (isValidMsg(msg) == false)
        {
            msg.setRspMsgType();
            msg.setField(39, "30");
        }
        else
        {
            if (val == "0100")
            {
                mh.processFinMsg(msg, tmdbh);
            }
            else if (val == "0200")
            {
                mh.processFinMsg(msg, tmdbh);
            }
            else if (val == "0220")
            {
                mh.process0220Msg(msg, tmdbh);
            }
            else if (val == "0320")
            {
                mh.process0320Msg(msg, tmdbh);
            }
            else if (val == "0420")
            {
                mh.process0420Msg(msg, tmdbh);
            }
            else if (val == "0520")
            {
                mh.process0520Msg(msg, tmdbh);
            }
            else if (val == "0620")
            {
                mh.process0620Msg(msg, tmdbh);
            }
            else if (val == "0800")
            {
                mh.process0800Msg(msg, tmdbh);
            }
        }

        m_logger.information(msg.dumpMsg());

        resp = msg.toMsg();
    }
    catch (Poco::Exception &e)
    {
        m_logger.error(Poco::format("Exception in TMRequestHandler::processMsg : (%s)", e.what()));

        msg.setRspMsgType();
        msg.setField(_039_RSP_CODE, _96_SYSTEM_MALFUNCTION);

        resp = msg.toMsg();
    }

    return resp;
}

//////////////////////////////////////////////////////

class TMRequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
    TMRequestHandlerFactory(Logger &alogger) : m_logger(alogger)
    {
    }

    ~TMRequestHandlerFactory()
    {
    }

    virtual HTTPRequestHandler *createRequestHandler(
        const HTTPServerRequest &request)
    {
        if (request.getURI() == "/getstatus")
        {
            return new TMGetStatusHandler(m_logger);
        }
        else if (request.getURI() == "/updatestatus")
        {
            return new TMGetStatusHandler(m_logger);
        }
        else if (request.getURI() == "/transaction")
        {
            return new TMRequestHandler(m_logger);
        }
        else if (request.getURI() == "/reload")
        {
            return new TMReloadHandler();
        }
        else
        {
            return 0;
        }
    }

private:
    Logger &m_logger;
};

/////////////////////////////////////////////////////////////////////////////////////////

class TMServerApp : public ServerApplication
{
protected:
    void initialize(Application &self)
    {
        loadConfiguration();

        ServerApplication::initialize(self);
    }

    ////////////////////////////////////

    void uninitialize()
    {
        ServerApplication::uninitialize();
    }

    ////////////////////////////////////

    void defineOptions(OptionSet &options)
    {
        ServerApplication::defineOptions(options);

        options.addOption(
            Option("help", "h", "display argument help information")
                .required(false)
                .repeatable(false)
                .callback(OptionCallback<TMServerApp>(this, &TMServerApp::handleHelp)));

        options.addOption(
            Option("config-file", "f", "load configuration data from a file")
                .required(false)
                .repeatable(true)
                .argument("file")
                .callback(OptionCallback<TMServerApp>(this, &TMServerApp::handleConfig)));
    }

    ////////////////////////////////////

    void handleConfig(const string &name, const string &value)
    {
        loadConfiguration(value);
    }

    ////////////////////////////////////

    void handleHelp(const string &name, const string &value)
    {
        HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS");
        helpFormatter.setHeader("Transaction Manager (Prime)");
        helpFormatter.format(cout);
        stopOptionsProcessing();
        _helpRequested = true;
    }
    ////////////////////////////////////

    int main(const vector<string> &)
    {
        if (!_helpRequested)
        {
            //Config::mq = config().getString("mq_name", "NOK");
            Config::moduleName = config().getString("ModuleName", "TranMgr");
            string path = config().getString("path", "NOK");
            string rotation = config().getString("rotation", "daily");
            string archive = config().getString("archive", "timestamp");
            string tmip = config().getString("TMIP", "127.0.0.1");
            string times = config().getString("times", "local");
            string compress = config().getString("compress", "true");
            string purgeAge = config().getString("purgeAge", "2 days");
            string purgeCount = config().getString("purgeCount", "2");
            string loglevel = config().getString("loglevel", "information");

            unsigned short tmport = (unsigned short)config().getInt("TMPort", 28080);
            unsigned short maxThreads = (unsigned short)config().getInt("MaxThreads", 2);
            unsigned short maxQueued = (unsigned short)config().getInt("MaxQueued", 300);
            unsigned short maxConns = (unsigned short)config().getInt("MaxConns", 300);

            Config::dburl = config().getString("DBURL");
            Config::hsmurl = config().getString("HSMURL");
            Config::maxCashAtPOS = config().getInt("CashAtPOSLimit", 0);
            Config::isFallbackallowed = config().getBool("AllowFallback", false);
            Config::fallbackExclusionList = config().getString("FallbackExclusionList", "");
            Config::keypath = config().getString("keypath", "");

            AutoPtr<FileChannel> pChannel(new FileChannel);
            AutoPtr<PatternFormatter> pPF(new PatternFormatter);
            pPF->setProperty("pattern", "%Y-%m-%d %H:%M:%S:%i - [%p] - %s-%T-%I: %t");
            pPF->setProperty("times", times);

            pChannel->setProperty("path", path);
            pChannel->setProperty("rotation", rotation);
            pChannel->setProperty("archive", archive);
            pChannel->setProperty("times", times);

            pChannel->setProperty("compress", compress);
            pChannel->setProperty("purgeAge", purgeAge);
            pChannel->setProperty("purgeCount", purgeCount);

            AutoPtr<FormattingChannel> pFC(new FormattingChannel(pPF, pChannel));

            Logger::root().setChannel(pFC);

            Logger &logger = Logger::get(Config::moduleName);

            if (loglevel.compare("fatal") == 0)
            {
                logger.setLevel(Poco::Message::PRIO_FATAL);
            }
            else if (loglevel.compare("critical") == 0)
            {
                logger.setLevel(Poco::Message::PRIO_CRITICAL);
            }
            else if (loglevel.compare("error") == 0)
            {
                logger.setLevel(Poco::Message::PRIO_ERROR);
            }
            else if (loglevel.compare("warning") == 0)
            {
                logger.setLevel(Poco::Message::PRIO_WARNING);
            }
            else if (loglevel.compare("notice") == 0)
            {
                logger.setLevel(Poco::Message::PRIO_NOTICE);
            }
            else if (loglevel.compare("information") == 0)
            {
                logger.setLevel(Poco::Message::PRIO_INFORMATION);
            }
            else if (loglevel.compare("debug") == 0)
            {
                logger.setLevel(Poco::Message::PRIO_DEBUG);
            }
            else if (loglevel.compare("trace") == 0)
            {
                logger.setLevel(Poco::Message::PRIO_TRACE);
            }

            logger.information("TMIP = " + tmip + " / TMPort = " + NumberFormatter::format(tmport));

            DBUpdate dbupd(Config::moduleName, logger);
            Poco::Thread thread2;
            thread2.start(dbupd);

            ProcessTimeout pto(logger);
            Poco::Thread th_pto;
            th_pto.start(pto);

            UpdateTransactionsInDB updtransindb(logger);
            Poco::Thread th_upddb;
            th_upddb.start(updtransindb);

            Timespan ts2(30 * Timespan::SECONDS); // 30s
            HTTPServerParams *pParams = new HTTPServerParams;
            pParams->setKeepAlive(true);
            pParams->setKeepAliveTimeout(ts2);
            pParams->setMaxKeepAliveRequests(100);
            pParams->setMaxThreads(maxThreads);
            pParams->setMaxQueued(maxQueued);
            //pParams->setThreadIdleTime(threadIdleTime);

            ServerSocket svs(tmport, maxConns);
            //svs.setReuseAddress(true);
            ThreadPool th(2, maxThreads, 60, 0);

            HTTPServer s(new TMRequestHandlerFactory(logger), th, svs, pParams);
            s.start();

            logger.notice("Server Started");

            waitForTerminationRequest(); // wait for CTRL-C or kill

            logger.notice("Shutting down");
            s.stopAll(true); // Forcefully close all the connections
        }

        return Application::EXIT_OK;
    }

private:
    bool _helpRequested;
};

////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    TMServerApp app;
    return app.run(argc, argv);
}
