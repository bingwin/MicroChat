#pragma once
#include <atlstr.h>
#include <string>
#include "define.h"
#include "interface.h"
using namespace std;

//protobuf与string转换
string Hex2bin(CStringA strHex);
string Bin2Hex(string &strBin);

//Utf8编码解码
CStringA CStringA2Utf8(CStringA strGbk);
CStringA Utf82CStringA(CStringA strUtf8);

//变长整数编码解码
string Dword2String(DWORD dw);
DWORD String2Dword(string str,DWORD &dwOutLen);

//异常;强退
void ForceQuit(CString str);
#define NEW_ERR(x)	if(!x){ForceQuit(L"New throw an exception,i am dying......");}

void LOG(const char *szLog, ...);






