// DlgPassthroughProxy.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "EHomeDemo.h"
#include "DlgPassthroughProxy.h"


// CDlgPassthroughProxy �Ի���

IMPLEMENT_DYNAMIC(CDlgPassthroughProxy, CDialog)

CDlgPassthroughProxy::CDlgPassthroughProxy(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgPassthroughProxy::IDD, pParent)
    , m_sSDKListenIP(_T(""))
    , m_dwSDKListenPort(0)
    , m_sHTTPListenIP(_T(""))
    , m_dwHTTPListenPort(0)
    , m_lListenSDK(-1)
    , m_lListenHTTP(-1)
{

}

CDlgPassthroughProxy::~CDlgPassthroughProxy()
{
}

void CDlgPassthroughProxy::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_IP_SDK, m_sSDKListenIP);
    DDX_Text(pDX, IDC_EDIT_PORT_SDK, m_dwSDKListenPort);
    DDX_Text(pDX, IDC_EDIT_IP_HTTP, m_sHTTPListenIP);
    DDX_Text(pDX, IDC_EDIT_PORT_HTTP, m_dwHTTPListenPort);
}


BEGIN_MESSAGE_MAP(CDlgPassthroughProxy, CDialog)
    ON_BN_CLICKED(IDC_BTN_START_PT_LISTEN, &CDlgPassthroughProxy::OnBnClickedBtnStartPtListen)
    ON_BN_CLICKED(IDC_BTN_STOP_PT_LISTEN, &CDlgPassthroughProxy::OnBnClickedBtnStopPtListen)
END_MESSAGE_MAP()


// CDlgPassthroughProxy ��Ϣ�������

void CDlgPassthroughProxy::OnBnClickedBtnStartPtListen()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    char szLan[512] = {0};
    if (m_lListenSDK > -1 || m_lListenHTTP > -1)
    {
        g_StringLanType(szLan, "͸�������ѿ��������ȹر�", "Please stop the passthrough proxy first.");
        AfxMessageBox(szLan);
        return;
    }

    UpdateData(TRUE);
   
    NET_EHOME_PT_PARAM struPTPara = {0};
    if (m_sSDKListenIP.GetLength() != 0)
    {
        strcpy(struPTPara.struIP.szIP, m_sSDKListenIP);
        struPTPara.struIP.wPort = (WORD)m_dwSDKListenPort;
        struPTPara.byProxyType = ENUM_PROXY_TYPE_NETSDK;
        m_lListenSDK = NET_ECMS_StartListenProxy(&struPTPara); //NetSDK����
        if (m_lListenSDK < 0)
        {
            g_StringLanType(szLan, "����NETSDK͸������ʧ��", "Start NetSDK PT proxy failed.");
            AfxMessageBox(szLan);
            return;
        }
    }

    if (m_sHTTPListenIP.GetLength() != 0)
    {
        strcpy(struPTPara.struIP.szIP, m_sHTTPListenIP);
        struPTPara.struIP.wPort = (WORD)m_dwHTTPListenPort;
        struPTPara.byProxyType = ENUM_PROXY_TYPE_HTTP;
        m_lListenHTTP = NET_ECMS_StartListenProxy(&struPTPara); //HTTP����
        if (m_lListenHTTP < 0)
        {
            g_StringLanType(szLan, "����HTTP͸������ʧ��", "Start HTTP PT proxy failed.");
            AfxMessageBox(szLan);
            return;
        }
    }
}

void CDlgPassthroughProxy::OnBnClickedBtnStopPtListen()
{
    // TODO: �ڴ���ӿؼ�֪ͨ����������
    if (m_lListenSDK > -1)
    {
        NET_ECMS_StopListenProxy(m_lListenSDK, ENUM_PROXY_TYPE_NETSDK);
        m_lListenSDK = -1;
    }
    if (m_lListenHTTP > -1)
    {
        NET_ECMS_StopListenProxy(m_lListenHTTP, ENUM_PROXY_TYPE_HTTP);
        m_lListenHTTP = -1;
    }
}
