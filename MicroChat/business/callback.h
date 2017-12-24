#pragma once

extern char g_wxid[100];
extern DWORD g_dwUin;

//接收sdk回调数据
void SDKCallBack(void *result, int nCgiType, int nTaskId, int nCode);

//登录结果
void LoginCallBack(LoginResult *res);

//短信授权回调函数
void MobileVerifyCallBack(MobileVerifyResult *res);

//首次登录初始化
void NewInitCallBack(NewInitResult *res);

//同步消息回调函数(初始化好友群组列表/接收推送消息）
void NewSyncCallBack(NewSyncResult *res);

//搜索好友回调函数
void SearchContactCallBack(SearchContactResult *res);

//加好友回调函数
void AddNewFriendCallBack(VerifyUserResult *res);