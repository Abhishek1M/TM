/* 
 * File:   Encrypt.h
 * Author: Abhishek1.M
 *
 * Created on 20 January, 2017, 1:01 PM
 */

#ifndef ENCRYPT_H
#define ENCRYPT_H

#include <Poco/Crypto/RSAKey.h>
#include <Poco/Crypto/CipherFactory.h>
#include <Poco/Crypto/Cipher.h>
#include <Poco/Crypto/CipherKey.h>
#include <Poco/Util/AbstractConfiguration.h>
#include <Poco/Util/XMLConfiguration.h>
#include <Poco/Logger.h>

using namespace Poco::Crypto;
using namespace Poco::Util;

class Encrypt {
public:

    Encrypt() {
        //Logger &lgr = Logger::get("TranMgr");
        try {
            AbstractConfiguration *cfg = new XMLConfiguration("/usr/share/antpay/tranmgr/conf.xml");

            //key = "FC6178E856B4F9A89A85B073A40100CE036DB5369CF8E2A16D9844CD5A2D9EC6";
            //iv = "8B27F80EC817DA209C7FDCD6B04113AF";

            key = cfg->getString("key");
            iv = cfg->getString("iv");

            CipherFactory& factory = CipherFactory::defaultFactory();

            Cipher::ByteVec bkey;
            Cipher::ByteVec biv;

            bkey.assign(key.begin(), key.end());
            biv.assign(iv.begin(), iv.end());

            pCipher = factory.createCipher(Poco::Crypto::CipherKey("aes256", bkey, biv));
        } catch (exception &e) {
            //lgr.error("Error in Encrypt");
            //lgr.error(e.what());
        }
    }

    /////////////////////////////////////////////////////
    string encryptdata(string data);
    string decryptdata(string edata);

private:
    Cipher* pCipher;
    string key;
    string iv;
};
///////////////////////////////////////////////////////////////////////////////

string Encrypt::encryptdata(string data) {
    string edata = pCipher->encryptString(data, Cipher::ENC_BASE64_NO_LF);

    return edata;
}
///////////////////////////////////////////////////////////////////////////////

string Encrypt::decryptdata(string edata) {
    string data = pCipher->decryptString(edata, Cipher::ENC_BASE64_NO_LF);

    return data;
}

#endif /* ENCRYPT_H */