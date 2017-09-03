/*
 * TranType.h
 *
 *  Created on: 17-Oct-2016
 *      Author: abhishek
 */

#include<Poco/NumberParser.h>

#include <iostream>
#include <string>

using Poco::NumberParser;
using namespace std;

class TranType {

	static bool isCredit(string tran_type) {
		int val = NumberParser::parse(tran_type);
		return val >= 20 && val <= 29;
	}

	static bool isDebit(string tran_type) {
		int val = NumberParser::parse(tran_type);
		return val >= 0 && val <= 19;
	}

	static bool isInquiry(string tran_type) {
		int val = NumberParser::parse(tran_type);
		return val >= 30 && val <= 39;
	}

	static bool isPayment(string tran_type) {
		int val = NumberParser::parse(tran_type);
		return val >= 50 && val <= 59;
	}

	static bool isTransfer(string tran_type) {
		int val = NumberParser::parse(tran_type);
		return val >= 40 && val <= 49;
	}
	static string _00_GOODS_SERVICES = "00";
	static string _01_CASH = "01";
	static string _02_ADJUSTMENTS = "02";
	static string _03_CHEQUE_GUARANTEE = "03";
	static string _04_CHEQUE_VERIFICATION = "04";
	static string _05_EUROCHEQUE = "05";
	static string _06_TRAVELLER_CHEQUE = "06";
	static string _07_LETTER_OF_CREDIT = "07";
	static string _08_GIRO = "08";
	static string _09_GOODS_SERVICES_CASH_BACK = "09";
	static string _10_NON_CASH_FIN_INSTRUMENT = "10";
	static string _11_QUASI_CASH_AND_SCRIP = "11";
	static string _20_RETURNS = "20";
	static string _21_DEPOSITS = "21";
	static string _22_CREDIT_ADJUSTMENTS = "22";
	static string _23_CHEQUE_DEPOSIT_GUARANTEE = "23";
	static string _24_CHEQUE_DEPOSIT = "24";
	static string _28_PAYMENT = "28";
	static string _30_AVAILABLE_FUNDS_INQUIRY = "30";
	static string _31_BALANCE_INQUIRY = "31";
	static string _35_FULL_STATEMENT_INQUIRY = "35";
	static string _40_CARDHOLDER_ACCOUNTS_TRANSFER = "40";
	static string _61_CARD_ACTIVATE = "61";
	static string _62_CARD_LOAD = "62";
	static string _63_CARD_DEACTIVATE = "63";

	TranType() {
	}
};
