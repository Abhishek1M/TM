#ifndef UPDATETRANSACTIONSINDB_H
#define UPDATETRANSACTIONSINDB_H

#include <Poco/Runnable.h>
#include <Poco/Logger.h>
#include <Iso8583JSON.h>

#include "Config.h"
#include "TranMgrDBHandler.h"

using namespace std;

class UpdateTransactionsInDB : public Poco::Runnable
{
  public:
    UpdateTransactionsInDB(Logger &alogger) : m_logger(alogger)
    {
    }

    bool processMsg(WriteToDBEntity data);
    ////////////////////////////////////////////////////////

    virtual void run()
    {
        WriteToDBEntity data;
        bool status;

        while (1)
        {
            while (!Config::commit_db.empty())
            {
                try
                {
                    data = Config::commit_db.front();

                    status = processMsg(data);

                    if (status == true)
                    {
                        Config::commit_db.pop();
                    }
                }
                catch (exception &e)
                {
                    m_logger.error("Error processing transaction #ProcessTimeout");
                    m_logger.error(e.what());

                    m_logger.error("Aborting transaction #");
                    Config::commit_db.pop();
                }
            }

            sleep(1);
        }
    }

  private:
    Logger &m_logger;
};
///////////////////////////////////////////////////////////////////
bool UpdateTransactionsInDB::processMsg(WriteToDBEntity data)
{
    int action;
    string s_msg;
    Iso8583JSON msg;
    string req_in;
    bool status;

    TranMgrDBHandler tmdbh(m_logger);

    action = data.getAction();
    s_msg = data.getMsg();

    msg.parseMsg(s_msg);

    switch (action)
    {
    case Config::_INSERT_TRANS:
        status = tmdbh.addToTrans(msg, data.getReq_in());
        break;

    case Config::_INSERT_TRANS_DECLINED:
        status = tmdbh.addTMDeclineToTrans(msg);
        break;

    case Config::_UPDATE_ADJUSTED:
        status = tmdbh.updateadjustment(msg.getExtendedField(_009_PREV_TRAN_NR));
        break;

    case Config::_UPDATE_REVERSAL:
        status = tmdbh.updatereversal(msg.getExtendedField(_009_PREV_TRAN_NR));
        break;

    case Config::_UPDATE_TRANS:
        status = tmdbh.updateTrans(msg);
        break;

    default:
        status = false;
    }

    return status;
}

#endif