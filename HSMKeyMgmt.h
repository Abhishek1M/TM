/* 
 * File:   HSMKeyMgmt.h
 * Author: Abhishek1.M
 *
 * Created on 16 March, 2017, 12:55 PM
 */

#ifndef HSMKEYMGMT_H
#define HSMKEYMGMT_H

#include "HSMKeyInfo.h"
#include "HSMMsg.h"
#include "Config.h"

class HSMKeyMgmt {
private:
    string dburl;
public:

    HSMKeyMgmt(string db_url) : dburl(db_url) {

    }

    HSMKeyInfo getKeyDetails(string keyname);
    bool createKey(HSMKeyInfo hki);
    bool updateKey(HSMKeyInfo hki);
    string translateBDKtoZPKTDES(string srckeyname, string destkeyname, string pinblock, string cardnumber, string ksn_pb);
    bool translateZPKFromZMKToLMK(string zpk_key_name, string zmk_key_name, string encrypted_key, string remote_kcv);
    string translateZPKtoZPK(string srckeyname, string destkeyname, string pinblock, string cardnumber);
    string translateTPKtoZPK(string srckeyname, string destkeyname, string pinblock, string cardnumber);
};

/////////////////////////////////////////////////////////////////////////

HSMKeyInfo HSMKeyMgmt::getKeyDetails(string keyname) {
    HSMKeyInfo hki;
    string query = "select * from fetchhsmkey('" + keyname + "');";

    // Create a connection
    pqxx::connection c(dburl);

    // Create a non-transactional object
    pqxx::work txn(c);

    // Execute query
    pqxx::result r = txn.exec(query);

    if (r.size() != 1) {
        hki.set_found(false);
    } else {
        hki.set_keyname(r[0][1].as<string>());
        hki.set_keytype(r[0][2].as<string>());
        hki.set_keylen(r[0][3].as<int>());
        hki.set_keyscheme(r[0][4].as<string>());
        hki.set_keyvalueunderlmk(r[0][5].as<string>());
        hki.set_keyvalueunderparent(r[0][6].as<string>());
        hki.set_keycheckvalue(r[0][7].as<string>());
        hki.set_parentkeyname(r[0][8].as<string>());
        hki.set_found(true);
    }

    c.disconnect();

    return hki;
}
/////////////////////////////////////////////////////////////////////////

bool HSMKeyMgmt::createKey(HSMKeyInfo hki) {
    pqxx::connection c(dburl);
    pqxx::work txn(c);

    string sqlQuery = "select AddHSMKey('"; //?, ?, ?, ?, ?, ?, ?, ?)}";

    sqlQuery.append(hki.get_keyname()).append("','");
    sqlQuery.append(hki.get_keytype()).append("',");
    sqlQuery.append(NumberFormatter::format(hki.get_keylen())).append(",'");
    sqlQuery.append(hki.get_keyscheme()).append("','");
    sqlQuery.append(hki.get_keyvalueunderlmk()).append("','");
    sqlQuery.append(hki.get_keyvalueunderparent()).append("','");
    sqlQuery.append(hki.get_keycheckvalue()).append("','");
    sqlQuery.append(hki.get_parentkeyname()).append("');");

    txn.exec(sqlQuery);
    txn.commit();

    c.disconnect();

    return true;
}
/////////////////////////////////////////////////////////////////////////

bool HSMKeyMgmt::updateKey(HSMKeyInfo hki) {
    pqxx::connection c(dburl);
    pqxx::work txn(c);

    string sqlQuery = "select UpdateHSMKey('";

    sqlQuery.append(hki.get_keyname()).append("','");
    sqlQuery.append(hki.get_keytype()).append("',");
    sqlQuery.append(NumberFormatter::format(hki.get_keylen())).append(",'");
    sqlQuery.append(hki.get_keyscheme()).append("','");
    sqlQuery.append(hki.get_keyvalueunderlmk()).append("','");
    sqlQuery.append(hki.get_keyvalueunderparent()).append("','");
    sqlQuery.append(hki.get_keycheckvalue()).append("','");
    sqlQuery.append(hki.get_parentkeyname()).append("');");

    txn.exec(sqlQuery);
    txn.commit();

    c.disconnect();

    return true;
}
///////////////////////////////////////////////////////////////////////////////

string HSMKeyMgmt::translateBDKtoZPKTDES(string srckeyname, string destkeyname,
        string pinblock, string cardnumber, string ksn_pb) {
    string newpinblock;
    int cardlen = cardnumber.length();
    int startpos = cardlen - 1 - 12;
    string accountnumber = cardnumber.substr(startpos, 12);

    HSMKeyInfo srchki = getKeyDetails(srckeyname);
    HSMKeyInfo desthki = getKeyDetails(destkeyname);

    if (srchki.get_found() == false) {
        return "NOK";
    } else if (desthki.get_found() == false) {
        return "NOK";
    }

    HSMMsg hsmrequest;
    hsmrequest.setField(_CMD, _G0_XLATE_BDK_ZPK_TDES);
    hsmrequest.setField(_BDK, srchki.get_keyvalueunderlmk());
    hsmrequest.setField(_DEST_ZPK, desthki.get_keyvalueunderlmk());
    hsmrequest.setField(_KSN_DESCRIPTOR, "000");
    hsmrequest.setField(_KSN, ksn_pb);
    hsmrequest.setField(_SOURCE_PIN_BLK, pinblock);
    hsmrequest.setField(_SOURCE_PIN_BLK_FORMAT, "01");
    hsmrequest.setField(_DEST_PIN_BLK_FORMAT, "01");
    hsmrequest.setField(_ACCOUNT_NR, accountnumber);

    string resp = Utility::ofPostRequest(Config::hsmurl, hsmrequest.toMsg(), 10);

    HSMMsg hsmresp;
    hsmresp.parseMsg(resp);

    string ec = hsmresp.getField(_ERROR_CODE);

    if (ec.compare("00") == 0) {
        newpinblock = hsmresp.getField(_DEST_PIN_BLK);
    } else {
        newpinblock = "NOK";
    }

    return newpinblock;
}

///////////////////////////////////////////////////////////////////////////////
/*
bool HSMKeyMgmt::translateZPKFromZMKToLMK(string zpk_key_name, string zmk_key_name,
        string encrypted_key, string remote_kcv) {
    string zpk_under_lmk, kcv;
    boolean check;

    if (!encrypted_key.empty()) {
        encrypted_key = "U" + encrypted_key;
    }

    HSMKeyInfo srczpk = getKeyDetails(zpk_key_name);
    HSMKeyInfo srczmk = getKeyDetails(zmk_key_name);
    HSMMsg hsmrequest;
    hsmrequest.setField(_CMD, _FA_XLATE_ZPK_ZMK_ZMK);
    hsmrequest.setField(_ZMK, srczmk.keyvalueunderlmk);
    hsmrequest.setField(_SOURCE_ZPK, encrypted_key);

    string resp = Utility::ofPostRequest(Config::get_hsmurl(), hsmrequest.getMsgToHSM(4), 10);

    if (resp.empty()) {
        return false;
    }

    HSMMsg hsmresp;
    hsmresp.parseMsg(resp);

    if (hsmresp.getField(_ERROR_CODE).equals("00")) {
        zpk_under_lmk = hsmresp.getField(_NEW_KEY_UNDER_CURR_KEY);
        kcv = hsmresp.getField(_KEY_CHECK_VALUE);

        srczpk.setKeyvalueunderlmk(zpk_under_lmk);
        srczpk.setKeycheckvalue(kcv);

        check = updateKey(srczpk);

        if (check) {
            check = kcv.startsWith(remote_kcv);
        }

        return check;
    }
    return false;
}
 */
///////////////////////////////////////////////////////////////////////////////
/*
string HSMKeyMgmt::translateTPKtoZPK(string srckeyname, string destkeyname,
        string pinblock, string cardnumber) {
    string newpinblock;
    string accountnumber;
    int cardlen = cardnumber.trim().length();
    int startpos;

    if (cardlen > 12) {
        startpos = cardlen - 1 - 12;
        accountnumber = cardnumber.substring(startpos, 12);
    } else {
        startpos = 0;
        accountnumber = "0" + cardnumber.substring(startpos, 12);
        //accountnumber = cardnumber.substring(startpos, cardlen);
    }

    HSMKeyInfo srchki = getKeyDetails(srckeyname);
    HSMKeyInfo desthki = getKeyDetails(destkeyname);

    if (srchki.get_found() == false) {
        return "NOK";
    }

    if (desthki.get_found() == false) {
        return "NOK";
    }

    HSMMsg hsmrequest;
    hsmrequest.setField(HSMJSON.Fields._CMD,
            HSMJSON.HSMCommands._CA_XLATE_TPK_ZPK);
    hsmrequest.setField(HSMJSON.Fields._SOURCE_TPK, srchki.keyvalueunderlmk);
    hsmrequest.setField(HSMJSON.Fields._DEST_ZPK, desthki.keyvalueunderlmk);
    hsmrequest.setField(HSMJSON.Fields._MAX_PIN_LEN, "12");
    hsmrequest.setField(HSMJSON.Fields._SOURCE_PIN_BLK, pinblock);
    hsmrequest.setField(HSMJSON.Fields._SOURCE_PIN_BLK_FORMAT, "01");
    hsmrequest.setField(HSMJSON.Fields._DEST_PIN_BLK_FORMAT, "01");
    hsmrequest.setField(HSMJSON.Fields._ACCOUNT_NR, accountnumber);

    string resp = Utility::ofPostRequest(Config::get_hsmurl(), hsmrequest.getMsgToHSM(4), 10);

    if (resp.empty()) {
        return false;
    }

    HSMMsg hsmresp;
    hsmresp.parseMsg(resp);

    if (hsmresp.getField(HSMJSON.Fields._ERROR_CODE).equals("00")) {
        newpinblock = hsmresp.getField(HSMJSON.Fields._DEST_PIN_BLK);
    } else {
        return null;
    }

    return newpinblock;
}
 */
///////////////////////////////////////////////////////////////////////////////

string HSMKeyMgmt::translateZPKtoZPK(string srckeyname, string destkeyname,
        string pinblock, string cardnumber) {
    string newpinblock;
    int cardlen = cardnumber.length();
    int startpos = cardlen - 1 - 12;
    string accountnumber = cardnumber.substr(startpos, 12);

    HSMKeyInfo srchki = getKeyDetails(srckeyname);
    HSMKeyInfo desthki = getKeyDetails(destkeyname);

    if (srchki.get_found() == false) {
        return "NOK";
    }

    if (desthki.get_found() == false) {
        return "NOK";
    }

    HSMMsg hsmrequest;

    hsmrequest.setField(_CMD, _CC_XLATE_ZPK_ZPK);
    hsmrequest.setField(_SOURCE_ZPK, srchki.get_keyvalueunderlmk());
    hsmrequest.setField(_DEST_ZPK, desthki.get_keyvalueunderlmk());
    hsmrequest.setField(_MAX_PIN_LEN, "12");
    hsmrequest.setField(_SOURCE_PIN_BLK, pinblock);
    hsmrequest.setField(_SOURCE_PIN_BLK_FORMAT, "01");
    hsmrequest.setField(_DEST_PIN_BLK_FORMAT, "01");
    hsmrequest.setField(_ACCOUNT_NR, accountnumber);


    string resp = Utility::ofPostRequest(Config::hsmurl, hsmrequest.toMsg(), 10);

    if (resp.empty()) {
        return false;
    }

    HSMMsg hsmresp;
    hsmresp.parseMsg(resp);

    string ec = hsmresp.getField(_ERROR_CODE);

    if (ec.compare("00") == 0) {
        newpinblock = hsmresp.getField(_DEST_PIN_BLK);
    } else {
        newpinblock = "NOK";
    }

    return newpinblock;
}

#endif /* HSMKEYMGMT_H */

