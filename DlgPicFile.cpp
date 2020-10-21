// DlgPicFile.cpp : 实现文件
//

#include "stdafx.h"
#include "EHomeDemo.h"
#include "DlgPicFile.h"


// CDlgPicFile 对话框

IMPLEMENT_DYNAMIC(CDlgPicFile, CDialog)

CDlgPicFile::CDlgPicFile(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgPicFile::IDD, pParent)
    , m_dateStart(0)
    , m_timeStart(0)
    , m_DateStop(0)
    , m_timeStop(0)
    , m_byTimeType(0)
{

}

CDlgPicFile::~CDlgPicFile()
{
}

void CDlgPicFile::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_PIC_FILE, m_lstPicFile);
    DDX_DateTimeCtrl(pDX, IDC_COMBO_PIC_DATE_START, m_dateStart);
    DDX_DateTimeCtrl(pDX, IDC_COMBO_PIC_TIME_START, m_timeStart);
    DDX_DateTimeCtrl(pDX, IDC_COMBO_PIC_DATE_STOP, m_DateStop);
    DDX_DateTimeCtrl(pDX, IDC_COMBO_PIC_TIME_STOP, m_timeStop);
    DDX_Control(pDX, IDC_COMBO_PIC_FILE_TYPE, m_cmbPicType);
    DDX_CBIndex(pDX, IDC_COMBO_TIME_TYPE, m_byTimeType);
}


BEGIN_MESSAGE_MAP(CDlgPicFile, CDialog)
    ON_BN_CLICKED(IDC_BTN_SEARCH_PIC_FILE, &CDlgPicFile::OnBnClickedBtnSearchPicFile)
END_MESSAGE_MAP()


BOOL CDlgPicFile::CheckInitParam()
{
    int iDeviceIndex = g_pMainDlg->GetCurDeviceIndex();
    if (iDeviceIndex == -1)
    {
        UpdateData(FALSE);
        return FALSE;
    }

    if ( m_iDeviceIndex == -1 || m_iDeviceIndex != iDeviceIndex)
    {
        m_iChanIndex = -1;
    }

    int iChanIndex = g_pMainDlg->GetCurChanIndex();
    if (iChanIndex == -1)
    {
        //g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, "set the channel index = 0");
        //iChanIndex = 0;
        AfxMessageBox("please select a channel!");
        UpdateData(FALSE);
        return FALSE;
    }

    UpdateData(TRUE);

    m_iDeviceIndex = iDeviceIndex;
    m_lLoginID = g_struDeviceInfo[m_iDeviceIndex].lLoginID;

    //initialize parameters again while switching channel
    if (m_iChanIndex != iChanIndex)
    {
        m_iChanIndex = iChanIndex;


        m_bSearching = FALSE;

        //m_hPareDlgWnd = NULL;
        //m_hPareTabWnd = NULL;

        m_hFileThread = NULL;
        m_dwFileNum = 0;

    }

    if (m_lLoginID < 0)
    {
        UpdateData(FALSE);
        return FALSE;
    }

    return TRUE;

}


/*********************************************************
Function:    GetFileThread
Desc:        get recorded file list thread
Input:        pParam,pointer to parameters
Output:    
Return:    
**********************************************************/
UINT CDlgPicFile::GetFileThread(LPVOID pParam)
{
    CDlgPicFile *pThis = static_cast<CDlgPicFile*>(pParam);
    LONG lRet = -1;
    //NET_EHOME_FINDDATA struFileInfo;
   // NET_EHOME_DEV_LOG struFileInfo;
    NET_EHOME_PIC_FILE struFileInfo;
    memset(&struFileInfo, 0, sizeof(struFileInfo));
    CString csTmp;
    char szLan[128] = {0};


    while (!pThis->m_bQuit)
    {
        lRet = lRet = NET_ECMS_FindNextFile_V11(pThis->m_lFileHandle, &struFileInfo, sizeof(struFileInfo));
        if (lRet == ENUM_GET_NEXT_STATUS_SUCCESS)
        {            
            memcpy(&pThis->m_struFindData, &struFileInfo, sizeof(struFileInfo)); 
            pThis->m_lstPicFile.InsertItem(pThis->m_dwFileNum, struFileInfo.sFileName, 0);            
            if (struFileInfo.dwFileSize / 1024 == 0)
            {
                csTmp.Format("%d",struFileInfo.dwFileSize);
            }
            else if (struFileInfo.dwFileSize / 1024 > 0 && struFileInfo.dwFileSize / (1024*1024) == 0)
            {
                csTmp.Format("%dK",struFileInfo.dwFileSize / 1024);
            }
            else// if ()
            {
                csTmp.Format("%dM",struFileInfo.dwFileSize / 1024/1024);//different from hard disk capacity, files need tranformation
            }
            csTmp.Format("%d",struFileInfo.dwFileSize);
            pThis->m_lstPicFile.SetItemText(pThis->m_dwFileNum, 1, csTmp);
            csTmp.Format("%04d%02d%02d%02d%02d%02d",struFileInfo.struPicTime.wYear, \
                struFileInfo.struPicTime.byMonth, struFileInfo.struPicTime.byDay, \
                struFileInfo.struPicTime.byHour, struFileInfo.struPicTime.byMinute, \
                struFileInfo.struPicTime.bySecond);
             pThis->m_lstPicFile.SetItemText(pThis->m_dwFileNum, 2, csTmp);
//             csTmp.Format("%04d%02d%02d%02d%02d%02d", struFileInfo.struStopTime.wYear, struFileInfo.struStopTime.byMonth,\
//                 struFileInfo.struStopTime.byDay, struFileInfo.struStopTime.byHour, \
//                 struFileInfo.struStopTime.byMinute, struFileInfo.struStopTime.bySecond);
//             pThis->m_lstPicFile.SetItemText(pThis->m_dwFileNum, 3, csTmp);


            csTmp.Format("%d",struFileInfo.dwFileMainType);
            pThis->m_lstPicFile.SetItemText(pThis->m_dwFileNum, 4, csTmp);

           // csTmp.Format("%d",struFileInfo.dwFileSubType);
          //  pThis->m_lstPicFile.SetItemText(pThis->m_dwFileNum, 5, csTmp);

            csTmp.Format("%d",struFileInfo.dwFileIndex);
            pThis->m_lstPicFile.SetItemText(pThis->m_dwFileNum, 6, csTmp);

            csTmp.Format("%d", struFileInfo.byTimeDiffH);
            pThis->m_lstPicFile.SetItemText(pThis->m_dwFileNum, 7, csTmp);

            csTmp.Format("%d", struFileInfo.byTimeDiffM);
            pThis->m_lstPicFile.SetItemText(pThis->m_dwFileNum, 8, csTmp);


            pThis->m_dwFileNum++;
        }
        else
        {
            if (lRet == ENUM_GET_NETX_STATUS_NEED_WAIT)
            {
                Sleep(5);
                continue;
            }

            if ((lRet == ENUM_GET_NETX_STATUS_NO_FILE) || (lRet == ENUM_GET_NEXT_STATUS_FINISH))
            {
                g_StringLanType(szLan, "查找", "Search");
                pThis->GetDlgItem(IDC_BTN_SEARCH_PIC_FILE)->SetWindowText(szLan);
                pThis->m_bSearching = FALSE;
                pThis->GetDlgItem(IDC_STATIC_FINDING_PIC_FILE)->ShowWindow(SW_HIDE);
                g_pMainDlg->AddLog(pThis->m_iDeviceIndex, OPERATION_SUCC_T, 1, "NET_ECMS_FindNextFile file num[%d]", pThis->m_dwFileNum);
                pThis->m_dwFileNum = 0;
                break;
            }
            else if(lRet == ENUM_GET_NEXT_STATUS_NOT_SUPPORT)
            {
                pThis->GetDlgItem(IDC_BTN_SEARCH_PIC_FILE)->SetWindowText("查找");
                pThis->m_bSearching = FALSE;
                pThis->GetDlgItem(IDC_STATIC_FINDING_PIC_FILE)->ShowWindow(SW_HIDE);
                g_pMainDlg->AddLog(pThis->m_iDeviceIndex, OPERATION_FAIL_T, 1, "NET_ECMS_FindNextFile");
                g_StringLanType(szLan, "设备不支持该操作", "Device do not support");
                AfxMessageBox(szLan);
                pThis->m_dwFileNum = 0;
                break;
            }
            else
            {
                pThis->GetDlgItem(IDC_BTN_SEARCH_PIC_FILE)->SetWindowText("查找");
                pThis->m_bSearching = FALSE;
                pThis->GetDlgItem(IDC_STATIC_FINDING_PIC_FILE)->ShowWindow(SW_HIDE);
                g_pMainDlg->AddLog(pThis->m_iDeviceIndex, OPERATION_FAIL_T, 1, "NET_ECMS_FindNextFile");
                g_StringLanType(szLan, "由于服务器忙,或网络故障,获取文件列表异常终止", "Since the server is busy, or network failure, abnormal termination of access to the file list");
                AfxMessageBox(szLan);
                pThis->m_dwFileNum = 0;
                break;
            }
        }
    }
    CloseHandle(pThis->m_hFileThread);
    pThis->m_hFileThread = NULL;
    NET_ECMS_StopFindFile(pThis->m_lFileHandle);
    pThis->m_lFileHandle = -1;
    pThis->m_bSearching = FALSE;
    return 0;
}

// CDlgPicFile 消息处理程序

void CDlgPicFile::OnBnClickedBtnSearchPicFile()
{
    // TODO: 在此添加控件通知处理程序代码
    UpdateData(TRUE);
    CheckInitParam();
    char szLan[128] = {0};
    if (m_iDeviceIndex < 0)
    {
        g_StringLanType(szLan,"请先登录设备!", "Please Login First!");
        AfxMessageBox(szLan);
        return;
    }

    if (!m_bSearching)
    {
        m_bQuit = FALSE;

        memset(&m_struFindCond, 0, sizeof(m_struFindCond));
        m_struFindCond.dwSize = sizeof(m_struFindCond);
        m_struFindCond.enumSearchType = ENUM_SEARCH_PICTURE_FILE;
        m_struFindCond.dwMaxFileCountPer = 10;
        m_struFindCond.iChannel = g_struDeviceInfo[m_iDeviceIndex].struChanInfo[m_iChanIndex].iChannelNO;
        m_struFindCond.unionSearchParam.struPicFileParam.dwFileType = m_cmbPicType.GetItemData(m_cmbPicType.GetCurSel());

        m_struFindCond.struStartTime.wYear = (WORD)m_dateStart.GetYear();
        m_struFindCond.struStartTime.byMonth = (BYTE)m_dateStart.GetMonth();
        m_struFindCond.struStartTime.byDay = (BYTE)m_dateStart.GetDay();
        m_struFindCond.struStartTime.byHour = (BYTE)m_timeStart.GetHour();
        m_struFindCond.struStartTime.byMinute = (BYTE)m_timeStart.GetMinute();
        m_struFindCond.struStartTime.bySecond = (BYTE)m_timeStart.GetSecond();

        m_struFindCond.struStopTime.wYear = (WORD)m_DateStop.GetYear();
        m_struFindCond.struStopTime.byMonth = (BYTE)m_DateStop.GetMonth();
        m_struFindCond.struStopTime.byDay = (BYTE)m_DateStop.GetDay();
        m_struFindCond.struStopTime.byHour = (BYTE)m_timeStop.GetHour();
        m_struFindCond.struStopTime.byMinute = (BYTE)m_timeStop.GetMinute();
        m_struFindCond.struStopTime.bySecond = (BYTE)m_timeStop.GetSecond();

        
        NET_EHOME_PIC_FILE_COND struPicCond = { 0 };
        struPicCond.dwMaxFileCountPer = 10;
        struPicCond.dwChannel = g_struDeviceInfo[m_iDeviceIndex].struChanInfo[m_iChanIndex].iChannelNO;
        struPicCond.dwPicType = m_cmbPicType.GetItemData(m_cmbPicType.GetCurSel());
        

        struPicCond.struStartTime.wYear = (WORD)m_dateStart.GetYear();
        struPicCond.struStartTime.byMonth = (BYTE)m_dateStart.GetMonth();
        struPicCond.struStartTime.byDay = (BYTE)m_dateStart.GetDay();
        struPicCond.struStartTime.byHour = (BYTE)m_timeStart.GetHour();
        struPicCond.struStartTime.byMinute = (BYTE)m_timeStart.GetMinute();
        struPicCond.struStartTime.bySecond = (BYTE)m_timeStart.GetSecond();

        struPicCond.struStopTime.wYear = (WORD)m_DateStop.GetYear();
        struPicCond.struStopTime.byMonth = (BYTE)m_DateStop.GetMonth();
        struPicCond.struStopTime.byDay = (BYTE)m_DateStop.GetDay();
        struPicCond.struStopTime.byHour = (BYTE)m_timeStop.GetHour();
        struPicCond.struStopTime.byMinute = (BYTE)m_timeStop.GetMinute();
        struPicCond.struStopTime.bySecond = (BYTE)m_timeStop.GetSecond();

        NET_EHOME_PIC_FILE_COND struPicFileCond = { 0 };
        struPicFileCond.dwChannel = g_struDeviceInfo[m_iDeviceIndex].struChanInfo[m_iChanIndex].iChannelNO;
        struPicFileCond.byLocalOrUTC = (BYTE)m_byTimeType;
        struPicFileCond.dwPicType = m_cmbPicType.GetItemData(m_cmbPicType.GetCurSel());

        struPicFileCond.struStartTime.wYear = (WORD)m_dateStart.GetYear();
        struPicFileCond.struStartTime.byMonth = (BYTE)m_dateStart.GetMonth();
        struPicFileCond.struStartTime.byDay = (BYTE)m_dateStart.GetDay();
        struPicFileCond.struStartTime.byHour = (BYTE)m_timeStart.GetHour();
        struPicFileCond.struStartTime.byMinute = (BYTE)m_timeStart.GetMinute();
        struPicFileCond.struStartTime.bySecond = (BYTE)m_timeStart.GetSecond();

        struPicFileCond.struStopTime.wYear = (WORD)m_DateStop.GetYear();
        struPicFileCond.struStopTime.byMonth = (BYTE)m_DateStop.GetMonth();
        struPicFileCond.struStopTime.byDay = (BYTE)m_DateStop.GetDay();
        struPicFileCond.struStopTime.byHour = (BYTE)m_timeStop.GetHour();
        struPicFileCond.struStopTime.byMinute = (BYTE)m_timeStop.GetMinute();
        struPicFileCond.struStopTime.bySecond = (BYTE)m_timeStop.GetSecond();
        struPicFileCond.dwMaxFileCountPer = 8;
        


       // m_lFileHandle = NET_ECMS_StartFindFile(m_lLoginID, &m_struFindCond);

       //m_lFileHandle = NET_ECMS_StartFindFile_V11(m_lLoginID, ENUM_SEARCH_PICTURE_FILE, &struPicCond, sizeof(struPicCond));
        m_lFileHandle = NET_ECMS_StartFindFile_V11(m_lLoginID, ENUM_SEARCH_PICTURE_FILE, &struPicFileCond, sizeof(struPicFileCond));
        if (m_lFileHandle < 0)
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, 1, "NET_ECMS_StartFindFile");
            g_StringLanType(szLan, "查找文件列表失败!", "Fail to get file list");
            AfxMessageBox(szLan);
            return;
        }
        m_lstPicFile.DeleteAllItems();
        DWORD dwThreadId;
        if (m_hFileThread == NULL)
        {
            m_hFileThread = CreateThread(NULL,0,LPTHREAD_START_ROUTINE(GetFileThread),this,0,&dwThreadId);      
            if (m_hFileThread == NULL)
            {
                g_StringLanType(szLan, "打开查找线程失败!", "Fail to open finding thread!");
                AfxMessageBox(szLan);
                return;
            }
        }


        g_StringLanType(szLan, "停止查找", "Stop Searching");
        GetDlgItem(IDC_BTN_SEARCH_PIC_FILE)->SetWindowText(szLan);
        m_bSearching = TRUE;
        GetDlgItem(IDC_STATIC_FINDING_PIC_FILE)->ShowWindow(SW_SHOW);
    }
    else
    {
        if (m_hFileThread)
        {
            m_bQuit = TRUE;
            //TerminateThread(m_hFileThread, 0);
        }
        CloseHandle(m_hFileThread);
        m_hFileThread = NULL;
        NET_ECMS_StopFindFile(m_lFileHandle);
        g_StringLanType(szLan, "查找", "Search");
        GetDlgItem(IDC_BTN_SEARCH_PIC_FILE)->SetWindowText(szLan);
        m_bSearching = FALSE;
        GetDlgItem(IDC_STATIC_FINDING_PIC_FILE)->ShowWindow(SW_HIDE);
        m_dwFileNum = 0;
    }
}

BOOL CDlgPicFile::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  在此添加额外的初始化

    char szLan[128] = {0};

    g_StringLanType(szLan, "文件名称", "File Name");
    m_lstPicFile.InsertColumn(0, szLan,LVCFMT_LEFT,150,-1);
    g_StringLanType(szLan, "大小", "Size");
    m_lstPicFile.InsertColumn(1, szLan,LVCFMT_LEFT,50,-1);
    g_StringLanType(szLan, "开始时间", "Start time");
    m_lstPicFile.InsertColumn(2, szLan, LVCFMT_LEFT, 120, -1);

    g_StringLanType(szLan, "结束时间", "Stop Time");
    m_lstPicFile.InsertColumn(3, szLan, LVCFMT_LEFT, 120, -1);

    g_StringLanType(szLan,"主类型","Main Type");
    m_lstPicFile.InsertColumn(4, szLan,LVCFMT_LEFT,50,-1);

    g_StringLanType(szLan,"次类型","Sub Type");
    m_lstPicFile.InsertColumn(5, szLan,LVCFMT_LEFT,50,-1);

    g_StringLanType(szLan,"索引","Index");
    m_lstPicFile.InsertColumn(6, szLan,LVCFMT_LEFT,50,-1);

    g_StringLanType(szLan, "时差时", "Time Diff H");
    m_lstPicFile.InsertColumn(7, szLan, LVCFMT_LEFT, 50, -1);

    g_StringLanType(szLan, "时差分", "Time Diff M");
    m_lstPicFile.InsertColumn(8, szLan, LVCFMT_LEFT, 50, -1);



    CTime timeCur = CTime::GetCurrentTime();
    CTime timeStart(timeCur.GetYear(),timeCur.GetMonth(),timeCur.GetDay(),0,0,0);
    CTime timeStop(timeCur.GetYear(),timeCur.GetMonth(),timeCur.GetDay(),23,59,59);
    m_dateStart = timeStart;
    m_timeStart = timeStart;
    m_DateStop = timeStop;
    m_timeStop = timeStop;

    m_bQuit = FALSE;

    int index = 0;
    m_cmbPicType.ResetContent();
    g_StringLanType(szLan, "全部", "All");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, FILE_ALL);
    index++;

    g_StringLanType(szLan, "定时录像", "Schedule");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, FILE_TIMING);
    index++;

    g_StringLanType(szLan, "移动侦测", "Motion Detect");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, FILE_MOTION_DETECT);
    index++;

    g_StringLanType(szLan, "报警触发", "Alarm");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, FILE_ALARMIN);
    index++;

    g_StringLanType(szLan, "报警|动测", "Alarm|Motion");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, FILE_MOTION_OR_ALARMIN);
    index++;

    g_StringLanType(szLan, "报警&动测", "Alarm&Motion");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, FILE_MOTION_AND_ALARMIN);
    index++;

    g_StringLanType(szLan, "命令触发", "Command Record");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, FILE_COMMAND);
    index++;

    g_StringLanType(szLan, "手动录像", "Manual Record");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, FILE_MANUAL);
    index++;

    g_StringLanType(szLan, "震动报警", "Shake Alarm");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, FILE_SHAKE_ALARM);
    index++;

    g_StringLanType(szLan, "环境报警", "Environment Alarm");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, FILE_ENVIRONMENT_ALARM);
    index++;

    g_StringLanType(szLan, "智能报警", "VCA Alarm");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, FILE_VCA_ALARM);
    index++;

    g_StringLanType(szLan, "PIR报警", "PIR Alarm");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, FILE_PIR_ALARM);
    index++;

    g_StringLanType(szLan, "无线报警", "WIRELESS Alarm");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, FILE_WIRELESS_ALARM);
    index++;

    g_StringLanType(szLan, "呼救报警", "CALLHELP Alarm");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, FILE_CALLHELP_ALARM);
    index++;

    g_StringLanType(szLan, "全部报警录像", "Alarm Record");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, FILE_ALL_ALARM);
    index++;

    g_StringLanType(szLan, "车牌识别图片", "License Recognition");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, PIC_LICENSE_RECOGNITION);
    index++;


    g_StringLanType(szLan, "稽查报警图片", "Inspection Alarm");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, PIC_INSPECT_ALARM);
    index++;


    g_StringLanType(szLan, "手动抓拍图片", "Manual Snap");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, PIC_MANUAL_SNAP);
    index++;

    g_StringLanType(szLan, "回放抓拍图片", "PlayBack Snap");
    m_cmbPicType.InsertString(index, szLan);
    m_cmbPicType.SetItemData(index, PIC_PLAYBACK_SNAP);
    index++;

    m_cmbPicType.SetCurSel(0);

    UpdateData(FALSE);

    return TRUE;  // return TRUE unless you set the focus to a control
    // 异常: OCX 属性页应返回 FALSE
}
