/* 
 * File:   HSMMsg.h
 * Author: Abhishek M
 *
 * Created on 8 February, 2017, 5:35 PM
 */

#ifndef HSMMSG_H
#define HSMMSG_H

#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/NumberFormatter.h>
#include <Poco/NumberParser.h>

#include <iostream>
#include <string>

#include <constants.h>
#include <Utility.h>

using namespace Poco::JSON;
using namespace Poco::Dynamic;

using Poco::NumberFormatter;
using Poco::NumberParser;

using namespace std;

class HSMMsg {
private:
    Poco::DynamicStruct ds_msg;
    Object::Ptr object;
    //
public:
    //string getHCRequest(int msghdrlen);
    //string getCARequest(int msghdrlen);
    //string getCCRequest(int msghdrlen);
    string getG0Request(int msghdrlen);
    void parseG0G1Rsp(string data, int beginIndex, int endIndex);
    void setField(string fieldname, string value);
    string getField(string fieldname);
    void removeField(string fieldname);
    void parseRspIntoFields(string data, int msghdrlen);
    void parseMsg(string data);
    string getMsgToHSM(int msghdrlen);
    string toMsg();
};

///////////////////////////////////////////////////////////////////////////////////////

string HSMMsg::toMsg() {
    return ds_msg.toString();
}

///////////////////////////////////////////////////////////////////////////////////////

string HSMMsg::getMsgToHSM(int msghdrlen) {
    string msg;
    string cmd;

    cmd = getField(_CMD);

    if (cmd.compare(_G0_XLATE_BDK_ZPK_TDES) == 0) {
        msg = getG0Request(msghdrlen);
    } else {
        msg = "";
    }

    return msg;
}
///////////////////////////////////////////////////////////////////////////////////////

void HSMMsg::parseMsg(string msg) {
    Parser parser;
    Var result = parser.parse(msg);

    // use pointers to avoid copying
    object = result.extract<Object::Ptr>();

    // copy/convert to Poco::DynamicStruct
    ds_msg = *object;
}
///////////////////////////////////////////////////////////////////////////////////////

void HSMMsg::setField(string fieldname, string value) {
    removeField(fieldname);

    ds_msg[fieldname] = value;
}
///////////////////////////////////////////////////////////////////////////////////////

string HSMMsg::getField(string fieldname) {
    string value;

    if (ds_msg.contains(fieldname)) {
        return ds_msg[fieldname];
    } else {
        value = " ";
    }

    return value;
}

///////////////////////////////////////////////////////////////////////////////////////

void HSMMsg::removeField(string fieldname) {
    if (ds_msg.contains(fieldname)) {
        ds_msg.erase(fieldname);
    }
}

///////////////////////////////////////////////////////////////////////////////////////

string HSMMsg::getG0Request(int msghdrlen) {
    string g0_request = NumberFormatter::format0(0, msghdrlen);

    g0_request = g0_request + _G0_XLATE_BDK_ZPK_TDES + getField(
            _BDK) + getField(_DEST_ZPK) + getField(
            _KSN_DESCRIPTOR) + getField(_KSN)
            + getField(_SOURCE_PIN_BLK) + getField(
            _SOURCE_PIN_BLK_FORMAT)
            + getField(_DEST_PIN_BLK_FORMAT) + getField(
            _ACCOUNT_NR);

    return g0_request;
}

///////////////////////////////////////////////////////////////////////////////////////

void HSMMsg::parseG0G1Rsp(string data, int beginIndex, int endIndex) {
    string temp;

    // get Error Code
    beginIndex = endIndex;
    endIndex = endIndex + 2;
    if (data.length() >= endIndex) {
        temp = data.substr(beginIndex, 2);
        setField(_ERROR_CODE, temp);
    }
    //    else {
    //        throw new XHSMFieldParseError(data, _ERROR_CODE,
    //                "Not enough data. Data Length = "
    //                + data.length() + " End Index = " + endIndex);
    //    }

    if (temp.compare("00") == 0) {
        // Get PIN Length
        beginIndex = endIndex;
        endIndex = endIndex + 2;
        if (data.length() >= endIndex) {
            temp = data.substr(beginIndex, 2);
            setField(_PIN_LEN, temp);
        }
        //        else {
        //            throw new XHSMFieldParseError(data,
        //                    _PIN_LEN, "Not enough data. Data Length = "
        //                    + data.length() + " End Index = " + endIndex);
        //        }

        // Get Destination PIN block
        beginIndex = endIndex;
        endIndex = endIndex + 16;
        if (data.length() >= endIndex) {
            temp = data.substr(beginIndex, 16);
            setField(_DEST_PIN_BLK, temp);
        }
        //        else {
        //            throw new XHSMFieldParseError(data, _DEST_PIN_BLK,
        //                    "Not enough data. Data Length = "
        //                    + data.length() + " End Index = " + endIndex);
        //        }

        // Get Destination PIN block format
        beginIndex = endIndex;
        endIndex = endIndex + 2;
        if (data.length() >= endIndex) {
            temp = data.substr(beginIndex, 2);
            setField(_DEST_PIN_BLK_FORMAT, temp);
        }
        //        else {
        //            throw new XHSMFieldParseError(data, _DEST_PIN_BLK_FORMAT,
        //                    "Not enough data. Data Length = "
        //                    + data.length() + " End Index = " + endIndex);
        //        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////

void HSMMsg::parseRspIntoFields(string data, int msghdrlen) {
    string header;
    string command;
    int beginIndex = 0;
    int endIndex;

    //    if (data == null) {
    //        throw new XHSMFieldParseError(data, "DATA", "Null data received");
    //    }

    endIndex = beginIndex + msghdrlen;
    if (data.length() > endIndex) {
        header = data.substr(beginIndex, msghdrlen);
    }
    //    else {
    //        throw new XHSMFieldParseError(data, "Header", "Not enough data");
    //    }

    beginIndex = endIndex;
    endIndex = endIndex + 2;
    if (data.length() > endIndex) {
        command = data.substr(beginIndex, 2);
    }
    //    else {
    //        throw new XHSMFieldParseError(data, "Command", "Not enough data");
    //    }

    setField(_CMD, command);

    if (command.compare(_G1_XLATE_BDK_ZPK_TDES) == 0) {
        parseG0G1Rsp(data, beginIndex, endIndex);
    }
    //    switch (command) {
    //        case HSMCommands._A1_GEN_KEY:
    //            parseA0A1Rsp(data, beginIndex, endIndex);
    //            break;
    //        case HSMCommands._A3_GEN_PRINT_COMPONENT:
    //            parseA2A3Rsp(data, beginIndex, endIndex);
    //            break;
    //        case HSMCommands._A5_FORM_KEY_ENC_COMPONENTS:
    //            parseA4A5Rsp(data, beginIndex, endIndex);
    //            break;
    //        case HSMCommands._A7_IMPORT_KEY:
    //            parseA6A7Rsp(data, beginIndex, endIndex);
    //            break;
    //        case HSMCommands._A9_EXPORT_KEY:
    //            parseA8A9Rsp(data, beginIndex, endIndex);
    //            break;
    //        case HSMCommands._CB_XLATE_TPK_ZPK:
    //            parseCACBRsp(data, beginIndex, endIndex);
    //            break;
    //        case HSMCommands._CD_XLATE_ZPK_ZPK:
    //            parseCCCDRsp(data, beginIndex, endIndex);
    //            break;
    //        case HSMCommands._CJ_XLATE_BDK_ZPK:
    //            parseCICJRsp(data, beginIndex, endIndex);
    //            break;
    //        case HSMCommands._CX_GEN_VISA_CVV:
    //            break;
    //        case HSMCommands._FB_XLATE_ZPK_ZMK_ZMK:
    //            parseFAFBRsp(data, beginIndex, endIndex);
    //            break;
    //        case HSMCommands._FF_XLATE_TMK_TPK_PVK_ZMK:
    //            parseFEFFRsp(data, beginIndex, endIndex);
    //            break;
    //        case _G1_XLATE_BDK_ZPK_TDES:
    //            parseG0G1Rsp(data, beginIndex, endIndex);
    //            break;
    //        case HSMCommands._HD_GEN_TMK_TPK_PVK:
    //            parseHCHDRsp(data, beginIndex, endIndex);
    //            break;
    //        case HSMCommands._M1_ENCRYPT_DATA:
    //            parseM0M1Rsp(data, beginIndex, endIndex);
    //            break;
    //        case HSMCommands._M3_DECRYPT_DATA:
    //            parseM2M3Rsp(data, beginIndex, endIndex);
    //            break;
    //        case HSMCommands._NP_HSM_STATUS:
    //            parseNONPRsp(data, beginIndex, endIndex);
    //            break;
    //        case HSMCommands._NF_GENERATE_PRINT_KEY:
    //            parseNENFRsp(data, beginIndex, endIndex);
    //            break;
    //        case HSMCommands._NZ_GENERATE_PRINT_KEY:
    //            parseNENFRsp(data, beginIndex, endIndex);
    //            break;
    //        case HSMCommands._PB_LOAD_FORMATTING_DATA_HSM:
    //            parsePAPBRsp(data, beginIndex, endIndex);
    //            break;
    //    }
}

#endif /* HSMMSG_H */

