// DlgChanInfo.cpp : 实现文件
//

#include "stdafx.h"
#include "EHomeDemo.h"
#include "DlgChanInfo.h"


// CDlgChanInfo 对话框

IMPLEMENT_DYNAMIC(CDlgChanInfo, CDialog)

CDlgChanInfo::CDlgChanInfo(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgChanInfo::IDD, pParent)
    , m_wPort(0)
    ,m_iDeviceIndex(-1)
    ,m_iChannelIndex(-1)
{

}

CDlgChanInfo::~CDlgChanInfo()
{
}

void CDlgChanInfo::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_IPADDRESS1, m_ipAddress);
    DDX_Text(pDX, IDC_EDIT_PORT, m_wPort);
    DDX_Control(pDX, IDC_COMBO_PROTOCOL, m_cmProtocol);
    DDX_Control(pDX, IDC_COMBO_STREAM_TYPE, m_cmStreamType);
    DDX_Text(pDX, IDC_EDIT_PORT, m_wPort);
}


BEGIN_MESSAGE_MAP(CDlgChanInfo, CDialog)
    ON_BN_CLICKED(IDC_BTN_SURE, &CDlgChanInfo::OnBnClickedBtnSure)
END_MESSAGE_MAP()


// CDlgChanInfo 消息处理程序

BOOL CDlgChanInfo::OnInitDialog()
{
    CDialog::OnInitDialog();
    m_cmProtocol.ResetContent();
    m_cmProtocol.AddString("TCP");
    m_cmProtocol.AddString("UDP");
    m_cmProtocol.AddString("HRUDP");

   m_cmProtocol.SetCurSel(g_struDeviceInfo[m_iDeviceIndex].struChanInfo[m_iChannelIndex].dwLinkMode);

    char szLan[128] = {0};
    CString csTemp = _T("");

    m_cmStreamType.ResetContent();

    g_StringLanType(szLan, "主码流", "main stream");
    csTemp = szLan;
    m_cmStreamType.AddString(csTemp);
    g_StringLanType(szLan, "子码流", "sub stream");
    csTemp = szLan;
    m_cmStreamType.AddString(csTemp);
    g_StringLanType(szLan, "三码流", "third stream");
    csTemp = szLan;
    m_cmStreamType.AddString(csTemp);
    m_cmStreamType.SetCurSel(g_struDeviceInfo[m_iDeviceIndex].struChanInfo[m_iChannelIndex].dwStreamType);

    DWORD dwIP = ntohl(inet_addr(g_pMainDlg->m_sLocalIP));
    if (g_struDeviceInfo[m_iDeviceIndex].struChanInfo[m_iChannelIndex].struIP.szIP[0] != '\0')
    {
        dwIP = ntohl(inet_addr(g_struDeviceInfo[m_iDeviceIndex].struChanInfo[m_iChannelIndex].struIP.szIP));
    }
    m_ipAddress.SetAddress(dwIP);

    m_wPort = g_struDeviceInfo[m_iDeviceIndex].struChanInfo[m_iChannelIndex].struIP.wPort;

    UpdateData(FALSE);
    return TRUE;
}

void CDlgChanInfo::OnBnClickedBtnSure()
{
    // TODO: 在此添加控件通知处理程序代码
    UpdateData(TRUE);
    g_struDeviceInfo[m_iDeviceIndex].struChanInfo[m_iChannelIndex].dwLinkMode = m_cmProtocol.GetCurSel();
    g_struDeviceInfo[m_iDeviceIndex].struChanInfo[m_iChannelIndex].dwStreamType = m_cmStreamType.GetCurSel();
    DWORD dwIP = 0;
    m_ipAddress.GetAddress(dwIP);
    memcpy(g_struDeviceInfo[m_iDeviceIndex].struChanInfo[m_iChannelIndex].struIP.szIP, IPToStr(dwIP), 16);
    g_struDeviceInfo[m_iDeviceIndex].struChanInfo[m_iChannelIndex].struIP.wPort = (WORD)m_wPort;
    OnOK();
}
