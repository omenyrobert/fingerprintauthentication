// DlgAudioTalk.cpp : implementation file
//

#include "stdafx.h"
#include "EHomeDemo.h"
#include "DlgAudioTalk.h"
#include <assert.h>
#include "Public/Convert/Convert.h"



// CDlgAudioTalk dialog

BOOL g_bStopAudioTalk = TRUE;

char g_szRemoteIP[128] = {0};
int  g_nRemotePort = 0;

IMPLEMENT_DYNAMIC(CDlgAudioTalk, CDialog)

#define CSTRING_TO_CHARS(str,sz) memcpy((sz),str.GetBuffer(str.GetLength()),str.GetLength())

CDlgAudioTalk *CDlgAudioTalk::s_pAudioTalkDlg = NULL;

/*********************************************************
  Function:    DataFromSoundIn
  Desc:        get local audio data and send to device
  Input:    buffer, local voice data buffer; dwSize, data length; dwOwner, owner data;
  Output:    none
  Return:    none
**********************************************************/
void CALLBACK DataFromSoundInEx(char* buffer, DWORD dwSize, DWORD dwOwner)
{
    ASSERT(dwOwner);
    CDlgAudioTalk* p = (CDlgAudioTalk*) dwOwner;
    p->SendDataToDevice(buffer, dwSize);
}


CDlgAudioTalk::CDlgAudioTalk(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgAudioTalk::IDD, pParent)
{
    memset(&m_struVoiceTalkPara, 0, sizeof(m_struVoiceTalkPara));
    m_lVoiceTalkHandle = -1;
    m_hVoiceTransmit = NULL;
    m_bExitThread = FALSE;
    m_bCMSAudioTalk = TRUE;

    m_csIP = g_pMainDlg->m_sLocalIP;
    m_nPort = 7500;

    m_lListenHandle = -1;

    m_bWaveDeal = FALSE;

    m_dwBufNum = 6;

    memset(&m_talkMr, 0, sizeof(m_talkMr));

    m_talkMr.byAudioType = AUDIOTALKTYPE_PCM;

    //init WAVEFORMATEX
    m_struWaveFormat.cbSize             = sizeof(WAVEFORMATEX);
    m_struWaveFormat.nBlockAlign        = CHANNEL * BITS_PER_SAMPLE / 8;
    m_struWaveFormat.nChannels          = CHANNEL;
    m_struWaveFormat.nSamplesPerSec     = SAMPLES_PER_SECOND;
    m_struWaveFormat.wBitsPerSample     = BITS_PER_SAMPLE;
    m_struWaveFormat.nAvgBytesPerSec    = SAMPLES_PER_SECOND * m_struWaveFormat.nBlockAlign;
    m_struWaveFormat.wFormatTag         = WAVE_FORMAT_PCM;

    m_hExitEvent = NULL;

    m_pRenderBuf = NULL;
    m_rIndexRV = 0;
    m_ReceiveIndexRV = 0;
    m_nBufNo = 0;
    m_bOpenPlayThread = FALSE;

    m_pRenderBuf = new BYTE[160*40];

    m_bRecord = FALSE;

    m_nLocalType = 0;
    m_bOpenTalk = FALSE;
}

CDlgAudioTalk::~CDlgAudioTalk()
{
    if (m_pRenderBuf != NULL)
    {
        delete[] m_pRenderBuf;
    }

    DeleteCriticalSection(&m_csAudioBuf);
    
}

void CDlgAudioTalk::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CMB_VOICETALK_CB_TYPE, m_combCBDataType);
    DDX_Control(pDX, IDC_CMB_VOICE_CHAN, m_cmbVoiceChan);
    DDX_Control(pDX, IDC_COM_LOCAL_TYPE, m_cmbLocalType);
    DDX_Text(pDX, IDC_EDT_STREAM_PORT, m_nPort);
    DDX_Text(pDX, IDC_IPADDR_STREAM_SERVER, m_csIP);
}


BEGIN_MESSAGE_MAP(CDlgAudioTalk, CDialog)
    ON_BN_CLICKED(IDC_BTN_START_VOICETALK, &CDlgAudioTalk::OnBnClickedBtnStartVoicetalk)
    ON_BN_CLICKED(IDC_BTN_STOP_VOICETALK, &CDlgAudioTalk::OnBnClickedBtnStopVoicetalk)
    ON_BN_CLICKED(IDC_BTN_START_TRANSMIT, &CDlgAudioTalk::OnBnClickedBtnStartTransmit)
    ON_BN_CLICKED(IDC_BTN_STOP_TRANSMIT, &CDlgAudioTalk::OnBnClickedBtnStopTransmit)
    ON_BN_CLICKED(IDC_BTN_LISTEN, &CDlgAudioTalk::OnBnClickedBtnStartVoicetalkListen)
    ON_BN_CLICKED(IDC_BTN_REQ_AUDIOTALK, &CDlgAudioTalk::OnBnClickedBtnReqAudioTalk)
    ON_BN_CLICKED(IDC_BTN_START_PUSH, &CDlgAudioTalk::OnBnClickedBtnStartPush)
    ON_BN_CLICKED(IDC_BTN_STOP_AUDIOTALK, &CDlgAudioTalk::OnBnClickedBtnStopPush)
    ON_BN_CLICKED(IDC_BTN_STOP_LISTEN, &CDlgAudioTalk::OnBnClickedBtnStopListen)
    ON_BN_CLICKED(IDC_BTN_AUDIOTALK_TEST, &CDlgAudioTalk::OnBnClickedBtnAudiotalkTest)
END_MESSAGE_MAP()



BOOL CDlgAudioTalk::OnInitDialog()
{
    CDialog::OnInitDialog();

    char szLan[128] = {0};
    CString csTemp = _T("");

    // TODO:  Add extra initialization here
    m_combCBDataType.DeleteString(0);
    g_StringLanType(szLan, "��������", "decode data");
    csTemp = szLan;
    m_combCBDataType.AddString(csTemp);
    g_StringLanType(szLan, "PCM����", "PCM data");
    csTemp = szLan;
    m_combCBDataType.AddString(csTemp);

    m_combCBDataType.SetCurSel(0);

    m_hExitEvent   = CreateEvent(NULL, TRUE, FALSE, NULL);
    m_hHaveRDataRV = CreateEvent(NULL, TRUE, FALSE, NULL);

    //encoder
    //m_pEncoder = NET_DVR_InitG722Encoder();

    //g726
    //m_pG726Enc = NET_DVR_InitG726Encoder(&m_pG726EncM);

    m_cmbLocalType.InsertString(0, _T("none"));
    m_cmbLocalType.InsertString(1, _T("Local Play"));
    m_cmbLocalType.InsertString(2, _T("ECHO"));
    m_cmbLocalType.InsertString(3, _T("ECHO & Local Play"));
    m_cmbLocalType.SetCurSel(0);


    UpdateData(FALSE);

    s_pAudioTalkDlg = this;

    InitializeCriticalSection(&m_csAudioBuf);


    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

FILE *g_file = NULL;

void CALLBACK g_fVoiceDataCallBack(LONG iVoiceComHandle, char *pRecvDataBuffer, DWORD dwBufSize, DWORD dwEncodeType, BYTE byAudioFlag, void *pUser)
{
    UN_REFERENCED_PARAMETER(iVoiceComHandle)
    UN_REFERENCED_PARAMETER(pRecvDataBuffer)
    UN_REFERENCED_PARAMETER(dwBufSize)
    UN_REFERENCED_PARAMETER(dwEncodeType)
    UN_REFERENCED_PARAMETER(pUser)
    UN_REFERENCED_PARAMETER(byAudioFlag)
    /*
    if (byAudioFlag == 1)
    {
        if (g_file == NULL)
        {
            char szAudioFilePath[MAX_PATH] = {0};
            char szAppPath[MAX_PATH] = {0};
            GetCurrentDirectory(MAX_PATH, szAppPath);                   //��ȡ����ĵ�ǰĿ¼
            sprintf(szAudioFilePath, "%s/AudioFiles/%s", szAppPath, "ehome_recv.au");

            g_file = fopen(szAudioFilePath, "wb");
        }
        fwrite(pRecvDataBuffer, dwBufSize, 1, g_file);
    }
    */
}

//�豸��������ʱ
BOOL CALLBACK CDlgAudioTalk::onDeviceAudioTalkConnectCB(LONG lAudioTalkHandle, NET_EHOME_VOICETALK_NEWLINK_CB_INFO *pNewLinkCBMsg, void *pUserData)
{
    CDlgAudioTalk *pVoiceTalk = (CDlgAudioTalk*)pUserData;
    pVoiceTalk->m_lAudioTalkHandle = lAudioTalkHandle;

    g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_SUCC_T, 2, "onDeviceAudioTalkConnectCB: szDeviceID[%s] iSessionID[%d] dwChannelNo[%d] dwEncodeType[%d]",
        pNewLinkCBMsg->szDeviceID, pNewLinkCBMsg->lSessionID, pNewLinkCBMsg->dwAudioChan, pNewLinkCBMsg->dwEncodeType);

    //���ûص�
    NET_EHOME_VOICETALK_DATA_CB_PARAM struAudioTalkCBParam = {0};

    struAudioTalkCBParam.fnVoiceTalkDataCB = CDlgAudioTalk::AudioTalkStreamCallback;
    struAudioTalkCBParam.pUserData = pUserData;

    if (!NET_ESTREAM_SetVoiceTalkDataCB(lAudioTalkHandle, &struAudioTalkCBParam))
    {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_FAIL_T, 2, "NET_ESTREAM_SetVoiceTalkDataCB, Error=%d", NET_ESTREAM_GetLastError());
    }
    else
    {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_SUCC_T, 1, "NET_ESTREAM_SetVoiceTalkDataCB");
    }

    //pVoiceTalk->OpenAudioOut();//���Ž��յ���Ƶ����
   // pVoiceTalk->OpenAudioIn(); //�ɼ�������Ƶ����

    return TRUE;
}

void CDlgAudioTalk::OnBnClickedBtnStartVoicetalkListen()
{
    UpdateData(TRUE);
    if (m_lListenHandle > -1)
    {
        MessageBox("Already StartListen");
        return;
    }

    NET_EHOME_LISTEN_VOICETALK_CFG struListen = {0};
    struListen.fnNewLinkCB = CDlgAudioTalk::onDeviceAudioTalkConnectCB;
    struListen.pUser = this;

    CSTRING_TO_CHARS(m_csIP, struListen.struIPAdress.szIP);
    struListen.struIPAdress.wPort = (WORD)m_nPort;     //�˿�

    m_lListenHandle = NET_ESTREAM_StartListenVoiceTalk(&struListen);
    if (m_lListenHandle > -1)
    {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_SUCC_T, 1, "NET_ESTREAM_StartListenAudioTalk, Error=[%d]", NET_ESTREAM_GetLastError());
    }
    else
    {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_FAIL_T, 2, "NET_ESTREAM_StartListenAudioTalk, Error=[%d]", NET_ESTREAM_GetLastError());
    }

    UpdateData(FALSE);
}

void CDlgAudioTalk::OnBnClickedBtnStopListen()
{
     if (NET_ESTREAM_StopListenVoiceTalk(m_lListenHandle))
     {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_SUCC_T, 1, "NET_ESTREAM_StopListenVoiceTalk");
     }
     else
     {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_FAIL_T, 2, "NET_ESTREAM_StopListenVoiceTalk");
     }
     m_lListenHandle = -1;
}

void CDlgAudioTalk::OnBnClickedBtnReqAudioTalk()
{
    // TODO: Add your control notification handler code here

    UpdateData(TRUE);

    if (m_bOpenTalk)
    {
        Sleep(5000);
    }


    m_bCMSAudioTalk = FALSE;

    int iDeviceIndex = g_pMainDlg->GetCurDeviceIndex();

    NET_EHOME_VOICE_TALK_IN struVoiceTalkIn = {0};
    struVoiceTalkIn.dwVoiceChan = 1;//m_cmbVoiceChan.GetCurSel() + 1;
    struVoiceTalkIn.struStreamSever.wPort = (WORD)m_nPort;
    CSTRING_TO_CHARS(m_csIP, struVoiceTalkIn.struStreamSever.szIP);

    NET_EHOME_DEVICE_INFO struDevInfo = { 0 };
    struDevInfo.dwSize = sizeof(NET_EHOME_DEVICE_INFO);
    NET_EHOME_CONFIG struCfg = { 0 };
    struCfg.pOutBuf = &struDevInfo;
    struCfg.dwOutSize = sizeof(NET_EHOME_DEVICE_INFO);
    if (!NET_ECMS_GetDevConfig(g_struDeviceInfo[iDeviceIndex].lLoginID, NET_EHOME_GET_DEVICE_INFO, &struCfg, sizeof(NET_EHOME_CONFIG)))
    {
        g_pMainDlg->AddLog(-1, OPERATION_FAIL_T, 1, "NET_EHOME_GET_DEVICE_INFO");
        //  return -1;
    }

    m_talkMr.byAudioType = struDevInfo.dwAudioEncType;


    NET_EHOME_VOICE_TALK_OUT struVoiceTalkOut = {0};

    if (!NET_ECMS_StartVoiceWithStmServer(g_struDeviceInfo[iDeviceIndex].lLoginID, &struVoiceTalkIn, &struVoiceTalkOut))
    {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_FAIL_T, 1, "NET_ECMS_StartVoiceWithStmServer");
    }
    else
    {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_SUCC_T, 1, "NET_ECMS_StartVoiceWithStmServer");
        m_iSessionID = struVoiceTalkOut.lSessionID;        

        OnBnClickedBtnStartPush();
        m_bOpenTalk = TRUE;
    }
}

void CDlgAudioTalk::OnBnClickedBtnStartPush()
{
    m_nLocalType = m_cmbLocalType.GetCurSel(); 
    int iDeviceIndex = g_pMainDlg->GetCurDeviceIndex();

    OpenAudioOut();//���Ž��յ���Ƶ����

    NET_EHOME_PUSHVOICE_IN struPushVoiceIn = {0};
    struPushVoiceIn.dwSize = sizeof(struPushVoiceIn);
    struPushVoiceIn.lSessionID = m_iSessionID;

    NET_EHOME_PUSHVOICE_OUT struPushVoiceOut = {0};
    struPushVoiceOut.dwSize = sizeof(struPushVoiceOut);

    if (!NET_ECMS_StartPushVoiceStream(g_struDeviceInfo[iDeviceIndex].lLoginID, &struPushVoiceIn, &struPushVoiceOut))
    {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_FAIL_T, 1, "NET_ECMS_StartPushVoiceStream");
    }
    else
    {
        g_bStopAudioTalk = FALSE;
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_SUCC_T, 1, "NET_ECMS_StartPushVoiceStream");
    }
}


void CDlgAudioTalk::OnBnClickedBtnStopPush()
{
    g_bStopAudioTalk = TRUE;

    m_bExitThread = TRUE;

    WaitForSingleObject(m_hVoiceTransmit, 3000);

    int iDeviceIndex = g_pMainDlg->GetCurDeviceIndex();
    if(!NET_ECMS_StopVoiceTalkWithStmServer(g_struDeviceInfo[iDeviceIndex].lLoginID, m_iSessionID))
    {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_FAIL_T, 1, "NET_ECMS_StopVoiceTalkWithStmServer");
    }
    else
    {
        g_bStopAudioTalk = FALSE;
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_SUCC_T, 1, "NET_ECMS_StopVoiceTalkWithStmServer");
    }
    if (g_file != NULL)
    {
        fclose(g_file);
        g_file = NULL;
    }

    NET_ESTREAM_StopVoiceTalk(m_lAudioTalkHandle);
    m_bOpenTalk = FALSE;
}


//�ϵ������Խ�
void CDlgAudioTalk::OnBnClickedBtnStartVoicetalk()
{
    // TODO: Add your control notification handler code here
    m_struVoiceTalkPara.bNeedCBNoEncData = m_combCBDataType.GetCurSel();
    m_struVoiceTalkPara.pUser  = this;
    m_struVoiceTalkPara.byVoiceTalk = 0;
    m_struVoiceTalkPara.cbVoiceDataCallBack = g_fVoiceDataCallBack;

    int iDeviceIndex = g_pMainDlg->GetCurDeviceIndex();

    m_lVoiceTalkHandle = NET_ECMS_StartVoiceTalk(g_struDeviceInfo[iDeviceIndex].lLoginID, m_cmbVoiceChan.GetCurSel() + 1, &m_struVoiceTalkPara);
    if (m_lVoiceTalkHandle < 0)
    {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_FAIL_T, 1, "NET_ECMS_StartVoiceTalk");
        return;
    }
    GetDlgItem(IDC_BTN_START_VOICETALK)->EnableWindow(FALSE);
    GetDlgItem(IDC_BTN_START_TRANSMIT)->EnableWindow(FALSE);
}

void CDlgAudioTalk::OnBnClickedBtnStopVoicetalk()
{
    // TODO: Add your control notification handler code here

//     if (NET_ECMS_StopListen(0))
//     {
//         g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_SUCC_T, 1, "NET_ECMS_StopListen() Success");
//     }
//     else
//     {
//         g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_FAIL_T, 1, "NET_ECMS_StopListen() Failed");
//     }
// 
//     //DEBUG
//     if (NET_ECMS_Fini())
//     {
//         g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_SUCC_T, 1, "NET_ECMS_Fini() Success");
//     }
//     else
//     {
//         g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_FAIL_T, 1, "NET_ECMS_Fini() Failed");
//     }
//     return;

    if (NET_ECMS_StopVoiceTalk(m_lVoiceTalkHandle))
    {
        m_lVoiceTalkHandle = -1;
        GetDlgItem(IDC_BTN_START_VOICETALK)->EnableWindow(TRUE);
        GetDlgItem(IDC_BTN_START_TRANSMIT)->EnableWindow(TRUE);
    }
}


int g_nRecvTotalLen = 0;


BOOL CALLBACK CDlgAudioTalk::AudioTalkStreamCallback(LONG iAudioTalkHandle, NET_EHOME_VOICETALK_DATA_CB_INFO *pVoiceTalkCBMsg, void *pUserData)
{
    UN_REFERENCED_PARAMETER(iAudioTalkHandle)
    //��������������Ľ���Ͳ���
    //WARNING:���ûص�ʱ��pUserĿǰ�����ó�NULL���Ȳ�Ҫʹ��

    CDlgAudioTalk *pVoiceTalk = (CDlgAudioTalk*)pUserData;

    //if (!pVoiceTalk->m_bExitThread)
    {

        g_nRecvTotalLen += pVoiceTalkCBMsg->dwDataLen;

        if (pVoiceTalk->m_nLocalType == 0)
        {
            return TRUE;
        }
        else if (pVoiceTalk->m_nLocalType == 1)
        {
            //���ز���
            //TALK_MR struTalkMr;
            //struTalkMr.byAudioType = 0;
            pVoiceTalk->PutIntoBuf((char*)pVoiceTalkCBMsg->pData, (int)pVoiceTalkCBMsg->dwDataLen, &pVoiceTalk->m_talkMr);
        }
        else if (pVoiceTalk->m_nLocalType == 2)
        {
            //ECHO
            pVoiceTalk->SendDataToDevice((char*)pVoiceTalkCBMsg->pData, pVoiceTalkCBMsg->dwDataLen);
        }
        else if (pVoiceTalk->m_nLocalType == 3)
        {
            //���ز���&Echo
            //TALK_MR struTalkMr;
            //struTalkMr.byAudioType = 0;
            pVoiceTalk->PutIntoBuf((char*)pVoiceTalkCBMsg->pData, (int)pVoiceTalkCBMsg->dwDataLen, &pVoiceTalk->m_talkMr);
            pVoiceTalk->SendDataToDevice((char*)pVoiceTalkCBMsg->pData, pVoiceTalkCBMsg->dwDataLen);
        }
        else
        {
            //�������Σ�ʲô��������
        }
    }

    return TRUE;
}


void CDlgAudioTalk::OnBnClickedBtnStartTransmit()
{
    // TODO: Add your control notification handler code here

    m_struVoiceTalkPara.pUser = this;
    m_struVoiceTalkPara.byVoiceTalk = 1;
    m_struVoiceTalkPara.cbVoiceDataCallBack = g_fVoiceDataCallBack;
    int iDeviceIndex = g_pMainDlg->GetCurDeviceIndex();

    m_lVoiceTalkHandle = NET_ECMS_StartVoiceTalk(g_struDeviceInfo[iDeviceIndex].lLoginID, m_cmbVoiceChan.GetCurSel() + 1, &m_struVoiceTalkPara);
    if (m_lVoiceTalkHandle < 0)
    {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_FAIL_T, 1, "NET_ECMS_StartVoiceTalk");
        return;
    }

    DWORD dwThreadId;
    m_bExitThread = FALSE;
    m_hVoiceTransmit = CreateThread(NULL,0,LPTHREAD_START_ROUTINE(SendVoiceDataThread),this,0,&dwThreadId);
    GetDlgItem(IDC_BTN_START_VOICETALK)->EnableWindow(FALSE);
    GetDlgItem(IDC_BTN_START_TRANSMIT)->EnableWindow(FALSE);
}

void CDlgAudioTalk::OnBnClickedBtnStopTransmit()
{
    // TODO: Add your control notification handler code here
    m_bExitThread = TRUE;
    if(WAIT_TIMEOUT == WaitForSingleObject(m_hVoiceTransmit, 3000))
    {
        TerminateThread(m_hVoiceTransmit, 0);
    }
    m_hVoiceTransmit = NULL;

    if (NET_ECMS_StopVoiceTalk(m_lVoiceTalkHandle))
    {
        m_lVoiceTalkHandle = -1;
        GetDlgItem(IDC_BTN_START_VOICETALK)->EnableWindow(TRUE);
        GetDlgItem(IDC_BTN_START_TRANSMIT)->EnableWindow(TRUE);
    }

    if (g_file != NULL)
    {
        fclose(g_file);
        g_file = NULL;
    }
}



DWORD WINAPI CDlgAudioTalk::SendVoiceDataThread (LPVOID lpArg)
{
    CDlgAudioTalk *pVoiceTalk = (CDlgAudioTalk*)lpArg;

    char szAudioFilePath[MAX_PATH] = {0};
    char szAppPath[MAX_PATH] = {0};
    GetCurrentDirectory(MAX_PATH, szAppPath);                   //��ȡ����ĵ�ǰĿ¼
    sprintf(szAudioFilePath, "%s\\AudioFiles\\%s", szAppPath, "ehome_send.au");

    FILE *pFile = fopen(szAudioFilePath, "rb");
    if (pFile == NULL)
    {
        return 0;
    }
    char szBuf[80] = {0};
    int nRemainLen = 0;

    int nCurPos = fseek(pFile,0, SEEK_END);
    nRemainLen = ftell(pFile);
    fseek(pFile, nCurPos, SEEK_SET);

    int nSendTotal = 0;
    while(!pVoiceTalk->m_bExitThread)
    {
        if (nRemainLen >= 80)
        {
            nSendTotal += 80;
            fread(szBuf, 80, 1, pFile);
            //CMS�����Խ�               
            if (pVoiceTalk->m_bCMSAudioTalk)
            {
                if (NET_ECMS_SendVoiceTransData(pVoiceTalk->m_lVoiceTalkHandle, szBuf, 80))
                {
                    int error = NET_ECMS_GetLastError();
                    if (error != 0)
                    {
                        error = error;
                    }
                }
            }
            else
            {
                //Stream�����Խ�
                NET_EHOME_VOICETALK_DATA struVoicTalkData = {0};
                struVoicTalkData.pSendBuf = (BYTE*)szBuf;
                struVoicTalkData.dwDataLen = 80;
                NET_ESTREAM_SendVoiceTalkData(pVoiceTalk->m_lAudioTalkHandle, &struVoicTalkData);
            }
            nRemainLen -= 80;
        }
        else
        {
            nSendTotal += nRemainLen;
            fread(szBuf, nRemainLen, 1, pFile);
            //CMS�����Խ�
            if (pVoiceTalk->m_bCMSAudioTalk)
            {
                NET_ECMS_SendVoiceTransData(pVoiceTalk->m_lVoiceTalkHandle, szBuf, nRemainLen);
            }
            else
            {
                //Stream�����Խ�
                NET_EHOME_VOICETALK_DATA struVoicTalkData = {0};
                struVoicTalkData.pSendBuf = (BYTE*)szBuf;
                struVoicTalkData.dwDataLen = nRemainLen;
                NET_ESTREAM_SendVoiceTalkData(pVoiceTalk->m_lAudioTalkHandle, &struVoicTalkData);
            }
            break;
        }
        Sleep(15);
    }

    g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_SUCC_T, 1, "SendVoiceDataThread, nSendTotal=[%d]", nSendTotal);

    fclose(pFile);
    return 0;
}

void  CDlgAudioTalk::CheckInitParam()
{
    int iDeviceIndex = g_pMainDlg->GetCurDeviceIndex();
    if (iDeviceIndex < 0)
    {
        char szLan[1024] = {0};
        g_StringLanType(szLan, "����ѡ��һ���豸!", "please select a device first!");
        g_pMainDlg->AddLog(-1, OPERATION_FAIL_T, 0, szLan);
        return;
    }

    char szAudioChan[32] = {0};
    for (unsigned int i = 0 ; i < g_struDeviceInfo[iDeviceIndex].dwAudioNum; i++)
    {
        sprintf(szAudioChan, "channel %d", i + 1);
        m_cmbVoiceChan.AddString(szAudioChan);
    }
    m_cmbVoiceChan.SetCurSel(0);
    UpdateData(FALSE);
}



BOOL CDlgAudioTalk::SendDataToDevice(char *buf, DWORD dwSize)
{
    NET_EHOME_VOICETALK_DATA struVoicTalkData = {0};

    struVoicTalkData.pSendBuf = (BYTE*)buf;
    struVoicTalkData.dwDataLen = dwSize;
    g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_SUCC_T, 1, "NET_ESTREAM_SendVoiceTalkData, dwDataLen=[%d]", dwSize);
    return (NET_ESTREAM_SendVoiceTalkData(m_lAudioTalkHandle, &struVoicTalkData/*NULL*/) > 0);
    
    /*
    if (!m_bWaveDeal)
    {
        return FALSE;
    }

    BYTE G711EncBuf[G711_AUDDECSIZE*2]  = {0};
    BYTE G711EncBufA[G711_AUDDECSIZE*2] = {0};
    BYTE G726EncBuf[G726_AUDDECSIZE]    = {0};
    UINT16 wVoiceBuf[G711_AUDDECSIZE*2] = {0};

    memcpy(m_wG711AudioTemp, buf, AUDENCSIZE);
    //AudioBufDownScale((short *)m_wG711AudioTemp, (short *)wVoiceBuf, AUDENCSIZE / 2);
    UINT16 wVoiceBuf1[G711_AUDDECSIZE] = {0};
    UINT16 wVoiceBuf2[G711_AUDDECSIZE] = {0};
    memcpy(wVoiceBuf1, wVoiceBuf, G711_AUDDECSIZE * 2);
    memcpy(wVoiceBuf2, wVoiceBuf + G711_AUDDECSIZE, G711_AUDDECSIZE * 2);

    NET_DVR_EncodeG711Frame(0, (BYTE*)wVoiceBuf1, G711EncBuf);
    NET_DVR_EncodeG711Frame(0, (BYTE*)wVoiceBuf2, G711EncBuf + G711_AUDDECSIZE);
    NET_DVR_EncodeG711Frame(1, (BYTE*)wVoiceBuf1, G711EncBufA);
    NET_DVR_EncodeG711Frame(1, (BYTE*)wVoiceBuf2, G711EncBufA + G711_AUDDECSIZE);

    if (NULL != m_pG726EncM)
    {
        NET_DVR_EncodeG726Frame(m_pG726EncM, (BYTE*)wVoiceBuf, G726EncBuf, m_bReset);
        m_bReset = 0;
    }

    int i = 0, j = 0;

    //encode 
    if (NULL != m_pEncoder)
    {
        NET_DVR_EncodeG722Frame(m_pEncoder, (BYTE*)buf, m_byEncBuf);
    }

    NET_EHOME_VOICETALK_DATA struVoicTalkData = {0};

    if (m_iAudioEncType == AUDIOTALKTYPE_G722)
    {
        struVoicTalkData.pSendBuf = (BYTE*)m_byEncBuf;
        struVoicTalkData.dwDataLen = AUDDECSIZE;

        //if (!NET_DVR_VoiceComSendData(g_struDeviceInfo[i].lVoiceCom[j], (char*)m_byEncBuf, AUDDECSIZE))
        if (NET_ESTREAM_SendVoiceTalkData(m_lAudioTalkHandle, &struVoicTalkData) < 0)
        {
            //failed
        }
    }
    else if(m_iAudioEncType == AUDIOTALKTYPE_G711_MU)
    {
        struVoicTalkData.pSendBuf = (BYTE*)G711EncBuf;
        struVoicTalkData.dwDataLen = G711_AUDDECSIZE;

        //if (!NET_DVR_VoiceComSendData(g_struDeviceInfo[i].lVoiceCom[j], (char*)G711EncBuf, G711_AUDDECSIZE))
        if (NET_ESTREAM_SendVoiceTalkData(m_lAudioTalkHandle, &struVoicTalkData) < 0)
        {
            //failed
        }

        struVoicTalkData.pSendBuf = (BYTE*)G711EncBuf+G711_AUDDECSIZE;
        struVoicTalkData.dwDataLen = G711_AUDDECSIZE;

        //if (!NET_DVR_VoiceComSendData(g_struDeviceInfo[i].lVoiceCom[j], (char*)G711EncBuf+G711_AUDDECSIZE, G711_AUDDECSIZE))
        if (NET_ESTREAM_SendVoiceTalkData(m_lAudioTalkHandle, &struVoicTalkData) < 0)
        {
            //failed
        }
    }
    else if (m_iAudioEncType == AUDIOTALKTYPE_G711_A)
    {
        struVoicTalkData.pSendBuf = (BYTE*)G711EncBufA;
        struVoicTalkData.dwDataLen = G711_AUDDECSIZE;
        //if (!NET_DVR_VoiceComSendData(g_struDeviceInfo[i].lVoiceCom[j], (char*)G711EncBufA, G711_AUDDECSIZE))
        if (NET_ESTREAM_SendVoiceTalkData(m_lAudioTalkHandle, &struVoicTalkData) < 0)
        {
            //failed
        }

        struVoicTalkData.pSendBuf = (BYTE*)G711EncBufA+G711_AUDDECSIZE;
        struVoicTalkData.dwDataLen = G711_AUDDECSIZE;
        //if (!NET_DVR_VoiceComSendData(g_struDeviceInfo[i].lVoiceCom[j], (char*)G711EncBufA+G711_AUDDECSIZE, G711_AUDDECSIZE))
        if (NET_ESTREAM_SendVoiceTalkData(m_lAudioTalkHandle, &struVoicTalkData) < 0)
        {
            //failed
        }
    }
    else if (m_iAudioEncType == AUDIOTALKTYPE_G726)
    {
        struVoicTalkData.pSendBuf = (BYTE*)G726EncBuf;
        struVoicTalkData.dwDataLen = G726_AUDDECSIZE;

        //if (!NET_DVR_VoiceComSendData(g_struDeviceInfo[i].lVoiceCom[j], (char*)G726EncBuf, G726_AUDDECSIZE))
        if (NET_ESTREAM_SendVoiceTalkData(m_lAudioTalkHandle, &struVoicTalkData) < 0)
        {
            //failed
        }
    }

    return TRUE;
    */
}


BOOL CDlgAudioTalk::OpenAudioIn()
{
#ifndef WIN64
    //�����ôӱ����ɼ���Ƶ���ݵĻص�
    m_SoundIn.SetSoundInDataCB(DataFromSoundInEx, (DWORD)this);
    //������Ƶ���룬
    if (m_SoundIn.Start(&m_struWaveFormat, m_dwBufNum, m_dwBufSize))
    {
        //success
        m_bOpenWavIn = TRUE;
        return TRUE;
    }
    else
    {
        //failed
        return FALSE;
    }
#else
    return FALSE;
#endif
}


BOOL CDlgAudioTalk::OpenAudioOut()
{
#ifndef WIN64
    m_ReceiveIndexRV = 0;
    m_rIndexRV = 0;
    m_nBufNo = 0;

    WAVEFORMATEX struWaveFormat = {0};
    DWORD dwBufSize = AUDENCSIZE;
    memcpy(&struWaveFormat, &m_struWaveFormat, sizeof(WAVEFORMATEX));
    //Ĭ�ϸ�ʽ�ǰ���G722�ɼ����ţ�������������ͣ��˴��޸Ĳ����ʵ�

    //FIXME
    if (m_talkMr.byAudioType != AUDIOTALKTYPE_G722) 
    {
        struWaveFormat.nSamplesPerSec  = SAMPLES_PER_SECOND / 2;
        struWaveFormat.nAvgBytesPerSec = SAMPLES_PER_SECOND * m_struWaveFormat.nBlockAlign/2;
        if (m_talkMr.byAudioType == AUDIOTALKTYPE_G726)
        {
            dwBufSize = G726_AUDENCSIZE; //G726
        }
        else
        {
            dwBufSize = G711_AUDENCSIZE; //G711
        }
    }

    //Ŀǰ֧�ֵĲ���Ϊ��ģ���豸ֱ�Ӳ������͹��������ݣ�PCM���ݣ�
    if (m_SoundOut.OpenSound(struWaveFormat, m_dwBufNum, dwBufSize, CALLBACK_FUNCTION, 0))
    {
        //m_SoundOut.SetVolume(0x7fff7fff);
        m_SoundOut.SetVolume(0xF000F000);
        if (m_SoundOut.PlaySound())
        {
            m_bOpenWavOut = TRUE;
            return TRUE;
        }
    }
    else
    {
        //failed
    }
    return FALSE;
#else
    return FALSE;
#endif
}


BOOL CDlgAudioTalk::PutIntoBuf(char *lpTemp, int Bytes, LPTALK_MR lpTalkMR)
{
    int nTemp = 0;
    int nPacketStart = 0;
    DWORD dwAudDecSize = 0;

    if (lpTalkMR->byAudioType == AUDIOTALKTYPE_G722)
    {
        dwAudDecSize = AUDDECSIZE;
    }
    else if (lpTalkMR->byAudioType == AUDIOTALKTYPE_G726)
    {
        dwAudDecSize = G726_AUDDECSIZE;
    }
    else
    {
        dwAudDecSize = G711_AUDDECSIZE;
    }

    EnterCriticalSection(&m_csAudioBuf);

    /*
    if (lpTalkMR->byAudioType == AUDIOTALKTYPE_G722)
    {
        dwAudDecSize = AUDDECSIZE;
    }
    else if (lpTalkMR->byAudioType == AUDIOTALKTYPE_G726)
    {
        dwAudDecSize = G726_AUDDECSIZE;
    }
    else
    {
        dwAudDecSize = G711_AUDDECSIZE;
    }
    */
    try
    {
        if ((m_ReceiveIndexRV + Bytes) <= AUDIOBUF)
        {
            if (!m_bOpenPlayThread)
            {
                memcpy(m_pRenderBuf+m_ReceiveIndexRV, lpTemp, Bytes);
                m_ReceiveIndexRV += Bytes;
                m_ReceiveIndexRV = m_ReceiveIndexRV % AUDIOBUF;
            }
            else
            {
                if (((m_ReceiveIndexRV + Bytes) >= m_rIndexRV)  
                    && (m_ReceiveIndexRV < m_rIndexRV))
                {   //buffer1 overflow
                    TRACE("buffer1 overflow.");
                    ::SetEvent(m_hHaveRDataRV);
                    nPacketStart = (m_rIndexRV - dwAudDecSize + m_ReceiveIndexRV % dwAudDecSize);
                    if ((nPacketStart + Bytes) <= (DWORD)AUDIOBUF)
                    {
                        memcpy(m_pRenderBuf + nPacketStart, lpTemp, Bytes);
                        m_ReceiveIndexRV = nPacketStart + Bytes;
                    }
                    else
                    {
                        nTemp = AUDIOBUF - nPacketStart;
                        memcpy(m_pRenderBuf + nPacketStart, lpTemp, nTemp);
                        memcpy(m_pRenderBuf, lpTemp + nTemp, Bytes - nTemp);
                        m_ReceiveIndexRV = Bytes - nTemp;
                    }
                }
                else    
                {
                    memcpy(m_pRenderBuf + m_ReceiveIndexRV, lpTemp, Bytes);
                    m_ReceiveIndexRV += Bytes;
                    m_ReceiveIndexRV = m_ReceiveIndexRV % AUDIOBUF;
                }
            }
        }
        else
        {
            if (m_bOpenPlayThread)
            {
                if ((Bytes >= (m_rIndexRV + AUDIOBUF - m_ReceiveIndexRV))
                    || (m_rIndexRV >= m_ReceiveIndexRV))
                {    //buffer2 overflow
                    TRACE("buffer2 overflow");
                    ::SetEvent(m_hHaveRDataRV);
                    if (m_rIndexRV != 0)
                    {
                        nPacketStart = (m_rIndexRV - dwAudDecSize + m_ReceiveIndexRV % dwAudDecSize);
                    }
                    else
                    {
                        nPacketStart = (m_rIndexRV + AUDIOBUF - dwAudDecSize + m_ReceiveIndexRV % dwAudDecSize);
                    }
                    if ((nPacketStart + Bytes) <= (DWORD)AUDIOBUF)
                    {
                        memcpy(m_pRenderBuf + nPacketStart, lpTemp, Bytes);
                        m_ReceiveIndexRV = nPacketStart + Bytes;
                    }
                    else
                    {
                        nTemp = AUDIOBUF - nPacketStart;
                        memcpy(m_pRenderBuf + nPacketStart, lpTemp, nTemp);
                        memcpy(m_pRenderBuf, lpTemp + nTemp, Bytes - nTemp);
                        m_ReceiveIndexRV = Bytes - nTemp;
                    }
                }
                else
                {
                    memcpy(m_pRenderBuf + m_ReceiveIndexRV, lpTemp, nTemp = AUDIOBUF - m_ReceiveIndexRV);
                    memcpy(m_pRenderBuf, lpTemp + nTemp, Bytes - nTemp);
                    m_ReceiveIndexRV = Bytes - nTemp;
                }
            }
        }
    }
    catch(...)
    {
        LeaveCriticalSection(&m_csAudioBuf);
        return FALSE;
    }
    //    TRACE("number:%d, m_nBufNo:%d\n", (m_ReceiveIndexRV[lpTalkMR->byIndex] + AUDIOBUF - m_rIndexRV[lpTalkMR->byIndex]) % (AUDIOBUF), m_nBufNo[lpTalkMR->byIndex]*dwAudDecSize);
    if ((((m_ReceiveIndexRV + AUDIOBUF - m_rIndexRV) % (AUDIOBUF)) >= (m_nBufNo * (int)dwAudDecSize)))
    {
        ::SetEvent(m_hHaveRDataRV);
        m_nBufNo = 1;
        if (!m_bOpenPlayThread)
        {
            m_bOpenPlayThread = TRUE;
            AfxBeginThread(PlayAudioThread, (LPVOID)lpTalkMR);
        }
    }

    LeaveCriticalSection(&m_csAudioBuf);
    return TRUE;
}

UINT CDlgAudioTalk::PlayAudioThread(LPVOID pParam)
{
    ASSERT(pParam);
    try
    {
        s_pAudioTalkDlg->InputAudioData((LPTALK_MR)pParam);
    }
    catch (...)
    {
        TRACE("Input data exception\n");
        return 1;
    }

    return 0;
}


void CDlgAudioTalk::InputAudioData(LPTALK_MR lpTalkMR)
{
#ifndef WIN64
    HANDLE hWaitEvents[2];

    hWaitEvents[0] = m_hExitEvent;
    hWaitEvents[1] = m_hHaveRDataRV;

    void *pDecoder = NULL;//g722 decoder
    void *pG726Dec = NULL;//g726������
    void *pG726DecM = NULL;//g726����ģ����

    DWORD dwReadLength = 0;
    DWORD dwPlayLength = 0;

    lpTalkMR->byAudioType = AUDIOTALKTYPE_PCM;

//     if (lpTalkMR->byAudioType == AUDIOTALKTYPE_G722)
//     {
//         dwReadLength = AUDDECSIZE;
//         dwPlayLength = AUDENCSIZE;
//         pDecoder = NET_DVR_InitG722Decoder(BIT_RATE_16000);
//     }
//     else if (lpTalkMR->byAudioType == AUDIOTALKTYPE_G726)
//     {
//         dwReadLength = G726_AUDDECSIZE;
//         dwPlayLength = G726_AUDENCSIZE;
//         pG726Dec = NET_DVR_InitG726Decoder(&pG726DecM);
//     }
//     else if (lpTalkMR->byAudioType == AUDIOTALKTYPE_PCM)
//     {
//         dwReadLength = AUDENCSIZE;
//         dwPlayLength = AUDENCSIZE;
//     }
//     else
//     {
//         dwReadLength = G711_AUDDECSIZE;
//         dwPlayLength = G711_AUDENCSIZE;
//     }


    dwReadLength = AUDENCSIZE;
    dwPlayLength = AUDENCSIZE;

    BYTE  *lpTemp = NULL;
    lpTemp = new BYTE[dwReadLength];
    BYTE  *lpPlayBuf = NULL;
    lpPlayBuf = new BYTE[dwPlayLength];
    DWORD dwWaitResult = 0;
    try
    {
        while (1)
        {
            dwWaitResult = WaitForMultipleObjects(2, hWaitEvents, FALSE, INFINITE);
            if (WAIT_OBJECT_0 == dwWaitResult || WAIT_FAILED == dwWaitResult)
            {
                ExitPlayAudio(lpTemp, lpPlayBuf, pDecoder, pG726Dec, pG726DecM, lpTalkMR->byIndex);
                ResetEvent(m_hExitEvent);
                return;
            }

            lpTalkMR->byAudioType = AUDIOTALKTYPE_PCM;

            EnterCriticalSection(&m_csAudioBuf);
            if (CopyAudioData(lpTemp, dwReadLength, lpTalkMR->byIndex))
            {
                /*
                if (lpTalkMR->byAudioType == AUDIOTALKTYPE_G722)
                {
                    if (NULL != pDecoder)
                    {
                        if (!NET_DVR_DecodeG722Frame(pDecoder, (BYTE*)lpTemp, lpPlayBuf))
                        {
                            int i = 0;
                            i++;
                        }
                    }
                }
                else if (lpTalkMR->byAudioType == AUDIOTALKTYPE_G711_MU)
                {
                    NET_DVR_DecodeG711Frame(0,(BYTE*)lpTemp,lpPlayBuf);
                }
                else if (lpTalkMR->byAudioType == AUDIOTALKTYPE_G711_A)
                {
                    NET_DVR_DecodeG711Frame(1,(BYTE*)lpTemp,lpPlayBuf);
                }
                else if (lpTalkMR->byAudioType == AUDIOTALKTYPE_G726)
                {
                    if (NULL != pG726DecM)
                    {
                        NET_DVR_DecodeG726Frame(pG726DecM, (BYTE*)lpTemp, lpPlayBuf, byReset);
                        byReset = 0;
                    }
                }
                else if (lpTalkMR->byAudioType == AUDIOTALKTYPE_PCM)
                {
                    memcpy(lpPlayBuf, lpTemp, dwPlayLength);
                }
                */

                memcpy(lpPlayBuf, lpTemp, dwPlayLength);

                if (!m_SoundOut.InputData((BYTE*)lpPlayBuf, 0))
                {
                    int i = 0;
                    i++;
                }
            }
            else if (m_pRenderBuf == NULL)
            {
                ExitPlayAudio(lpTemp, lpPlayBuf, pDecoder, pG726Dec, pG726DecM, lpTalkMR->byIndex);
                LeaveCriticalSection(&m_csAudioBuf);
                return;
            }

            LeaveCriticalSection(&m_csAudioBuf);
        }
    }
    catch (...)
    {
        TRACE("InputAudioData exception\n");
        ExitPlayAudio(lpTemp, lpPlayBuf, pDecoder, pG726Dec, pG726DecM, lpTalkMR->byIndex);
        return ;
    }

    ExitPlayAudio(lpTemp, lpPlayBuf, pDecoder, pG726Dec, pG726DecM, lpTalkMR->byIndex);
#endif
    return ;
}


void CDlgAudioTalk::ExitPlayAudio(BYTE *lpTemp, BYTE *lpPlayBuf, void *pDecoder, void *pG726Dec, void *pG726DecM, BYTE byIndex)
{
    UN_REFERENCED_PARAMETER(byIndex)
    m_bOpenPlayThread = FALSE;
    if (lpTemp != NULL)
    {
        delete[] lpTemp;
        lpTemp = NULL;
    }
    if (lpPlayBuf != NULL)
    {
        delete[] lpPlayBuf;
        lpPlayBuf = NULL;
    }
    if (pDecoder)
    {
        //FIXME
        //NET_DVR_ReleaseG722Decoder(pDecoder);
        pDecoder = NULL;
    }
    if (pG726Dec)
    {
        //FIXME
        //NET_DVR_ReleaseG726Decoder(pG726Dec);
        pG726Dec = NULL;
        pG726DecM = NULL;
    }
}



BOOL CDlgAudioTalk::CopyAudioData(PBYTE lpTemp, DWORD dwReadLength, int nIndex)
{
    UN_REFERENCED_PARAMETER(nIndex)
    DWORD dwTemp;
    int  dwPacketNumber = 0;
    int itemp = 0;
    __try
    {
        if (m_rIndexRV == m_ReceiveIndexRV)
        {
            dwPacketNumber = 0;
            m_nBufNo = 6;
            ::ResetEvent(m_hHaveRDataRV);
            itemp = 0;
            return FALSE;
        }
        else if (m_rIndexRV < m_ReceiveIndexRV)    //read < write
        {
            if (dwReadLength > (DWORD)(m_ReceiveIndexRV - m_rIndexRV))
            {
                dwPacketNumber = 0;
                m_nBufNo = 6;
                ::ResetEvent(m_hHaveRDataRV);
                itemp = 0;
                return FALSE;
            }
            else
            {
                CopyMemory(lpTemp, m_pRenderBuf + m_rIndexRV, dwReadLength);
                m_rIndexRV += dwReadLength;
                dwPacketNumber = (m_ReceiveIndexRV - m_rIndexRV) / dwReadLength;
                itemp = 1;
            }
        }
        else//read > write
        {
            if (dwReadLength > (DWORD)(AUDIOBUF - m_rIndexRV))
            {
                dwTemp = AUDIOBUF - m_rIndexRV;
                if ((dwReadLength-dwTemp) < (DWORD)(m_ReceiveIndexRV + 1))
                {
                    CopyMemory(lpTemp, m_pRenderBuf + m_rIndexRV, dwTemp);
                    CopyMemory(lpTemp + dwTemp, m_pRenderBuf, dwReadLength - dwTemp);
                    m_rIndexRV = dwReadLength - dwTemp;
                    dwPacketNumber = (m_ReceiveIndexRV - m_rIndexRV) / dwReadLength;
                    itemp = 2;
                }
                else
                {
                    dwPacketNumber = 0;
                    ::ResetEvent(m_hHaveRDataRV);
                    m_nBufNo = 6;
                    itemp = 3;
                    return FALSE;
                }
            }
            else
            {
                itemp = 4;
                CopyMemory(lpTemp, m_pRenderBuf + m_rIndexRV, dwReadLength);
                m_rIndexRV += dwReadLength;
                dwPacketNumber = (AUDIOBUF + m_ReceiveIndexRV - m_rIndexRV) / dwReadLength;
            }
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        return FALSE;
    }
    return TRUE;
}






/////////////////////////////////////////////////////////////////
//                      ��·����ר��                       //
/////////////////////////////////////////////////////////////////


BOOL g_bTestExitThread = FALSE;
BOOL  CALLBACK  onTestDeviceAudioTalkConnectCB(LONG lAudioTalkHandle, NET_EHOME_VOICETALK_NEWLINK_CB_INFO *pNewLinkCBMsg, void *pUserData);
DWORD CALLBACK  TestVoiceTransThread(LPVOID lpArg);
BOOL  CALLBACK  onTestVoiceDataCallBack(LONG iAudioTalkHandle, NET_EHOME_VOICETALK_DATA_CB_INFO *pVoiceTalkCBMsg, void *pUserData);

void CDlgAudioTalk::OnBnClickedBtnAudiotalkTest()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������

   
    NET_EHOME_VOICETALK_DATA struVoicTalkData = {0};

    char szSendBuf[1280];
    struVoicTalkData.pSendBuf = (BYTE*)szSendBuf;
    struVoicTalkData.dwDataLen = 1280;
    //DEBUG
    //return (NET_ESTREAM_SendVoiceTalkData(m_lAudioTalkHandle, NULL) > 0);
    int nSendRet = NET_ESTREAM_SendVoiceTalkData(m_lAudioTalkHandle, &struVoicTalkData);

    int nError = NET_ESTREAM_GetLastError();

    return;
    
    /*
    UpdateData(TRUE);

    CSTRING_TO_CHARS(m_csIP, g_szRemoteIP);
    g_nRemotePort = m_nPort;

    DWORD dwThreadId;

    for (int i = 0; i < MAX_DEVICES; i++)
    {
        LONG lLoginID = g_struDeviceInfo[i].lLoginID;
        if (lLoginID < 0)
        {
            continue;
        }
        CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(TestVoiceTransThread), (void*)lLoginID, 0, &dwThreadId);
    }
    */
}


BOOL CALLBACK onTestDeviceAudioTalkTestConnectCB(LONG lAudioTalkHandle, NET_EHOME_VOICETALK_NEWLINK_CB_INFO *pNewLinkCBMsg, void *pUserData)
{
    UN_REFERENCED_PARAMETER(pNewLinkCBMsg)

    //���ûص�
    NET_EHOME_VOICETALK_DATA_CB_PARAM struAudioTalkCBParam = {0};
    struAudioTalkCBParam.fnVoiceTalkDataCB = onTestVoiceDataCallBack;
    struAudioTalkCBParam.pUserData = pUserData;
    if (!NET_ESTREAM_SetVoiceTalkDataCB(lAudioTalkHandle, &struAudioTalkCBParam))
    {
        //���ûص�ʧ��
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), 1, OPERATION_FAIL_T, "NET_ESTREAM_SetVoiceTalkDataCB failed!, Handle=[%d]", lAudioTalkHandle);
        return FALSE;
    }
    else
    {
        //���ûص��ɹ�
        //g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), 1, OPERATION_SUCC_T, "NET_ESTREAM_SetVoiceTalkDataCB success!, Handle=[%d]", lAudioTalkHandle);
        return TRUE;
    }
}

BOOL CALLBACK onTestVoiceDataCallBack(LONG iAudioTalkHandle, NET_EHOME_VOICETALK_DATA_CB_INFO *pVoiceTalkCBMsg, void *pUserData)
{
    //Stream�����Խ��ص�
    //Echo�豸����������������
    UN_REFERENCED_PARAMETER(pUserData)
    char* pAudioBuf = new char[pVoiceTalkCBMsg->dwDataLen];
    memcpy(pAudioBuf, pVoiceTalkCBMsg->pData, pVoiceTalkCBMsg->dwDataLen);

    NET_EHOME_VOICETALK_DATA struVoicTalkData = {0};

    struVoicTalkData.pSendBuf = (BYTE*)pAudioBuf;
    struVoicTalkData.dwDataLen = pVoiceTalkCBMsg->dwDataLen;

    if (NET_ESTREAM_SendVoiceTalkData(iAudioTalkHandle, &struVoicTalkData) < 0)
    {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), 1, OPERATION_FAIL_T, "NET_ESTREAM_SendVoiceTalkData Error[%d]!, iAudioTalkHandle=[%d]\n", NET_ESTREAM_GetLastError(), iAudioTalkHandle);
    }

    delete[] pAudioBuf;
    pAudioBuf = NULL;

    return TRUE;
}


DWORD WINAPI TestVoiceTransThread(LPVOID lpArg)
{
    LONG lLoginID = (LONG)lpArg;

    LONG *pHandle = new(::std::nothrow) LONG[1];

    if (NULL == pHandle)
    {
        printf("Alloc MEM failed");
        return NULL;
    }

    *pHandle = -1;

    //�ȿ�������
    LONG lListenHandle = -1;
    //Stream ���������Խ�����
    NET_EHOME_LISTEN_VOICETALK_CFG struListen = {0};
    struListen.fnNewLinkCB = onTestDeviceAudioTalkTestConnectCB;
    struListen.pUser = pHandle;
    strncpy(struListen.struIPAdress.szIP, g_szRemoteIP, 128);
    struListen.struIPAdress.wPort = (WORD)g_nRemotePort + (WORD)lLoginID;     //�˿�
    lListenHandle = NET_ESTREAM_StartListenVoiceTalk(&struListen);
    if (lListenHandle == -1)
    {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), 1, OPERATION_FAIL_T, "NET_ESTREAM_StartListenVoiceTalk Error!, LoginID[%d]\n", lLoginID);
    }

    else
    {

        //CMS���� ����Խ�
        NET_EHOME_VOICE_TALK_IN struVoiceTalkIn = {0};
        struVoiceTalkIn.dwVoiceChan = 1;
        struVoiceTalkIn.struStreamSever.wPort = (WORD)g_nRemotePort + (WORD)lLoginID;
        memcpy(struVoiceTalkIn.struStreamSever.szIP, g_szRemoteIP, sizeof(struVoiceTalkIn.struStreamSever.szIP));
        NET_EHOME_VOICE_TALK_OUT struVoiceTalkOut = {0};
        if (!NET_ECMS_StartVoiceWithStmServer(lLoginID, &struVoiceTalkIn, &struVoiceTalkOut))
        {
            //����Խ�ʧ��
            g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), 1, OPERATION_FAIL_T, "NET_ECMS_StartVoiceWithStmServer Error!, LoginID[%d], lSessionID[%d]\n", lLoginID, struVoiceTalkOut.lSessionID);
        }

        //CMS ����ʼ����
        NET_EHOME_PUSHVOICE_IN struPushVoiceIn = {0};
        struPushVoiceIn.dwSize = sizeof(struPushVoiceIn);
        struPushVoiceIn.lSessionID = struVoiceTalkOut.lSessionID;
        NET_EHOME_PUSHVOICE_OUT struPushVoiceOut = {0};
        struPushVoiceOut.dwSize = sizeof(struPushVoiceOut);
        if (!NET_ECMS_StartPushVoiceStream(lLoginID, &struPushVoiceIn, &struPushVoiceOut))
        {
            g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), 1, OPERATION_FAIL_T, "NET_ECMS_StartPushVoiceStream Error!, LoginID[%d], lSessionID[%d]\n", lLoginID, struVoiceTalkOut.lSessionID);
        }

    }

    if (pHandle != NULL)
    {
        delete[] pHandle;
        pHandle = NULL;
    }

    return 0;
}








//----------------------------------------------------------------------------------------------------------------------------













