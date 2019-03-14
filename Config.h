#include <queue>
#include <map>
#include <Poco/Logger.h>
#include <Poco/Dynamic/VarHolder.h>
#include <Poco/Exception.h>

#include <Encrypt.h>
#include <pqxx/field.hxx>
#include <pqxx/connection_base.hxx>

#include "WriteToDBEntity.h"

using namespace std;

#ifndef CONFIG_H
#define CONFIG_H

class Config
{
  public:
    static string hsmurl;
    static string dburl;
    static string mq;
    static string moduleName;
    static bool isFallbackallowed;
    static string fallbackExclusionList;
    static int maxCashAtPOS;
    static string keypath;
    //
    static queue<string> msg;
    static queue<WriteToDBEntity> commit_db;
    //
    //
    static map<string, int> timeOffset;
    static Encrypt e;
    // constants
    static const int _INSERT_TRANS = 0;
    static const int _UPDATE_TRANS = 1;
    static const int _INSERT_TRANS_DECLINED = 2;
    static const int _UPDATE_REVERSAL = 3;
    static const int _UPDATE_ADJUSTED = 4;
    // methods
    static bool loadTimeOffset();
};

////////////////////////////////////////////////////////////////////////

bool Config::loadTimeOffset()
{
    pqxx::connection c(Config::dburl);
    string query = "select code_alpha_2, timeoffset from countries ";

    Application &app = Application::instance();
    Logger &lgr = app.logger().get(moduleName);

    try
    {
        if (!c.is_open())
        {
            lgr.fatal("loadTimeOffset - Could not connect to database");

            return false;
        }

        // Create a non-transactional object
        pqxx::work txn(c);

        // Execute query
        pqxx::result r = txn.exec(query);

        if (r.size() == 0)
        {
            c.disconnect();
        }
        else
        {
            timeOffset.clear();

            for (pqxx::result::const_iterator row = r.begin(); row != r.end(); ++row)
            {
                // Fields within a row can be accessed by column name.
                // You can also iterate the fields in a row, or index the row
                // by column number just like an array.
                // Values are stored internally as plain strings.  You access
                // them by converting each to the desired C++ type using the
                // "as()" function template.
                timeOffset.insert(pair<string, int>(row["code_alpha_2"].as<std::string>(), row["timeoffset"].as<int>()));
            }
        }

        c.disconnect();
    }
    catch (Poco::Exception ex)
    {
        lgr.error(ex.message());

        return false;
    }

    return true;
}

#endif