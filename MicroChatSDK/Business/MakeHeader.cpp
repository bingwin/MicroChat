#include "stdafx.h"
#include "MakeHeader.h"
#include "fun.h"
#include "AuthInfo.h"

DWORD	BaseHeader::s_dwVersion = CLIENT_VERSION;
int		BaseHeader::s_dwUin = 0;
string  BaseHeader::s_cookie = "";

string BaseHeader::MakeHeader(string cookie, int cgiType, int nLenProtobuf, int nLenCompressed)
{
	string strHeader;

	int nCur = 0;

	//添加移动设备封包标志
	strHeader.push_back(0xbf);
	nCur++;
	
	//是否使用压缩算法(最后2bits)(1表示使用zlib压缩)(压缩后长度可能变长,不一定使用压缩算法)
	unsigned char SecondByte = (nLenProtobuf == nLenCompressed) ? 0x2 : 0x1;

	//包头长度最后写入
	strHeader.push_back(SecondByte);
	nCur++;

	//加密算法(前4bits),默认使用aes加密(5),需要rsa加密的CGI重载此虚函数
	unsigned char ThirdByte = 0x5 << 4;

	//cookie长度(后4bits)，当前协议默认15位
	ThirdByte += 0xf;

	strHeader.push_back(ThirdByte);
	nCur++;

	//写入版本号(大端4字节整数)
	DWORD dwVer = htonl(s_dwVersion);
	strHeader = strHeader + string((const char *)&dwVer,4);
	nCur += 4;

	//写入uin(大端4字节整数)
	DWORD dwUin = htonl(pAuthInfo->m_uin);
	strHeader = strHeader + string((const char *)&dwUin, 4);
	nCur += 4;

	//写入cookie
	strHeader = strHeader + pAuthInfo->m_cookie;
	nCur += 0xf;

	//cgi type(变长整数)
	string strCgi = Dword2String(cgiType);
	strHeader = strHeader + strCgi;
	nCur += strCgi.size();

	//protobuf长度(变长整数)
	string strProtobuf = Dword2String(nLenProtobuf);
	strHeader = strHeader + strProtobuf;
	nCur += strProtobuf.size();

	//protobuf压缩后长度(变长整数)
	string strCompressed = Dword2String(nLenCompressed);
	strHeader = strHeader + strCompressed;
	nCur += strCompressed.size();

	//ecdh校验值等mmtls协议才用到的参数(15 byte)(用0 补位)
	char szBuf[15] = { 0 };
	strHeader = strHeader + string(szBuf, 15);
	nCur += 15;

	//将包头长度写入第二字节前6bits(包头长度不会超出6bits)
	SecondByte += (nCur << 2);


	//将正确的第二字节写入包头
	strHeader[1] = SecondByte;

	return strHeader;
}

std::string BaseHeader::UnPackHeader(string pack)
{
	string body;

	int nCur = 0;

	if (pack.size() < 0x20)	return body;

	//跳过安卓标志bf(计入包头总长度)
	if (0xbf == (unsigned char)pack[nCur])
	{
		nCur++;
	}

	//解析包头长度(前6bits)
	int nHeadLen = (unsigned char)pack[nCur] >> 2;

	//是否使用压缩(后2bits)
	m_bCompressed = (1 == ((unsigned char)pack[nCur] & 0x3)) ? TRUE : FALSE;

	nCur++;

	//解密算法(前4 bits)(05:aes / 07:rsa)(仅握手阶段的发包使用rsa公钥加密,由于没有私钥收包一律aes解密)
	m_nDecryptType = (unsigned char)pack[nCur] >> 4;

	//cookie长度(后4 bits)
	int nCookieLen = (unsigned char)pack[nCur] & 0xF;

	nCur++;

	//服务器版本,无视(4字节)
	nCur += 4;

	//uin 非登录包,无视(4字节)
	nCur += 4;

	//刷新cookie(超过15字节说明协议头已更新)
	if (nCookieLen && nCookieLen <= 0xf)
	{
		s_cookie = pack.substr(nCur, nCookieLen);
		pAuthInfo->m_cookie = s_cookie;

		nCur += 0xf;
	}
	else if (nCookieLen > 0xf)
	{
		return body;
	}

	//cgi type,变长整数,无视
	DWORD dwLen = 0;
	DWORD dwCgiType = String2Dword(pack.substr(nCur, 5), dwLen);
	nCur += dwLen;

	//解压后protobuf长度，变长整数
	m_nLenRespProtobuf = String2Dword(pack.substr(nCur, 5), dwLen);
	nCur += dwLen;

	//压缩后(加密前)的protobuf长度，变长整数
	m_nLenRespCompressed = String2Dword(pack.substr(nCur, 5), dwLen);
	nCur += dwLen;

	//后面数据无视

	//解包完毕,取包体
	if (nHeadLen < pack.size())
	{
		body = pack.substr(nHeadLen);
	}

	return body;
}

