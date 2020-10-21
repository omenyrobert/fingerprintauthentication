
// EHomeDemo.h : PROJECT_NAME 应用程序的主头文件
//

#pragma once

#ifndef __AFXWIN_H__
    #error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

#include "resource.h"      

#include "EHomeDemoDlg.h"
#include "GlobalDefinition.h"
#include "HCEHomeCMS.h"
#include "plaympeg4.h"
#include "HCEHomeAlarm.h"

#define CONFIG_GET_PARAM_XML "<Params>\r\n<ConfigCmd>%s</ConfigCmd>\r\n<ConfigParam1>%d</ConfigParam1>\r\n<ConfigParam2>%d</ConfigParam2>\r\n<ConfigParam3>%d</ConfigParam3>\r\n<ConfigParam4>%d</ConfigParam4>\r\n</Params>\r\n"
#define CONFIG_SET_PARAM_XML "<Params>\r\n<ConfigCmd>%s</ConfigCmd>\r\n<ConfigParam1>%d</ConfigParam1>\r\n<ConfigParam2>%d</ConfigParam2>\r\n<ConfigParam3>%d</ConfigParam3>\r\n<ConfigParam4>%d</ConfigParam4>\r\n<ConfigXML>%s</ConfigXML>\r\n</Params>\r\n"

// CEHomeDemoApp:
// 有关此类的实现，请参阅 EHomeDemo.cpp
//
BOOL CALLBACK AlarmMsgCallBack(LONG lHandle, NET_EHOME_ALARM_MSG *pAlarmMsg, void *pUser);

extern CEHomeDemoDlg *g_pMainDlg;
//extern CDlgOutput* g_pDlgPreview;
extern BOOL g_bExitDemo;
extern LOCAL_DEVICE_INFO g_struDeviceInfo[MAX_DEVICES];
extern LOCAL_PARAM g_struLocalParam;
extern LISTEN_INFO g_struPreviewListen[MAX_LISTEN_NUM];
extern int g_pCycleTimer;
extern BOOL g_bTCPLink;

class CEHomeDemoApp : public CWinAppEx
{
public:
    CEHomeDemoApp();

// 重写
    public:
    virtual BOOL InitInstance();

// 实现

    DECLARE_MESSAGE_MAP()
};

extern CEHomeDemoApp theApp;

CString IPToStr(DWORD dwIP);