#include <queue>
#include <Poco/Logger.h>
#include <Poco/Dynamic/VarHolder.h>

#include "Encrypt.h"

using namespace std;

#ifndef CONFIG_H
#define CONFIG_H

class Config {
public:
    static string hsmurl;
    static string dburl;
    static string mq;
    static string moduleName;
    //
    static queue<string> msg;
    //
    static Encrypt e;
};

////////////////////////////////////////////////////////////////////////


#endif