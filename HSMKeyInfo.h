/* 
 * File:   HSMKeyInfo.h
 * Author: Abhishek M
 *
 * Created on 12 February, 2017, 10:02 AM
 */

#ifndef HSMKEYINFO_H
#define HSMKEYINFO_H

#include<iostream>

using namespace std;

class HSMKeyInfo {
public:
    // getters
    string get_keyname();
    string get_keytype();
    int get_keylen();
    string get_keyscheme();
    string get_keyvalueunderlmk();
    string get_keyvalueunderparent();
    string get_keycheckvalue();
    string get_parentkeyname();
    bool get_found();

    // setters
    void set_keyname(string value);
    void set_keytype(string value);
    void set_keylen(int val);
    void set_keyscheme(string value);
    void set_keyvalueunderlmk(string value);
    void set_keyvalueunderparent(string value);
    void set_keycheckvalue(string value);
    void set_parentkeyname(string value);
    void set_found(bool found);
private:
    string keyname;
    string keytype;
    int keylen;
    string keyscheme;
    string keyvalueunderlmk;
    string keyvalueunderparent;
    string keycheckvalue;
    string parentkeyname;
    bool found;
};
//////////////////////////////////////////////////////////////////////

string HSMKeyInfo::get_keyname() {
    return keyname;
}
//////////////////////////////////////////////////////////////////////

string HSMKeyInfo::get_keytype() {
    return keytype;
}

//////////////////////////////////////////////////////////////////////

int HSMKeyInfo::get_keylen() {
    return keylen;
}

//////////////////////////////////////////////////////////////////////

string HSMKeyInfo::get_keyscheme() {
    return keyscheme;
}

//////////////////////////////////////////////////////////////////////

string HSMKeyInfo::get_keyvalueunderlmk() {
    return keyvalueunderlmk;
}

//////////////////////////////////////////////////////////////////////

string HSMKeyInfo::get_keyvalueunderparent() {
    return keyvalueunderparent;
}

//////////////////////////////////////////////////////////////////////

string HSMKeyInfo::get_keycheckvalue() {
    return keycheckvalue;
}

//////////////////////////////////////////////////////////////////////

string HSMKeyInfo::get_parentkeyname() {
    return parentkeyname;
}

//////////////////////////////////////////////////////////////////////

bool HSMKeyInfo::get_found() {
    return found;
}

//////////////////////////////////////////////////////////////////////

void HSMKeyInfo::set_keyname(string value) {
    keyname = value;
}

//////////////////////////////////////////////////////////////////////

void HSMKeyInfo::set_keytype(string value) {
    keytype = value;
}

//////////////////////////////////////////////////////////////////////

void HSMKeyInfo::set_keylen(int val) {
    keylen = val;
}

//////////////////////////////////////////////////////////////////////

void HSMKeyInfo::set_keyscheme(string value) {
    keyscheme = value;
}

//////////////////////////////////////////////////////////////////////

void HSMKeyInfo::set_keyvalueunderlmk(string value) {
    keyvalueunderlmk = value;
}

//////////////////////////////////////////////////////////////////////

void HSMKeyInfo::set_keyvalueunderparent(string value) {
    keyvalueunderparent = value;
}

//////////////////////////////////////////////////////////////////////

void HSMKeyInfo::set_keycheckvalue(string value) {
    keycheckvalue = value;
}

//////////////////////////////////////////////////////////////////////

void HSMKeyInfo::set_parentkeyname(string value) {
    parentkeyname = value;
}

//////////////////////////////////////////////////////////////////////

void HSMKeyInfo::set_found(bool f) {
    found = f;
}


#endif /* HSMKEYINFO_H */