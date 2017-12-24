#pragma once

enum ChannelType
{
	ChannelType_ShortConn = 1,
	ChannelType_LongConn = 2,
	ChannelType_All = 3
};

enum LongLinkCmdId
{
	//长链接未确认时,有新消息服务端下发cmdid(仅通知,无消息内容,需要主动newsync)
	RECV_PUSH_CMDID = 24,

	//向服务器发送心跳
	SEND_NOOP_CMDID = 6,

	//服务器下发心跳
	RECV_NOOP_CMDID = 1000000006,

	//向服务器请求同步消息
	SEND_NEWSYNC_CMDID = 121,

	//服务器返回同步消息
	RECV_NEWSYNC_CMDID = 1000000121,

	//长链接确认请求
	LONGLINK_IDENTIFY_REQ = 205,

	//服务器返回长链接确认
	LONGLINK_IDENTIFY_RESP = 1000000205,

	//服务器下发推送(包含消息内容)
	PUSH_DATA_CMDID = 122,

	//回复服务器同步成功
	SEND_SYNC_SUCCESS = 1000000190,

	SIGNALKEEP_CMDID = 243,

	//登录
	MANUALAUTH = 253,
};

//长链接协议版本
#define LONGLINK_CLIENT_VER		0x01

//长链接地址
#define LONGLINK_HOST		"long.weixin.qq.com"

//长链接端口
#define LONGLINK_PORT_443		443
#define LONGLINK_PORT_8080		8080
#define LONGLINK_PORT_80		80

//短链接地址
#define SHORTLINK_HOST		"short.weixin.qq.com";

//短链接端口
#define SHORTLINK_PORT		80

//客户端版本
#define CLIENT_VERSION  637927472

//ECDH握手椭圆曲线参数
#define ECDH_NID			713

//登录设备硬件信息
#define DEVICE_INFO_GUID					"A31d2152a33d83e7"
#define DEVICE_INFO_CLIENT_SEQID			"A31cc712ad2d83e6_1512965043210"
#define DEVICE_INFO_CLIENT_SEQID_SIGN		"e89b238e77cf988ebd09eb65f5378e99"
#define DEVICE_INFO_IMEI					"865167123366678"
#define DEVICE_INFO_ANDROID_ID				"eabe1f220561a49f"
#define DEVICE_INFO_ANDROID_VER				"android-26"
#define DEVICE_INFO_MANUFACTURER			CStringA2Utf8("iPhone")
#define DEVICE_INFO_MODELNAME			    CStringA2Utf8("X")
#define DEVICE_INFO_MOBILE_WIFI_MAC_ADDRESS	"01:67:33:56:78:11"
#define DEVICE_INFO_AP_BSSID				"41:25:99:22:3f:14"
#define DEVICE_INFO_LANGUAGE				"zh_CN"

#define DEVICE_INFO_SOFTINFO				"<softtype><lctmoc>0</lctmoc><level>1</level><k1>ARMv7 processor rev 1 (v7l) </k1><k2></k2><k3>5.1.1</k3><k4>%s</k4><k5>460007337766541</k5><k6>89860012221746527381</k6><k7>%s</k7><k8>unknown</k8><k9>%s</k9><k10>2</k10><k11>placeholder</k11><k12>0001</k12><k13>0000000000000001</k13><k14>%s</k14><k15></k15><k16>neon vfp swp half thumb fastmult edsp vfpv3 idiva idivt</k16><k18>%s</k18><k21>\"wireless\"</k21><k22></k22><k24>%s</k24><k26>0</k26><k30>\"wireless\"</k30><k33>com.tencent.mm</k33><k34>Android-x86/android_x86/x86:5.1.1/LMY48Z/denglibo08021647:userdebug/test-keys</k34><k35>vivo v3</k35><k36>unknown</k36><k37>%s</k37><k38>x86</k38><k39>android_x86</k39><k40>%s</k40><k41>1</k41><k42>%s</k42><k43>null</k43><k44>0</k44><k45></k45><k46></k46><k47>wifi</k47><k48>%s</k48><k49>/data/data/com.tencent.mm/</k49><k52>0</k52><k53>0</k53><k57>1080</k57><k58></k58><k59>0</k59></softtype>"
#define DEVICE_INFO_DEVICEINFO				"<deviceinfo><MANUFACTURER name=\"%s\"><MODEL name=\%s\"><VERSION_RELEASE name=\"5.1.1\"><VERSION_INCREMENTAL name=\"eng.denglibo.20171224.164708\"><DISPLAY name=\"android_x86-userdebug 5.1.1 LMY48Z eng.denglibo.20171224.164708 test-keys\"></DISPLAY></VERSION_INCREMENTAL></VERSION_RELEASE></MODEL></MANUFACTURER></deviceinfo>"

#define LOGIN_RSA_VER						158
#define LOGIN_RSA_VER158_KEY_E				"30 31 30 30 30 31"
#define LOGIN_RSA_VER158_KEY_N				"45 31 36 31 44 41 30 33 44 30 42 36 41 41 44 32 31 46 39 41 34 46 42 32 37 43 33 32 41 33 32 30 38 41 46 32 35 41 37 30 37 42 42 30 45 38 45 43 45 37 39 35 30 36 46 42 42 41 46 39 37 35 31 39 44 39 37 39 34 42 37 45 31 42 34 34 44 32 43 36 46 32 35 38 38 34 39 35 43 34 45 30 34 30 33 30 33 42 34 43 39 31 35 46 31 37 32 44 44 35 35 38 41 34 39 35 35 32 37 36 32 43 42 32 38 41 42 33 30 39 43 30 38 31 35 32 41 38 43 35 35 41 34 44 46 43 36 45 41 38 30 44 31 46 34 44 38 36 30 31 39 30 41 38 45 45 32 35 31 44 46 38 44 45 43 42 39 42 30 38 33 36 37 34 44 35 36 43 44 39 35 36 46 46 36 35 32 43 33 43 37 32 34 42 39 46 30 32 42 45 35 43 37 43 42 43 36 33 46 43 30 31 32 34 41 41 32 36 30 44 38 38 39 41 37 33 45 39 31 32 39 32 42 36 41 30 32 31 32 31 44 32 35 41 41 41 37 43 31 41 38 37 37 35 32 35 37 35 43 31 38 31 46 46 42 32 35 41 36 32 38 32 37 32 35 42 30 43 33 38 41 32 41 44 35 37 36 37 36 45 30 38 38 34 46 45 32 30 43 46 35 36 32 35 36 45 31 34 35 32 39 42 43 37 45 38 32 43 44 31 46 34 41 31 31 35 35 39 38 34 35 31 32 42 44 32 37 33 44 36 38 46 37 36 39 41 46 34 36 45 31 42 30 45 33 30 35 33 38 31 36 44 33 39 45 42 31 46 30 35 38 38 33 38 34 46 32 46 34 42 32 38 36 45 35 43 46 41 46 42 34 44 30 34 33 35 42 44 46 37 44 33 41 41 38 44 33 45 30 43 34 35 37 31 36 45 41 44 31 39 30 46 44 43 36 36 38 38 34 42 32 37 35 42 41 30 38 44 38 45 44 39 34 42 31 46 38 34 45 37 37 32 39 43 32 35 42 44 30 31 34 45 37 46 41 33 41 32 33 31 32 33 45 31 30 44 33 41 39 33 42 34 31 35 34 34 35 32 44 44 42 39 45 45 35 46 38 44 41 42 36 37"