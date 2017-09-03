/* 
 * File:   DBManager.h
 * Author: Abhishek1.M
 *
 * Created on 15 January, 2017, 2:25 PM
 */

#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <string>

class DBManager {
public:

    DBManager() {

    }

    ~DBManager() {

    }

    string getConnectionURL();
private:

};
///////////////////////////////////////////////////////////////////////////////

string DBManager::getConnectionURL() {
    string c("dbname=etpsonline user=etpsappuser");

    return c;
}


#endif /* DBMANAGER_H */

