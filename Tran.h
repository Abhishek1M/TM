/* 
 * File:   Tran.h
 * Author: Abhishek1.M
 *
 * Created on 12 April, 2017, 3:15 PM
 */

#ifndef TRAN_H
#define TRAN_H

#include <iostream>

#include <Poco/NumberFormatter.h>
#include <Poco/NumberParser.h>

#include <ap/Iso8583JSON.h>

#include <ap/Encrypt.h>

using Poco::NumberFormatter;
using Poco::NumberParser;

class Tran {
public:
    long tran_nr;
    string mti;
    string pan;
    string tran_type;
    string from_account;
    string to_account;
    long amount_tran;
    long amount_stl;
    long amount_chb;
    string datetime_trans;
    long fee_amount_chb;
    string convrate_stl;
    string convrate_chb;
    string stan;
    string time_local;
    string date_local;
    string date_exp;
    string date_stl;
    string date_conv;
    string date_capture;
    string merchant_type;
    string countrycode_acqinstt;
    string countrycode_panextd;
    string countrycode_fwsinstt;
    string pos_entry_mode;
    string app_pan_nr;
    string nii;
    string pos_cond_code;
    string pos_capture_code;
    string auth_id_rsp_len;
    long fee_amount_tran;
    long fee_amount_stl;
    long proc_fee_amount_tran;
    long proc_fee_amount_stl;
    string acq_instt_code;
    string fwd_instt_id;
    string extd_pan;
    string retr_ref_nr;
    string auth_id_rsp;
    string rsp_code;
    string service_rest_code;
    string ca_term_id;
    string ca_id_code;
    string ca_name_loc;
    string addl_rsp_data;
    string addl_data_private;
    string currencycode_tran;
    string currencycode_stl;
    string currencycode_chb;
    long amount_cash;
    string emv_request;
    string emv_response;
    string advice_reason_code;
    string pos_data;
    string settlement_code;
    string extd_payment_code;
    string countrycode_rcvinstt;
    string countrycode_stlinstt;
    string payee;
    string stl_instt_id_code;
    string rcv_instt_id_code;
    string account_id_1;
    string account_id_2;
    string trans_data_req;
    string trans_data_rsp;
    string acquirer_node;
    string issuer_node;
    string super_merchant_id;
    string routing_info;
    int tran_state;
    string pan_encrypted;
    string req_in;
    string req_out;
    string rsp_in;
    string rsp_out;
    string acq_node_key;
    long prev_trannr;
    string transaction_id;
    int term_batch_nr;
    int acq_node_batch_nr;
    string prev_acq_node_key;
    int acq_entity_id;
    string acq_part_name;
    string retailer_id;
    string store_id;
    string device_id;
    string org_type;

    void mapFromResultSet(pqxx::result rs);
    void mapIntoIso(Iso8583JSON& msg_in);
};
///////////////////////////////////////////////////////////////////////////////

void Tran::mapFromResultSet(pqxx::result rs) {
    tran_nr = rs[0][0].as<long>();
    mti = rs[0][1].as<string>();

    pan = rs[0][70].as<string>(); // We get the Encrypted PAN so that we can decrypt it later

    //Encrypt e;
    pan = Config::e.decryptdata(pan);

    tran_type = rs[0][3].as<string>();
    from_account = rs[0][4].as<string>();
    to_account = rs[0][5].as<string>();
    amount_tran = rs[0][6].as<long>();
    amount_stl = rs[0][7].as<long>();
    amount_chb = rs[0][8].as<long>();
    datetime_trans = rs[0][9].as<string>();
    fee_amount_chb = rs[0][10].as<long>();
    convrate_stl = rs[0][11].as<string>();
    convrate_chb = rs[0][12].as<string>();
    stan = rs[0][13].as<string>();
    time_local = rs[0][14].as<string>();
    date_local = rs[0][15].as<string>();
    date_exp = rs[0][16].as<string>();
    date_stl = rs[0][17].as<string>();
    date_conv = rs[0][18].as<string>();
    date_capture = rs[0][19].as<string>();
    merchant_type = rs[0][20].as<string>();
    countrycode_acqinstt = rs[0][21].as<string>();
    countrycode_panextd = rs[0][22].as<string>();
    countrycode_fwsinstt = rs[0][23].as<string>();
    pos_entry_mode = rs[0][24].as<string>();
    app_pan_nr = rs[0][25].as<string>();
    nii = rs[0][26].as<string>();
    pos_cond_code = rs[0][27].as<string>();
    pos_capture_code = rs[0][28].as<string>();
    auth_id_rsp_len = rs[0][29].as<string>();
    fee_amount_tran = rs[0][30].as<long>();
    fee_amount_stl = rs[0][31].as<long>();
    proc_fee_amount_tran = rs[0][32].as<long>();
    proc_fee_amount_stl = rs[0][33].as<long>();
    acq_instt_code = rs[0][34].as<string>();
    fwd_instt_id = rs[0][35].as<string>();
    extd_pan = rs[0][36].as<string>();
    retr_ref_nr = rs[0][37].as<string>();
    auth_id_rsp = rs[0][38].as<string>();
    rsp_code = rs[0][39].as<string>();
    service_rest_code = rs[0][40].as<string>();
    ca_term_id = rs[0][41].as<string>();
    ca_id_code = rs[0][42].as<string>();
    ca_name_loc = rs[0][43].as<string>();
    addl_rsp_data = rs[0][44].as<string>();
    addl_data_private = rs[0][45].as<string>();
    currencycode_tran = rs[0][46].as<string>();
    currencycode_stl = rs[0][47].as<string>();
    currencycode_chb = rs[0][48].as<string>();
    amount_cash = rs[0][49].as<long>();
    emv_request = rs[0][50].as<string>();
    emv_response = rs[0][51].as<string>();
    advice_reason_code = rs[0][52].as<string>();
    pos_data = rs[0][53].as<string>();
    settlement_code = rs[0][54].as<string>();
    extd_payment_code = rs[0][55].as<string>();
    countrycode_rcvinstt = rs[0][56].as<string>();
    countrycode_stlinstt = rs[0][57].as<string>();
    payee = rs[0][58].as<string>();
    stl_instt_id_code = rs[0][59].as<string>();
    rcv_instt_id_code = rs[0][60].as<string>();
    account_id_1 = rs[0][61].as<string>();
    account_id_2 = rs[0][62].as<string>();
    trans_data_req = rs[0][63].as<string>();
    trans_data_rsp = rs[0][64].as<string>();
    acquirer_node = rs[0][65].as<string>();
    issuer_node = rs[0][66].as<string>();
    super_merchant_id = rs[0][67].as<string>();
    routing_info = rs[0][68].as<string>();
    tran_state = rs[0][69].as<int>();
    pan_encrypted = rs[0][70].as<string>();
    req_in = rs[0][71].as<string>();
    req_out = rs[0][72].as<string>();
    rsp_in = rs[0][73].as<string>();
    rsp_out = rs[0][74].as<string>();
    acq_node_key = rs[0][75].as<string>();
    prev_trannr = rs[0][76].as<long>();
    transaction_id = rs[0][77].as<string>();
    term_batch_nr = rs[0][78].as<int>();
    acq_node_batch_nr = rs[0][79].as<int>();
    prev_acq_node_key = rs[0][80].as<string>();
    //acq_entity_id = rs[0][81].as<int>();
    acq_part_name = rs[0][82].as<string>();
    retailer_id = rs[0][83].as<string>();
    store_id = rs[0][84].as<string>();
    device_id = rs[0][85].as<string>();
    org_type = rs[0][86].as<string>();
}
////////////////////////////////////////////////////////////////////////////////

void Tran::mapIntoIso(Iso8583JSON& msg) {
    msg.setMsgType(mti);

    string clear_pan;
    Encrypt e;
    clear_pan = e.decryptdata(pan_encrypted);
    msg.setField(_002_PAN, clear_pan);

    string pcode;
    pcode.append(tran_type).append(from_account).append(to_account);
    msg.setField(_003_PROCESSING_CODE, pcode);

    msg.setField(_004_AMOUNT_TRANSACTION, NumberFormatter::format(amount_tran));
    msg.setField(_007_TRANSMISSION_DATE_TIME, datetime_trans);
    msg.setField(_011_SYSTEMS_TRACE_AUDIT_NR, stan);
    msg.setField(_012_TIME_LOCAL, time_local);
    msg.setField(_013_DATE_LOCAL, date_local);
    msg.setField(_014_DATE_EXPIRATION, date_exp);
    msg.setField(_018_MERCHANT_TYPE, merchant_type);
    msg.setField(_022_POS_ENTRY_MODE, pos_entry_mode);
    msg.setField(_025_POS_CONDITION_CODE, pos_cond_code);
    msg.setField(_032_ACQUIRING_INST_ID_CODE, acq_instt_code);
    msg.setField(_033_FORWARDING_INST_ID_CODE, fwd_instt_id);
    msg.setField(_037_RETRIEVAL_REF_NR, retr_ref_nr);
    msg.setField(_038_AUTH_ID_RSP, auth_id_rsp);
    msg.setField(_039_RSP_CODE, rsp_code);
    msg.setField(_041_CARD_ACCEPTOR_TERM_ID, ca_term_id);
    msg.setField(_042_CARD_ACCEPTOR_ID_CODE, ca_id_code);
    msg.setField(_043_CARD_ACCEPTOR_NAME_LOC, ca_name_loc);
    msg.setField(_049_CURRENCY_CODE_TRAN, currencycode_tran);
    msg.setField(_055_EMV_DATA, emv_request);
    msg.setField(_062_TRANS_ID, transaction_id);
    msg.setField(_067_EXTENDED_PAYMENT_CODE, extd_payment_code);
    msg.setField(_102_ACCOUNT_ID_1, account_id_1);
    msg.setField(_103_ACCOUNT_ID_2, account_id_2);
    msg.setField(_120_TRAN_DATA_REQ, trans_data_req);
    msg.setField(_121_TRAN_DATA_RSP, trans_data_rsp);

    NodeInfo ni(acquirer_node, issuer_node, super_merchant_id, routing_info);
    msg.setField(_122_NODE_INFO, ni.getNodeInfo());

    msg.setExtendedField(_001_ACQ_NODE_KEY, acq_node_key);
    msg.setExtendedField(_002_ORG_ACQ_NODE_KEY, prev_acq_node_key);
    msg.setExtendedField(_003_STORE_ID, store_id);
    msg.setExtendedField(_004_DEVICE_ID, device_id);
    msg.setExtendedField(_005_ACQ_PART_NAME, acq_part_name);
    msg.setExtendedField(_006_TERM_BATCH_NR, NumberFormatter::format(term_batch_nr));
    msg.setExtendedField(_008_RETAILER_ID, retailer_id);
    msg.setExtendedField(_009_PREV_TRAN_NR, NumberFormatter::format(prev_trannr));
    msg.setExtendedField(_011_ORIGINATOR_TYPE, org_type);
}
#endif /* TRAN_H */

