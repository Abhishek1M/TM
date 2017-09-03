/*
 * TranMgr.cpp
 *
 *  Created on: 14-Oct-2016
 *      Author: abhishek
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

#include <iostream>
#include <string>
#include <vector>

#include <pqxx/pqxx>

#include <ap/Iso8583JSON.h>
#include "TranMgrDBHandler.h"
#include <ap/constants.h>
#include "MsgHandler.h"
#include "Config.h"
#include "ProcessTimeout.h"

using namespace Poco::Net;
using namespace Poco::Util;
using Poco::Net::NameValueCollection;
using namespace std;
using Poco::CountingInputStream;
using Poco::NullOutputStream;
using Poco::StreamCopier;

using Poco::ThreadPool;

using Poco::Util::Option;
using Poco::Util::OptionSet;
using Poco::Util::OptionCallback;
using Poco::Util::HelpFormatter;
using Poco::Logger;
using Poco::FileChannel;
using Poco::AutoPtr;
using Poco::FormattingChannel;
using Poco::PatternFormatter;

using Poco::NumberFormatter;
using Poco::NumberParser;

using namespace Poco::JSON;
using namespace Poco::Dynamic;

////////////////////////////////////////////////////////////////////////////////

string Config::dburl;
string Config::hsmurl;
string Config::mq;
string Config::moduleName;
//
queue<string> Config::msg;
//
Encrypt Config::e;
///////////////////////////////////////////////////////////////////////////////

class DBUpdate : public Poco::Runnable {
public:

    DBUpdate(string modulename, Logger& alogger) :
    _modulename(modulename), m_logger(alogger) {
    }

    virtual void run() {
        try {
            DBManager dbm;

            //pqxx::connection c(dbm.getConnectionURL());
            pqxx::connection c(Config::dburl);

            string query =
                    "UPDATE onl_process set last_update_datetime=now() where pname='"
                    + _modulename + "';";

            while (1) {
                pqxx::work txn(c);
                txn.exec(query);
                txn.commit();

                sleep(30);
            }
        } catch (exception &e) {
            m_logger.error(e.what());
        }
    }

private:
    string _modulename;
    Logger& m_logger;
};

////////////////////////////////////////////////////////////////////////////////

class TMGetStatusHandler : public HTTPRequestHandler {
public:

    TMGetStatusHandler(Logger& alogger) :
    m_logger(alogger) {
    }

    ~TMGetStatusHandler() {
    }

    virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp) {
        string responseStr = "OK";

        resp.setStatus(HTTPResponse::HTTP_OK);
        resp.setContentType("application/json; charset=UTF-8");

        ostream& out = resp.send();
        out << responseStr;

        out.flush();

        m_logger.notice("Responded with OK");
    }

private:
    Logger& m_logger;
};

////////////////////////////////////////////////////////////////////////////////

class TMRequestHandler : public HTTPRequestHandler {
public:

    TMRequestHandler(Logger& logger) : m_logger(logger), tmdbh(logger) {
    }

    ~TMRequestHandler() {
    }

    virtual void handleRequest(HTTPServerRequest &req,
            HTTPServerResponse &resp) {
        string responseStr;
        string requestStr;
        istream& istr = req.stream();

        StreamCopier::copyToString(istr, requestStr);

        try {
            responseStr = processMsg(requestStr);

            resp.setStatus(HTTPResponse::HTTP_OK);
            resp.setContentType("application/json");

            ostream& out = resp.send();
            out << responseStr;

            out.flush();
        } catch (Poco::Exception &e) {
            m_logger.error(Poco::format("Exception in TMRequestHandler::handleRequest : (%s)", e.what()));
        }
    }

private:
    Logger& m_logger;
    TranMgrDBHandler tmdbh;

    string processMsg(string request);

    bool isValidMsg(Iso8583JSON& msg);
};

//////////////////////////////////////////////////////

bool TMRequestHandler::isValidMsg(Iso8583JSON& msg) {
    string msgtype = msg.getMsgType();

    if ((msgtype.compare("0100") == 0) || (msgtype.compare("0200") == 0)) {
        // Check Field 002
        if (!msg.isFieldSet(2)) {
            msg.setField(44, "002");
            return false;
        }

        // Check Field 003
        if (!msg.isFieldSet(3)) {
            msg.setField(44, "003");
            return false;
        }

        // Check Field 004
        if (!msg.isFieldSet(4)) {
            msg.setField(44, "004");
            return false;
        }

        // Check Field 007
        if (!msg.isFieldSet(7)) {
            msg.setField(44, "007");
            return false;
        }

        // Check Field 011
        if (!msg.isFieldSet(11)) {
            msg.setField(44, "011");
            return false;
        }

        // Check Field 012
        if (!msg.isFieldSet(12)) {
            msg.setField(44, "012");
            return false;
        }

        // Check Field 013
        if (!msg.isFieldSet(13)) {
            msg.setField(44, "013");
            return false;
        }

        // Check Field 022
        if (!msg.isFieldSet(22)) {
            msg.setField(44, "022");
            return false;
        }

        // Check Field 035
        //        if (!msg.isFieldSet(35)) {
        //            msg.setField(44, "035");
        //            return false;
        //        }

        // Check Field 041
        if (!msg.isFieldSet(41)) {
            msg.setField(44, "041");
            return false;
        }

        // Check Field 042
        if (!msg.isFieldSet(42)) {
            msg.setField(44, "042");
            return false;
        }
    }

    return true;
}
//////////////////////////////////////////////////////

string TMRequestHandler::processMsg(string request) {
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

class TMRequestHandlerFactory : public HTTPRequestHandlerFactory {
public:

    TMRequestHandlerFactory(Logger& alogger) :
    m_logger(alogger) {
    }

    ~TMRequestHandlerFactory() {
    }

    virtual HTTPRequestHandler* createRequestHandler(
            const HTTPServerRequest& request) {
        if (request.getURI() == "/getstatus") {
            return new TMGetStatusHandler(m_logger);
        } else if (request.getURI() == "/updatestatus") {
            return new TMGetStatusHandler(m_logger);
        } else if (request.getURI() == "/transaction") {
            return new TMRequestHandler(m_logger);
        } else {
            return 0;
        }
    }

private:
    Logger& m_logger;
};

/////////////////////////////////////////////////////////////////////////////////////////

class TMServerApp : public ServerApplication {
protected:

    void initialize(Application& self) {
        loadConfiguration();

        ServerApplication::initialize(self);
    }

    ////////////////////////////////////

    void uninitialize() {
        ServerApplication::uninitialize();
    }

    ////////////////////////////////////

    void defineOptions(OptionSet& options) {
        ServerApplication::defineOptions(options);

        options.addOption(
                Option("help", "h", "display argument help information").required(
                false).repeatable(false).callback(
                OptionCallback<TMServerApp>(this,
                &TMServerApp::handleHelp)));
    }

    ////////////////////////////////////

    void handleHelp(const string& name, const string& value) {
        HelpFormatter helpFormatter(options());
        helpFormatter.setCommand(commandName());
        helpFormatter.setUsage("OPTIONS");
        helpFormatter.setHeader("Transaction Manager (Prime)");
        helpFormatter.format(cout);
        stopOptionsProcessing();
        _helpRequested = true;
    }

    ////////////////////////////////////

    int main(const vector<string> &) {
        if (!_helpRequested) {
            Config::mq = config().getString("mq_name", "NOK");
            Config::moduleName = config().getString("ModuleName", "NOK");
            string path = config().getString("path", "NOK");
            string rotation = config().getString("rotation", "NOK");
            string archive = config().getString("archive", "NOK");
            string tmip = config().getString("TMIP", "127.0.0.1");
            string times = config().getString("times", "local");
            string compress = config().getString("compress", "true");
            string purgeAge = config().getString("purgeAge", "2 days");
            string purgeCount = config().getString("purgeCount", "2");
            string loglevel = config().getString("loglevel", "information");

            unsigned short tmport = (unsigned short) config().getInt("TMPort",
                    28080);
            unsigned short maxThreads = (unsigned short) config().getInt(
                    "MaxThreads", 100);
            unsigned short maxQueued = (unsigned short) config().getInt(
                    "MaxQueued", 100);
            unsigned short maxConns = (unsigned short) config().getInt(
                    "MaxConns", 100);

            Config::dburl = config().getString("DBURL");
            Config::hsmurl = config().getString("HSMURL");

            AutoPtr<FileChannel> pChannel(new FileChannel);
            AutoPtr<PatternFormatter> pPF(new PatternFormatter);
            pPF->setProperty("pattern", "%Y-%m-%d %H:%M:%S %s: %t");
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

            Logger& logger = Logger::get(Config::moduleName);

            if (loglevel.compare("fatal") == 0) {
                logger.setLevel(Poco::Message::PRIO_FATAL);
            } else if (loglevel.compare("critical") == 0) {
                logger.setLevel(Poco::Message::PRIO_CRITICAL);
            } else if (loglevel.compare("error") == 0) {
                logger.setLevel(Poco::Message::PRIO_ERROR);
            } else if (loglevel.compare("warning") == 0) {
                logger.setLevel(Poco::Message::PRIO_WARNING);
            } else if (loglevel.compare("notice") == 0) {
                logger.setLevel(Poco::Message::PRIO_NOTICE);
            } else if (loglevel.compare("information") == 0) {
                logger.setLevel(Poco::Message::PRIO_INFORMATION);
            } else if (loglevel.compare("debug") == 0) {
                logger.setLevel(Poco::Message::PRIO_DEBUG);
            } else if (loglevel.compare("trace") == 0) {
                logger.setLevel(Poco::Message::PRIO_TRACE);
            }

            logger.information("TMIP = " + tmip + " / TMPort = " + NumberFormatter::format(tmport));

            DBUpdate dbupd(Config::moduleName, logger);
            Poco::Thread thread2;
            thread2.start(dbupd);

            ProcessTimeout pto(logger);
            Poco::Thread th_pto;
            th_pto.start(pto);

            HTTPServerParams* pParams = new HTTPServerParams;
            pParams->setKeepAlive(false);
            //pParams->setMaxThreads(maxThreads);
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

int main(int argc, char** argv) {
    TMServerApp app;
    return app.run(argc, argv);
}
