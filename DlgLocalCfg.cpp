// DlgLocalCfg.cpp : implementation file
//

#include "stdafx.h"
#include "EHomeDemo.h"
#include "DlgLocalCfg.h"
#include "HCEHomeAlarm.h"
#include "DlgCmsAlarm.h"
#include "EHomeDemoDlg.h"

//#include "HCEHomeAlarm.h"


// CDlgLocalCfg dialog

IMPLEMENT_DYNAMIC(CDlgLocalCfg, CDialog)

CDlgLocalCfg::CDlgLocalCfg(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgLocalCfg::IDD, pParent)
    , m_strLogPath(_T(""))
    , m_iLogLevel(0)
    , m_strSMSIP(_T(""))
    , m_strAMSIP(_T(""))
    , m_bAutoDel(FALSE)
    , m_dwKeepLive(0)
    , m_dwCount(0)
    , m_csPicServerIP(_T(""))
    ,m_csAlarmServerIP(_T(""))
    ,m_iAlarmUdpPort(0)
    ,m_iAlarmTcpPort(0)
    ,m_iPicServerPort(0)
    , m_iAlarmType(0)
    , m_iPicType(0)
{

}

CDlgLocalCfg::~CDlgLocalCfg()
{
}

void CDlgLocalCfg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_LOG_PATH, m_strLogPath);
    DDV_MaxChars(pDX, m_strLogPath, 255);
    DDX_Text(pDX, IDC_EDIT_SMS_IP, m_strSMSIP);
    DDV_MaxChars(pDX, m_strSMSIP, 15);
    DDX_Text(pDX, IDC_EDIT_AMS_IP, m_strAMSIP);
    DDV_MaxChars(pDX, m_strAMSIP, 15);
    DDX_Check(pDX, IDC_CHK_LOG_AUTO_DEL, m_bAutoDel);
    DDX_Control(pDX, IDC_COM_CMS_ACCESS_TYPE, m_comCmsAccess);
    DDX_Control(pDX, IDC_COM_ALARM_ACCESS_TYPE, m_comAlarmAccess);
    DDX_Control(pDX, IDC_COM_STREAM_ACCESS_TYPE, m_comStreamAccess);
    DDX_Text(pDX, IDC_EDT_KEEPALIVE_INTERVAL, m_dwKeepLive);
    DDX_Text(pDX, IDC_EDT_TIME_COUNT, m_dwCount);
    DDX_Text(pDX,IDC_EDT_ALARM_SERVER_IP,m_csAlarmServerIP);
    DDX_Text(pDX,IDC_EDT_ALARM_SERVER_UDP_PORT,m_iAlarmUdpPort);
    DDX_Text(pDX,IDC_EDT_ALARM_SERVER_TCP_PORT,m_iAlarmTcpPort);
    DDX_Text(pDX, IDC_EDT_PIC_SERVER_IP, m_csPicServerIP);
    DDX_Text(pDX,IDC_EDT_PIC_SERVER_PORT,m_iPicServerPort);
    DDX_CBIndex(pDX, IDC_COM_ALARM_TYPE, m_iAlarmType);
    DDX_CBIndex(pDX, IDC_COM_PIC_TYPE, m_iPicType);
}


BEGIN_MESSAGE_MAP(CDlgLocalCfg, CDialog)
    ON_BN_CLICKED(IDC_BTN_OK, &CDlgLocalCfg::OnBnClickedBtnOk)
    ON_BN_CLICKED(IDC_BTN_GET_SECURE_ACCESS, &CDlgLocalCfg::OnBtnGetSecureAccessClicked)
    ON_BN_CLICKED(IDC_BTN_SET_SECURE_ACCESS, &CDlgLocalCfg::OnBtnSetSecureAccessClicked)
    ON_BN_CLICKED(IDC_BTN_CMS_ALARM, &CDlgLocalCfg::OnBnClickedBtnCmsAlarm)
    ON_BN_CLICKED(IDC_BTN_REG_CFG, &CDlgLocalCfg::OnBnClickedBtnRegCfg)
END_MESSAGE_MAP()


// CDlgLocalCfg message handlers

void CDlgLocalCfg::OnBnClickedBtnOk()
{
    // TODO: 在此添加控件通知处理程序代码
    UpdateData(TRUE);

    char *pPath = m_strLogPath.GetBuffer(0);
    NET_ECMS_SetLogToFile(m_iLogLevel, pPath, m_bAutoDel);
    NET_EALARM_SetLogToFile(m_iLogLevel, pPath, m_bAutoDel);
    NET_ESTREAM_SetLogToFile(m_iLogLevel, pPath, m_bAutoDel);


}

BOOL CDlgLocalCfg::OnInitDialog()
{
    m_iLogLevel = 3;
    m_strLogPath.SetString("C:/EHomeSdkLog/");
    m_bAutoDel = TRUE;
    m_dwKeepLive = g_pMainDlg->m_struServInfo.dwKeepAliveSec;
    m_iAlarmType = g_pMainDlg->m_struServInfo.dwAlarmServerType;
    m_iPicType = g_pMainDlg->m_struServInfo.dwPicServerType;
    m_iAlarmUdpPort = g_pMainDlg->m_struServInfo.struUDPAlarmSever.wPort;
    m_iAlarmTcpPort = g_pMainDlg->m_struServInfo.struTCPAlarmSever.wPort;
    m_iPicServerPort = g_pMainDlg->m_struServInfo.struPictureSever.wPort;

    m_csPicServerIP = g_pMainDlg->m_struServInfo.struPictureSever.szIP;
    m_csAlarmServerIP = g_pMainDlg->m_struServInfo.struTCPAlarmSever.szIP;

    UpdateData(FALSE);

    return CDialog::OnInitDialog();
}

void CDlgLocalCfg::OnBtnGetSecureAccessClicked()
{
    NET_EHOME_LOCAL_ACCESS_SECURITY struAccessSecure = {0};
    struAccessSecure.dwSize = sizeof(struAccessSecure);

    if (!NET_ECMS_GetSDKLocalCfg(ACTIVE_ACCESS_SECURITY, &struAccessSecure))
    {
        g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 2, "NET_ECMS_GetSDKLocalCfg, Error=[%d]", NET_ECMS_GetLastError());
    }
    else
    {
        g_pMainDlg->AddLog(0, OPERATION_SUCC_T, 1, "NET_ECMS_GetSDKLocalCfg");
        m_comCmsAccess.SetCurSel(struAccessSecure.byAccessSecurity);
    }

    memset(&struAccessSecure, 0, sizeof(struAccessSecure));
    struAccessSecure.dwSize = sizeof(struAccessSecure);
    if (!NET_EALARM_GetSDKLocalCfg(ACTIVE_ACCESS_SECURITY, &struAccessSecure))
    {
        g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 2, "NET_EALARM_GetSDKLocalCfg, Error=[%d]", NET_EALARM_GetLastError());
    }
    else
    {
        g_pMainDlg->AddLog(0, OPERATION_SUCC_T, 3, "NET_EALARM_GetSDKLocalCfg");
        m_comAlarmAccess.SetCurSel(struAccessSecure.byAccessSecurity);
    }

    memset(&struAccessSecure, 0, sizeof(struAccessSecure));
    struAccessSecure.dwSize = sizeof(struAccessSecure);
    if (!NET_ESTREAM_GetSDKLocalCfg(ACTIVE_ACCESS_SECURITY, &struAccessSecure))
    {
        g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 2, "NET_ESTREAM_GetSDKLocalCfg, Error=[%d]", NET_ESTREAM_GetLastError());
    }
    else
    {
        g_pMainDlg->AddLog(0, OPERATION_SUCC_T, 2, "NET_EALARM_GetSDKLocalCfg");
        m_comStreamAccess.SetCurSel(struAccessSecure.byAccessSecurity);
    }
}

void CDlgLocalCfg::OnBtnSetSecureAccessClicked()
{
    NET_EHOME_LOCAL_ACCESS_SECURITY struAccessSecure = {0};
    struAccessSecure.dwSize = sizeof(struAccessSecure);

    struAccessSecure.byAccessSecurity = (BYTE)m_comCmsAccess.GetCurSel();

    if (!NET_ECMS_SetSDKLocalCfg(ACTIVE_ACCESS_SECURITY, &struAccessSecure))
    {
        g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 2, "NET_ECMS_SetSDKLocalCfg, Error=[%d]", NET_ECMS_GetLastError());
    }
    else
    {
        g_pMainDlg->AddLog(0, OPERATION_SUCC_T, 1, "NET_ECMS_SetSDKLocalCfg");
    }

    struAccessSecure.byAccessSecurity = (BYTE)m_comAlarmAccess.GetCurSel();

    if (!NET_EALARM_SetSDKLocalCfg(ACTIVE_ACCESS_SECURITY, &struAccessSecure))
    {
        g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 2, "NET_EALARM_SetSDKLocalCfg, Error=[%d]", NET_EALARM_GetLastError());
    }
    else
    {
        g_pMainDlg->AddLog(0, OPERATION_SUCC_T, 3, "NET_EALARM_SetSDKLocalCfg");
    }

    struAccessSecure.byAccessSecurity = (BYTE)m_comStreamAccess.GetCurSel();
    if (!NET_ESTREAM_SetSDKLocalCfg(ACTIVE_ACCESS_SECURITY, &struAccessSecure))
    {
        g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 2, "NET_ESTREAM_SetSDKLocalCfg, Error=[%d]", NET_ESTREAM_GetLastError());
    }
    else
    {
        g_pMainDlg->AddLog(0, OPERATION_SUCC_T, 2, "NET_ESTREAM_SetSDKLocalCfg");
    }
}


void CDlgLocalCfg::OnBnClickedBtnCmsAlarm()
{
    // TODO:  在此添加控件通知处理程序代码
    CDlgCmsAlarm dlg;
    dlg.DoModal();
}

void CDlgLocalCfg::OnBnClickedBtnRegCfg()
{
    // TODO: 在此添加控件通知处理程序代码
    UpdateData(TRUE);
    memset(&g_pMainDlg->m_struServInfo, 0, sizeof(g_pMainDlg->m_struServInfo));
    g_pMainDlg->m_struServInfo.dwKeepAliveSec = m_dwKeepLive;
    g_pMainDlg->m_struServInfo.dwTimeOutCount = m_dwCount;
    if(m_iAlarmType == 0)
    {
        memcpy(g_pMainDlg->m_struServInfo.struUDPAlarmSever.szIP,m_csAlarmServerIP.GetBuffer(0),m_csAlarmServerIP.GetLength());
        g_pMainDlg->m_struServInfo.struUDPAlarmSever.wPort = m_iAlarmUdpPort;
        NET_EALARM_StopListen(g_pMainDlg->m_lUdpAlarmHandle);
        NET_EHOME_ALARM_LISTEN_PARAM struAlarmListenParam = { 0 };
        memcpy(struAlarmListenParam.struAddress.szIP, m_csAlarmServerIP.GetBuffer(0), m_csAlarmServerIP.GetLength());

        struAlarmListenParam.struAddress.wPort = m_iAlarmUdpPort;
        struAlarmListenParam.fnMsgCb = AlarmMsgCallBack;
        struAlarmListenParam.pUserData = this;
        struAlarmListenParam.byProtocolType = 1;
        struAlarmListenParam.byUseThreadPool = 0;
        g_pMainDlg->m_lUdpAlarmHandle = NET_EALARM_StartListen(&struAlarmListenParam);
        if (-1 == g_pMainDlg->m_lUdpAlarmHandle)
        {
            g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 3, "SNET_EALARM_StartListen Failed");
        }
        else
        {
            g_pMainDlg->AddLog(0, OPERATION_SUCC_T, 3, "SNET_EALARM_StartListen succ");
        }
    }
    else if(m_iAlarmType == 1)
    {
        memcpy(g_pMainDlg->m_struServInfo.struUDPAlarmSever.szIP,m_csAlarmServerIP.GetBuffer(0),m_csAlarmServerIP.GetLength());
        g_pMainDlg->m_struServInfo.struUDPAlarmSever.wPort = m_iAlarmUdpPort;
        memcpy(g_pMainDlg->m_struServInfo.struTCPAlarmSever.szIP,m_csAlarmServerIP.GetBuffer(0),m_csAlarmServerIP.GetLength());
        g_pMainDlg->m_struServInfo.struTCPAlarmSever.wPort = m_iAlarmTcpPort;
        NET_EALARM_StopListen(g_pMainDlg->m_lUdpAlarmHandle);
        NET_EALARM_StopListen(g_pMainDlg->m_lAlarmHandle);

        NET_EHOME_ALARM_LISTEN_PARAM struAlarmListenParam = { 0 };
        memcpy(struAlarmListenParam.struAddress.szIP, m_csAlarmServerIP.GetBuffer(0), m_csAlarmServerIP.GetLength());

        struAlarmListenParam.struAddress.wPort = m_iAlarmTcpPort;
        struAlarmListenParam.fnMsgCb = AlarmMsgCallBack;
        struAlarmListenParam.pUserData = this;
        struAlarmListenParam.byProtocolType = 0;
        struAlarmListenParam.byUseThreadPool = 0;

        g_pMainDlg->m_lAlarmHandle = NET_EALARM_StartListen(&struAlarmListenParam);
        if (-1 == g_pMainDlg->m_lAlarmHandle)
        {
            g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 3, "SNET_EALARM_StartListen Failed");
        }
        else
        {
            g_pMainDlg->AddLog(0, OPERATION_SUCC_T, 3, "SNET_EALARM_StartListen succ");
        }

        struAlarmListenParam.struAddress.wPort = m_iAlarmUdpPort;
        struAlarmListenParam.fnMsgCb = AlarmMsgCallBack;
        struAlarmListenParam.pUserData = this;
        struAlarmListenParam.byProtocolType = 1;
        struAlarmListenParam.byUseThreadPool = 0;

        g_pMainDlg->m_lUdpAlarmHandle = NET_EALARM_StartListen(&struAlarmListenParam);
        if (-1 == g_pMainDlg->m_lUdpAlarmHandle)
        {
            g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 3, "SNET_EALARM_StartListen Failed");
        }
        else
        {
            g_pMainDlg->AddLog(0, OPERATION_SUCC_T, 3, "SNET_EALARM_StartListen succ");
        }

    }

    memcpy(g_pMainDlg->m_struServInfo.struPictureSever.szIP,m_csPicServerIP.GetBuffer(0),m_csPicServerIP.GetLength());
    g_pMainDlg->m_struServInfo.struPictureSever.wPort = m_iPicServerPort;
    g_pMainDlg->m_struServInfo.dwPicServerType = m_iPicType;


}
