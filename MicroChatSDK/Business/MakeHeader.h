#pragma once
#include <string>
using namespace std;


class BaseHeader
{
public:

	virtual string MakeHeader(string cookie,int cgiType,int nLenProtobuf,int nLenCompressed);
	virtual string UnPackHeader(string pack);

	static DWORD		s_dwVersion;
	static int			s_dwUin;
	static string		s_cookie;

protected:
	int		m_nLenRespCompressed = 0;
	int		m_nLenRespProtobuf = 0;
	int		m_nDecryptType = 5;
	bool	m_bCompressed = FALSE;
};
