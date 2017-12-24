#pragma once
#include <string>
using namespace std;

//最大缓冲区长度
#define MAX_BUF 65535

//callback错误码
enum CGI_ERR_CODE {
	
	CGI_CODE_LOGIN_NEED_MOBILE_MSG = -205,			//设备首次登录需要短信授权
	CGI_CODE_LOGIN_NEED_SCAN_QRCODE = -106,			//设备首次登录需要扫码授权
	CGI_CODE_LOGIN_PWD_WRONG = -3,					//密码错误
	CGI_CODE_LOGIN_FAIL = -1,						//登录失败(未知错误)
	CGI_CODE_LOGIN_SUCC = 0,						//登陆成功

	CGI_CODE_OK = 0,								//cgi调用成功

	CGI_CODE_NETWORK_ERR = 100001,					//网络通讯失败
	CGI_CODE_DECRYPT_ERR = 100002,					//封包解密失败
	CGI_CODE_PARSE_PROTOBUF_ERR = 100003,			//protobuf解析失败
	CGI_CODE_UNPACK_ERR = 100004,					//封包包头解析失败
	CGI_CODE_LOGIN_ECDH_ERR = 100005,				//ECDH握手失败
	CGI_CODE_LOGIN_SCAN_QRCODE_ERR = 100006,		//扫码授权地址获取失败


	CGI_ERR_UNKNOWN = 0x7FFFFFFF,					//未知错误
};

enum CGI_TYPE {
	CGI_TYPE_INVALID = -1,
	CGI_TYPE_UNKNOWN = 0,

	CGI_TYPE_NEWSYNC = 138,
	CGI_TYPE_NEWINIT = 139,
	CGI_TYPE_GETPROFILE = 302,
	CGI_TYPE_MANUALAUTH = 701,
	CGI_TYPE_NEWSENDMSG = 522,
	CGI_TYPE_SEARCHCONTACT = 106,
	CGI_TYPE_GETCONTACT = 182,
	CGI_TYPE_VERIFYUSER = 30,
	CGI_TYPE_BIND = 145,

	CGI_TYPE_PUSH = 10001,

	CGI_TYPE_MAX = 0xFFFF
};

#define CGI_NEWSYNC			"/cgi-bin/micromsg-bin/newsync"					//同步服务端最新消息
#define CGI_MANUALAUTH		"/cgi-bin/micromsg-bin/manualauth"				//登录
#define CGI_NEWSENDMSG		"/cgi-bin/micromsg-bin/newsendmsg"				//发送文字消息
#define CGI_NEWINIT			"/cgi-bin/micromsg-bin/newinit"					//首次登录,初始化数据库
#define	CGI_GETPROFILE		"/cgi-bin/micromsg-bin/getprofile"				//获取个人信息
#define	CGI_SEARCHCONTACT	"/cgi-bin/micromsg-bin/searchcontact"			//搜索新朋友
#define	CGI_GETCONTACT		"/cgi-bin/micromsg-bin/getcontact"				//查找新朋友
#define	CGI_VERIFYUSER		"/cgi-bin/micromsg-bin/verifyuser"				//添加好友
#define CGI_BIND			"/cgi-bin/micromsg-bin/bindopmobileforreg"		//首次登录短信授权

//
//本文件内结构体内字符串一律'\0'结尾;UI层需调用SDK接口SafeFree释放内存!
//

//
//扩展其他功能接口:
//编写相应的protobuf并将生成的c++文件导入
//配置cgi type与uri地址;编写回调函数与包含必要数据的结构体
//编写CGITask类,可参考登录函数,正确配置组包与加密
//编写胶水代码
//

//登录结果
struct LoginResult
{
	LoginResult() 
	{
		ZeroMemory(szMsg,sizeof(szMsg));
		ZeroMemory(szUrl, sizeof(szUrl));
	}

	char szMsg[4096];			//提示信息
	char szUrl[4096];			//需要扫码授权时这里传递扫码认证地址;需要短信验证时,这里传递邀请好友短信认证地址
	char szWxid[100] = {0};		//当前登录账号wxid(登录成功后才返回)
	DWORD dwUin = 0;			//当前登录账号uin(登陆成功后才返回)
};

//短信验证结果
struct MobileVerifyResult
{
	int  nCode = 0;						//错误码
	char szMsgResult[1024] = { 0 };		//错误信息
	int  nOption = 10;					//操作码,10==接收验证码,11==发送验证码,13==测试设备是否需要绑定(可跳过该步骤,设备首次登录成功会提示授权的)
};

//好友信息
struct ContactInfo
{
	ContactInfo()
	{
		ZeroMemory(wxid, sizeof(wxid));
		ZeroMemory(nickName, sizeof(nickName));
		ZeroMemory(headicon, sizeof(headicon));
		ZeroMemory(v1_Name, sizeof(v1_Name));
		ZeroMemory(v2_Name, sizeof(v2_Name));
	}
	
	char	wxid[100];
	char	nickName[100];
	char	headicon[200];		//头像url
	char	v1_Name[200];		//v1数据
	char	v2_Name[200];		//v2数据
};

//未读消息
struct NewMsg
{
	NewMsg()
	{
		svrid = 0;
		utc = 0;
		nType = 0;
		ZeroMemory(szFrom, sizeof(szFrom));
		ZeroMemory(szTo, sizeof(szTo));
		ZeroMemory(szContent, sizeof(szContent));
	}
	UINT64  svrid;				//每个消息在服务端唯一标识
	char	szFrom[1024];		//发消息人
	char	szTo[1024];			//收消息人
	int		nType;				//消息类型
	DWORD	utc;				//消息创建时间
	char szContent[MAX_BUF];	//消息内容(有可能是xml格式,根据type判定,需要UI层自行解析)
};

//newinit callback
struct NewInitResult
{
	NewInitResult()
	{
		dwContanct = 0;
		dwNewMsg = 0;
		dwSize = 0;
		ppNewMsg = NULL;
		ppContanct = NULL;
		
	}

	ContactInfo **ppContanct;		//好友信息
	DWORD		dwContanct;			//好友信息数量
	NewMsg		**ppNewMsg;			//新消息
	DWORD		dwNewMsg;			//新消息数量

	DWORD		dwSize;				//二级指针数组大小(释放内存时使用)
};

//newsync callback
struct NewSyncResult
{
	NewSyncResult()
	{
		dwNewMsg		= 0;
		dwContanct		= 0;
		dwSize = 0;
		ppNewMsg		= NULL;
		ppContanct		= NULL;
	}
	
	NewMsg **ppNewMsg;			//新消息
	DWORD dwNewMsg;				//新消息数量
	ContactInfo **ppContanct;	//好友
	DWORD dwContanct;			//好友数量

	DWORD		dwSize;			//二级指针数组大小(释放内存时使用)
};

//searchcontact callback
struct SearchContactResult
{
	int  nCode = 0;						//错误码
	char szMsgResult[1024] = { 0 };		//查询结果
	char szNickName[100] = { 0 };		//昵称
	char szV1_Name[200] = { 0 };		//v1数据
	char szV2_Name[200] = { 0 };		//v2数据
	char szHeadIcon[200] = { 0 };		//头像
};

//加好友请求 callback
struct VerifyUserResult
{
	int  nCode = 0;						//错误码
	char szMsgResult[1024] = { 0 };		//错误信息
};

//cgi任务回调函数,任务结束后通知上层结果
typedef void(*CGICallBack)(void *result,int nCgiType,int nTaskId,int nCode);

//注册cgi回调函数
void RegisterCgiCallBack(CGICallBack callback);

//启动sdk
void StartSDK();

//停用sdk
void StopSDK();

//账号密码登录,返回taskid(bRandomDeivce参数表示是否使用随机设备登录:首次登录需要短信或扫码验证,默认使用固定设备登录)
//首次登录成功后,使用NewInit接口初始化好友列表,获取未读消息,刷新并保存同步key,并报告客户端同步key给服务器
//后续登录会从db中加载好友列表、历史消息与同步key,直接使用newsync接口同步消息
//TODO:bRandomDeivce 随机硬件信息暂未实现
int Login(const char *user, int nLenUser, const char *pwd, int nLenPwd,bool bRandomDeivce = FALSE);

//接收短信验证码(默认使用登录账号作为手机号码)
void RecvMobileVerifyCode();

//发送短信授权验证码
int SendMobileVerifyCode(const char *code, int nLenCode);


//尝试从DB加载信息,结果通过newinit回调给上层
//若无法从db中读取同步key,则调用newinit初始化,否则取出synckey执行同步
void LoadFromDB();

//设备第一次登录初始化(好友列表,历史消息,syncKey)
//不需要UI层主动调用,LoadFromDB中会根据自动需要调用
int NewInit();

//发送文字消息(content必须Utf8格式,否则中文乱码)(支持小表情:格式为"[表情]")
void NewSendMsg(const char *content, int nLenContent, const char *toWxid,int nLenToWxid);

//同步服务器消息(默认使用长链接推送获取消息;登录后调用一次刷新同步key)
void NewSync();

//账号查询:76位v1数据,v2数据,手机号,微信号搜索,暂不支持wxid、108位v1搜索
void SearchContact(const char *szName, int nLenName);

//加好友(暂失效,待更新:服务端更新了协议,需要有v2数据对方才会收到请求;加好友时会发送设备硬件信息)
void AddNewFriend(const char *szUserName, int nLenUserName, const char *szV2Name, int nLenV2Name,const char *szContent, int nLenContent);

//确认设备是否已授权(废弃该接口,直接登录即可)
//void IsNeedVerify(const char *MobileNum, int nLenMobileNum);

//安全释放内存(结构体中指针需要单独释放)
void SafeFree(void *p);
#define SAFE_DELETE(p) {SafeFree((void *)p);p=NULL;}

