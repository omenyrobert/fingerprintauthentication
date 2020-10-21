// DlgListen.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "EHomeDemo.h"
#include "DlgListen.h"


// CDlgListen �Ի���

IMPLEMENT_DYNAMIC(CDlgListen, CDialog)

CDlgListen::CDlgListen(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgListen::IDD, pParent)
    , m_wPort(0)
    , m_pFatherDlg(NULL)
{

}

CDlgListen::~CDlgListen()
{
}

void CDlgListen::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_LISTEN_TYPE, m_cmListenType);
    DDX_Control(pDX, IDC_COMBO_PORT_NO, m_cmPortNo);
    DDX_Control(pDX, IDC_IPADDRESS1, m_ipAddress);
    DDX_Text(pDX, IDC_EDIT_PORT, m_wPort);
    DDX_Control(pDX, IDC_COMBO_LINK_TYPE, m_cmLinkType);
}


BEGIN_MESSAGE_MAP(CDlgListen, CDialog)
    ON_CBN_SELCHANGE(IDC_COMBO_PORT_NO, &CDlgListen::OnCbnSelchangeComboPortNo)
    ON_BN_CLICKED(IDC_BTN_START_LISTEN, &CDlgListen::OnBnClickedBtnStartListen)
    ON_BN_CLICKED(IDC_BTN_STOP_LISTEN, &CDlgListen::OnBnClickedBtnStopListen)
END_MESSAGE_MAP()


// CDlgListen ��Ϣ�������

BOOL CDlgListen::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  �ڴ���Ӷ���ĳ�ʼ��
    char szLan[128] = {0};
    CString csTemp = _T("");
    m_cmListenType.ResetContent();
    g_StringLanType(szLan, "Ԥ������", "preview listen");
    csTemp = szLan;
    m_cmListenType.AddString(csTemp);
    m_cmPortNo.ResetContent();
    int i = 0;
    char sTemp[16] = {0};
    for (i=0; i<MAX_LISTEN_NUM; i++)
    {
        sprintf(sTemp, "%d", i+1);
        m_cmPortNo.AddString(sTemp);
    }
    m_cmListenType.SetCurSel(0);
    m_cmPortNo.SetCurSel(0);
    UpdateData(FALSE);

    OnCbnSelchangeComboPortNo();
    return TRUE;  // return TRUE unless you set the focus to a control
    // �쳣: OCX ����ҳӦ���� FALSE
}

void CDlgListen::OnCbnSelchangeComboPortNo()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    UpdateData(TRUE);
    int iIndex = m_cmPortNo.GetCurSel();

    DWORD dwIP = ntohl(inet_addr(g_struPreviewListen[iIndex].struIP.szIP));
    m_ipAddress.SetAddress(dwIP);

    if (0 != g_struPreviewListen[iIndex].struIP.wPort)
    {
        m_wPort = g_struPreviewListen[iIndex].struIP.wPort;
    }
    else
    {
        m_wPort = 8000;
    }
    

    m_cmLinkType.SetCurSel(g_struPreviewListen[iIndex].iLinkType);

    if (g_struPreviewListen[iIndex].lHandle > -1)
    {
        GetDlgItem(IDC_BTN_START_LISTEN)->EnableWindow(FALSE);
        GetDlgItem(IDC_BTN_STOP_LISTEN)->EnableWindow(TRUE);
    }
    else
    {
        GetDlgItem(IDC_BTN_START_LISTEN)->EnableWindow(TRUE);
        GetDlgItem(IDC_BTN_STOP_LISTEN)->EnableWindow(FALSE);
    }
    UpdateData(FALSE);
}

void CDlgListen::OnBnClickedBtnStartListen()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    UpdateData(TRUE);
    int iIndex = m_cmPortNo.GetCurSel();

    DWORD dwIP = 0;
    m_ipAddress.GetAddress(dwIP);
    memcpy(g_struPreviewListen[iIndex].struIP.szIP, IPToStr(dwIP), 16);
    g_struPreviewListen[iIndex].struIP.wPort = (WORD)m_wPort;
    g_struPreviewListen[iIndex].iLinkType = m_cmLinkType.GetCurSel();

    if (g_struPreviewListen[iIndex].lHandle > -1)
    {
        if(NET_ESTREAM_StopListenPreview(g_struPreviewListen[iIndex].lHandle))
        {
            g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_SUCC_T, 2, "NET_ESTREAM_StopListenPreview");
            g_struPreviewListen[iIndex].lHandle = -1;
        }
        else
        {
            g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_FAIL_T, 2, "NET_ESTREAM_StopListenPreview");
        }
    }

    NET_EHOME_LISTEN_PREVIEW_CFG struListen = {0};
    memcpy(&struListen.struIPAdress, &g_struPreviewListen[iIndex].struIP, sizeof(NET_EHOME_IPADDRESS));
    struListen.fnNewLinkCB = CEHomeDemoDlg::fnPREVIEW_NEWLINK_CB;
    struListen.pUser = g_pMainDlg;
    struListen.byLinkMode = (BYTE)g_struPreviewListen[iIndex].iLinkType;

    LONG lHandle = NET_ESTREAM_StartListenPreview(&struListen);
    if (lHandle > -1)
    {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_SUCC_T, 2, "NET_ESTREAM_StartListenPreview");
        g_struPreviewListen[iIndex].lHandle = lHandle;
        GetDlgItem(IDC_BTN_START_LISTEN)->EnableWindow(FALSE);
        GetDlgItem(IDC_BTN_STOP_LISTEN)->EnableWindow(TRUE);
    }
    else
    {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_FAIL_T, 2, "NET_ESTREAM_StartListenPreview");
    }


    UpdateData(FALSE);
}

void CDlgListen::OnBnClickedBtnStopListen()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    UpdateData(TRUE);
    int iIndex = m_cmPortNo.GetCurSel();
    if(NET_ESTREAM_StopListenPreview(g_struPreviewListen[iIndex].lHandle))
    {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_SUCC_T, 2, "NET_ESTREAM_StopListenPreview");
        g_struPreviewListen[iIndex].lHandle = -1;
        GetDlgItem(IDC_BTN_START_LISTEN)->EnableWindow(TRUE);
        GetDlgItem(IDC_BTN_STOP_LISTEN)->EnableWindow(FALSE);
        if (g_pMainDlg->m_pDlgPreview[m_pFatherDlg->m_iCurWndIndex].m_bRecord)
        {
            g_pMainDlg->m_pDlgPreview[m_pFatherDlg->m_iCurWndIndex].m_bRecord = FALSE;
            m_pFatherDlg->m_bRecord = FALSE;
            fclose(g_pMainDlg->m_pDlgPreview[m_pFatherDlg->m_iCurWndIndex].m_fVideoFile);
            g_pMainDlg->m_pDlgPreview[m_pFatherDlg->m_iCurWndIndex].m_fVideoFile = NULL;
            char szLan[128] = { 0 };
            CString csTemp = _T("");
            g_StringLanType(szLan, "��ʼ¼��", "Start Record");
            csTemp = szLan;
            m_pFatherDlg->GetDlgItem(IDC_BTN_SALVE)->SetWindowText(csTemp);
        }
    }
    else
    {
        g_pMainDlg->AddLog(g_pMainDlg->GetCurDeviceIndex(), OPERATION_FAIL_T, 2, "NET_ESTREAM_StopListenPreview");
    }
}
