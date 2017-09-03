/* 
 * File:   Track2.h
 * Author: Abhishek1.M
 *
 * Created on 14 April, 2017, 10:24 AM
 */

#ifndef TRACK2_H
#define TRACK2_H

#include <iostream>

using namespace std;

class Track2 {
private:
    string cardnumber;
    string expirydate;
    string service_rest_code;
    string track2;
public:

    Track2(string track) : track2(track) {
        int pos = 0;

        if (!track2.empty()) {
            if (track2.find("D") != std::string::npos) {
                pos = track2.find("D");
                if (pos > 1) {
                    cardnumber = track2.substr(0, pos);
                }
            } else if (track2.find("=") != std::string::npos) {
                pos = track2.find("=");
                if (pos > 1) {
                    cardnumber = track2.substr(0, pos);
                }
            }

            if (trim(cardnumber).length() > 0) {
                expirydate = track2.substr(pos + 1, 4);
                service_rest_code = track2.substr(pos + 1 + 4, 3);
            }
        }
    }

    string getCardnumber();
    string getExpirydate();
    string getService_rest_code();
    string toString();
};

///////////////////////////////////////////////////////////////////////////////

string Track2::getCardnumber() {
    return cardnumber;
}

///////////////////////////////////////////////////////////////////////////////

string Track2::getExpirydate() {
    return expirydate;
}

///////////////////////////////////////////////////////////////////////////////

string Track2::getService_rest_code() {
    return service_rest_code;
}

///////////////////////////////////////////////////////////////////////////////

string Track2::toString() {
    return track2;
}


#endif /* TRACK2_H */