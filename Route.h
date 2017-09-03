/* 
 * File:   Route.h
 * Author: Abhishek1.M
 *
 * Created on 15 January, 2017, 9:26 PM
 */

#ifndef ROUTE_H
#define ROUTE_H

#include <iostream>

using namespace std;

class Route {
public:

    Route() {
        pname = "";
        url = "";
        timeout = 20000;
    }

    ~Route() {
    }

    //
    string getPname();
    string getUrl();
    long getTimeout();
    string getisskeyname();
    //
    void setPname(string z);
    void setUrl(string z);
    void setTimeout(long z);
    void setisskeyname(string z);

private:
    string pname;
    string url;
    long timeout;
    string isskeyname;
};
///////////////////////////////////////////////////////////////////////////////

string Route::getPname() {
    return pname;
}
///////////////////////////////////////////////////////////////////////////////

string Route::getUrl() {
    return url;
}
///////////////////////////////////////////////////////////////////////////////

long Route::getTimeout() {
    return timeout;
}
///////////////////////////////////////////////////////////////////////////////

void Route::setPname(string z) {
    pname = z;
}
///////////////////////////////////////////////////////////////////////////////

void Route::setUrl(string z) {
    url = z;
}
///////////////////////////////////////////////////////////////////////////////

void Route::setTimeout(long z) {
    timeout = z;
}
///////////////////////////////////////////////////////////////////////////////

void Route::setisskeyname(string z) {
    isskeyname = z;
}
///////////////////////////////////////////////////////////////////////////////

string Route::getisskeyname() {
    return isskeyname;
}
#endif /* ROUTE_H */