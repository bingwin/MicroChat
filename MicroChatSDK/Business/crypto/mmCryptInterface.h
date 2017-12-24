#pragma once
#include <atlstr.h>
#include <string>
using namespace std;

#define MD5_DIGEST_LENGTH 16

string compress_aes(string key, string in, DWORD &dwLenCompressed);
string nocompress_aes(string key, string in);
string compress_rsa(string keye, string keyn, string in, DWORD &dwLenCompressed);
string nocompress_rsa(string keye, string keyn, string in);

string aes_uncompress(string key, string in, DWORD dwLenOut);
string aes_nouncompress(string key, string in);

bool GenEcdh(int nid, string &strPubKey,string &strPriKey);
string DoEcdh(int nid, string &strServerPubKey, string &strLocalPriKey);

string GetMd5(string in);
string GetMd5_32(string in);