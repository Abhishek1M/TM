/* 
 * File:   MapKeyValue.h
 * Author: abhi
 *
 * Created on May 18, 2018, 4:46 PM
 */

#ifndef MAPKEYVALUE_H
#define MAPKEYVALUE_H

#include <Poco/HashMap.h>
#include <Poco/Exception.h>
#include <map>
#include <string>
#include <Poco/NumberParser.h>
#include <Poco/NumberFormatter.h>
#include <Poco/Util/Application.h>

using namespace std;
using namespace Poco::Util;

class MapKeyValue
{
public:
    MapKeyValue()
    {
    }

    MapKeyValue(string data)
    {
        if (!data.empty())
        {
            fromMsg(data);
        }
    }

    void clear();
    void put(string key, string value);
    string get(string key);
    string toMsg();
    void fromMsg(string data);
    bool isKeyPresent(string key);

private:
    map<string, string> hm_input;
};

/////////////////////////////////////////////////////////////////

void MapKeyValue::clear()
{
    hm_input.clear();
}

/////////////////////////////////////////////////////////////////
void MapKeyValue::put(string key, string value)
{
    hm_input.insert(pair<string, string>(key, value));
}

/////////////////////////////////////////////////////////////////

string MapKeyValue::get(string key)
{
    return hm_input.find(key)->second;
}

/////////////////////////////////////////////////////////////////
string MapKeyValue::toMsg()
{
    map<string, string>::iterator itr;
    string sb;

    for (itr = hm_input.begin(); itr != hm_input.end(); ++itr)
    {
        sb = sb + NumberFormatter::format0(itr->first.length(), 3) + itr->first +
             NumberFormatter::format0(itr->second.length(), 5) + itr->second;
    }

    return sb;
}

/////////////////////////////////////////////////////////////////

void MapKeyValue::fromMsg(string data)
{
    int key_len;
    int value_len;
    string key;
    string value;

    int startpos = 0;

    Application &app = Application::instance();
    Logger &lgr = app.logger().get(Config::moduleName);

    try
    {
        while (startpos < data.length())
        {
            key_len = NumberParser::parse(data.substr(startpos, 3));
            startpos = startpos + 3;
            key = data.substr(startpos, key_len);

            startpos = startpos + key_len;
            value_len = NumberParser::parse(data.substr(startpos, 5));

            startpos = startpos + 5;
            value = data.substr(startpos, value_len);
            startpos = startpos + value_len;

            put(key, value);
        }
    }
    catch (Poco::Exception ex)
    {
        lgr.error("Error in parsing string - ");
        lgr.error(data);
        lgr.error(ex.what());
    }
}

/////////////////////////////////////////////////////////////////

bool MapKeyValue::isKeyPresent(string key)
{
    if (hm_input.count(key) > 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

#endif /* MAPKEYVALUE_H */
