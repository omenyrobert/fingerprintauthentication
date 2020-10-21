
// EHomeDemoDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "EHomeDemo.h"
#include "EHomeDemoDlg.h"
#include "math.h"

#include "HCEHomeCMS.h"
#include "HCEHomeAlarm.h"
#include "DlgListen.h"
#include "DlgChanInfo.h"
#include "DlgPassthroughProxy.h"
#include "DlgGpsInfo.h"
#include "DlgWirelessInfoPara.h"
#include "Public/tinyXML/XmlBase.h"
#include "Public/Convert/Convert.h"


#include "DlgUpgradeHttp.h"
#include "DlgISAPIConfig.h"
#include "DlgISAPIPassthrough.h"
#include "DlgUpgrade.h"
#include "DlgMotionConfig.h"
#include "DlgRecordCfg.h"
#include "DlgCaptureCfg.h"
#include "DlgMotionArea.h"
#include "DlgPrivateArea.h"
#include "DlgHidArea.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#pragma warning (disable: 4996)

void CALLBACK fnPassthroughDataCb(DWORD dwProxyType, LONG lLisetenHandle, void* pDeviceID, DWORD dwDevIDLen, void* pDataBuffer, DWORD dwDataLen, void* pUser)
{
    UN_REFERENCED_PARAMETER(dwProxyType)
    UN_REFERENCED_PARAMETER(lLisetenHandle)
    UN_REFERENCED_PARAMETER(dwDevIDLen)
    UN_REFERENCED_PARAMETER(pDataBuffer)
    UN_REFERENCED_PARAMETER(dwDataLen)
    UN_REFERENCED_PARAMETER(pUser)
	char sz[1024] = {0};
	sprintf(sz, "deviceID[%s]", pDeviceID);
	return;
}


//CDlgOutput* m_pDlgPreview;
LOCAL_DEVICE_INFO g_struDeviceInfo[MAX_DEVICES];
CEHomeDemoDlg *g_pMainDlg = NULL;
BOOL g_bExitDemo = FALSE;//control post message when exit, so can release the buffer.
LOCAL_PARAM g_struLocalParam;        //demo local configure
LISTEN_INFO g_struPreviewListen[MAX_LISTEN_NUM];
int g_pCycleTimer;
BOOL g_bTCPLink = TRUE;
LONG g_lCmsAlarm = -1;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
int g_iCurScreenWidth = 0;        //current screen width
int g_iCurScreenHeight = 0;       //current screen height


void g_StringLanType(char *szDstLan, char *szLanCn, char *szLanEn);


/*********************************************************
  Function:    g_StringLanType
  Desc:        get the current operation language string type
  Input:    szLanCn, Chinese string; szLanEn, English string;
  Output:    szDstLan, current string
  Return:    none
**********************************************************/
void g_StringLanType(char *szDstLan, char *szLanCn, char *szLanEn)
{
        UN_REFERENCED_PARAMETER(szLanEn)
#ifdef DEMO_LAN_CN
        sprintf(szDstLan, "%s", szLanCn);    
#else    
        sprintf(szDstLan, "%s", szLanEn);
#endif    
}

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// 对话框数据
    enum { IDD = IDD_ABOUTBOX };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CEHomeDemoDlg 对话框

BOOL _stdcall EHOME_REGISTER(LONG iUserID, DWORD dwDataType, void *pOutBuffer, DWORD dwOutLen, void *pInBuffer, DWORD dwInLen, void *pUser)
{
    UN_REFERENCED_PARAMETER(dwInLen)
    UN_REFERENCED_PARAMETER(dwOutLen)
    CEHomeDemoDlg *pDlg = static_cast<CEHomeDemoDlg *>(pUser);

    if (NULL == pDlg || g_bExitDemo == TRUE)
    {
        return FALSE;
    }

    if (ENUM_DEV_ON == dwDataType)
    {
        if (pInBuffer == NULL)
        {
            return FALSE;
        }
        NET_EHOME_SERVER_INFO *pServInfo = (NET_EHOME_SERVER_INFO *)pInBuffer;

        memcpy(pServInfo, &pDlg->m_struServInfo, sizeof(pDlg->m_struServInfo));
        /*
        pServInfo->dwKeepAliveSec = 15;//PU的保活间隔
        LOCAL_DEVICE_INFO *lpTemp = new LOCAL_DEVICE_INFO();
        NET_EHOME_DEV_REG_INFO_V12 *pDevInfo = (NET_EHOME_DEV_REG_INFO_V12 *)pOutBuffer;
        sprintf(pServInfo->struUDPAlarmSever.szIP, "%s", pDlg->m_sLocalIP);
        pServInfo->struUDPAlarmSever.wPort = 7660;
        //sprintf(pServInfo->struTCPAlarmSever.szIP, "%s", pDlg->m_sLocalIP);
        //pServInfo->struTCPAlarmSever.wPort = 7331;
        */
        LOCAL_DEVICE_INFO *lpTemp = new LOCAL_DEVICE_INFO();
        NET_EHOME_DEV_REG_INFO_V12 *pDevInfo = (NET_EHOME_DEV_REG_INFO_V12 *)pOutBuffer;
        //UTF-8转GBK
        DWORD dwOutLen = 0;
        UTF82A((char*)pDevInfo->struRegInfo.byDeviceID, (char*)pDevInfo->struRegInfo.byDeviceID, MAX_DEVICE_ID_LEN, &dwOutLen);
        UTF82A((char*)pDevInfo->struRegInfo.sDeviceSerial, (char*)pDevInfo->struRegInfo.sDeviceSerial, NET_EHOME_SERIAL_LEN, &dwOutLen);

        if (pDevInfo != NULL)
        {
            memcpy(lpTemp->byDeviceID, pDevInfo->struRegInfo.byDeviceID, MAX_DEVICE_ID_LEN);
            lpTemp->lLoginID = iUserID;
            memcpy(lpTemp->sDeviceSerial, pDevInfo->struRegInfo.sDeviceSerial, NET_EHOME_SERIAL_LEN);
            //memcpy(lpTemp->sIdentifyCode, pDevInfo->struRegInfo.sIdentifyCode, CODE_LEN);
        }

        memcpy(lpTemp->byDeviceID, pDevInfo->struRegInfo.byDeviceID, MAX_DEVICE_ID_LEN);
        memcpy(lpTemp->sDeviceSerial, pDevInfo->struRegInfo.sDeviceSerial, NET_EHOME_SERIAL_LEN);
        lpTemp->lLoginID = iUserID;

        //先拷贝到一个大的缓冲区，在获取长度
        char szDeviceSerial[NET_EHOME_SERIAL_LEN+1] = {0};
        memcpy(szDeviceSerial, pDevInfo->struRegInfo.sDeviceSerial, NET_EHOME_SERIAL_LEN);
        if (strlen(szDeviceSerial) < 0)
        {
            lpTemp->dwVersion = 2;
        }
        else
        {
            lpTemp->dwVersion = 4;
        }

        //在这里进行设备的验证，只有序列号在m_stAccessDeviceList中的设备才会允许注册

        if (pDlg->m_bUseAccessList)
        {
            for (int i = 0; i < 64; i++)
            {
                if (lpTemp->dwVersion >= 4)
                {
                    if (strcmp((char*)pDevInfo->struRegInfo.sDeviceSerial, pDlg->m_stAccessDeviceList[i].sSerialNumber) == 0)
                    {
                        //允许注册
                        ::PostMessage(pDlg->m_hWnd, WM_ADD_DEV, NULL, (LPARAM)lpTemp);
                        return TRUE;
                    }
                }
                else
                {
                    if (strcmp((char*)pDevInfo->struRegInfo.byDeviceID, pDlg->m_stAccessDeviceList[i].sSerialNumber) == 0)
                    {
                        //允许注册
                        ::PostMessage(pDlg->m_hWnd, WM_ADD_DEV, NULL, (LPARAM)lpTemp);
                        return TRUE;
                    }
                }
            }
        }
        else
        {
            //允许注册
            ::PostMessage(pDlg->m_hWnd, WM_ADD_DEV, NULL, (LPARAM)lpTemp);
            return TRUE;
        }

        //不允许注册
        return FALSE;
    }
    else if (ENUM_DEV_OFF == dwDataType)
    {
        ::PostMessage(pDlg->m_hWnd, WM_DEL_DEV, NULL, (LPARAM)iUserID); 
    }
    else if (ENUM_DEV_ADDRESS_CHANGED == dwDataType)
    {
        //先把原来的设备删了吧
        ::PostMessage(pDlg->m_hWnd, WM_CHANGE_IP, NULL, (LPARAM)iUserID);

        g_pMainDlg->AddLog(iUserID, OPERATION_SUCC_T, 0, "设备IP发生变化后，重新注册!");

        LOCAL_DEVICE_INFO *lpTemp = new LOCAL_DEVICE_INFO();
        NET_EHOME_DEV_REG_INFO_V12 *pDevInfo = (NET_EHOME_DEV_REG_INFO_V12 *)pOutBuffer;

        //UTF-8转GBK
        DWORD dwOutLen = 0;
        UTF82A((char*)pDevInfo->struRegInfo.byDeviceID, (char*)pDevInfo->struRegInfo.byDeviceID, MAX_DEVICE_ID_LEN, &dwOutLen);
        UTF82A((char*)pDevInfo->struRegInfo.sDeviceSerial, (char*)pDevInfo->struRegInfo.sDeviceSerial, NET_EHOME_SERIAL_LEN, &dwOutLen);

        if (pDevInfo != NULL)
        {
            memcpy(lpTemp->byDeviceID, pDevInfo->struRegInfo.byDeviceID, MAX_DEVICE_ID_LEN);
            lpTemp->lLoginID = iUserID;
            memcpy(lpTemp->sDeviceSerial, pDevInfo->struRegInfo.sDeviceSerial, NET_EHOME_SERIAL_LEN);
            //memcpy(lpTemp->sIdentifyCode, pDevInfo->struRegInfo.sIdentifyCode, CODE_LEN);
        }

        memcpy(lpTemp->byDeviceID, pDevInfo->struRegInfo.byDeviceID, MAX_DEVICE_ID_LEN);
        memcpy(lpTemp->sDeviceSerial, pDevInfo->struRegInfo.sDeviceSerial, NET_EHOME_SERIAL_LEN);
        lpTemp->lLoginID = iUserID;

        //先拷贝到一个大的缓冲区，在获取长度
        char szDeviceSerial[NET_EHOME_SERIAL_LEN+1] = {0};
        memcpy(szDeviceSerial, pDevInfo->struRegInfo.sDeviceSerial, NET_EHOME_SERIAL_LEN);
        if (strlen(szDeviceSerial) < 0)
        {
            lpTemp->dwVersion = 2;
        }
        else
        {
            lpTemp->dwVersion = 4;
        }


        //在这里进行设备的验证，只有序列号在m_stAccessDeviceList中的设备才会允许注册

        if (pDlg->m_bUseAccessList)
        {
            for (int i = 0; i < 64; i++)
            {
                if (lpTemp->dwVersion >= 4)
                {
                    if (strcmp((char*)pDevInfo->struRegInfo.sDeviceSerial, pDlg->m_stAccessDeviceList[i].sSerialNumber) == 0)
                    {
                        //允许注册
                        ::PostMessage(pDlg->m_hWnd, WM_ADD_DEV, NULL, (LPARAM)lpTemp);
                        return TRUE;
                    }
                }
                else
                {
                    if (strcmp((char*)pDevInfo->struRegInfo.byDeviceID, pDlg->m_stAccessDeviceList[i].sSerialNumber) == 0)
                    {
                        //允许注册
                        ::PostMessage(pDlg->m_hWnd, WM_ADD_DEV, NULL, (LPARAM)lpTemp);
                        return TRUE;
                    }
                }
            }
        }
        else
        {
            //允许注册
            ::PostMessage(pDlg->m_hWnd, WM_ADD_DEV, NULL, (LPARAM)lpTemp);
            return TRUE;
        }

        //不允许注册
        return FALSE;
    }

    return TRUE;
}


BOOL CEHomeDemoDlg::IsInAccessDeviceList(char* pSerialNumber)
{
    if (pSerialNumber == NULL)
    {
        return FALSE;
    }
    for (int i = 0; i < 64; i++)
    {
        if (strcmp((char*)pSerialNumber,m_stAccessDeviceList[i].sSerialNumber) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CALLBACK AlarmMsgCallBack(LONG lHandle, NET_EHOME_ALARM_MSG *pAlarmMsg, void *pUser)
{
    UN_REFERENCED_PARAMETER(lHandle)
    CEHomeDemoDlg *pDlg = static_cast<CEHomeDemoDlg *>(pUser);

    if (NULL == pDlg)
    {
        return FALSE;
    }

    //在这里根据设备的序列号，进行Token认证
    if (pDlg->m_bUseAccessList && !pDlg->IsInAccessDeviceList(pAlarmMsg->sSerialNumber))
    {
        return FALSE;
    }

    pDlg->ProcessAlarmData(pAlarmMsg->dwAlarmType, pAlarmMsg->pAlarmInfo, pAlarmMsg->dwAlarmInfoLen, pAlarmMsg->pXmlBuf, pAlarmMsg->dwXmlBufLen);

    return TRUE;
}


CEHomeDemoDlg::CEHomeDemoDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CEHomeDemoDlg::IDD, pParent)
, m_iSelListType(0)
, m_bListLogEnlarge(FALSE)
, m_bListAlarmEnlarge(FALSE)
, m_iMainType(PREVIEW_T)
, m_iCurWndNum(4)
, m_iCurWndIndex(0)
, m_lAlarmHandle(-1)
, m_lUdpAlarmHandle(-1)
, m_iCurDeviceIndex(-1)
, m_iCurChanIndex(-1)
, m_bCyclePreview(FALSE)
, m_iPreviewHandle(0)
, m_bRecord(FALSE)
, m_bSound(FALSE)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

    //清空可接入设备列表
    memset(&m_stAccessDeviceList, 0, sizeof(m_stAccessDeviceList));
    memset(&struServInfo,0,sizeof(struServInfo));

    m_bUseAccessList = FALSE; //不启用可接入设备列表

    //初始化可接入设备列表（不在该列表中的设备不允许注册）
    //目前只允许设备序列号为ESimulator0000的设备接入
    //memcpy(m_stAccessDeviceList[0].sSerialNumber, "哈-00001", 12);
    //memcpy(m_stAccessDeviceList[1].sSerialNumber, "哈-00001", 12);

    memset(&m_struServInfo, 0, sizeof(m_struServInfo));
    memset(m_sLocalIP, 0, sizeof(m_sLocalIP));
    m_byCmsSecureAccessType = 0;
    m_byAlarmSecureAccessType = 0;
    m_byStreamSecureAccessType = 0;
}

CEHomeDemoDlg::~CEHomeDemoDlg()
{
    //停止预览监听后，才能把播放窗口释放
    StopPreviewListen();
    if(m_pDlgPreview != NULL)
    {
        delete[] m_pDlgPreview;
        m_pDlgPreview = NULL;
    }
    if (m_dlgPlayBack != NULL)
    {
        delete m_dlgPlayBack;
        m_dlgPlayBack = NULL;
    }

    if (m_dlgAudioTalk != NULL)
    {
        delete m_dlgAudioTalk;
        m_dlgAudioTalk = NULL;
    }

    if (m_dlgCommonCfg != NULL)
    {
        delete m_dlgCommonCfg;
        m_dlgCommonCfg = NULL;
    }

    if (m_dlgLocalCfg != NULL)
    {
        delete m_dlgLocalCfg;
        m_dlgLocalCfg = NULL;
    }

    if (m_dlgOutputCtrl != NULL)
    {
        delete m_dlgOutputCtrl;
        m_dlgOutputCtrl = NULL;
    }

    NET_EHOME_AMS_ADDRESS struAmsAddr = {0};
    struAmsAddr.dwSize = sizeof(struAmsAddr);
    struAmsAddr.byEnable = 2;
    //CMS停止接收
    NET_ECMS_SetSDKLocalCfg(AMS_ADDRESS, &struAmsAddr);
    NET_EALARM_StopListen(m_lUdpAlarmHandle);
    NET_EALARM_StopListen(m_lAlarmHandle);
    NET_EALARM_StopListen(m_lCmsAlarm);
    NET_EALARM_Fini();
    NET_ECMS_StopListen(0);
    NET_ECMS_Fini();
    NET_ESTREAM_Fini();
}
void CEHomeDemoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_WNDNUM, m_comboWinNum);
    DDX_Control(pDX, IDC_TREE_DEV, m_treeDeviceList);
    DDX_Control(pDX, IDC_LIST_ALL_LOG, m_listAllLog);
    DDX_Control(pDX, IDC_LIST_ALARM_INFO, m_listAlarmInfo);
    DDX_Text(pDX, IDC_EDIT_PREVIEW_HANDLE, m_iPreviewHandle);
}

BEGIN_MESSAGE_MAP(CEHomeDemoDlg, CDialog)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_RADIO_ALARM_INFO, &CEHomeDemoDlg::OnBnClickedRadioAlarmInfo)
    ON_BN_CLICKED(IDC_RADIO_LOCAL_LOG, &CEHomeDemoDlg::OnBnClickedRadioLocalLog)
    ON_CBN_SELCHANGE(IDC_COMBO_WNDNUM, &CEHomeDemoDlg::OnCbnSelchangeComboWndnum)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_ALL_LOG, &CEHomeDemoDlg::OnLvnColumnclickListAllLog)
    ON_BN_CLICKED(IDC_BTN_PLAYBACK, &CEHomeDemoDlg::OnBnClickedBtnPlayback)
    ON_BN_CLICKED(IDC_BTN_PREVIEW, &CEHomeDemoDlg::OnBnClickedBtnPreview)
    ON_BN_CLICKED(IDC_BTN_AUDIO_TALK, &CEHomeDemoDlg::OnBnClickedBtnAudioTalk)
    ON_BN_CLICKED(IDC_BTN_CFG, &CEHomeDemoDlg::OnBnClickedBtnCfg)
    ON_BN_CLICKED(IDC_BTN_LOCAL_CFG, &CEHomeDemoDlg::OnBnClickedBtnLocalCfg)
    ON_BN_CLICKED(IDC_BTN_EXIT, &CEHomeDemoDlg::OnBnClickedBtnExit)
    ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_ALARM_INFO, &CEHomeDemoDlg::OnLvnColumnclickListAlarmInfo)
    ON_MESSAGE(WM_ADD_LOG, OnWMAddLog)
    ON_MESSAGE(WM_ADD_DEV, OnWMAddDev)
    ON_MESSAGE(WM_DEL_DEV, OnWMDelDev)
    ON_MESSAGE(WM_PROC_EXCEPTION, OnWMProcException)
    ON_MESSAGE(WM_CHANGE_CHANNEL_ITEM_IMAGE,ChangeChannelItemImage)
    ON_MESSAGE(WM_CHANGE_IP, OnWMChangeIPAddr)

    //    ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_DEV, &CEHomeDemoDlg::OnTvnSelchangedTreeDev)
    ON_NOTIFY(NM_CLICK, IDC_TREE_DEV, &CEHomeDemoDlg::OnNMClickTreeDev)
    ON_NOTIFY(NM_DBLCLK, IDC_TREE_DEV, &CEHomeDemoDlg::OnNMDblclkTreeDev)

    ON_BN_CLICKED(IDC_BTN_OTHER, &CEHomeDemoDlg::OnBnClickedBtnOther)
    ON_BN_CLICKED(IDC_BTN_PREVIEW_LISTEN, &CEHomeDemoDlg::OnBnClickedBtnPreviewListen)
    ON_NOTIFY(NM_RCLICK, IDC_TREE_DEV, &CEHomeDemoDlg::OnNMRClickTreeDev)
    ON_BN_CLICKED(IDC_BTN_CIRCLE_PREVIEW, &CEHomeDemoDlg::OnBnClickedBtnCirclePreview)
    ON_WM_TIMER()
    ON_WM_HSCROLL()
    ON_BN_CLICKED(IDC_BTN_CLOSE_PREVIEW, &CEHomeDemoDlg::OnBnClickedBtnClosePreview)
    ON_COMMAND(ID_CHANNEL_INFO, &CEHomeDemoDlg::OnMenuChannelInfo)
    ON_COMMAND(ID_MENU_PROXY, &CEHomeDemoDlg::OnMenuProxy)
    ON_COMMAND(ID_MENU_GPS_INFO, &CEHomeDemoDlg::OnGetGpsInfo)
    ON_COMMAND(ID_MENU_WIRELESS_INFO, &CEHomeDemoDlg::OnBtnWirelessInfo)
    ON_COMMAND(ID_MENU_ISAPI_PT, &CEHomeDemoDlg::OnMenuIsapiPt)
    ON_BN_CLICKED(IDC_BTN_SALVE, &CEHomeDemoDlg::OnBnClickedBtnSalve)
    ON_COMMAND(ID_MENU_UPGRADE, &CEHomeDemoDlg::OnMenuUpgrade)
    ON_BN_CLICKED(IDC_BTN_SOUND, &CEHomeDemoDlg::OnBnClickedBtnSound)
    ON_COMMAND(ID_ENABLE_CFG, &CEHomeDemoDlg::OnEnableCfg)
    ON_COMMAND(ID_RECODR_CFG, &CEHomeDemoDlg::OnRecodrCfg)
    ON_COMMAND(ID_CAPTURE_PIC, &CEHomeDemoDlg::OnCapturePic)
    ON_COMMAND(ID_MONTION_AREA, &CEHomeDemoDlg::OnMontionArea)
    ON_COMMAND(ID_PRIVATE_AREA, &CEHomeDemoDlg::OnPrivateArea)
    ON_COMMAND(ID_HIDE_ALARM, &CEHomeDemoDlg::OnHideAlarm)
    ON_COMMAND(ID_MENU_HTTP_UPGRADE, &CEHomeDemoDlg::OnMenuHttpUpgrade)
    ON_COMMAND(ID_MENU_ISAPI_CFG, &CEHomeDemoDlg::OnMenuIsapiCfg)
END_MESSAGE_MAP()


// CEHomeDemoDlg 消息处理程序

BOOL CEHomeDemoDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // 将“关于...”菜单项添加到系统菜单中。

    // IDM_ABOUTBOX 必须在系统命令范围内。
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
    //  执行此操作
    SetIcon(m_hIcon, TRUE);            // 设置大图标
    SetIcon(m_hIcon, FALSE);        // 设置小图标

    //ShowWindow(SW_MINIMIZE);

    // TODO: 在此添加额外的初始化代码
    g_pMainDlg = this;


    GetLocalIP((char*)m_sLocalIP);
    InitChildWindow();
    InitMainWin();    
    LoadTreeImage();
    CreateTree();
    InitParamFromXML();


    InitLib();

    InitPreviewListenParam();
    StartPreviewListen();

    CRect rtDesk;
    CRect rtDlg;

    ::GetWindowRect(::GetDesktopWindow(), &rtDesk);
    GetWindowRect(&rtDlg);

    int iXpos = rtDesk.Width() / 2 - rtDlg.Width() / 2;
    int iYpos = rtDesk.Height() / 2 - rtDlg.Height() / 2;

    SetWindowPos(NULL, iXpos, iYpos, 0, 0, SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
    //get current system resolution
    g_iCurScreenWidth = GetSystemMetrics(SM_CXSCREEN);
    g_iCurScreenHeight = GetSystemMetrics(SM_CYSCREEN);
    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CEHomeDemoDlg::SetAddr(CString csIP, int nPort)
{
    strncpy(m_sLocalIP, csIP.GetBuffer(), csIP.GetLength());
    m_nPort = nPort;
}

void CEHomeDemoDlg::InitParamFromXML()
{
    CXmlBase   struXMLBase;
    struXMLBase.LoadFile(".\\EHomeDemo.xml");

    if (struXMLBase.FindElem("LocalCfg") && struXMLBase.IntoElem())
    {
        if (struXMLBase.FindElem("Param") && struXMLBase.IntoElem())
        {
            ConvertSingleNodeData(&m_struServInfo.dwKeepAliveSec, struXMLBase, "KeepAliveSeconds", NODE_STRING_TO_INT);
            ConvertSingleNodeData(m_struServInfo.struTCPAlarmSever.szIP, struXMLBase, "AlarmServerIP", NODE_STRING_TO_ARRAY, 128);
            memcpy(m_struServInfo.struUDPAlarmSever.szIP, m_struServInfo.struTCPAlarmSever.szIP, 128);
            ConvertSingleNodeData(&m_struServInfo.struUDPAlarmSever.wPort, struXMLBase, "AlarmServerPort", NODE_STRING_TO_WORD);
            ConvertSingleNodeData(&m_struServInfo.dwAlarmServerType, struXMLBase, "AlarmServerType", NODE_STRING_TO_INT);
            ConvertSingleNodeData(&m_struServInfo.struTCPAlarmSever.wPort, struXMLBase, "AlarmServerTcpPort", NODE_STRING_TO_WORD);
            ConvertSingleNodeData(m_struServInfo.struNTPSever.szIP, struXMLBase, "NTPServerIP", NODE_STRING_TO_ARRAY, 128);
            ConvertSingleNodeData(&m_struServInfo.struNTPSever.wPort, struXMLBase, "NTPServerPort", NODE_STRING_TO_WORD);
            ConvertSingleNodeData(&m_struServInfo.dwNTPInterval, struXMLBase, "NTPInterval", NODE_STRING_TO_INT);
            ConvertSingleNodeData(m_struServInfo.struPictureSever.szIP, struXMLBase, "PictureServer", NODE_STRING_TO_ARRAY, 128);
            ConvertSingleNodeData(&m_struServInfo.struPictureSever.wPort, struXMLBase, "PictureServerPort", NODE_STRING_TO_WORD);

            ConvertSingleNodeData(&m_byCmsSecureAccessType, struXMLBase, "CmsAccessSecurity", NODE_STRING_TO_BYTE);
            ConvertSingleNodeData(&m_byAlarmSecureAccessType, struXMLBase, "AlarmAccessSecurity", NODE_STRING_TO_BYTE);
            ConvertSingleNodeData(&m_byStreamSecureAccessType, struXMLBase, "StreamAccessSecurity", NODE_STRING_TO_BYTE);

            struXMLBase.OutOfElem();
        }

        struXMLBase.OutOfElem();
    }
}

void CEHomeDemoDlg::InitPreviewListenParam()
{
    int i = 0;
    //char szLocalIP[128] = {0};
    //GetLocalIP(szLocalIP);
    for (i=0; i<MAX_LISTEN_NUM; i++)
    {
        g_struPreviewListen[i].lHandle = -1;
        memcpy(g_struPreviewListen[i].struIP.szIP, m_sLocalIP, 128);
    }
    //开启预览监听
    g_struPreviewListen[0].struIP.wPort = 8000;
    g_struPreviewListen[0].iLinkType = 0;


    NET_EHOME_LISTEN_PREVIEW_CFG struListen = { 0 };
    memcpy(&struListen.struIPAdress, &g_struPreviewListen[0].struIP, sizeof(NET_EHOME_IPADDRESS));
    struListen.fnNewLinkCB = fnPREVIEW_NEWLINK_CB;
    struListen.pUser = g_pMainDlg;
    struListen.byLinkMode = 0;

    LONG lHandle = NET_ESTREAM_StartListenPreview(&struListen);
    if (lHandle > -1)
    {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_SUCC_T, 2, "NET_ESTREAM_StartListenPreview");
        g_struPreviewListen[0].lHandle = lHandle;
        // GetDlgItem(IDC_BTN_START_LISTEN)->EnableWindow(FALSE);
        // GetDlgItem(IDC_BTN_STOP_LISTEN)->EnableWindow(TRUE);
    }
    else
    {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_FAIL_T, 2, "NET_ESTREAM_StartListenPreview");
    }
}

void CEHomeDemoDlg::StopPreviewListen()
{
    int i = 0;
    for (i=0; i<MAX_LISTEN_NUM; i++)
    {
        if(g_struPreviewListen[i].lHandle != -1)
        {
            NET_ESTREAM_StopListenPreview(g_struPreviewListen[i].lHandle);
            g_struPreviewListen[i].lHandle = -1;
        }
    }
}

void CALLBACK CEHomeDemoDlg::fnPREVIEW_DATA_CB( LONG  iPreviewHandle, NET_EHOME_PREVIEW_CB_MSG *pPreviewCBMsg, void *pUserData)
{
    UN_REFERENCED_PARAMETER(iPreviewHandle)
    //int iWinIndex = *(int*)pUserData;
    int iWinIndex = (int)pUserData;
    if (NULL == pPreviewCBMsg)
    {
        return ;
    }
    g_pMainDlg->m_pDlgPreview[iWinIndex].InputStreamData(pPreviewCBMsg->byDataType, (char*)pPreviewCBMsg->pRecvdata, pPreviewCBMsg->dwDataLen);
}

BOOL CALLBACK CEHomeDemoDlg::fnPREVIEW_NEWLINK_CB(LONG lPreviewHandle,NET_EHOME_NEWLINK_CB_MSG *pNewLinkCBMsg, void *pUserData)
{
    //需要将字符串字段转换成GB2312
    DWORD dwConvertLen = 0;
    UTF82A((char*)pNewLinkCBMsg->szDeviceID, (char*)pNewLinkCBMsg->szDeviceID, MAX_DEVICE_ID_LEN, &dwConvertLen);

    CEHomeDemoDlg *pThis = (CEHomeDemoDlg*)pUserData;
    NET_EHOME_PREVIEW_DATA_CB_PARAM struDataCB = {0};
    struDataCB.fnPreviewDataCB = fnPREVIEW_DATA_CB;
    struDataCB.pUserData = (void*)g_pMainDlg->m_iCurWndIndex;

    if(!g_pMainDlg->m_pDlgPreview[g_pMainDlg->m_iCurWndIndex].SetPlayParam(lPreviewHandle, g_pMainDlg->m_iCurWndIndex, pNewLinkCBMsg->iSessionID))
    {
        g_pMainDlg->AddLog(g_pMainDlg->m_iCurDeviceIndex, OPERATION_FAIL_T, 2, "SetPlayParam failed. lPreviewHandle[%d],\
                                                                              m_iCurWndIndex[%d] ", lPreviewHandle, g_pMainDlg->m_iCurWndIndex);
        return FALSE;
    }
    //m_pDlgPreview[g_pMainDlg->m_iCurWndIndex].m_lPlayHandle = lPreviewHandle;
    if (!NET_ESTREAM_SetPreviewDataCB(lPreviewHandle, &struDataCB))
    {
        g_pMainDlg->AddLog(g_pMainDlg->m_iCurDeviceIndex, OPERATION_FAIL_T, 2, "stream handle = %d", lPreviewHandle);
        //出问题先崩了吧，不然问题不好查
        char *p = NULL;
        *p = 1;
        return FALSE;
    }
    g_pMainDlg->AddLog(g_pMainDlg->m_iCurDeviceIndex, OPERATION_SUCC_T, 2, "stream handle = %d", lPreviewHandle);
    g_pMainDlg->AddLog(g_pMainDlg->m_iCurDeviceIndex, OPERATION_SUCC_T, 2, "Device ID = [%s]", pNewLinkCBMsg->szDeviceID);
    SetEvent(g_pMainDlg->m_pDlgPreview[g_pMainDlg->m_iCurWndIndex].m_hPlayEvent);
    if (pThis->m_bCyclePreview)
    {
        g_pMainDlg->m_iCurWndIndex++;
    }
    return TRUE;
}

void CALLBACK CEHomeDemoDlg::fnPREVIEW_EXCEPTION_CB(DWORD dwType, LONG iUserID, LONG iHandle, void* pUser)
{
        UN_REFERENCED_PARAMETER(pUser)
    if(EHOME_PREVIEW_EXCEPTION == dwType)
    {
        g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 2, "Preview exception, handle=%d", iHandle);
        ::PostMessage(g_pMainDlg->m_hWnd, WM_PROC_EXCEPTION, iUserID, iHandle);
    }
}

LRESULT CEHomeDemoDlg::OnWMProcException(WPARAM wParam, LPARAM lParam)
{
    UN_REFERENCED_PARAMETER(wParam)
    int iHandle = lParam;
    if(!NET_ESTREAM_StopPreview(iHandle))
    {
        g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 2, "OnWMProcException NET_ESTREAM_StopPreview failed");
    }
    return TRUE;
}

void CEHomeDemoDlg::StartPreviewListen()
{

    NET_ESTREAM_SetExceptionCallBack(0, 0, fnPREVIEW_EXCEPTION_CB, this);
/*
    NET_EHOME_LISTEN_PREVIEW_CFG struListenParam = {0};
    struListenParam.byLinkMode = 0; //0-TCP, 1-UDP
    sprintf(struListenParam.struIPAdress.szIP, "%s", "0.0.0.0");
    struListenParam.struIPAdress.wPort = 8003;
    struListenParam.fnNewLinkCB = fnPREVIEW_NEWLINK_CB;
    struListenParam.pUser = this;
    m_lPreviewListen1 = NET_ESTREAM_StartListenPreview(&struListenParam);
    if (-1 == m_lPreviewListen1)
    {
        MessageBox("NET_ESTREAM_StartListen failed");
        return ;
    }

    //NET_ESTREAM_SetExceptionCallBack(0, 0, fPREVIEW_EXCEPTION_CB, this);
    struListenParam.byLinkMode = 1; //0-TCP, 1-UDP
    sprintf(struListenParam.struIPAdress.szIP, "%s", "0.0.0.0");
    struListenParam.struIPAdress.wPort = 8004;
    struListenParam.fnNewLinkCB = fnPREVIEW_NEWLINK_CB;
    struListenParam.pUser = this;
    m_lPreviewListen2 = NET_ESTREAM_StartListenPreview(&struListenParam);
    if (-1 == m_lPreviewListen2)
    {
        MessageBox("NET_ESTREAM_StartListen failed");
        return ;
    }*/
}

void CEHomeDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialog::OnSysCommand(nID, lParam);
    }
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CEHomeDemoDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // 用于绘制的设备上下文

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 使图标在工作区矩形中居中
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 绘制图标
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CEHomeDemoDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}


void CEHomeDemoDlg::InitChildWindow()
{
    m_pDlgPreview = new CDlgOutput[MAX_WIN_NUM];
    if (NULL == m_pDlgPreview)
    {
        return;
    }
    for (int i = 0; i < MAX_WIN_NUM; i++)
    {
        m_pDlgPreview[i].m_iWndIndex = i;
        m_pDlgPreview[i].Create(IDD_DLG_OUTPUT,this);
        m_pDlgPreview[i].m_pFatherDlg = this;
    }

    m_dlgPlayBack = new CDlgPlayBack;
    m_dlgPlayBack->Create(IDD_DLG_PLAY_BACK,this);
    m_dlgPlayBack->ShowWindow(SW_HIDE);

    m_dlgAudioTalk = new CDlgAudioTalk;
    m_dlgAudioTalk->Create(IDD_DLG_AUDIO_TALK,this);
    m_dlgAudioTalk->ShowWindow(SW_HIDE);

    m_dlgCommonCfg = new CDlgCommonCfg;
    m_dlgCommonCfg->Create(IDD_DLG_COMMON_CFG,this);
    m_dlgCommonCfg->ShowWindow(SW_HIDE);

    m_dlgLocalCfg = new CDlgLocalCfg;
    m_dlgLocalCfg->Create(IDD_DLG_LOCAL_CFG,this);
    m_dlgLocalCfg->ShowWindow(SW_HIDE);

    m_dlgOutputCtrl = new CDlgOutputCtrl;
    m_dlgOutputCtrl->Create(IDD_DLG_OUTPUT_CTRL,this);
    m_dlgOutputCtrl->ShowWindow(SW_HIDE);

}

void CEHomeDemoDlg::InitMainWin()
{
    m_iTreeWidth = 165;        //device tree width
    m_iRightWidth = 170;    //PTZ pic box width
    m_iFunBtnHeight = 70;     //column height
    m_iListLogHeight = 110 ;//list log height
    char szLan[128] = {0};
    CString csTemp = _T("");

    m_listAlarmInfo.InsertColumn(0,csTemp,LVCFMT_LEFT,0,-1); 
    g_StringLanType(szLan, "时间", "Time");
    csTemp = szLan;
    m_listAlarmInfo.InsertColumn(1, csTemp,LVCFMT_LEFT,150);
    g_StringLanType(szLan, "报警信息", "Alarm Info");
    csTemp = szLan;
    m_listAlarmInfo.InsertColumn(2, csTemp,LVCFMT_LEFT,350);
    g_StringLanType(szLan, "设备信息", "Device Info");
    csTemp = szLan;
    m_listAlarmInfo.InsertColumn(3, csTemp,LVCFMT_LEFT,160);
    m_listAlarmInfo.SetExtendedStyle(m_listAlarmInfo.GetExtendedStyle()|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_SUBITEMIMAGES);

    OnBnClickedRadioLocalLog();

    memset(szLan, 0, sizeof(szLan));
    csTemp = szLan;
    m_listAllLog.InsertColumn(0, csTemp, LVCFMT_LEFT, 0, -1); 
    g_StringLanType(szLan, "时间", "Time");
    csTemp = szLan;
    m_listAllLog.InsertColumn(1, csTemp, LVCFMT_LEFT, 120);
    g_StringLanType(szLan, "状态", "State");
    csTemp = szLan;
    m_listAllLog.InsertColumn(2, csTemp, LVCFMT_LEFT, 40);
    g_StringLanType(szLan, "操作", "Operation");
    csTemp = szLan;
    m_listAllLog.InsertColumn(3, csTemp, LVCFMT_LEFT, 300);
    g_StringLanType(szLan, "设备信息", "Device Info");
    csTemp = szLan;
    m_listAllLog.InsertColumn(4,csTemp,LVCFMT_LEFT,140);
    g_StringLanType(szLan, "错误信息", "Error Info");
    csTemp = szLan;
    m_listAllLog.InsertColumn(5,csTemp,LVCFMT_LEFT,80);
    m_listAllLog.SetExtendedStyle(m_listAllLog.GetExtendedStyle()|LVS_EX_GRIDLINES|LVS_EX_FULLROWSELECT|LVS_EX_SUBITEMIMAGES);

    m_comboWinNum.ResetContent();
    csTemp = _T("1");
    m_comboWinNum.AddString(csTemp);
    csTemp = _T("4");
    m_comboWinNum.AddString(csTemp);
    csTemp = _T("9");
    m_comboWinNum.AddString(csTemp);
    csTemp = _T("16");
    m_comboWinNum.AddString(csTemp);
    csTemp = _T("25");
    m_comboWinNum.AddString(csTemp);
    csTemp = _T("36");
    m_comboWinNum.AddString(csTemp);
    csTemp = _T("49");
    m_comboWinNum.AddString(csTemp);
    csTemp = _T("64");
    m_comboWinNum.AddString(csTemp);    

    m_comboWinNum.SetCurSel(1);

    m_iSelListType = 0;//local log
    ((CButton *)GetDlgItem(IDC_RADIO_LOCAL_LOG))->SetCheck(TRUE);
    ((CButton *)GetDlgItem(IDC_RADIO_ALARM_INFO))->SetCheck(FALSE);
    //OnSelchangeComboListType();
    MoveWindow(0, 0, DEMO_FULL_WIDTH, DEMO_FULL_HEIGHT, TRUE);

    csTemp.Format(_T("Demo version: V1.0.0 %d"));

    GetDlgItem(IDC_STATIC_DEMO_VERSION)->SetWindowText(csTemp);

    InitPreviewRect();//preview window, part of main window
}

void CEHomeDemoDlg::CreateTree()
{
    m_treeDeviceList.DeleteAllItems();
    char szLan[128] = {0};
    g_StringLanType(szLan, "设备树", "Device Tree");
    HTREEITEM hRoot = m_treeDeviceList.InsertItem(szLan, TREE_ALL, TREE_ALL);

    m_treeDeviceList.Expand(hRoot,TVE_EXPAND);
    m_treeDeviceList.Expand(m_treeDeviceList.GetRootItem(),TVE_EXPAND);
}

void CEHomeDemoDlg::InitPreviewRect()
{
    m_rectPreviewBG.top    = m_iFunBtnHeight;
    m_rectPreviewBG.left   = m_iTreeWidth;
    m_rectPreviewBG.right  = DEMO_FULL_WIDTH - m_iRightWidth;
    m_rectPreviewBG.bottom = m_rectPreviewBG.Width()*3/4 + m_iFunBtnHeight+4;//+4

    m_rectRightArea.top    = m_iFunBtnHeight - 3;
    m_rectRightArea.left   = DEMO_FULL_WIDTH - m_iRightWidth + 5;//
    m_rectRightArea.right  = DEMO_FULL_WIDTH;//
    m_rectRightArea.bottom = m_rectPreviewBG.Width()*3/4 + m_iFunBtnHeight + 4; //+55button height

    GetDlgItem(IDC_STATIC_PREVIEWBG)->MoveWindow(&m_rectPreviewBG,TRUE);

    m_dlgOutputCtrl->MoveWindow(&m_rectRightArea,TRUE);
    m_dlgOutputCtrl->ShowWindow(SW_SHOW);

    //output box layout
    ArrangeOutputs(m_iCurWndNum);//
}

void CEHomeDemoDlg::ArrangeOutputs(int iNumber)
{
    if (iNumber == 0)
    {
        //AddLog(m_iCurDeviceIndex, OPERATION_FAIL_T, "ArrangeOutputs number=0!");
        return;
    }
    int i = 0;
    CRect crect;

    int iSqrtNum = 0;//sqrt value of window number
    int iWidth = 0;//window width
    int iHeight = 0;//window height

    iSqrtNum = (int)sqrt((double)iNumber);

    for (i = 0;i < MAX_WIN_NUM;i++)
    {
        m_pDlgPreview[i].ShowWindow(SW_HIDE);    
    }

    
        iWidth = (m_rectPreviewBG.Width()-OUTPUT_INTERVAL*(iSqrtNum-1))/iSqrtNum;//a single pic width in partition
        iHeight = (m_rectPreviewBG.Height()-OUTPUT_INTERVAL*(iSqrtNum-1))/iSqrtNum;//a single pic height in partition
        int iPlayIndex = 0;
        for (i = 0; i < iNumber; i++)
        {
            
            if (g_struLocalParam.bEnlarged)
            {//double click to zoom some pic, iNumber = 1
                iPlayIndex = m_iCurWndIndex;
            }
            else
            {
                iPlayIndex = i;
            }
            m_pDlgPreview[iPlayIndex].MoveWindow(m_iTreeWidth + (i%iSqrtNum)*(iWidth+OUTPUT_INTERVAL),\
                m_iFunBtnHeight+(i/iSqrtNum)*(iHeight+OUTPUT_INTERVAL),iWidth,iHeight,TRUE);            
            m_pDlgPreview[iPlayIndex].ShowWindow(SW_SHOW);
            m_pDlgPreview[iPlayIndex].DrawOutputBorder();
        }    
}


/*********************************************************
Function:    LoadTreeImage
Desc:        load tree iamge
Input:    
Output:    
Return:    
**********************************************************/
#define MAX_BMPS 14
void CEHomeDemoDlg::LoadTreeImage(void)
{
    CBitmap cBmp[MAX_BMPS];

    m_imageTreeList.Create(16,16,ILC_COLOR32 | ILC_MASK,1,1);

    cBmp[TREE_ALL].LoadBitmap(IDB_BITMAP_TREE);
    m_imageTreeList.Add(&cBmp[TREE_ALL],RGB(1,1,1));

    cBmp[DEVICE_LOGOUT].LoadBitmap(IDB_BITMAP_LOGOUT);
    m_imageTreeList.Add(&cBmp[DEVICE_LOGOUT],RGB(1,1,1));
    cBmp[DEVICE_LOGIN].LoadBitmap(IDB_BITMAP_LOGIN);
    m_imageTreeList.Add(&cBmp[DEVICE_LOGIN],RGB(1,1,1));
    cBmp[DEVICE_FORTIFY].LoadBitmap(IDB_BITMAP_FORTIFY);
    m_imageTreeList.Add(&cBmp[DEVICE_FORTIFY],RGB(1,1,1));
    cBmp[DEVICE_ALARM].LoadBitmap(IDB_BITMAP_DEV_ALARM);
    m_imageTreeList.Add(&cBmp[DEVICE_ALARM],RGB(1,1,1));

    cBmp[DEVICE_FORTIFY_ALARM].LoadBitmap(IDB_BITMAP_FORTIFY_ALARM);
    m_imageTreeList.Add(&cBmp[DEVICE_FORTIFY_ALARM],RGB(1,1,1));

    cBmp[CHAN_ORIGINAL].LoadBitmap(IDB_BITMAP_CAMERA);
    m_imageTreeList.Add(&cBmp[CHAN_ORIGINAL],RGB(1,1,1));
    cBmp[CHAN_PLAY].LoadBitmap(IDB_BITMAP_PLAY);
    m_imageTreeList.Add(&cBmp[CHAN_PLAY],RGB(1,1,1));
    cBmp[CHAN_RECORD].LoadBitmap(IDB_BITMAP_REC);
    m_imageTreeList.Add(&cBmp[CHAN_RECORD],RGB(1,1,1));
    cBmp[CHAN_PLAY_RECORD].LoadBitmap(IDB_BITMAP_PLAYANDREC);
    m_imageTreeList.Add(&cBmp[CHAN_PLAY_RECORD],RGB(1,1,1));

    cBmp[CHAN_ALARM].LoadBitmap(IDB_BITMAP_ALARM);
    m_imageTreeList.Add(&cBmp[CHAN_ALARM],RGB(1,1,1));
    cBmp[CHAN_PLAY_ALARM].LoadBitmap(IDB_BITMAP_PLAY_ALARM);
    m_imageTreeList.Add(&cBmp[CHAN_PLAY_ALARM],RGB(1,1,1));

    cBmp[CHAN_PLAY_RECORD_ALARM].LoadBitmap(IDB_BITMAP_P_R_A);
    m_imageTreeList.Add(&cBmp[CHAN_PLAY_RECORD_ALARM],RGB(1,1,1));

    cBmp[CHAN_OFF_LINE].LoadBitmap(IDB_BITMAP_CHAN_OFF);
    m_imageTreeList.Add(&cBmp[CHAN_OFF_LINE],RGB(1,1,1));

    m_treeDeviceList.SetImageList(&m_imageTreeList, LVSIL_NORMAL);
}
void CEHomeDemoDlg::OnBnClickedRadioAlarmInfo()
{
    // TODO: Add your control notification handler code here
    UpdateData(TRUE);
    m_iSelListType = 1;
    OnSelchangeComboListType();
    UpdateData(FALSE);    
}

void CEHomeDemoDlg::OnBnClickedRadioLocalLog()
{
    // TODO: Add your control notification handler code here
    UpdateData(TRUE);
    m_iSelListType = 0;
    OnSelchangeComboListType();
    UpdateData(FALSE);
}

void CEHomeDemoDlg::OnSelchangeComboListType()
{
    UpdateData(TRUE);
    ListRestore();
//     if (m_iMainType == CONFIG_ALL_T)
//     {
//         m_dlgConfigAll->ConfigWndUpdate();            
//     }
//     else if (m_iMainType == PLAY_BACK_T)
//     {
//         m_dlgPlayBack->ShowWindow(SW_SHOW);
//         m_dlgPlayBack->PlayBackWinUpdate();
//     }
//     else if (m_iMainType == PREVIEW_T)
//     {
//         PreviewReferShow(TRUE);
//     }
    switch (m_iSelListType)
    {
    case 0:
        m_listAlarmInfo.ShowWindow(SW_HIDE);
        m_listAllLog.ShowWindow(SW_SHOW);    
        break;
    case 1:        
        m_listAllLog.ShowWindow(SW_HIDE);
        m_listAlarmInfo.ShowWindow(SW_SHOW);
        break;
    default:
        break;
    }

}

void CEHomeDemoDlg::ListRestore()
{
    CRect rc(0,0,0,0);
    if (m_bListLogEnlarge)
    {
        m_listAllLog.GetWindowRect(&rc);
        ScreenToClient(&rc);
        rc.top+=LIST_ENLARGE_HIGH;//move top down, compress
        m_listAllLog.MoveWindow(&rc);
        m_bListLogEnlarge = FALSE;
    }

    if (m_bListAlarmEnlarge)
    {
        m_listAlarmInfo.GetWindowRect(&rc);
        ScreenToClient(&rc);
        rc.top+=LIST_ENLARGE_HIGH;//move top down, compress
        m_listAlarmInfo.MoveWindow(&rc);
        m_bListAlarmEnlarge = FALSE;
    }
}
void CEHomeDemoDlg::OnCbnSelchangeComboWndnum()
{
    // TODO: Add your control notification handler code here
    int iIndex = m_comboWinNum.GetCurSel();
    m_iCurWndNum = (int)pow(double(iIndex+1), 2);//current window number
    if (!IsPlaying())
    {
        m_iCurWndIndex = 0;//initialize current window index while switch window
        ArrangeOutputs(m_iCurWndNum);
        GetDlgItem(IDC_STATIC_PREVIEWBG)->Invalidate(TRUE);
    }
    else
    {
        char szLan[128] = {0};
        g_StringLanType(szLan, "请先停止播放", "Please stop previewing");
        AfxMessageBox(szLan); 
    }
}

/*********************************************************
Function:    IsPlaying
Desc:        whether any window is previewing
Input:    
Output:    
Return:    
**********************************************************/
BOOL CEHomeDemoDlg::IsPlaying(void)
{
    if (m_iMainType == PREVIEW_T)
    {
        for (int i = 0; i < MAX_WIN_NUM; i ++)
        {
            if (m_pDlgPreview[i].m_bPlay)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

void CEHomeDemoDlg::OnLvnColumnclickListAllLog(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: Add your control notification handler code here
    UN_REFERENCED_PARAMETER(pNMHDR)
    EnlargeList(m_listAllLog, m_bListLogEnlarge);

    *pResult = 0;
}

void CEHomeDemoDlg::OnLvnColumnclickListAlarmInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: Add your control notification handler code here
    UN_REFERENCED_PARAMETER(pNMHDR)
    EnlargeList(m_listAlarmInfo, m_bListAlarmEnlarge);
    *pResult = 0;
}

/*********************************************************
Function:    EnlargeList
Desc:        enlarge list when click the column
Input:    list, log or alarm information tree; bEnargeList, TRUE/FALSE;
Output:    none
Return:    none
**********************************************************/
void CEHomeDemoDlg::EnlargeList(CListCtrl &list, BOOL &bEnargeList)
{
    CRect rc(0,0,0,0);
    list.GetWindowRect(&rc);
    ScreenToClient(&rc);

    if (!bEnargeList)
    {
        rc.top-=LIST_ENLARGE_HIGH;//move top upper, stretch
        list.MoveWindow(&rc);
        bEnargeList = TRUE;
        if (m_iMainType == PLAY_BACK_T)
        {
            m_dlgPlayBack->ShowWindow(SW_HIDE);
        }
        else if (m_iMainType == AUDIO_TALK_T)
        {
            m_dlgAudioTalk->ShowWindow(SW_HIDE);    
        }
        else if(m_iMainType == COM_CONFIG_T)
        {
            m_dlgCommonCfg->ShowWindow(SW_HIDE);
        }
        else if (m_iMainType == LOCAL_CONFIG_T)
        {
            m_dlgLocalCfg->ShowWindow(SW_HIDE);
        }
        else
        {
            PreviewReferShow(FALSE);
        }
    }
    else
    {
        rc.top+=LIST_ENLARGE_HIGH;//move top down, compress
        list.MoveWindow(&rc);
        bEnargeList = FALSE;
        if (m_iMainType == PLAY_BACK_T)
        {
            m_dlgPlayBack->ShowWindow(SW_SHOW);
        }
        else if (m_iMainType == AUDIO_TALK_T)
        {
            m_dlgAudioTalk->ShowWindow(SW_SHOW);    
        }
        else if(m_iMainType == COM_CONFIG_T)
        {
            m_dlgCommonCfg->ShowWindow(SW_SHOW);
        }
        else if (m_iMainType == LOCAL_CONFIG_T)
        {
            m_dlgLocalCfg->ShowWindow(SW_SHOW);
        }
        else
        {
            PreviewReferShow(TRUE);            
        }
    }
}

void CEHomeDemoDlg::PreviewReferShow(BOOL bShow)
{
    int iShowStat = bShow?SW_SHOW:SW_HIDE;

    if (bShow)
    {
        ArrangeOutputs(m_iCurWndNum);
    }
    else
    {            
        for (int i=0; i<MAX_WIN_NUM; i++)
        {
            m_pDlgPreview[i].ShowWindow(SW_HIDE);
        }
    }
    GetDlgItem(IDC_STATIC_PREVIEWBG)->ShowWindow(iShowStat);
    GetDlgItem(IDC_STATIC_WINNUM)->ShowWindow(iShowStat);        
    GetDlgItem(IDC_COMBO_WNDNUM)->ShowWindow(iShowStat);
    GetDlgItem(IDC_BTN_PREVIEW_LISTEN)->ShowWindow(iShowStat);
    //GetDlgItem(IDC_BTN_CIRCLE_PREVIEW)->ShowWindow(iShowStat);
    GetDlgItem(IDC_EDIT_PREVIEW_HANDLE)->ShowWindow(iShowStat);
    GetDlgItem(IDC_BTN_CLOSE_PREVIEW)->ShowWindow(iShowStat);
    m_dlgOutputCtrl->ShowWindow(iShowStat);
}

void CEHomeDemoDlg::MoveChildWin(DWORD dwWinType)
{
    CRect rect(0,0,0,0);    

    rect.top    = m_iFunBtnHeight-2;
    rect.left   = m_iTreeWidth-2;//-2 cover preview box
    rect.right  = DEMO_FULL_WIDTH - m_iRightWidth ;
    rect.bottom = DEMO_FULL_HEIGHT - m_iListLogHeight - 25;//+4

    switch (dwWinType)
    {
    case PLAY_BACK_T:
        m_dlgPlayBack->MoveWindow(&rect,TRUE);
        break;
    case AUDIO_TALK_T:
        m_dlgAudioTalk->MoveWindow(&rect,TRUE);
        break;
    case COM_CONFIG_T:
        m_dlgCommonCfg->MoveWindow(&rect,TRUE);
        break;
    case LOCAL_CONFIG_T:
        m_dlgLocalCfg->MoveWindow(&rect,TRUE);
        break;
    default:
        break;
    }
}

void CEHomeDemoDlg::OnBnClickedBtnPlayback()
{
    // TODO: Add your control notification handler code here
    if (m_iMainType != PLAY_BACK_T)
    {
        m_iMainType = PLAY_BACK_T;
        m_dlgPlayBack->ShowWindow(SW_SHOW);
        m_dlgAudioTalk->ShowWindow(SW_HIDE);
        m_dlgCommonCfg->ShowWindow(SW_HIDE);
        m_dlgLocalCfg->ShowWindow(SW_HIDE);
        PreviewReferShow(FALSE);

        MoveChildWin(m_iMainType);    
    }
}

void CEHomeDemoDlg::OnBnClickedBtnPreview()
{
    // TODO: Add your control notification handler code here
    if (m_iMainType != PREVIEW_T)
    {
        //modify preview flag
        m_iMainType = PREVIEW_T;
        m_dlgPlayBack->ShowWindow(SW_HIDE);
        m_dlgAudioTalk->ShowWindow(SW_HIDE);
        m_dlgCommonCfg->ShowWindow(SW_HIDE);
        m_dlgLocalCfg->ShowWindow(SW_HIDE);
        PreviewReferShow(TRUE);
    }
}

void CEHomeDemoDlg::OnBnClickedBtnAudioTalk()
{
    // TODO: Add your control notification handler code here
    if (m_iMainType != AUDIO_TALK_T)
    {
        m_iMainType = AUDIO_TALK_T;
        m_dlgPlayBack->ShowWindow(SW_HIDE);
        m_dlgAudioTalk->ShowWindow(SW_SHOW);
        m_dlgCommonCfg->ShowWindow(SW_HIDE);
        m_dlgLocalCfg->ShowWindow(SW_HIDE);
        PreviewReferShow(FALSE);

        MoveChildWin(m_iMainType);        

        m_dlgAudioTalk->CheckInitParam();
    }
}

void CEHomeDemoDlg::OnBnClickedBtnCfg()
{
    // TODO: Add your control notification handler code here
    if (m_iMainType != COM_CONFIG_T)
    {
        m_iMainType = COM_CONFIG_T;
        m_dlgPlayBack->ShowWindow(SW_HIDE);
        m_dlgAudioTalk->ShowWindow(SW_HIDE);
        m_dlgCommonCfg->ShowWindow(SW_SHOW);
        m_dlgLocalCfg->ShowWindow(SW_HIDE);
        PreviewReferShow(FALSE);

        MoveChildWin(m_iMainType);        

        m_dlgCommonCfg->OnBnClickedBtnRefresh();
    }

    CMenu pMenu;
    CRect rectBtnElse(0, 0, 0, 0);
    GetDlgItem(IDC_BTN_CFG)->GetWindowRect(&rectBtnElse);

    if (!pMenu.LoadMenu(IDR_MENU_CONFIG))
    {
        return;
    }

    pMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN, rectBtnElse.left, rectBtnElse.bottom, this);
}

void CEHomeDemoDlg::OnBnClickedBtnLocalCfg()
{
    // TODO: Add your control notification handler code here
    if (m_iMainType != LOCAL_CONFIG_T)
    {
        m_iMainType = LOCAL_CONFIG_T;
        m_dlgPlayBack->ShowWindow(SW_HIDE);
        m_dlgAudioTalk->ShowWindow(SW_HIDE);
        m_dlgCommonCfg->ShowWindow(SW_HIDE);
        m_dlgLocalCfg->ShowWindow(SW_SHOW);
        PreviewReferShow(FALSE);

        MoveChildWin(m_iMainType);        
    }
}

void CEHomeDemoDlg::OnBnClickedBtnExit()
{
    // TODO: Add your control notification handler code here
    char szLan[128] = {0};
    g_StringLanType(szLan, "确定要退出吗?", "Sure to exit?");
    if (IDOK != MessageBox(szLan,"Warning",IDOK))
    {
        return;
    }
    g_bExitDemo = TRUE;//make not post message
    StopPlayAll();
    CDialog::OnCancel();
}


void CEHomeDemoDlg::OnCancel()
{
    // TODO: Add your specialized code here and/or call the base class
    StopPlayAll();
    OnBnClickedBtnExit();    
}



/*********************************************************
Function:    AddLog
Desc:        add local log
Input:    iLogType, log type, 0-alam, 1-operate log, 2-debug info; csLogInfo log info
Output:    
Return:    
**********************************************************/
/** @fn void CEHomeDemoDlg::AddLog(int iDeviceIndex, int iLogType, int iComType, const char* format, ...)
 *  @brief
 *  @param (in)    int iDeviceIndex    
 *  @param (in)    int iLogType    
 *  @param (in)    int iComType 模块类型，0-模块无关，1-CMS模块，2-STREAM模块，3-ALARM模块
 *  @param (in)    const char * format    
 *  @param (in)    ...    
 *  @return void
 */
void CEHomeDemoDlg::AddLog(int iDeviceIndex, int iLogType, int iComType, const char* format, ...)
{    
    if (g_bExitDemo)
    {
        return;
    }
    CTime  cTime = CTime::GetCurrentTime();
    char szTime[64] = {0};
    char szLogType[32] = "FAIL";
    char szLogInfo[1024] = {0};
    char szDevInfo[256] = {0};
    char szErrInfo[256] = {0};
    char szLog[1024] = {0};
    va_list arglist;
    va_start(arglist,format);
    vsprintf(szLogInfo,format,arglist); 
    va_end(arglist);

    sprintf(szTime, "%s", cTime.Format("%y-%m-%d %H:%M:%S").GetBuffer(0));

    if (iDeviceIndex != -1 && iDeviceIndex < 512)
    {
        sprintf(szDevInfo, "[%s]", g_struDeviceInfo[iDeviceIndex].byDeviceID);
    }

    switch (iLogType)
    {
    case OPERATION_SUCC_T:
    case PLAY_SUCC_T:
        sprintf(szErrInfo, "");
        sprintf(szLogType, "SUCC");
        break;
    case PLAY_FAIL_T:
        sprintf(szErrInfo, "PLAY_M4 Eorror!!!");    
        break;
    case OPERATION_FAIL_T:
    default:
        switch (iComType)
        {
        case 1:
            sprintf(szErrInfo, "CMS_ERR[%d]", NET_ECMS_GetLastError());
            break;
        case 2:
            sprintf(szErrInfo, "PREVIEW_ERR[%d]", NET_ESTREAM_GetLastError());
            break;
        case 3:
            sprintf(szErrInfo, "ALARM_ERR[%d]", NET_EALARM_GetLastError());
            break;
        default:
            break;
        }
        break;
    }
    //sprintf(szErrInfo, "err[%d:%s]", NET_DVR_GetLastError(), NET_DVR_GetErrorMsg());
    

    if (g_struLocalParam.bOutputDebugString)
    {
        OutputDebugString(szLog);
    }
    if (!g_struLocalParam.bSuccLog && !g_struLocalParam.bFailLog && (iLogType != ALARM_INFO_T ))
    {
        return;
    }
    LPLOCAL_LOG_INFO pLogInfo = NULL;
    try
    {  

        pLogInfo = new LOCAL_LOG_INFO;
        memset(pLogInfo, 0, sizeof(LOCAL_LOG_INFO));
        if (pLogInfo == NULL)
        {
            return;
        }
        pLogInfo->iLogType = iLogType;
        memcpy(pLogInfo->szTime, szTime, 64);
        memcpy(pLogInfo->szLogInfo, szLogInfo, 512);
        memcpy(pLogInfo->szDevInfo, szDevInfo, 128);
        memcpy(pLogInfo->szErrInfo, szErrInfo, 256);
        ::PostMessage(g_pMainDlg->m_hWnd, WM_ADD_LOG, iDeviceIndex, (LONG)pLogInfo);
    }
    catch (...)
    {
        if (pLogInfo != NULL)
        {
            return;
        }
        OutputDebugString("New Log Exception!!\n");
    }
}

/*********************************************************
Function:    OnWMAddLog
Desc:        responding to the message WM_ADD_LOG
Input:    wParam, parameter 1;lParam, parameter 2;
Output:    none
Return:    result code
**********************************************************/
LRESULT CEHomeDemoDlg::OnWMAddLog(WPARAM wParam, LPARAM lParam)
{
    UN_REFERENCED_PARAMETER(wParam)
    LPLOCAL_LOG_INFO pLogInfo = LPLOCAL_LOG_INFO(lParam);
    if (NULL == pLogInfo)
    {
        return 0;
    }
    char szLogType[32] = "FAIL";

    char szTime[64] = {0};
    char szLogInfo[512] = {0};
    char szDevInfo[128] = {0};
    char szErrInfo[256] = {0};
    memcpy(szTime, pLogInfo->szTime, 64);
    memcpy(szLogInfo, pLogInfo->szLogInfo, 512);
    memcpy(szDevInfo, pLogInfo->szDevInfo, 128);
    memcpy(szErrInfo, pLogInfo->szErrInfo, 256);
    if ( 5000 == m_listAllLog.GetItemCount())
    {
        m_listAllLog.DeleteAllItems();
    }

    switch (pLogInfo->iLogType)
    {
    case ALARM_INFO_T:
        sprintf(szLogType, "Alarm");
        m_listAlarmInfo.InsertItem(0, "", -1);
        m_listAlarmInfo.SetItemText(0, 1, szTime);
        m_listAlarmInfo.SetItemText(0, 2, szLogInfo);
        m_listAlarmInfo.SetItemText(0, 3, szDevInfo);
        break;
    case OPERATION_SUCC_T:
        sprintf(szLogType, "SUCC");    
        if (g_struLocalParam.bSuccLog)
        {
            m_listAllLog.InsertItem(0, "", -1);
            m_listAllLog.SetItemText(0, 1, szTime);
            m_listAllLog.SetItemText(0, 2, szLogType);
            m_listAllLog.SetItemText(0, 3, szLogInfo);
            m_listAllLog.SetItemText(0, 4, szDevInfo);
            m_listAllLog.SetItemText(0, 5, szErrInfo);
        }
        break;
    case OPERATION_FAIL_T:
        sprintf(szLogType, "FAIL");        
        if (g_struLocalParam.bFailLog)
        {
            m_listAllLog.InsertItem(0, "", -1);
            m_listAllLog.SetItemText(0, 1, szTime);
            m_listAllLog.SetItemText(0, 2, szLogType);
            m_listAllLog.SetItemText(0, 3, szLogInfo);
            m_listAllLog.SetItemText(0, 4, szDevInfo);
            m_listAllLog.SetItemText(0, 5, szErrInfo);
        }
        break;
    case PLAY_SUCC_T:
        sprintf(szLogType, "SUCC");
        if (g_struLocalParam.bSuccLog)
        {
            m_listAllLog.InsertItem(0, "", -1);
            m_listAllLog.SetItemText(0, 1, szTime);
            m_listAllLog.SetItemText(0, 2, szLogType);
            m_listAllLog.SetItemText(0, 3, szLogInfo);
            m_listAllLog.SetItemText(0, 4, szDevInfo);
            m_listAllLog.SetItemText(0, 5, szErrInfo);
        }
        break;
    case PLAY_FAIL_T:
        sprintf(szLogType, "FAIL");    
        if (g_struLocalParam.bFailLog)
        {
            m_listAllLog.InsertItem(0, "", -1);
            m_listAllLog.SetItemText(0, 1, szTime);
            m_listAllLog.SetItemText(0, 2, szLogType);
            m_listAllLog.SetItemText(0, 3, szLogInfo);
            m_listAllLog.SetItemText(0, 4, szDevInfo);
            m_listAllLog.SetItemText(0, 5, szErrInfo);
        }
        break;
    default:
        sprintf(szLogType, "FAIL");    
        if (g_struLocalParam.bFailLog)
        {
            m_listAllLog.InsertItem(0, "", -1);
            m_listAllLog.SetItemText(0, 1, szTime);
            m_listAllLog.SetItemText(0, 2, szLogType);
            m_listAllLog.SetItemText(0, 3, szLogInfo);
            m_listAllLog.SetItemText(0, 4, szDevInfo);
            m_listAllLog.SetItemText(0, 5, szErrInfo);
        }
        break;
    }
    if (pLogInfo != NULL)
    {
        delete pLogInfo;
        pLogInfo = NULL;
    }

    return 0;
}

/*********************************************************
Function:    GetAfterItem
Desc:        get device insert point, make sure new device insert afterword last node
Input:    hRoot:handle of item tree root
Output:    
Return:    
**********************************************************/
HTREEITEM CEHomeDemoDlg::GetAfterItem(HTREEITEM hRoot)
{
    HTREEITEM hReturn = TVI_FIRST;
    HTREEITEM hChild = m_treeDeviceList.GetChildItem(hRoot);

    while (hChild)
    {
        if (TREE_ALL_T ==  m_treeDeviceList.GetItemData(hChild) / 1000)   //break if it is device tree node
        {
            break;
        }

        hReturn = hChild;
        hChild = m_treeDeviceList.GetNextSiblingItem(hChild);
    }

    return hReturn;
}

LRESULT CEHomeDemoDlg::OnWMAddDev(WPARAM wParam, LPARAM lParam)
{
    UN_REFERENCED_PARAMETER(wParam)
    LPLOCAL_DEVICE_INFO pDevInfo = LPLOCAL_DEVICE_INFO(lParam);
    if (NULL == pDevInfo)
    {
        return 0;
    }

    int i = 0;
    unsigned int j = 0;
    char szLan[64] = {0};

    HTREEITEM hRoot = m_treeDeviceList.GetRootItem();

    for (i = 0; i < MAX_DEVICES; i++)
    {
        if (g_struDeviceInfo[i].iDeviceIndex == -1)
        {
            //m_iCurDeviceIndex = i;
            memcpy(&g_struDeviceInfo[i], pDevInfo, sizeof(LOCAL_DEVICE_INFO));
            g_struDeviceInfo[i].iDeviceIndex = i;

            HTREEITEM hDevice;

            if (pDevInfo->dwVersion >= 4)
            {
                if (strlen((char*)g_struDeviceInfo[i].sDeviceSerial) > 0)
                {
                    hDevice = m_treeDeviceList.InsertItem((LPCTSTR)g_struDeviceInfo[i].sDeviceSerial, DEVICE_LOGIN, DEVICE_LOGIN, hRoot,GetAfterItem(hRoot));
                }
                else
                {
                    AddLog(-1, OPERATION_FAIL_T, 1, "version is larger than four, serial is null，show DeviceID!");
                    hDevice = m_treeDeviceList.InsertItem((LPCTSTR)g_struDeviceInfo[i].byDeviceID, DEVICE_LOGIN, DEVICE_LOGIN, hRoot,GetAfterItem(hRoot));
                }
            }
            else
            {
                hDevice = m_treeDeviceList.InsertItem((LPCTSTR)g_struDeviceInfo[i].byDeviceID, DEVICE_LOGIN, DEVICE_LOGIN, hRoot,GetAfterItem(hRoot));
            }
            
            m_treeDeviceList.SetItemData(hDevice, DEVICETYPE * 1000 + i);//
            HTREEITEM hChannel = NULL;

            //获取设备信息
            NET_EHOME_DEVICE_INFO struDevInfo = {0};
            struDevInfo.dwSize = sizeof(NET_EHOME_DEVICE_INFO);
            NET_EHOME_CONFIG struCfg = {0};
            struCfg.pOutBuf = &struDevInfo;
            struCfg.dwOutSize = sizeof(NET_EHOME_DEVICE_INFO);
            if (!NET_ECMS_GetDevConfig(g_struDeviceInfo[i].lLoginID, NET_EHOME_GET_DEVICE_INFO, &struCfg, sizeof(NET_EHOME_CONFIG)))
            {
                AddLog(-1, OPERATION_FAIL_T, 1, "NET_EHOME_GET_DEVICE_INFO");
              //  return -1;
            }
            else
            {
                //需要将字符串字段转换成GB2312
                DWORD dwConvertLen = 0;
                UTF82A((char*)struDevInfo.sSerialNumber, (char*)struDevInfo.sSerialNumber, MAX_SERIALNO_LEN, &dwConvertLen);
                UTF82A((char*)struDevInfo.sSIMCardSN, (char*)struDevInfo.sSIMCardSN, MAX_SERIALNO_LEN, &dwConvertLen);
                UTF82A((char*)struDevInfo.sSIMCardPhoneNum, (char*)struDevInfo.sSIMCardPhoneNum, MAX_PHOMENUM_LEN, &dwConvertLen);

                AddLog(g_struDeviceInfo[i].iDeviceIndex, OPERATION_SUCC_T, 1, "NET_EHOME_GET_DEVICE_INFO sSerialNumber=[%s]", struDevInfo.sSerialNumber);
                AddLog(g_struDeviceInfo[i].iDeviceIndex, OPERATION_SUCC_T, 1, "NET_EHOME_GET_DEVICE_INFO sSIMCardSN=[%s]", struDevInfo.sSIMCardSN);
                AddLog(g_struDeviceInfo[i].iDeviceIndex, OPERATION_SUCC_T, 1, "NET_EHOME_GET_DEVICE_INFO sSIMCardPhoneNum=[%s]", struDevInfo.sSIMCardPhoneNum);
            }

            g_struDeviceInfo[i].dwAlarmInNum = struDevInfo.dwAlarmInPortNum;
            g_struDeviceInfo[i].dwAlarmOutNum = struDevInfo.dwAlarmOutPortNum;
            g_struDeviceInfo[i].dwAnalogChanNum = struDevInfo.dwChannelNumber;
            g_struDeviceInfo[i].dwDeviceChanNum = struDevInfo.dwChannelAmount;
            g_struDeviceInfo[i].dwIPChanNum = struDevInfo.dwChannelAmount - struDevInfo.dwChannelNumber;
            g_struDeviceInfo[i].dwAudioNum = struDevInfo.dwAudioChanNum;
            //analog channel
            for (j = 0; j < struDevInfo.dwChannelNumber; j++)
            {
                //由于目前的通道无法判断是否可用，因此都当做可用
                g_struDeviceInfo[i].struChanInfo[j].bEnable = TRUE;
                //这是模拟通道
                g_struDeviceInfo[i].struChanInfo[j].iChannelNO = j + 1;
                g_struDeviceInfo[i].struChanInfo[j].iChanType = DEMO_CHANNEL_TYPE_ANALOG;
                sprintf(szLan, "Camera%d", g_struDeviceInfo[i].struChanInfo[j].iChannelNO);
                hChannel = m_treeDeviceList.InsertItem(szLan,CHAN_ORIGINAL,CHAN_ORIGINAL,hDevice);

                g_struDeviceInfo[i].struChanInfo[j].iDeviceIndex = i;
                g_struDeviceInfo[i].struChanInfo[j].iChanIndex = j;

                m_treeDeviceList.SetItemData(hChannel, CHANNELTYPE * 1000 + g_struDeviceInfo[i].struChanInfo[j].iChanIndex);

            }

            //然后添加IP通道
            for (; j < struDevInfo.dwChannelAmount; j++)
            {
                //由于目前的通道无法判断是否可用，因此都当做可用
                g_struDeviceInfo[i].struChanInfo[j].bEnable = TRUE;
                //这是IP通道
                g_struDeviceInfo[i].struChanInfo[j].iChannelNO = struDevInfo.dwStartChannel + (j - struDevInfo.dwChannelNumber);
                g_struDeviceInfo[i].struChanInfo[j].iChanType = DEMO_CHANNEL_TYPE_IP;
                sprintf(szLan, "IPCamera%d", g_struDeviceInfo[i].struChanInfo[j].iChannelNO);
                hChannel = m_treeDeviceList.InsertItem(szLan,CHAN_ORIGINAL,CHAN_ORIGINAL,hDevice);

                g_struDeviceInfo[i].struChanInfo[j].iDeviceIndex = i;
                g_struDeviceInfo[i].struChanInfo[j].iChanIndex = j;

                m_treeDeviceList.SetItemData(hChannel, CHANNELTYPE * 1000 + g_struDeviceInfo[i].struChanInfo[j].iChanIndex);

            }

            //零通道处理逻辑
            g_struDeviceInfo[i].dwZeroChanNum = struDevInfo.dwSupportZeroChan;// SupportZeroChan:支持零通道的个数：0-不支持，1-支持1路，2-支持2路，以此类推
            g_struDeviceInfo[i].dwZeroChanStart = struDevInfo.dwStartZeroChan;// 零通道起始编号，默认10000
            for (int k = 0; k < (int)struDevInfo.dwSupportZeroChan; k++)
            {
                int nZeroChannelIndex = j + k;
                g_struDeviceInfo[i].struChanInfo[nZeroChannelIndex].bEnable = TRUE;

                //这是零通道
                g_struDeviceInfo[i].struChanInfo[nZeroChannelIndex].iChannelNO = struDevInfo.dwStartZeroChan + k;
                g_struDeviceInfo[i].struChanInfo[nZeroChannelIndex].iChanType = DEMO_CHANNEL_TYPE_ZERO;
                sprintf(szLan, "ZeroChannel%d", g_struDeviceInfo[i].struChanInfo[nZeroChannelIndex].iChannelNO);
                hChannel = m_treeDeviceList.InsertItem(szLan, CHAN_ORIGINAL, CHAN_ORIGINAL, hDevice);

                g_struDeviceInfo[i].struChanInfo[nZeroChannelIndex].iDeviceIndex = i;
                g_struDeviceInfo[i].struChanInfo[nZeroChannelIndex].iChanIndex = nZeroChannelIndex;

                m_treeDeviceList.SetItemData(hChannel, CHANNELTYPE * 1000 + g_struDeviceInfo[i].struChanInfo[nZeroChannelIndex].iChanIndex);
            }

            for (int j = 0; j < 256; j++)
            {
                char szLocalIP[128] = { 0 };
                //g_pMainDlg->GetLocalIP(szLocalIP);
                strncpy(szLocalIP, g_pMainDlg->m_sLocalIP, 128);

                DWORD dwIP = ntohl(inet_addr(szLocalIP));
                memcpy(g_struDeviceInfo[i].struChanInfo[j].struIP.szIP, IPToStr(dwIP), 16);
            }

            ////analog channel & IP channel
            //for (j= 0; j < MAX_CHAN_NUM_DEMO; j++)
            //{
            //    if (g_struDeviceInfo[i].struChanInfo[j].iChannelNO != -1 )
            //    {
            //        if (g_struDeviceInfo[i].struChanInfo[j].bEnable)
            //        {
            //            if (g_struDeviceInfo[i].struChanInfo[j].iChanType == DEMO_CHANNEL_TYPE_ANALOG)
            //            {
            //                sprintf(szLan, "Camera%d", g_struDeviceInfo[i].struChanInfo[j].iChannelNO);
            //                hChannel = m_treeDeviceList.InsertItem(szLan,CHAN_ORIGINAL,CHAN_ORIGINAL,hDevice);
            //            }
            //            else if (g_struDeviceInfo[i].struChanInfo[j].iChanType == DEMO_CHANNEL_TYPE_IP)
            //            {
            //                sprintf(szLan, "IPCamera%d", g_struDeviceInfo[i].struChanInfo[j].iChannelNO);
            //                hChannel = m_treeDeviceList.InsertItem(szLan,CHAN_ORIGINAL,CHAN_ORIGINAL,hDevice);
            //            }                            
            //        }

            //        g_struDeviceInfo[i].struChanInfo[j].iDeviceIndex = i;
            //        g_struDeviceInfo[i].struChanInfo[j].iChanIndex = j;

            //        m_treeDeviceList.SetItemData(hChannel, CHANNELTYPE * 1000 + g_struDeviceInfo[i].struChanInfo[j].iChanIndex);
            //    }  
            //}

            m_treeDeviceList.Expand(hRoot,TVE_EXPAND);
            m_treeDeviceList.Expand(hDevice,TVE_EXPAND);
            break;
        }
    }

    delete pDevInfo;

    return 0;
}

LRESULT CEHomeDemoDlg::OnWMDelDev(WPARAM wParam, LPARAM lParam)
{
    UN_REFERENCED_PARAMETER(wParam)
    int lLoginID = (int)lParam;

    NET_ECMS_ForceLogout(lLoginID);


    //if (g_struDeviceInfo[iDevindex].iDeviceIndex == -1)
    //{
    //    return 0;
    //}

    DelDev(lLoginID);

    return 0;
}

LRESULT CEHomeDemoDlg::OnWMChangeIPAddr(WPARAM wParam, LPARAM lParam)
{
    UN_REFERENCED_PARAMETER(wParam)
    int lLoginID = (int)lParam;
    DelDev(lLoginID);
    return 0;
}


void CEHomeDemoDlg::DelDev(LONG lLoginID)
{
    int iDevindex = -1;
    int i = 0;
    int j = 0;
    HTREEITEM hRoot = m_treeDeviceList.GetRootItem();

    HTREEITEM hDev = m_treeDeviceList.GetChildItem(hRoot);

    for (i = 0; i < MAX_DEVICES; i++)
    {
        iDevindex = m_treeDeviceList.GetItemData(hDev) % 1000;
        if (g_struDeviceInfo[iDevindex].iDeviceIndex != -1)
        {
            //说明有添加设备，那么就判断一下
            if (g_struDeviceInfo[iDevindex].lLoginID == lLoginID)
            {
                for (int k = 0; k<(int)g_struDeviceInfo[iDevindex].dwDeviceChanNum; k++)
                {
                    //如果设备要下线，先把这个设备的预览通道都关了
                    int iPlayWndIndex = g_struDeviceInfo[iDevindex].struChanInfo[k].iPlayWndIndex;
                    if (iPlayWndIndex >= 0)
                    {
                        if (!m_pDlgPreview[iPlayWndIndex].StopPlay())
                        {
                            g_pMainDlg->AddLog(iDevindex, OPERATION_FAIL_T, 0, "Stop previewing Failed in OnWMDelDev");
                        }
                    }
                }

                //找到设备
                m_treeDeviceList.DeleteItem(hDev);
                memset(&g_struDeviceInfo[iDevindex], 0, sizeof(g_struDeviceInfo[iDevindex]));
                g_struDeviceInfo[iDevindex].iDeviceIndex = -1;
                for (j = 0; j < MAX_CHAN_NUM_DEMO; j++)
                {
                    g_struDeviceInfo[iDevindex].struChanInfo[j].iDeviceIndex = -1;
                    g_struDeviceInfo[iDevindex].struChanInfo[j].iChanIndex = -1;
                    g_struDeviceInfo[iDevindex].struChanInfo[j].iChannelNO = -1;
                }
                break;
            }
        }

        //能到这里说明设备还没找到，那么进行到下一个节点
        hDev = m_treeDeviceList.GetNextSiblingItem(hDev);
        if (hDev == NULL)
        {
            break;
        }
    }
}

void CEHomeDemoDlg::ProcessAlarmData(DWORD dwAlarmType, void *pStru, DWORD dwStruLen, void *pXml, DWORD dwXmlLen)
{
    char szBuf[1024] = { 0 };
    memcpy(szBuf, pXml, dwXmlLen);
    //AddLog(-1, ALARM_INFO_T, 0, "%s--%d", (char*)pXml, dwXmlLen);
    switch (dwAlarmType)
    {
    case EHOME_ALARM:
        ProcessEhomeAlarm(pStru, dwStruLen, pXml, dwXmlLen);
        break;
    case EHOME_ALARM_HEATMAP_REPORT:
        ProcessEhomeHeatMapReport(pStru, dwStruLen, pXml, dwXmlLen);
        break;
    case EHOME_ALARM_FACESNAP_REPORT:
        ProcessEhomeFaceSnapReport(pStru, dwStruLen, pXml, dwXmlLen);
        break;;
    case EHOME_ALARM_GPS:
        ProcessEhomeGps(pStru, dwStruLen, pXml, dwXmlLen);
        break;
    case EHOME_ALARM_CID_REPORT:
        ProcessEhomeCid(pStru, dwStruLen, pXml, dwXmlLen);
        break;
//     case EHOME_ALARM_GPS_DATA:
//         ProcessEhomeGpsData(pStru, dwStruLen, pXml, dwXmlLen);
//         break;
    case EHOME_ALARM_NOTICE_PICURL:
        ProcessEhomeNoticePicUrl(pStru, dwStruLen, pXml, dwXmlLen);
        break;
    case  EHOME_ALARM_NOTIFY_FAIL:
        ProcessEhomeNotifyFail(pStru, dwStruLen, pXml, dwXmlLen);
        break;
    case  EHOME_ALARM_ACS:
        ProcessEhomeAlarmAcs(pXml, dwXmlLen);
        break;
    case EHOME_ALARM_WIRELESS_INFO:
        ProcessEhomeWirelessInfo(pStru, dwStruLen, pXml, dwXmlLen);
        break;
    default:        
        AddLog(-1, ALARM_INFO_T, 0, "[Unknown_Alarm]");
        break;
    }
}

int CEHomeDemoDlg::GetCurDeviceIndex()
{
    if (m_iCurDeviceIndex < MAX_DEVICES && m_iCurDeviceIndex >= 0)
    {
        return m_iCurDeviceIndex;
    }
    //AddLog(-1, OPERATION_SUCC_T, 0, "please select a device at first!");
    return -1;
}

int CEHomeDemoDlg::GetCurChanIndex()
{
    if (m_iCurChanIndex >= 0 && m_iCurChanIndex < 512)
    {
        return m_iCurChanIndex;
    }

    return -1;
} 

void CEHomeDemoDlg::ProcessEhomeAlarm(void *pStru, DWORD dwStruLen, void *pXml, DWORD dwXmlLen)
{
    UN_REFERENCED_PARAMETER(dwXmlLen)
    UN_REFERENCED_PARAMETER(pXml)
    UN_REFERENCED_PARAMETER(dwStruLen)
    if (pStru == NULL)
    {
        return;
    }
    NET_EHOME_ALARM_INFO *pStruAlarmInfo = static_cast<NET_EHOME_ALARM_INFO *>(pStru);
    for (int i = 0; i < 256; i++)
    {
        if (pStruAlarmInfo->szDeviceID[i] == 0xcd)
        {
            pXml = pXml;
        }
    }
    AddLog(-1, ALARM_INFO_T, 0, "[ALARM]DeviceID:%s,Time:%s,Type:%d,Action:%d,Channel:%d,AlarmIn:%d,DiskNo:%d", pStruAlarmInfo->szDeviceID, \
        pStruAlarmInfo->szAlarmTime, pStruAlarmInfo->dwAlarmType, pStruAlarmInfo->dwAlarmAction, pStruAlarmInfo->dwVideoChannel, \
        pStruAlarmInfo->dwAlarmInChannel, pStruAlarmInfo->dwDiskNumber);
    switch (pStruAlarmInfo->dwAlarmType)
    {
    case ALARM_TYPE_DEV_CHANGED_STATUS:
        AddLog(-1, ALARM_INFO_T, 0, "[ALARM_TYPE_DEV_CHANGED_STATUS]byDeviceStatus:%d", pStruAlarmInfo->uStatusUnion.struDevStatusChanged.byDeviceStatus);
        break;
    case ALARM_TYPE_CHAN_CHANGED_STATUS:
        AddLog(-1, ALARM_INFO_T, 0, "[ALARM_TYPE_CHAN_CHANGED_STATUS]byChanStatus:%d,wChanNO:%d", pStruAlarmInfo->uStatusUnion.struChanStatusChanged.byChanStatus,\
            pStruAlarmInfo->uStatusUnion.struChanStatusChanged.wChanNO);
        break;
    case ALARM_TYPE_HD_CHANGED_STATUS:
        AddLog(-1, ALARM_INFO_T, 0, "[ALARM_TYPE_HD_CHANGED_STATUS]byHDStatus:%d,wHDNo:%d,dwVolume:%d", pStruAlarmInfo->uStatusUnion.struHdStatusChanged.byHDStatus,\
            pStruAlarmInfo->uStatusUnion.struHdStatusChanged.wHDNo, pStruAlarmInfo->uStatusUnion.struHdStatusChanged.dwVolume);
        break;
    case ALARM_TYPE_DEV_TIMING_STATUS:
        AddLog(-1, ALARM_INFO_T, 0, "[ALARM_TYPE_DEV_TIMING_STATUS]byCPUUsage:%d,byMainFrameTemp:%d,byBackPanelTemp:%d,dwMemoryTotal:%d,dwMemoryUsage:%d",\
            pStruAlarmInfo->uStatusUnion.struDevTimeStatus.byCPUUsage, pStruAlarmInfo->uStatusUnion.struDevTimeStatus.byMainFrameTemp,\
            pStruAlarmInfo->uStatusUnion.struDevTimeStatus.byBackPanelTemp, pStruAlarmInfo->uStatusUnion.struDevTimeStatus.dwMemoryTotal,\
            pStruAlarmInfo->uStatusUnion.struDevTimeStatus.dwMemoryUsage);
        break;
    case ALARM_TYPE_CHAN_TIMING_STATUS:
        AddLog(-1, ALARM_INFO_T, 0, "[ALARM_TYPE_CHAN_TIMING_STATUS]byLinkNum:%d,wChanNO:%d,dwBitRate:%d", pStruAlarmInfo->uStatusUnion.struChanTimeStatus.byLinkNum,\
            pStruAlarmInfo->uStatusUnion.struChanTimeStatus.wChanNO, pStruAlarmInfo->uStatusUnion.struChanTimeStatus.dwBitRate);
        break;
    case ALARM_TYPE_HD_TIMING_STATUS:
        AddLog(-1, ALARM_INFO_T, 0, "[ALARM_TYPE_HD_TIMING_STATUS]wHDNo:%d,dwHDFreeSpace:%d", pStruAlarmInfo->uStatusUnion.struHdTimeStatus.wHDNo,\
            pStruAlarmInfo->uStatusUnion.struHdTimeStatus.dwHDFreeSpace);
        break;
    default:
        break;
    }
    
}

void CEHomeDemoDlg::ProcessEhomeHeatMapReport(void *pStru, DWORD dwStruLen, void *pXml, DWORD dwXmlLen)
{
    UN_REFERENCED_PARAMETER(dwXmlLen)
    UN_REFERENCED_PARAMETER(pXml)
    UN_REFERENCED_PARAMETER(dwStruLen)
    if (pStru == NULL)
    {
        return;
    }
    NET_EHOME_HEATMAP_REPORT *pStruHeatMapReport = static_cast<NET_EHOME_HEATMAP_REPORT *>(pStru);
    AddLog(-1, ALARM_INFO_T, 0, "[HEATMAPREPORT]DeviceID:%s,Channel:%d,Start:%s,Stop:%s,HeatMapValue:%d %d %d,Size:%d %d",\
        pStruHeatMapReport->byDeviceID, pStruHeatMapReport->dwVideoChannel, pStruHeatMapReport->byStartTime, pStruHeatMapReport->byStopTime,\
        pStruHeatMapReport->struHeatmapValue.dwMaxHeatMapValue, pStruHeatMapReport->struHeatmapValue.dwMinHeatMapValue, \
        pStruHeatMapReport->struHeatmapValue.dwTimeHeatMapValue, pStruHeatMapReport->struPixelArraySize.dwLineValue, \
        pStruHeatMapReport->struPixelArraySize.dwColumnValue);
}

void CEHomeDemoDlg::ProcessEhomeFaceSnapReport(void *pStru, DWORD dwStruLen, void *pXml, DWORD dwXmlLen)
{
    UN_REFERENCED_PARAMETER(dwXmlLen)
    UN_REFERENCED_PARAMETER(pXml)
    UN_REFERENCED_PARAMETER(dwStruLen)
    if (pStru == NULL)
    {
        return;
    }
    NET_EHOME_FACESNAP_REPORT *pStruFaceSnapReport = static_cast<NET_EHOME_FACESNAP_REPORT *>(pStru);
    AddLog(-1, ALARM_INFO_T, 0, "[FACESNAPREPORT]DeviceID:%s,Channel:%d,Time:%s,PicID:%d,Score:%d,TargetID:%d,Target Zone[%d %d %d %d],"
        "FacePicZone[%d %d %d %d],HumanFeature:[%d %d %d],Duration:%d,FacePicLen:%d,BackGroundPicLen:%d", \
        pStruFaceSnapReport->byDeviceID, pStruFaceSnapReport->dwVideoChannel, pStruFaceSnapReport->byAlarmTime, pStruFaceSnapReport->dwFacePicID, \
        pStruFaceSnapReport->dwFaceScore,  pStruFaceSnapReport->dwTargetID, pStruFaceSnapReport->struTarketZone.dwX, pStruFaceSnapReport->struTarketZone.dwY,\
        pStruFaceSnapReport->struTarketZone.dwWidth, pStruFaceSnapReport->struTarketZone.dwHeight, pStruFaceSnapReport->struFacePicZone.dwX, pStruFaceSnapReport->struFacePicZone.dwY, \
        pStruFaceSnapReport->struFacePicZone.dwWidth, pStruFaceSnapReport->struFacePicZone.dwHeight, pStruFaceSnapReport->struHumanFeature.byAgeGroup, \
        pStruFaceSnapReport->struHumanFeature.bySex, pStruFaceSnapReport->struHumanFeature.byEyeGlass,pStruFaceSnapReport->dwStayDuration, \
        pStruFaceSnapReport->dwFacePicLen, pStruFaceSnapReport->dwBackgroudPicLen);
}


void CEHomeDemoDlg::ProcessEhomeGps(void *pStru, DWORD dwStruLen, void *pXml, DWORD dwXmlLen)
{
    UN_REFERENCED_PARAMETER(dwXmlLen)
    UN_REFERENCED_PARAMETER(pXml)
    UN_REFERENCED_PARAMETER(dwStruLen)
    if (pStru == NULL)
    {
        return;
    }
    NET_EHOME_GPS_INFO *pStruGps = static_cast<NET_EHOME_GPS_INFO *>(pStru);
    AddLog(-1, ALARM_INFO_T, 0, "[GPS]DeviceID:%s,SampleTime:%s,Division:[%c %c],Satelites:%d,Precision:%d,Longitude:%d,Latitude:%d,Direction:%d,Speed:%d,Height:%d",\
        pStruGps->byDeviceID, pStruGps->bySampleTime, pStruGps->byDivision[0], pStruGps->byDivision[1], pStruGps->bySatelites, pStruGps->byPrecision, pStruGps->dwLongitude,\
        pStruGps->dwLatitude, pStruGps->dwDirection, pStruGps->dwSpeed, pStruGps->dwHeight);
}

void CEHomeDemoDlg::ProcessEhomeCid(void *pStru, DWORD dwStruLen, void *pXml, DWORD dwXmlLen)
{
    UN_REFERENCED_PARAMETER(dwXmlLen)
    UN_REFERENCED_PARAMETER(pXml)
    UN_REFERENCED_PARAMETER(dwStruLen)
    if (pStru == NULL)
    {
        return;
    }
    if (NULL != pStru)
    {
        NET_EHOME_CID_INFO *pStruCidInfo = static_cast<NET_EHOME_CID_INFO *>(pStru);
        for (int i = 0; i < 256; i++)
        {
            if (pStruCidInfo->byDeviceID[i] == 0xcd)
            {
                pXml = pXml;
            }
        }
        AddLog(-1, ALARM_INFO_T, 0, "[CID]DeviceID:%s,CID code:%d,CID type:%d,Subsys No:%d,Describe:%s,TriggerTime:%s,UploadTime:%s,CID param[%d %d %d %d %d %d %d %s]", \
            pStruCidInfo->byDeviceID, pStruCidInfo->dwCIDCode, pStruCidInfo->dwCIDType, pStruCidInfo->dwSubSysNo, pStruCidInfo->byCIDDescribe, \
            pStruCidInfo->byTriggerTime, pStruCidInfo->byUploadTime, pStruCidInfo->struCIDParam.dwUserType, pStruCidInfo->struCIDParam.lUserNo, \
            pStruCidInfo->struCIDParam.lZoneNo, pStruCidInfo->struCIDParam.lKeyboardNo, pStruCidInfo->struCIDParam.lVideoChanNo, \
            pStruCidInfo->struCIDParam.lDiskNo, pStruCidInfo->struCIDParam.lModuleAddr, pStruCidInfo->struCIDParam.byUserName);
    }
}

void CEHomeDemoDlg::ProcessEhomeNoticePicUrl(void *pStru, DWORD dwStruLen, void *pXml, DWORD dwXmlLen)
{
    UN_REFERENCED_PARAMETER(dwXmlLen)
    UN_REFERENCED_PARAMETER(pXml)
    UN_REFERENCED_PARAMETER(dwStruLen)
    if (pStru == NULL)
    {
        return;
    }
    NET_EHOME_NOTICE_PICURL *pStruNoticePicUrl = static_cast<NET_EHOME_NOTICE_PICURL *>(pStru);
    for (int i = 0; i < 256; i++)
    {
        if (pStruNoticePicUrl->byDeviceID[i] == 0xcd)
        {
            pXml = pXml;
        }
    }
    AddLog(-1, ALARM_INFO_T, 0, "[NOTICEPICURL]DeviceID:%s,PicType:%d,AlarmType:%d,AlarmChan:%d,AlarmTime:%s,CaptureChan:%d,PicTime:%s,URL:%s,ManualSeq:%d", \
        pStruNoticePicUrl->byDeviceID, pStruNoticePicUrl->wPicType, pStruNoticePicUrl->wAlarmType, pStruNoticePicUrl->dwAlarmChan, pStruNoticePicUrl->byAlarmTime, pStruNoticePicUrl->dwCaptureChan,\
        pStruNoticePicUrl->byPicTime, pStruNoticePicUrl->byPicUrl, pStruNoticePicUrl->dwManualSnapSeq);
}

void CEHomeDemoDlg::ProcessEhomeNotifyFail(void *pStru, DWORD dwStruLen, void *pXml, DWORD dwXmlLen)
{
    UN_REFERENCED_PARAMETER(dwXmlLen)
    UN_REFERENCED_PARAMETER(pXml)
    UN_REFERENCED_PARAMETER(dwStruLen)
    if (pStru == NULL)
    {
        return;
    }
    NET_EHOME_NOTIFY_FAIL_INFO *pStruNotifyFail= static_cast<NET_EHOME_NOTIFY_FAIL_INFO *>(pStru);
    for (int i = 0; i < 256; i++)
    {
        if (pStruNotifyFail->byDeviceID[i] == 0xcd)
        {
            pXml = pXml;
        }
    }
    AddLog(-1, ALARM_INFO_T, 0, "[NOTIFYFAIL]DeviceID:%s,FailedCommand:%d,PicType:%d,ManualSeq:%d", \
        pStruNotifyFail->byDeviceID, pStruNotifyFail->wFailedCommand, pStruNotifyFail->wPicType, pStruNotifyFail->dwManualSnapSeq);
}

void CEHomeDemoDlg::ProcessEhomeAlarmAcs(void *pXml, DWORD dwXmlLen)
{
    if ((pXml == NULL) || (dwXmlLen == 0))
    {
        return;
    }

    char szInfoBuf[1024] = { 0 };
    char byOutput[32] = { 0 };

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<Command>", "</Command>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "%s ", byOutput);
    }

    /*memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<DeviceID>", "</DeviceID>", byOutput))
    {
    sprintf(szInfoBuf + strlen(szInfoBuf), "DeviceID:%s ", byOutput);
    }*/

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<MajorType>", "</MajorType>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "Major:%s ", byOutput);
    }

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<MinorType>", "</MinorType>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "Minor:%s ", byOutput);
    }

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<Time>", "</Time>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "Time:%s ", byOutput);
    }

    /*memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<NetUserName>", "</NetUserName>", byOutput))
    {
    sprintf(szInfoBuf + strlen(szInfoBuf), "Name:%s ", byOutput);
    }*/

    /*memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<NetUserIp>", "</NetUserIp>", byOutput))
    {
    sprintf(szInfoBuf + strlen(szInfoBuf), "Ip:%s ", byOutput);
    }*/

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<CardNo>", "</CardNo>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "CardNo:%s ", byOutput);
    }

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<CardType>", "</CardType>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "CardType:%s ", byOutput);
    }

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<CardReaderNo>", "</CardReaderNo>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "CardReaderNo:%s ", byOutput);
    }

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<DoorNo>", "</DoorNo>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "DoorNo:%s ", byOutput);
    }

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<MultiCardVerifyNo>", "</MultiCardVerifyNo>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "MultiCardVerifyNo:%s ", byOutput);
    }

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<MultiCardGroupNo>", "</MultiCardGroupNo>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "MultiCardGroupNo:%s ", byOutput);
    }

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<AlarmInNo>", "</AlarmInNo>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "AlarmInNo:%s ", byOutput);
    }

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<AlarmOutNo>", "</AlarmOutNo>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "AlarmOutNo:%s ", byOutput);
    }

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<CaseSensorNo>", "</CaseSensorNo>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "CaseSensorNo:%s ", byOutput);
    }

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<Rs485No>", "</Rs485No>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "Rs485No:%s ", byOutput);
    }

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<RetransFlag>", "</RetransFlag>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "RetransFlag:%s ", byOutput);
    }

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<WhiteListNo>", "</WhiteListNo>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "WhiteListNo:%s ", byOutput);
    }

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<ReportChannel>", "</ReportChannel>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "ReportChannel:%s ", byOutput);
    }

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<MacAddr>", "</MacAddr>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "MacAddr:%s ", byOutput);
    }

    memset(byOutput, 0, 32);
    if (XmlPrase((char*)pXml, "<SwipeCardType>", "</SwipeCardType>", byOutput))
    {
        sprintf(szInfoBuf + strlen(szInfoBuf), "SwipeCardType:%s ", byOutput);
    }

    AddLog(-1, ALARM_INFO_T, 0, "%s", szInfoBuf);
}

BOOL CEHomeDemoDlg::XmlPrase(char* pXml, char* pInputBefore, char* pInputAfter, char* pOutput)
{
    if ((pXml == NULL) || (pInputBefore == NULL) || (pInputAfter == NULL) || (pOutput == NULL))
    {
        return false;
    }
    if ((strlen(pInputBefore) == 0) || (strlen(pInputAfter) == 0))
    {
        return false;
    }

    char* pBefore = strstr(pXml, pInputBefore);
    char* pAfter = strstr(pXml, pInputAfter);
    if ((pBefore == NULL) || (pAfter == NULL) || ((pBefore + strlen(pInputBefore)) == pAfter))
    {
        return false;
    }

    int iLength = (pAfter - pBefore - strlen(pInputBefore)) / sizeof(char);
    memcpy(pOutput, (pBefore + strlen(pInputBefore)), iLength);

    return true;
}

void CEHomeDemoDlg::ProcessEhomeWirelessInfo(void *pStru, DWORD dwStruLen, void *pXml, DWORD dwXmlLen)
{
    UN_REFERENCED_PARAMETER(dwXmlLen)
        UN_REFERENCED_PARAMETER(pXml)
        UN_REFERENCED_PARAMETER(dwStruLen)
        NET_EHOME_ALARMWIRELESSINFO *pStruWirelessInfo = static_cast<NET_EHOME_ALARMWIRELESSINFO *>(pStru);
    AddLog(-1, ALARM_INFO_T, 0, "[Wireless]DeviceID:%s,DataTraffic:%f,SignalIntensity:%d", pStruWirelessInfo->byDeviceID, ((float)pStruWirelessInfo->dwDataTraffic) / 100, pStruWirelessInfo->bySignalIntensity);
}

// void CEHomeDemoDlg::ProcessEhomeGpsData(void *pStru, DWORD dwStruLen, void *pXml, DWORD dwXmlLen)
// {
//     UN_REFERENCED_PARAMETER(dwXmlLen)
//     UN_REFERENCED_PARAMETER(pXml)
//     UN_REFERENCED_PARAMETER(dwStruLen)
//     NET_EHOME_GPS_INFO *pStruGpsData = static_cast<NET_EHOME_GPS_INFO *>(pStru);
//     AddLog(-1, ALARM_INFO_T, 0, "[GPSDATA]DeviceID:%s,Time:%s,bySvsNum:%d,byLocateMode:%d,wPrecision:%d,dwHeight:%d,iLatitude:%d,iLongitude:%d,dwVehicleSpeed:%d,dwVehicleDirection:%d,dwMileage:%d",
//         pStruGpsData->byDeviceID, pStruGpsData->bySampleTime, pStruGpsData->bySatelites, pStruGpsData->byLocateMode, pStruGpsData->byPrecision,
//         pStruGpsData->dwHeight, pStruGpsData->dwLatitude,pStruGpsData->dwLongitude, pStruGpsData->dwSpeed, pStruGpsData->dwDirection, pStruGpsData->dwMileage);
// }

//void CEHomeDemoDlg::OnTvnSelchangedTreeDev(NMHDR *pNMHDR, LRESULT *pResult)
//{
//    LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
//    // TODO: Add your control notification handler code here
//    *pResult = 0;
//}

void CEHomeDemoDlg::OnNMClickTreeDev(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: Add your control notification handler code here
    UN_REFERENCED_PARAMETER(pNMHDR)
    CPoint pt(0,0);
    GetCursorPos(&pt);

    CRect rect(0,0,0,0);
    ScreenToClient(&pt);
    GetDlgItem(IDC_TREE_DEV)->GetWindowRect(&rect);
    ScreenToClient(&rect);

    pt.x = pt.x-rect.left;
    pt.y = pt.y-rect.top;

    UINT uFlags = 0;
    HTREEITEM hSelect = m_treeDeviceList.HitTest(pt,&uFlags);
    if (hSelect == NULL)
    {
        AddLog(m_iCurDeviceIndex, 0, OPERATION_FAIL_T, "please click the right node!");
        return;
    }

    DWORD dwNoteData = m_treeDeviceList.GetItemData(hSelect);
    int iType = dwNoteData/1000;
    int iIndex = dwNoteData%1000;
    if (DEVICETYPE == iType)
    {
        m_iCurDeviceIndex = iIndex;
        m_hCurDeviceItem = hSelect;
        m_iCurChanIndex = 0;    //如果仅选中设备树，默认第1个通道
        m_hCurChanItem = NULL;
    }
    else if (CHANNELTYPE == iType)
    {
        m_iCurChanIndex = iIndex;
        m_hCurChanItem = hSelect;
        HTREEITEM hParent = m_treeDeviceList.GetParentItem(hSelect);
        if (hParent != NULL)
        {
            dwNoteData = m_treeDeviceList.GetItemData(hParent);
            iType = dwNoteData/1000;
            iIndex = dwNoteData%1000;
            if (DEVICETYPE == iType)
            {
                m_iCurDeviceIndex = iIndex;
                m_hCurDeviceItem = hParent;
            }
        }
    }
    else
    {
        m_iCurDeviceIndex = -1;
        m_hCurDeviceItem = NULL;
        m_iCurChanIndex = -1;
        m_hCurChanItem = NULL;
    }



    if (m_iMainType == PLAY_BACK_T)
    {
        m_dlgPlayBack->PlayBackWinUpdate();
    }

    *pResult = 0;
}

void CEHomeDemoDlg::OnNMDblclkTreeDev(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: 在此添加控件通知处理程序代码
    UN_REFERENCED_PARAMETER(pNMHDR)
    HTREEITEM hSelect = m_treeDeviceList.GetSelectedItem();
    if (NULL == hSelect || m_iMainType != PREVIEW_T)//it works to double click device tree only while preview
    {
        return;
    }

//     if (g_bPlayAllDevice)//disable to single designated play while all play
//     {
//         char szLan[128] = {0};
//         g_StringLanType(szLan, "请先停止全部播放", "Please stop all play first");
//         AfxMessageBox(szLan);
//         return;
//     }

    DWORD dwNodeData = (DWORD)m_treeDeviceList.GetItemData(hSelect);
    HTREEITEM hParent = NULL;
    int iType = dwNodeData/1000;
    int iIndex = dwNodeData%1000;
    switch (iType)
    {
    case TREE_ALL_T:
        //DblAllTree();
        break;
    case DEVICETYPE:
        m_iCurDeviceIndex = iIndex;
        m_hCurDeviceItem = hSelect;
        m_iCurChanIndex = -1;
        m_hCurChanItem = NULL;
        if (GetCurDeviceIndex() != -1)
        {
            PlayDevice(m_iCurDeviceIndex, m_iCurWndIndex);
        }        
        m_treeDeviceList.Expand(hSelect, TVE_COLLAPSE);//expend reverse operation
        //    m_treeDeviceList.Expand(hSelect,TVE_EXPAND);
        break;
    case CHANNELTYPE:  //double click channel
        m_iCurChanIndex = iIndex;
        m_hCurChanItem = hSelect;
        hParent = m_treeDeviceList.GetParentItem(hSelect);
        if (hParent != NULL)
        {
            if (DEVICETYPE == m_treeDeviceList.GetItemData(hParent)/1000)
            {
                m_iCurDeviceIndex = m_treeDeviceList.GetItemData(hParent)%1000;
                m_hCurDeviceItem = hParent;
            }
        }

        g_struDeviceInfo[g_pMainDlg->m_iCurDeviceIndex].struChanInfo[g_pMainDlg->m_iCurChanIndex].iPlayWndIndex = m_iCurWndIndex;
        PlayChan(m_iCurDeviceIndex,iIndex,hSelect);

        break;
    default:
        {
            m_iCurChanIndex = -1;
            m_hCurChanItem = NULL;
            m_iCurDeviceIndex = -1;
            m_hCurDeviceItem = NULL;
        }
        break;
    }
    *pResult = 0;
}

void CEHomeDemoDlg::PlayDevice(int iDeviceIndex, int iStartOutputIndex)
{
    UN_REFERENCED_PARAMETER(iStartOutputIndex)
    HTREEITEM hSelect = m_treeDeviceList.GetSelectedItem();
    m_bCyclePreview = true;

    int i=0;
    int iChanIndex = 0;

    HTREEITEM hChannel = m_treeDeviceList.GetChildItem(hSelect);

    if (hChannel ==  NULL)
    {
        g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, 0, "there is no channels!!");
        return;
    }
    
    if (g_struDeviceInfo[iDeviceIndex].bPlayDevice)
    {
        g_struDeviceInfo[iDeviceIndex].bPlayDevice = FALSE;
        //StopPlayAll();

        for (i=0; i< m_iCurWndNum; i++)
        {
            if (m_pDlgPreview[i].m_iDeviceIndex == iDeviceIndex)
            {
                m_pDlgPreview[i].StopPlay();
            }    
        }
        m_iCurWndIndex = 0;
    }
    else
    {
        for (i=0; i<m_iCurWndNum; i++)
        {
            //shut down all play channel
            if (m_pDlgPreview[i].m_iDeviceIndex == iDeviceIndex)
            {
                m_pDlgPreview[i].StopPlay();
            }    

            if (m_pDlgPreview[i].m_lPlayHandle < 0)
            {//play the window that is not previewed
                iChanIndex = m_treeDeviceList.GetItemData(hChannel)%1000;
                m_pDlgPreview[i].StartPlay(&g_struDeviceInfo[iDeviceIndex].struChanInfo[iChanIndex], hChannel);
                hChannel = m_treeDeviceList.GetNextSiblingItem(hChannel);
                if (hChannel == NULL)
                {
                    break;
                }
            }

        }
        g_struDeviceInfo[iDeviceIndex].bPlayDevice = TRUE;
     }

    //ChangePlayBtnState();
}
void CEHomeDemoDlg::PlayChan(int iDeviceIndex, int iChanIndex,HTREEITEM hChanItem)
{
    //设备下线时候，会对deviceindex进行赋值，多线程操作，所以需要判断下
    if (iDeviceIndex < 0)
    {
        g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, 0, "CEHomeDemoDlg::PlayChan iDeviceIndex < 0");
        return;
    }
    m_bCyclePreview = false;
    char szLan[128] = {0};
    int iPlayWndIndex = g_struDeviceInfo[iDeviceIndex].struChanInfo[iChanIndex].iPlayWndIndex;
    //iPlayWndIndex小于0，说明没有为该通道指定过播放窗口
    //if (/*m_pDlgPreview[iPlayWndIndex].m_bPlay*/iPlayWndIndex >= 0)
    if(g_struDeviceInfo[iDeviceIndex].struChanInfo[iChanIndex].bPlay)
    {
        if (iPlayWndIndex < 0)
        {
            g_struDeviceInfo[iDeviceIndex].struChanInfo[iChanIndex].bPlay = false;
            g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, 0, "CEHomeDemoDlg::PlayChan iPlayWndIndex < 0");
            return;
        }

        if (!m_pDlgPreview[iPlayWndIndex].StopPlay())
        {
            g_StringLanType(szLan, "停止预览失败！", "Stop previewing Failed");
            g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, 0, szLan);
            //AfxMessageBox(szLan);
        }
        g_struDeviceInfo[iDeviceIndex].struChanInfo[iChanIndex].bPlay = false;
        g_struDeviceInfo[iDeviceIndex].struChanInfo[iChanIndex].iPlayWndIndex = -1;
    }
    else
    {
        STRU_CHANNEL_INFO struChanInfo;
        memcpy(&struChanInfo, &g_struDeviceInfo[iDeviceIndex].struChanInfo[iChanIndex], sizeof(STRU_CHANNEL_INFO));

        if (!m_pDlgPreview[m_iCurWndIndex].StartPlay(&struChanInfo,hChanItem))
        {
            g_StringLanType(szLan, "预览失败！", "Preview Failed");
            g_pMainDlg->AddLog(iDeviceIndex, OPERATION_FAIL_T, 0, szLan);
            //AfxMessageBox(szLan);
        }
        g_struDeviceInfo[iDeviceIndex].struChanInfo[iChanIndex].bPlay = true;
        return;
    }

    //if the channel is already previewed, stop it
//     if (!m_pDlgPreview[m_iCurWndIndex].StopPlay())
//     {
//         g_StringLanType(szLan, "停止预览失败！", "Stop previewing Failed");
//         //AfxMessageBox(szLan);
//     }
}

void CEHomeDemoDlg::ChangePlayBtnState(void)
{
    int i = 0;
    char szLan[32] = {0};
    if (m_iMainType == PREVIEW_T)
    {
        for (i = 0; i < MAX_WIN_NUM; i++)
        {
            if (!m_pDlgPreview[i].m_bPlay)
            {
                continue;
            }

            m_comboWinNum.EnableWindow(FALSE);
            g_StringLanType(szLan, "停 止", "Stop");
            GetDlgItem(IDC_BTN_CIRCLE_PREVIEW)->SetWindowText(szLan);
            return;
        }
    }

    m_comboWinNum.EnableWindow(TRUE);
    g_StringLanType(szLan, "播 放", "Play");
    GetDlgItem(IDC_BTN_CIRCLE_PREVIEW)->SetWindowText(szLan);
}

LRESULT CEHomeDemoDlg::ChangeChannelItemImage(WPARAM wParam, LPARAM lParam)
{
    int iDeviceIndex = int(wParam);
    int iChanIndex = int(lParam);
    if (iDeviceIndex < 0 || iDeviceIndex >MAX_DEVICES\
        || iChanIndex > 512 || iChanIndex < 0)
    {
        OutputDebugString("dev index and chan num err!\n");
        return NULL;
    }

    HTREEITEM hChanItem = GetChanItem(iDeviceIndex, iChanIndex);
    if (NULL == hChanItem)
    {
        OutputDebugString("chan item NULL!\n");
        return NULL;
    }
    int iImage = CHAN_ORIGINAL;
    STRU_CHANNEL_INFO struChanInfo;
    BOOL bPlaying = FALSE;
    memcpy(&struChanInfo, &g_struDeviceInfo[iDeviceIndex].struChanInfo[iChanIndex], sizeof(struChanInfo));


    //bPlaying = struChanInfo.iPlayWndIndex >= 0? TRUE:FALSE;
    bPlaying = struChanInfo.bPlay;
    if (bPlaying)
    {
        iImage = CHAN_PLAY;    
    }
    else
    {
        iImage = CHAN_ORIGINAL;    
    }

    m_treeDeviceList.SetItemImage(hChanItem,iImage,iImage);    

    g_struDeviceInfo[struChanInfo.iDeviceIndex].struChanInfo[struChanInfo.iChanIndex].dwImageType = iImage;

    return 0;
}

HTREEITEM CEHomeDemoDlg::GetChanItem(int iDeviceIndex, int iChanIndex)
{
    HTREEITEM hRoot = m_treeDeviceList.GetRootItem();
    if (hRoot == NULL)
    {
        return NULL;
    }
    int iChanData = 0;
    HTREEITEM hChannel = NULL;

    HTREEITEM hDevItem = m_treeDeviceList.GetChildItem(hRoot);
    while (hDevItem != NULL)
    {
        if (DEVICETYPE == (int)m_treeDeviceList.GetItemData(hDevItem)/1000)
        {
            if (iDeviceIndex ==  (int)m_treeDeviceList.GetItemData(hDevItem)%1000)
            {
                hChannel = m_treeDeviceList.GetChildItem(hDevItem);    
                if (hChannel == NULL)
                {
                    return NULL;
                }
                iChanData = m_treeDeviceList.GetItemData(hChannel);
                while (iChanData%1000 != iChanIndex)
                {
                    hChannel = m_treeDeviceList.GetNextSiblingItem(hChannel);
                    if (hChannel == NULL)
                    {
                        AddLog(iDeviceIndex, OPERATION_FAIL_T, 0, "don't find chanindex[%d] node!!!", iChanIndex);
                        return NULL;
                    }
                    else
                    {
                        iChanData = m_treeDeviceList.GetItemData(hChannel);
                    }

                }
                return hChannel;
            }
        }
        hDevItem = m_treeDeviceList.GetNextVisibleItem(hDevItem);
    }//end while

    return NULL;
}

void CEHomeDemoDlg::OnBnClickedBtnOther()
{
    // TODO: Add your control notification handler code here
    CMenu pMenu;
    CRect rectBtnElse(0,0,0,0);
    GetDlgItem(IDC_BTN_OTHER)->GetWindowRect(&rectBtnElse);

    if (!pMenu.LoadMenu(IDR_MENU_ELSE))
    {
        return;
    }

    pMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN, rectBtnElse.left,rectBtnElse.bottom,this);
}

void CEHomeDemoDlg::OnBnClickedBtnPreviewListen()
{
    // TODO: 在此添加控件通知处理程序代码
    CDlgListen dlg;
    dlg.m_pFatherDlg = this;
    dlg.DoModal();
}


void CEHomeDemoDlg::OnNMRClickTreeDev(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: 在此添加控件通知处理程序代码
    UN_REFERENCED_PARAMETER(pNMHDR)
    CMenu pMenu;
    HTREEITEM hParent=NULL;
    CPoint pt(0,0);
    GetCursorPos(&pt);

    CRect rect(0,0,0,0);
    ScreenToClient(&pt);
    GetDlgItem(IDC_TREE_DEV)->GetWindowRect(&rect);
    ScreenToClient(&rect);

    pt.x = pt.x-rect.left;
    pt.y = pt.y-rect.top;
    //while select first node or blank
    //HTREEITEM hSelect = m_treeDeviceList.GetSelectedItem();
    UINT uFlags = 0;
    HTREEITEM hSelect = m_treeDeviceList.HitTest(pt,&uFlags);
    if (hSelect == NULL )//|| m_iMainType != PREVIEW_T
    {
        return;
    }

    m_treeDeviceList.SelectItem(hSelect);

    int iType = int(m_treeDeviceList.GetItemData(hSelect) / 1000);

    switch (iType)
    {
    case CHANNELTYPE:
        m_iCurChanIndex = int(m_treeDeviceList.GetItemData(hSelect) % 1000);
        m_hCurChanItem = hSelect;

        hParent = m_treeDeviceList.GetParentItem(hSelect);
        if (hParent != NULL)
        {
            if (DEVICETYPE == m_treeDeviceList.GetItemData(hParent)/1000)
            {
                m_iCurDeviceIndex = m_treeDeviceList.GetItemData(hParent)%1000;
                m_hCurDeviceItem = hParent;
            }
        }
        if (!pMenu.LoadMenu(IDR_MENU_CHANNEL))
        {
            return;
        }

        break;
    default:
        return;
    }

    //display menu
    GetCursorPos(&pt);
    pMenu.GetSubMenu(0)->TrackPopupMenu(TPM_LEFTALIGN,pt.x ,pt.y,this);    
    *pResult = 0;
}

void CEHomeDemoDlg::CyclePreview()
{
    if (IsPlaying())
    {
        StopPlayAll();
    }
    else
    {
        int i = 0;
        int j = 0;
        m_iCurWndIndex = 0;
        for (i=0; i<m_iCurWndNum; i++)
        {
            while (j < MAX_DEVICES)
            {
                if (g_struDeviceInfo[j].iDeviceIndex != -1 )
                {
                    m_iCurDeviceIndex = j;
                    m_iCurChanIndex = 0;
                    g_struDeviceInfo[g_pMainDlg->m_iCurDeviceIndex].struChanInfo[g_pMainDlg->m_iCurChanIndex].iPlayWndIndex = m_iCurWndIndex;
                    PlayChan(g_struDeviceInfo[j].iDeviceIndex, 0, 0);
                    j++;
                    break;
                }
                j++;
            }
        }
    }
}

// void CEHomeDemoDlg::OnTimer(UINT_PTR nIDEvent)
// {
//     // TODO: Add your message handler code here and/or call default
//     char szLan[128] = {0};
//     switch (nIDEvent)
//     {
//     case CYCLE_PREVIEW_TIMER:
//         CyclePreview();
//         break;
//     default:
//         break;
//     }
// 
//     CDialog::OnTimer(nIDEvent);
// }

void CEHomeDemoDlg::OnBnClickedBtnCirclePreview()
{
    // TODO: 在此添加控件通知处理程序代码
    UpdateData(TRUE);
    if (m_bCyclePreview)
    {
        KillTimer(CYCLE_PREVIEW_TIMER);
        StopPlayAll();
        m_bCyclePreview = FALSE;
    }
    else
    {
        m_bCyclePreview = TRUE;
        CyclePreview();
        g_pCycleTimer = SetTimer(CYCLE_PREVIEW_TIMER, 5*1000,NULL);
    }
    ChangePlayBtnState();
}

void CEHomeDemoDlg::StopPlayAll(void)
{
    int i = 0;
    for (i = 0; i < MAX_WIN_NUM; i ++)
    {
        if (!m_pDlgPreview[i].m_bPlay)
        {
            continue;
        }
        m_pDlgPreview[i].StopPlay();
    }
}

void CEHomeDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    switch (nIDEvent)
    {
    case CYCLE_PREVIEW_TIMER:
        CyclePreview();
        break;
    default:
        break;
    }
    CDialog::OnTimer(nIDEvent);
}

void CEHomeDemoDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // TODO: Add your message handler code here and/or call default

    CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CEHomeDemoDlg::OnBnClickedBtnClosePreview()
{
    // TODO: 在此添加控件通知处理程序代码
    UpdateData(TRUE);
//     NET_ESTREAM_StopListenPreview(m_lPreviewListen1);
//     NET_ESTREAM_StopListenPreview(m_lPreviewListen2);
// 
    if (!NET_ESTREAM_StopPreview(m_iPreviewHandle))
    {
        g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 2, "NET_ESTREAM_StopPreview failed");
    }
    else
    {
        g_pMainDlg->AddLog(0, OPERATION_SUCC_T, 2, "NET_ESTREAM_StopPreview successful");
    }
}

void CEHomeDemoDlg::InitLib()
{
	BOOL bRet = false;
	bRet = NET_EALARM_Init();

	NET_EHOME_LOCAL_ACCESS_SECURITY struAccessSecure = { 0 };
	struAccessSecure.dwSize = sizeof(struAccessSecure);
	struAccessSecure.byAccessSecurity = m_byAlarmSecureAccessType;
	if (!NET_EALARM_SetSDKLocalCfg(ACTIVE_ACCESS_SECURITY, &struAccessSecure))
	{
		g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 2, "NET_EALARM_SetSDKLocalCfg ACTIVE_ACCESS_SECURITY Failed");
	}
	else
	{
		g_pMainDlg->AddLog(0, OPERATION_SUCC_T, 1, "NET_EALARM_SetSDKLocalCfg ACTIVE_ACCESS_SECURITY Success");
	}

    NET_EHOME_ALARM_LISTEN_PARAM struAlarmListenParam = { 0 };
    memcpy(struAlarmListenParam.struAddress.szIP, m_sLocalIP, sizeof(m_sLocalIP));

    struAlarmListenParam.struAddress.wPort = 7332;
    struAlarmListenParam.fnMsgCb = AlarmMsgCallBack;
    struAlarmListenParam.pUserData = this;
    struAlarmListenParam.byProtocolType = 1;
    struAlarmListenParam.byUseThreadPool = 0;

    m_lUdpAlarmHandle = NET_EALARM_StartListen(&struAlarmListenParam);

    /*
    struAlarmListenParam.struAddress.wPort = 7331;
    struAlarmListenParam.fnMsgCb = AlarmMsgCallBack;
    struAlarmListenParam.pUserData = this;
    struAlarmListenParam.byProtocolType = 0;
    struAlarmListenParam.byUseThreadPool = 1;

    m_lAlarmHandle = NET_EALARM_StartListen(&struAlarmListenParam);
    */
//     memset(struAlarmListenParam.struAddress.szIP, 0, sizeof(struAlarmListenParam.struAddress.szIP));
//     memcpy(struAlarmListenParam.struAddress.szIP, "127.0.0.1", strlen("127.0.0.1"));

    struAlarmListenParam.struAddress.wPort = (WORD)7333;
    struAlarmListenParam.fnMsgCb = AlarmMsgCallBack;
    struAlarmListenParam.pUserData = this;
    struAlarmListenParam.byUseCmsPort = 1;
    struAlarmListenParam.byUseThreadPool = 0;
    LONG lCmsAlarm = NET_EALARM_StartListen(&struAlarmListenParam);

    if (-1 == lCmsAlarm)
    {
        g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 3, "SNET_EALARM_StartListen Failed, port7333");
    }
    else
    {
        m_lCmsAlarm = lCmsAlarm;
        g_pMainDlg->AddLog(0, OPERATION_SUCC_T, 3, "SNET_EALARM_StartListen succ port7333");
    }

    NET_EALARM_SetLogToFile(3, "c:/EHomeSdkLog/", TRUE);
    NET_EALARM_GetBuildVersion();

    //初始化库
    LONG lRet = -1;
    bRet = NET_ECMS_Init();
    bRet = NET_ECMS_SetLogToFile(2, "c:/EHomeSdkLog/", TRUE);
    struAccessSecure.byAccessSecurity = m_byCmsSecureAccessType;
    if (!NET_ECMS_SetSDKLocalCfg(ACTIVE_ACCESS_SECURITY, &struAccessSecure))
    {
        g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 1, "NET_ECMS_SetSDKLocalCfg ACTIVE_ACCESS_SECURITY Failed");
    }
    else
    {
        g_pMainDlg->AddLog(0, OPERATION_SUCC_T, 1, "NET_ECMS_SetSDKLocalCfg ACTIVE_ACCESS_SECURITY Success");
    }

    NET_EHOME_AMS_ADDRESS struAmsAddr = { 0 };
    struAmsAddr.dwSize = sizeof(struAmsAddr);
    memcpy(struAmsAddr.struAddress.szIP, m_sLocalIP, sizeof(m_sLocalIP));
    struAmsAddr.struAddress.wPort = (WORD)7333;
    struAmsAddr.byEnable = 1;
    //CMS开始接收
    if (!NET_ECMS_SetSDKLocalCfg(AMS_ADDRESS, &struAmsAddr))
    {
        g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 1, "open cms-alarm Failed 7333");
    }
    else
    {
        g_pMainDlg->AddLog(0, OPERATION_SUCC_T, 1, "open cms-alarm  Succ 7333");
    }


    NET_EHOME_SEND_PARAM struSendParam = { 0 };
    struSendParam.dwSize = sizeof(struSendParam);
    struSendParam.bySendTimes = 3;
    //CMS开始接收
    if (!NET_ECMS_SetSDKLocalCfg(SEND_PARAM, &struSendParam))
    {
        g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 1, "NET_ECMS_SetSDKLocalCfg SEND_PARAM failed");
    }
    else
    {
        g_pMainDlg->AddLog(0, OPERATION_SUCC_T, 1, "NET_ECMS_SetSDKLocalCfg SEND_PARAM succ");
    }

    //开启监听
    NET_EHOME_CMS_LISTEN_PARAM struCMSListenPara = { 0 };
    memcpy(struCMSListenPara.struAddress.szIP, m_sLocalIP, strlen(m_sLocalIP));
    struCMSListenPara.struAddress.wPort = (WORD)m_nPort;
    struCMSListenPara.fnCB = EHOME_REGISTER;
    struCMSListenPara.pUserData = this;
    lRet = NET_ECMS_StartListen(&struCMSListenPara);
    if (-1 == lRet)
    {
        g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 1, "NET_ECMS_StartListen Failed");
    }
    else
    {
        g_pMainDlg->AddLog(0, OPERATION_SUCC_T, 1, "NET_ECMS_StartListen Succ");
    }
    NET_ECMS_GetBuildVersion();

    NET_ESTREAM_Init();
    NET_ESTREAM_SetLogToFile(3, "c:/EHomeSdkLog/", TRUE);

    struAccessSecure.byAccessSecurity = m_byStreamSecureAccessType;
    if (!NET_ESTREAM_SetSDKLocalCfg(ACTIVE_ACCESS_SECURITY, &struAccessSecure))
    {
        g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 2, "NET_ESTREAM_SetSDKLocalCfg ACTIVE_ACCESS_SECURITY Failed");
    }
    else
    {
        g_pMainDlg->AddLog(0, OPERATION_SUCC_T, 1, "NET_ESTREAM_SetSDKLocalCfg ACTIVE_ACCESS_SECURITY Success");
    }
    NET_ESTREAM_GetBuildVersion();

    
}


void CEHomeDemoDlg::GetLocalIP(char* pIPAddr)
{
    if(NULL == pIPAddr)
    {
        return;
    }
    WSADATA wsaData;
    char name[255];//定义用于存放获得的主机名的变量
    char *ip;//定义IP地址变量
    PHOSTENT hostinfo;

    //调用MAKEWORD（）获得Winsock版本的正确值，用于加载Winsock库

    if (WSAStartup(MAKEWORD(2,0), &wsaData) == 0) {
        //现在是加载Winsock库，如果WSAStartup（）函数返回值为0，说明加载成功，程序可以继续
        if (gethostname(name, sizeof(name)) == 0) {
            //如果成功地将本地主机名存放入由name参数指定的缓冲区中
            if ((hostinfo = gethostbyname(name)) != NULL) {
                //这是获取主机名，如果获得主机名成功的话，将返回一个指针，指向hostinfo，hostinfo
                //为PHOSTENT型的变量，下面即将用到这个结构体
                ip = inet_ntoa(*(struct in_addr *)*hostinfo->h_addr_list);
                //调用inet_ntoa（）函数，将hostinfo结构变量中的h_addr_list转化为标准的点分表示的IP
                //地址（如192.168.0.1）
                if ((strlen(ip)+1) > 128)
                {
                    WSACleanup(); 
                    return;
                }
                memcpy(pIPAddr, ip, strlen(ip)+1);                //printf("%s\n",ip);//输出IP地址
            }
        }
        WSACleanup(); //卸载Winsock库，并释放所有资源
    }
}




void CEHomeDemoDlg::OnMenuChannelInfo()
{
    // TODO: 在此添加命令处理程序代码
    int iDeviceIndex  = GetCurDeviceIndex();
    if (iDeviceIndex == -1)
    {
        return;
    }
    int iChanIndex = GetCurChanIndex();
    if (iChanIndex == -1)
    {
        char szLan[128] = {0};
        g_StringLanType(szLan, "通道号错误！", "Channel Error");
        AfxMessageBox(szLan);
        return;
    }
    CDlgChanInfo dlgChannelInfo;
    dlgChannelInfo.m_iDeviceIndex = iDeviceIndex;
    dlgChannelInfo.m_iChannelIndex = iChanIndex;
    dlgChannelInfo.DoModal();
}


BOOL ConvertSingleNodeData(void *pOutVale, CXmlBase &struXml, const char* pNodeName, BYTE byDataType, int iArrayLen /*= 0*/ )
{
    if (byDataType < NODE_TYPE_REVERSE)
    {
        if (struXml.FindElem(pNodeName))
        {
            //string to
            if (byDataType == NODE_STRING_TO_BOOL)    //bool类型
            {
                if (struXml.GetData().compare("true") == 0)
                {
                    *(BOOL*)pOutVale = TRUE;
                    return TRUE;
                }
                else if(struXml.GetData().compare("false") == 0)
                {
                    *(BOOL*)pOutVale = FALSE;
                    return TRUE;
                }            
            }
            else if (byDataType == NODE_STRING_TO_INT)   //int类型
            {
                if (struXml.GetData() != "")
                {
                    *(DWORD*)pOutVale = (DWORD)atoi(struXml.GetData().c_str());
                    return TRUE;
                }
            }
            else if (byDataType == NODE_STRING_TO_ARRAY)
            {
                string strTmp = struXml.GetData().c_str();
                if (strTmp != "")
                {
                    int nLen = (int)strTmp.length();
                    if (nLen > iArrayLen)
                    {
                        nLen = iArrayLen;
                    }
                    memcpy((char*)pOutVale, strTmp.c_str(), (DWORD)nLen);
                    return TRUE;
                }
            }
            else if (byDataType == NODE_STRING_TO_BYTE)
            {
                if (struXml.GetData() != "")
                {
                    *(BYTE*)pOutVale = (BYTE)atoi(struXml.GetData().c_str());
                    return TRUE;
                }
            }
            else if (byDataType == NODE_STRING_TO_WORD)
            {
                if (struXml.GetData() != "")
                {
                    *(WORD*)pOutVale = (WORD)atoi(struXml.GetData().c_str());
                    return TRUE;
                }
            }
            else if(byDataType == NODE_STRING_TO_FLOAT)
            {
                if (struXml.GetData() != "")
                {
                    *(float*)pOutVale = (float)atof(struXml.GetData().c_str());
                    return TRUE;
                }
            }
        }
        else
        {
            return FALSE;
        }

    }
    else if(byDataType > NODE_TYPE_REVERSE)
    {
        if (byDataType == NODE_BOOL_TO_STRING)
        {
            if (*(BOOL*)pOutVale)
            {
                if (struXml.AddNode(pNodeName, "true"))
                {
                    struXml.OutOfElem();
                    return TRUE;
                }
            }
            else if (!(*(BOOL*)pOutVale))
            {
                if(struXml.AddNode(pNodeName, "false"))
                {
                    struXml.OutOfElem();
                    return TRUE;
                }                
            }
        }
        else if (byDataType == NODE_INT_TO_STRING)
        {
            char szBuf[16] = {0};
            itoa(*(int*)pOutVale, szBuf, 10);
            if (struXml.AddNode(pNodeName, szBuf))
            {
                struXml.OutOfElem();
                return TRUE;
            }
        }
        else if (byDataType == NODE_ARRAY_TO_STRING)
        {
            string strValue = "";
            if (iArrayLen == 0)
            {
                strValue = (char*)pOutVale;
            }
            else 
            {
                int nValueLen = (int)strlen((char*)pOutVale);
                if (nValueLen < iArrayLen) //数组没有被填满,有结束符
                {
                    strValue = (char*)pOutVale;
                }
                else    
                {
                    char *lpTmp = NULL;
                    lpTmp = new char[iArrayLen + 1];
                    if (lpTmp == NULL)
                    {
                        return FALSE;
                    }

                    memset(lpTmp, 0, iArrayLen + 1);
                    memcpy(lpTmp, (char*)pOutVale, (DWORD)iArrayLen);//增加一个结束符
                    strValue = lpTmp;
                    delete[] lpTmp;    
                }
            }
            if (struXml.AddNode(pNodeName, strValue.c_str()))
            {
                struXml.OutOfElem();
                return TRUE;
            }
        }
        else if (byDataType == NODE_BYTE_TO_STRING)
        {
            char szBuf[16] = {0};
            int nTmp = (int)*(BYTE*)pOutVale; 
            itoa(nTmp, szBuf, 10);
            if (struXml.AddNode(pNodeName, szBuf))
            {
                struXml.OutOfElem();
                return TRUE;
            }
        }
        else if (byDataType == NODE_WORD_TO_STRING)
        {
            char szBuf[16] = {0};
            int nTmp = (int)*(WORD*)pOutVale; 
            itoa(nTmp, szBuf, 10);
            if (struXml.AddNode(pNodeName, szBuf))
            {
                struXml.OutOfElem();
                return TRUE;
            }
        }
    }

    return FALSE;
}

void CEHomeDemoDlg::OnMenuProxy()
{
    // TODO: 在此添加命令处理程序代码
    CDlgPassthroughProxy dlg;
    dlg.DoModal();
}


void CEHomeDemoDlg::OnGetGpsInfo()
{
    // TODO:  在此添加命令处理程序代码
    CDlgGpsInfo dlg;
    dlg.DoModal();
}

void CEHomeDemoDlg::OnBtnWirelessInfo()
{
    // TODO:  在此添加命令处理程序代码
    CDlgWirelessInfoPara dlg;
    dlg.DoModal();
}

void CEHomeDemoDlg::OnBnClickedBtnSalve()
{
    // TODO:  在此添加控件通知处理程序代码
    char szFile[256] = { 0 };
    memcpy(szFile, "C:\\EhomeRecord", strlen("C:\\EhomeRecord"));
    if (!PathFileExists(szFile))
    {
        if (!CreateDirectory(szFile, NULL))
        {
            g_pMainDlg->AddLog(0, OPERATION_FAIL_T, 2, "Create file failed!");
            return;
        }       
    }
    
    char szLan[128] = { 0 };
    CString csTemp = _T("");

    if (g_pMainDlg->m_pDlgPreview[m_iCurWndIndex].m_lPlayHandle >= 0)
    {
        if (g_pMainDlg->m_pDlgPreview[m_iCurWndIndex].m_bRecord == FALSE)
        {
            g_pMainDlg->m_pDlgPreview[m_iCurWndIndex].m_bRecord = TRUE;
           // m_bRecord = TRUE;
            g_StringLanType(szLan, "停止录像", "Stop Record");
            csTemp = szLan;
            GetDlgItem(IDC_BTN_SALVE)->SetWindowText(csTemp);
            char szLan[256] = { 0 };
            CTime time = CTime::GetCurrentTime();
            g_pMainDlg->m_pDlgPreview[m_iCurWndIndex].m_time = time;
            sprintf(szLan, "Save Record File To C:\\EhomeRecord\\%s_%4d-%02d-%02d_%02d_%02d_%02d_%d.mp4", g_struDeviceInfo[g_pMainDlg->m_pDlgPreview[m_iCurWndIndex].m_iDeviceIndex].byDeviceID,\
                time.GetYear(),time.GetMonth(),time.GetDay(),time.GetHour(),time.GetMinute(),time.GetSecond(),\
                m_iCurChanIndex + 1);
            g_pMainDlg->AddLog(0, OPERATION_SUCC_T, 2, szLan);
        }
        else
        {
            g_pMainDlg->m_pDlgPreview[m_iCurWndIndex].m_bRecord = FALSE;
            //m_bRecord = FALSE;
            if (g_pMainDlg->m_pDlgPreview[m_iCurWndIndex].m_fVideoFile != NULL)
            {
                fclose(g_pMainDlg->m_pDlgPreview[m_iCurWndIndex].m_fVideoFile);
                g_pMainDlg->m_pDlgPreview[m_iCurWndIndex].m_fVideoFile = NULL;
            }
            g_StringLanType(szLan, "开始录像", "Start Record");
            csTemp = szLan;
            GetDlgItem(IDC_BTN_SALVE)->SetWindowText(csTemp);
        }
    }
}


void CEHomeDemoDlg::OnMenuIsapiPt()
{
    // TODO:  在此添加命令处理程序代码
    CDlgISAPIPassthrough dlg;
    dlg.DoModal();
}


void CEHomeDemoDlg::OnMenuUpgrade()
{
    // TODO:  在此添加命令处理程序代码
    CDlgUpgrade dlg;
    dlg.DoModal();
}

void CEHomeDemoDlg::FullScreen(BOOL bFullScreen)
{
    int iShowStat = bFullScreen ? SW_HIDE : SW_SHOW;

    m_treeDeviceList.ShowWindow(iShowStat);

    GetDlgItem(IDC_STATIC_VERSION)->ShowWindow(iShowStat);
    GetDlgItem(IDC_LIST_ALL_LOG)->ShowWindow(iShowStat);
    GetDlgItem(IDC_RADIO_ALARM_INFO)->ShowWindow(iShowStat);
    GetDlgItem(IDC_STATIC)->ShowWindow(iShowStat);
    GetDlgItem(IDC_BTN_EXIT)->ShowWindow(iShowStat);
    GetDlgItem(IDC_TREE_DEV)->ShowWindow(iShowStat);
    GetDlgItem(IDC_BTN_EXIT)->ShowWindow(iShowStat);
    GetDlgItem(IDC_RADIO_LOCAL_LOG)->ShowWindow(iShowStat);
    GetDlgItem(IDC_BTN_PREVIEW)->ShowWindow(iShowStat);
    GetDlgItem(IDC_BTN_AUDIO_TALK)->ShowWindow(iShowStat);
    GetDlgItem(IDC_BTN_CFG)->ShowWindow(iShowStat);
    GetDlgItem(IDC_BTN_PLAYBACK)->ShowWindow(iShowStat);
    GetDlgItem(IDC_BTN_LOCAL_CFG)->ShowWindow(iShowStat);
    GetDlgItem(IDC_BTN_OTHER)->ShowWindow(iShowStat);
    GetDlgItem(IDC_STATIC_DEMO_VERSION)->ShowWindow(iShowStat);
    GetDlgItem(IDC_STATIC_WINNUM)->ShowWindow(iShowStat);
    GetDlgItem(IDC_COMBO_WNDNUM)->ShowWindow(iShowStat);
    GetDlgItem(IDC_BTN_PREVIEW_LISTEN)->ShowWindow(iShowStat);
    GetDlgItem(IDC_BTN_SALVE)->ShowWindow(iShowStat);
    GetDlgItem(IDC_BTN_CIRCLE_PREVIEW)->ShowWindow(iShowStat);
    GetDlgItem(IDC_EDIT_PREVIEW_HANDLE)->ShowWindow(iShowStat);
    GetDlgItem(IDC_BTN_CLOSE_PREVIEW)->ShowWindow(iShowStat);

    if (bFullScreen)
    {
        //for full screen while backplay
        g_pMainDlg->AddLog(g_pMainDlg->m_iCurDeviceIndex, OPERATION_SUCC_T, 0, "full screen display");
        m_dlgOutputCtrl->ShowWindow(SW_HIDE);
        GetWindowPlacement(&m_struOldWndpl);
        CRect rectWholeDlg;//entire client(including title bar)
        CRect rectClient;//client area(not including title bar)
        CRect rectFullScreen;
        GetWindowRect(&rectWholeDlg);
        RepositionBars(0, 0xffff, AFX_IDW_PANE_FIRST, reposQuery, &rectClient);
        ClientToScreen(&rectClient);
        
        rectFullScreen.left = rectWholeDlg.left - rectClient.left;
        rectFullScreen.top = rectWholeDlg.top - rectClient.top;
        rectFullScreen.right = rectWholeDlg.right + g_iCurScreenWidth - rectClient.right;
        rectFullScreen.bottom = rectWholeDlg.bottom + g_iCurScreenHeight - rectClient.bottom;

        //enter into full screen;
        WINDOWPLACEMENT struWndpl;
        struWndpl.length = sizeof(WINDOWPLACEMENT);
        struWndpl.flags = 0;
        struWndpl.showCmd = SW_SHOWNORMAL;
        struWndpl.rcNormalPosition = rectFullScreen;
        SetWindowPlacement(&struWndpl);
        g_pMainDlg->ArrangeOutputs(1);
    }
    else
    {
        m_dlgOutputCtrl->ShowWindow(SW_SHOW);
        SetWindowPlacement(&m_struOldWndpl);
    }
    if (m_iMainType == PREVIEW_T)//dealing while preview
    {
        //refresh backgroud box
        if (bFullScreen)
        {
            GetDlgItem(IDC_STATIC_PREVIEWBG)->MoveWindow(0, 0, g_iCurScreenWidth, g_iCurScreenHeight, true);
            g_pMainDlg->m_pDlgPreview[m_iCurWndIndex].MoveWindow(0, 0, g_iCurScreenWidth, g_iCurScreenHeight, TRUE);
            g_pMainDlg->m_pDlgPreview[m_iCurWndIndex].ShowWindow(SW_SHOW);
        }
        else
        {
            GetDlgItem(IDC_STATIC_PREVIEWBG)->MoveWindow(&m_rectPreviewBG, true);
            g_pMainDlg->m_pDlgPreview[m_iCurWndIndex].ShowWindow(SW_SHOW);
        }
    }
}

void CEHomeDemoDlg::OnBnClickedBtnSound()
{
    // TODO:  在此添加控件通知处理程序代码
    char szLan[128] = { 0 };
    if (m_bSound)
    {
        m_bSound = FALSE;
        g_StringLanType(szLan, "打开声音", "Open Audio");
    }
    else
    {
        m_bSound = TRUE;
        g_StringLanType(szLan, "关闭声音", "Close Audio");
    }
    GetDlgItem(IDC_BTN_SOUND)->SetWindowText(szLan);

}


void CEHomeDemoDlg::OnEnableCfg()
{
    // TODO:  在此添加命令处理程序代码
    CDlgMotionConfig dlg;
    dlg.DoModal();
}


void CEHomeDemoDlg::OnRecodrCfg()
{
    // TODO:  在此添加命令处理程序代码
    CDlgRecordCfg dlg;
    dlg.DoModal();
}


void CEHomeDemoDlg::OnCapturePic()
{
    // TODO:  在此添加命令处理程序代码
    CDlgCaptureCfg dlg;
    dlg.DoModal();
}


void CEHomeDemoDlg::OnMontionArea()
{
    // TODO:  在此添加命令处理程序代码
    CDlgMotionArea dlg;
    dlg.DoModal();
}



void CEHomeDemoDlg::OnPrivateArea()
{
    // TODO:  在此添加命令处理程序代码
    CDlgPrivateArea dlg;
    dlg.DoModal();
}


void CEHomeDemoDlg::OnHideAlarm()
{
    // TODO:  在此添加命令处理程序代码
    CDlgHidArea dlg;
    dlg.DoModal();
}


void CEHomeDemoDlg::OnMenuHttpUpgrade()
{
    // TODO:  在此添加命令处理程序代码
    // TODO:  在此添加命令处理程序代码
    CDlgUpgradeHttp dlg;
    dlg.DoModal();
}


void CEHomeDemoDlg::OnMenuIsapiCfg()
{
    // TODO:  在此添加命令处理程序代码
    CDlgISAPIConfig dlg;
    dlg.DoModal();
}
