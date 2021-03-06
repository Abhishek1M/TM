/*
 * constants.h
 *
 *  Created on: 20-Oct-2016
 *      Author: abhishek
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <iostream>

using namespace std;

// Sign
const string _D_DEBIT = "D";
const string _C_CREDIT = "C";

//SettlementCode
const string _0_UNKNOWN = "0";
const string _1_IN_BALANCE = "1";
const string _2_OUT_OF_BALANCE = "2";
const string _3_ERROR = "3";

// AdviceReasonCode
const string _910_0011_TM_TO = "9100011";
const string _910_0012_RM_TO = "9100012";

// Response Codes
const string _00_SUCCESSFUL = "00";
const string _01_REFER_TO_CI = "01";
const string _02_REFER_TO_CI_SPECIAL = "02";
const string _03_INVALID_MERCHANT = "03";
const string _04_PICK_UP = "04";
const string _05_DO_NOT_HONOUR = "05";
const string _06_ERROR = "06";
const string _07_PICK_UP_SPECIAL = "07";
const string _08_HONOUR_WITH_ID = "08";
const string _09_REQUEST_IN_PROGRESS = "09";
const string _10_APPROVED_PARTIAL = "10";
const string _11_APPROVED_VIP = "11";
const string _12_INVALID_TRAN = "12";
const string _13_INVALID_AMOUNT = "13";
const string _14_INVALID_CARD_NUMBER = "14";
const string _15_NO_SUCH_ISSUER = "15";
const string _16_APPROVED_UPDATE_TRACK_3 = "16";
const string _17_CUSTOMER_CANCELLATION = "17";
const string _18_CUSTOMER_DISPUTE = "18";
const string _19_REENTER_TRAN = "19";
const string _20_INVALID_RESPONSE = "20";
const string _21_NO_ACTION_TAKEN = "21";
const string _22_SUSPECTED_MALFUNCTION = "22";
const string _23_UNACCEPTABLE_TRAN_FEE = "23";
const string _24_FILE_UPDATE_NOT_SUPPORTED = "24";
const string _25_UNABLE_TO_LOCATE_RECORD = "25";
const string _26_DUPLICATE_RECORD = "26";
const string _27_FILE_UPDATE_EDIT_ERROR = "27";
const string _28_FILE_UPDATE_FILE_LOCKED = "28";
const string _29_FILE_UPDATE_FAILED = "29";
const string _30_FORMAT_ERROR = "30";
const string _31_BANK_NOT_SUPPORTED = "31";
const string _32_COMPLETED_PARTIALLY = "32";
const string _33_EXPIRED_CARD_PICK_UP = "33";
const string _34_SUSPECTED_FRAUD_PICK_UP = "34";
const string _35_CONTACT_ACQ_PICK_UP = "35";
const string _36_RESTRICTED_CARD_PICK_UP = "36";
const string _37_CALL_ACQ_SECURITY_PICK_UP = "37";
const string _38_PIN_TRIES_EXCEEDED_PICK_UP = "38";
const string _39_NO_CREDIT_ACCOUNT = "39";
const string _40_FUNCTION_NOT_SUPPORTED = "40";
const string _41_LOST_CARD = "41";
const string _42_NO_UNIVERSAL_ACCOUNT = "42";
const string _43_STOLEN_CARD = "43";
const string _44_NO_INVESTMENT_ACCOUNT = "44";
const string _48_NO_CUSTOMER_RECORD = "48";
const string _51_NOT_SUFFICIENT_FUNDS = "51";
const string _52_NO_CHEQUING_ACCOUNT = "52";
const string _53_NO_SAVINGS_ACCOUNT = "53";
const string _54_EXPIRED_CARD = "54";
const string _55_INCORRECT_PIN = "55";
const string _56_NO_CARD_RECORD = "56";
const string _57_TRAN_NOT_PERMITTED_CARDHOLDER = "57";
const string _58_TRAN_NOT_PERMITTED_TERMINAL = "58";
const string _59_SUSPECTED_FRAUD_DECLINED = "59";
const string _60_CONTACT_ACQUIRER = "60";
const string _61_EXCEEDS_WITHDRAWAL_LIMIT = "61";
const string _62_RESTRICTED_CARD = "62";
const string _63_SECURITY_VIOLATION = "63";
const string _64_ORIGINAL_AMOUNT_INCORRECT = "64";
const string _65_EXCEEDS_WITHDRAWAL_FREQUENCY = "65";
const string _66_CALL_ACQ_SECURITY = "66";
const string _67_HARD_CAPTURE = "67";
const string _68_RESPONSE_RECEIVED_TOO_LATE = "68";
const string _75_PIN_TRIES_EXCEEDED = "75";
const string _77_INTERVENE_BANK_APPROVAL_REQD = "77";
const string _78_INTERVENE_PARTIAL_AMOUNT = "78";
const string _81_CRYPTO_ERROR = "81";
const string _90_CUTOFF_IN_PROGRESS = "90";
const string _91_ISSUER_OR_SWITCH_INOPERATIVE = "91";
const string _92_ROUTING_ERROR = "92";
const string _93_VIOLATION_OF_LAW = "93";
const string _94_DUPLICATE_TRANSMISSION = "94";
const string _95_RECONCILE_ERROR = "95";
const string _96_SYSTEM_MALFUNCTION = "96";
const string _98_EXCEEDS_CASHLIMIT = "98";
const string _L1_AMOUNT_EXCEEDED = "L1";
const string _L2_CARD_USE_EXCEEDED = "L2";
const string _L3_ICA_DISABLED = "L3";

// POS Condition Codes
const string _00_NORMAL_PRESENTMENT = "00";
const string _01_CUSTOMER_NOT_PRESENT = "01";
const string _02_UNATTENDED_RETAIN_TERMINAL = "02";
const string _03_MERCHANT_SUSPICIOUS = "03";
const string _04_ECR_INTERFACE = "04";
const string _05_CUSTOMER_PRESENT_CARD_NOT = "05";
const string _06_PRE_AUTHORISED_REQ = "06";
const string _07_TELEPHONE_DEVICE_REQ = "07";
const string _08_MAIL_TELEPHONE_ORDER = "08";
const string _09_POS_SECURITY_ALERT = "09";
const string _10_CUSTOMER_ID_VERIFIED = "10";
const string _11_SUSPECTED_FRAUD = "11";
const string _12_SECURITY_REASONS = "12";
const string _13_REPRESENTATION_OF_ITEM = "13";
const string _14__UTILITY_TERMINAL = "14";
const string _15_CUSTOMER_TERMINAL = "15";
const string _16_ADMIN_TERMINAL = "16";
const string _17_RETURNED_ITEM = "17";
const string _18_NO_CHEQUE_IN_ENVELOPE_RETURN = "18";
const string _19_DEPOSIT_OUT_OF_BALANCE_RETURN = "19";
const string _20_PAYMENT_OUT_OF_BALANCE_RETURN = "20";
const string _21_MANUAL_REVERSAL = "21";
const string _22_TERMINAL_ERROR_COUNTED = "22";
const string _23_TERMINAL_ERROR_NOT_COUNTED = "23";
const string _24_DEPOSIT_OUT_OF_BALANCE_APPLY = "24";
const string _25_PAYMENT_OUT_OF_BALANCE_APPLY = "25";
const string _26_WITHDRAWAL_ERROR_REVERSED = "26";
const string _27_UNATTENDED_NO_RETAIN_TERMINAL = "27";
const string _41_PARTIAL_APPROVAL_ALLOWED = "41";

// Network Management Information Code (NMICD)
const string _001_SIGN_ON = "001";
const string _002_SIGN_OFF = "002";
const string _003_TARGET_SYSTEM_UNAVAILABLE = "003";
const string _004_BACK_UP_MODE = "004";
const string _005_SPECIAL_INSTRUCTION = "005";
const string _006_INITIATE_ALT_ROUTING = "006";
const string _101_KEY_CHANGE = "101";
const string _102_SECURITY_ALERT = "102";
const string _103_PASSWORD_CHANGE = "103";
const string _104_DEVICE_AUTHENTICATION = "104";
const string _201_INITIATE_CUTOFF = "201";
const string _202_CUTOFF_COMPLETE = "202";
const string _301_ECHO_TEST = "301";

// Message Type
const string UNKNOWN = "0000";
const string _0100_AUTH_REQ = "0100";
const string _0101_AUTH_REQ_REP = "0101";
const string _0110_AUTH_REQ_RSP = "0110";
const string _0120_AUTH_ADV = "0120";
const string _0121_AUTH_ADV_REP = "0121";
const string _0130_AUTH_ADV_RSP = "0130";
const string _0200_TRAN_REQ = "0200";
const string _0201_TRAN_REQ_REP = "0201";
const string _0210_TRAN_REQ_RSP = "0210";
const string _0220_TRAN_ADV = "0220";
const string _0221_TRAN_ADV_REP = "0221";
const string _0230_TRAN_ADV_RSP = "0230";
const string _0300_ACQUIRER_FILE_UPDATE_REQ = "0300";
const string _0301_ACQUIRER_FILE_UPDATE_REQ_REP = "0301";
const string _0310_ACQUIRER_FILE_UPDATE_REQ_RSP = "0310";
const string _0320_ACQUIRER_FILE_UPDATE_ADV = "0320";
const string _0321_ACQUIRER_FILE_UPDATE_ADV_REP = "0321";
const string _0330_ACQUIRER_FILE_UPDATE_ADV_RSP = "0330";
const string _0400_ACQUIRER_REV_REQ = "0400";
const string _0401_ACQUIRER_REV_REQ_REP = "0401";
const string _0410_ACQUIRER_REV_REQ_RSP = "0410";
const string _0420_ACQUIRER_REV_ADV = "0420";
const string _0421_ACQUIRER_REV_ADV_REP = "0421";
const string _0430_ACQUIRER_REV_ADV_RSP = "0430";
const string _0500_ACQUIRER_RECONCILE_REQ = "0500";
const string _0501_ACQUIRER_RECONCILE_REQ_REP = "0501";
const string _0510_ACQUIRER_RECONCILE_REQ_RSP = "0510";
const string _0520_ACQUIRER_RECONCILE_ADV = "0520";
const string _0521_ACQUIRER_RECONCILE_ADV_REP = "0521";
const string _0530_ACQUIRER_RECONCILE_ADV_RSP = "0530";
const string _0600_ADMIN_REQ = "0600";
const string _0601_ADMIN_REQ_REP = "0601";
const string _0610_ADMIN_REQ_RSP = "0610";
const string _0620_ADMIN_ADV = "0620";
const string _0621_ADMIN_ADV_REP = "0621";
const string _0630_ADMIN_ADV_RSP = "0630";
const string _0800_NWRK_MNG_REQ = "0800";
const string _0801_NWRK_MNG_REQ_REP = "0801";
const string _0810_NWRK_MNG_REQ_RSP = "0810";
const string _0820_NWRK_MNG_ADV = "0820";
const string _0830_NWRK_MNG_ADV_RSP = "0830";

// Bit / Fields
const int FIRST = 1;
const int MAP_EXTENDED = 1;
const int FIRST_FIELD = 2;
const int _002_PAN = 2;
const int _003_PROCESSING_CODE = 3;
const int _004_AMOUNT_TRANSACTION = 4;
const int _005_AMOUNT_SETTLE = 5;
const int _006_AMOUNT_CARDHOLDER_BILL = 6;
const int _007_TRANSMISSION_DATE_TIME = 7;
const int _008_AMOUNT_CARDHOLDER_BILL_FEE = 8;
const int _009_CONV_RATE_SETTLE = 9;
const int _010_CONV_RATE_CARDHOLDER_BILL = 10;
const int _011_SYSTEMS_TRACE_AUDIT_NR = 11;
const int _012_TIME_LOCAL = 12;
const int _013_DATE_LOCAL = 13;
const int _014_DATE_EXPIRATION = 14;
const int _015_DATE_SETTLE = 15;
const int _016_DATE_CONV = 16;
const int _017_DATE_CAPTURE = 17;
const int _018_MERCHANT_TYPE = 18;
const int _019_ACQUIRING_INST_COUNTRY_CODE = 19;
const int _020_PAN_EXTENDED_COUNTRY_CODE = 20;
const int _021_FORWARDING_INST_COUNTRY_CODE = 21;
const int _022_POS_ENTRY_MODE = 22;
const int _023_CARD_SEQ_NR = 23;
const int _024_NETWORK_INTL_ID = 24;
const int _025_POS_CONDITION_CODE = 25;
const int _026_POS_PIN_CAPTURE_CODE = 26;
const int _027_AUTH_ID_RSP_LEN = 27;
const int _028_AMOUNT_TRAN_FEE = 28;
const int _029_AMOUNT_SETTLE_FEE = 29;
const int _030_AMOUNT_TRAN_PROC_FEE = 30;
const int _031_AMOUNT_SETTLE_PROC_FEE = 31;
const int _032_ACQUIRING_INST_ID_CODE = 32;
const int _033_FORWARDING_INST_ID_CODE = 33;
const int _034_PAN_EXTENDED = 34;
const int _035_TRACK_2_DATA = 35;
const int _036_TRACK_3_DATA = 36;
const int _037_RETRIEVAL_REF_NR = 37;
const int _038_AUTH_ID_RSP = 38;
const int _039_RSP_CODE = 39;
const int _040_SERVICE_RESTRICTION_CODE = 40;
const int _041_CARD_ACCEPTOR_TERM_ID = 41;
const int _042_CARD_ACCEPTOR_ID_CODE = 42;
const int _043_CARD_ACCEPTOR_NAME_LOC = 43;
const int _044_ADDITIONAL_RSP_DATA = 44;
const int _045_TRACK_1_DATA = 45;
const int _046_ADDITIONAL_DATA_ISO = 46;
const int _047_ADDITIONAL_DATA_NATIONAL = 47;
const int _048_ADDITIONAL_DATA = 48;
const int _049_CURRENCY_CODE_TRAN = 49;
const int _050_CURRENCY_CODE_SETTLE = 50;
const int _051_CURRENCY_CODE_BILL = 51;
const int _052_PIN_DATA = 52;
const int _053_SECURITY_INFO = 53;
const int _054_ADDITIONAL_AMOUNTS = 54;
const int _055_EMV_DATA = 55;
const int _060_ADVICE_REASON_CODE = 60;
const int _061_POS_DATA = 61;
const int _062_TRANS_ID = 62;
const int _063_PRIVATE_USE = 63;
const int _064_MAC_NORMAL = 64;
const int _066_SETTLEMENT_CODE = 66;
const int _067_EXTENDED_PAYMENT_CODE = 67;
const int _068_RECEIVING_INST_COUNTRY_CODE = 68;
const int _069_SETTLE_INST_COUNTRY_CODE = 69;
const int _070_NETWORK_MNG_INFO_CODE = 70;
const int _071_MSG_NR = 71;
const int _072_MSG_NR_LAST = 72;
const int _073_DATE_ACTION = 73;
const int _074_NR_CR = 74;
const int _075_NR_CR_REV = 75;
const int _076_NR_DT = 76;
const int _077_NR_DT_REV = 77;
const int _078_NR_TRANSFER = 78;
const int _079_NR_TRANSFER_REV = 79;
const int _080_NR_INQUIRIES = 80;
const int _081_NR_AUTH = 81;
const int _082_AMOUNT_CR_PROC_FEE = 82;
const int _083_AMOUNT_CR_TRAN_FEE = 83;
const int _084_AMOUNT_DT_PROC_FEE = 84;
const int _085_AMOUNT_DT_TRAN_FEE = 85;
const int _086_AMOUNT_CR = 86;
const int _087_AMOUNT_CR_REV = 87;
const int _088_AMOUNT_DT = 88;
const int _089_AMOUNT_DT_REV = 89;
const int _090_ORIGINAL_DATA_ELEMENTS = 90;
const int _091_FILE_UPDATE_CODE = 91;
const int _092_FILE_SECURITY_CODE = 92;
const int _093_RSP_IND = 93;
const int _094_SERVICE_IND = 94;
const int _095_REPLACEMENT_AMOUNTS = 95;
const int _096_MSG_SECURITY_CODE = 96;
const int _097_AMOUNT_NET_SETTLE = 97;
const int _098_PAYEE = 98;
const int _099_SETTLE_INST_ID_CODE = 99;
const int _100_RECEIVING_INST_ID_CODE = 100;
const int _101_FILE_NAME = 101;
const int _102_ACCOUNT_ID_1 = 102;
const int _103_ACCOUNT_ID_2 = 103;
const int _104_TRAN_DESCRIPTION = 104;
const int _120_TRAN_DATA_REQ = 120;
const int _121_TRAN_DATA_RSP = 121;
const int _122_NODE_INFO = 122;
const int _123_EXTENDED_FIELD = 123;
const int _124_PREV_ACQ_NODE_KEY = 124;
const int _128_MAC_EXTENDED = 128;

// Extended Bit
const int _001_ACQ_NODE_KEY = 1;
const int _002_ORG_ACQ_NODE_KEY = 2;
const int _003_STORE_ID = 3;
const int _004_DEVICE_ID = 4;
const int _005_ACQ_PART_NAME = 5;
const int _006_TERM_BATCH_NR = 6;
const int _007_TRAN_NR = 7;
const int _008_RETAILER_ID = 8;
const int _009_PREV_TRAN_NR = 9;
const int _010_ACQ_NODE_KEY_NAME = 10;
const int _011_ORIGINATOR_TYPE = 11;
const int _012_MOBILE_NR = 12;
const int _013_IMEI = 13;
const int _014_USER_NAME = 14;
const int _015_PASSWORD = 15;
const int _016_PIN_KEY = 16;
const int _017_DATA_KEY = 17;
const int _018_CVV2 = 18;
const int _020_UCAF_DATA = 20;
const int _021_XID = 21;
const int _022_CAVV = 22;
const int _023_ECI = 23;
const int _024_ALGO = 24;
const int _025_3DS_ENROLLMENT = 25;
const int _026_3DS_AUTHENTICATION_RETURN_CODE = 26;

// Originator Type
const string _001_ATM = "001";
const string _002_IPOS = "002";
const string _003_POS = "003";
const string _004_MPOS = "004";
const string _005_PG = "005";
const string _006_SI = "006";
const string _101_HOST = "101";

// Transaction State
const int _000_TRAN_INPROGRESS = 0;
const int _001_TRAN_COMPLETE = 1;
const int _002_TRAN_ABORTED = 2;

// Amount Type
const string _01_LEDGER_BALANCE = "01";
const string _02_AVAILABLE_BALANCE = "02";
const string _03_AMOUNT_OWING = "03";
const string _04_AMOUNT_DUE = "04";
const string _40_CASH = "40";
const string _41_GOODS_SERVICES = "41";
const string _53_APPROVED = "53";
const string _56_AMOUNT_TIP = "56";
const string _90_AVAILABLE_CREDIT = "90";
const string _91_CREDIT_LIMIT = "91";

// Account Type
const string _00_DEFAULT = "00";
const string _10_SAVINGS = "10";
const string _20_CHECK = "20";
const string _30_CREDIT = "30";
const string _40_UNIVERSAL = "40";
const string _50_INVESTMENT = "50";
const string _60_ELECTRONIC_PURSE_DEFAULT = "60";

// HSM Fields
const string _A2_PRINT_FIELD = "_A2_PRINT_FIELD";
const string _PA_FORMATTING_DATA = "_PA_FORMATTING_DATA";
const string _A4_N_KEY_COMPONENTS = "_A4_N_KEY_COMPONENTS";
const string _ACCOUNT_NR = "_ACCOUNT_NR";
const string _BDK = "_BDK";
const string _CMD = "CMD";
const string _CURR_TMK_TPK_PVK = "_CURR_TMK_TPK_PVK";
const string _DEST_PIN_BLK = "_DEST_PIN_BLK";
const string _DEST_PIN_BLK_FORMAT = "_DEST_PIN_BLK_FORMAT";
const string _ZMK = "_DEST_ZMK";
const string _DATA = "_DATA";
const string _DEST_ZPK = "_DEST_ZPK";
const string _DSP_FIRMWARE_NR = "_DSP_FIRMWARE_NR";
const string _DSP_FITTED = "_DSP_FITTED";
const string _ERROR_CODE = "_ERROR_CODE";
const string _ETHERNET_TYPE = "_ETHERNET_TYPE";
const string _FIRMWARE_NR = "_FIRMWARE_NR";
const string _HEADER = "HEADER";
const string _IO_BUFFER_SIZE = "_IO_BUFFER_SIZE";
const string _KEY = "_KEY";
const string _KEY_CHECK_VALUE = "_KEY_CHECK_VALUE";
const string _KEY_SCHEME = "_KEY_SCHEME";
const string _KEY_TYPE = "_KEY_TYPE";
const string _KEY_UNDER_ZMK = "_ZMK";
const string _KSN = "_KSN";
const string _KSN_DESCRIPTOR = "_KSN_DESCRIPTOR";
const string _MAX_PIN_LEN = "_MAX_PIN_LEN";
const string _MODE = "_MODE";
const string _NEW_KEY_UNDER_CURR_KEY = "_NEW_KEY_UNDER_CURR_KEY";
const string _NEW_KEY_UNDER_LMK = "_NEW_KEY_UNDER_LMK";
const string _NE_PRINT_FIELD = "_NE_PRINT_FIELD";
const string _NR_KEY_COMPONENTS = "_NR_KEY_COMPONENTS";
const string _NR_TCP_SOCKETS = "_NR_TCP_SOCKETS";
const string _PIN_LEN = "_PIN_LEN";
const string _SOURCE_PIN_BLK = "_SOURCE_PIN_BLK";
const string _SOURCE_PIN_BLK_FORMAT = "_SOURCE_PIN_BLK_FORMAT";
const string _SOURCE_TPK = "_SOURCE_TPK";
const string _SOURCE_ZPK = "_SOURCE_ZPK";

// HSM Commands
const string _A0_GEN_KEY = "A0";
const string _A1_GEN_KEY = "A1";
const string _A2_GEN_PRINT_COMPONENT = "A2";
const string _A3_GEN_PRINT_COMPONENT = "A3";
const string _A4_FORM_KEY_ENC_COMPONENTS = "A4";
const string _A5_FORM_KEY_ENC_COMPONENTS = "A5";
const string _A6_IMPORT_KEY = "A6";
const string _A7_IMPORT_KEY = "A7";
const string _A8_EXPORT_KEY = "A8";
const string _A9_EXPORT_KEY = "A9";
const string _CA_XLATE_TPK_ZPK = "CA";
const string _CB_XLATE_TPK_ZPK = "CB";
const string _CC_XLATE_ZPK_ZPK = "CC";
const string _CD_XLATE_ZPK_ZPK = "CD";
const string _CI_XLATE_BDK_ZPK = "CI";
const string _CJ_XLATE_BDK_ZPK = "CJ";
const string _CW_GEN_VISA_CVV = "CW";
const string _CX_GEN_VISA_CVV = "CX";
const string _CY_VERIFY_VISA_CVV = "CY";
const string _CZ_VERIFY_VISA_CVV = "CZ";
const string _DE_GEN_PIN_OFFSET = "DE";
const string _DF_GEN_PIN_OFFSET = "DF";
const string _EA_VERIFY_IBM_PIN = "EA";
const string _EB_VERIFY_IBM_PIN = "EB";
const string _EE_DERIVE_IBM_PIN = "EE";
const string _EF_DERIVE_IBM_PIN = "EF";
const string _FA_XLATE_ZPK_ZMK_ZMK = "FA";
const string _FB_XLATE_ZPK_ZMK_ZMK = "FB";
const string _FE_XLATE_TMK_TPK_PVK_ZMK = "FE";
const string _FF_XLATE_TMK_TPK_PVK_ZMK = "FF";
const string _G0_XLATE_BDK_ZPK_TDES = "G0";
const string _G1_XLATE_BDK_ZPK_TDES = "G1";
const string _HC_GEN_TMK_TPK_PVK = "HC";
const string _HD_GEN_TMK_TPK_PVK = "HD";
const string _JA_GEN_RANDOM_PIN = "JA";
const string _JB_GEN_RANDOM_PIN = "JB";
const string _M0_ENCRYPT_DATA = "M0";
const string _M1_ENCRYPT_DATA = "M1";
const string _M2_DECRYPT_DATA = "M2";
const string _M3_DECRYPT_DATA = "M3";
const string _NE_GENERATE_PRINT_KEY = "NE";
const string _NF_GENERATE_PRINT_KEY = "NF";
const string _NZ_GENERATE_PRINT_KEY = "NZ";
const string _NO_HSM_STATUS = "NO";
const string _NP_HSM_STATUS = "NP";
const string _PA_LOAD_FORMATTING_DATA_HSM = "PA";
const string _PB_LOAD_FORMATTING_DATA_HSM = "PB";

#endif /* CONSTANTS_H_ */
