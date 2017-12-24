#pragma once
#include <atlstr.h>
#include <string>

using namespace std;

string Hex2bin(CStringA strHex);

string Bin2Hex(string &strBin);

CStringA CStringA2Utf8(CStringA strGbk);
CStringA Utf82CStringA(CStringA strUtf8);


string Dword2String(DWORD dw);
DWORD String2Dword(string str,DWORD &dwOutLen);

CStringA Utc2BeijingTime(DWORD dwUtc);






