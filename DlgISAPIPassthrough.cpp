// DlgISAPIPassthrough.cpp : 实现文件
//

#include "stdafx.h"
#include "EHomeDemo.h"
#include "DlgISAPIPassthrough.h"
#include "afxdialogex.h"


// CDlgISAPIPassthrough 对话框

IMPLEMENT_DYNAMIC(CDlgISAPIPassthrough, CDialog)

CDlgISAPIPassthrough::CDlgISAPIPassthrough(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgISAPIPassthrough::IDD, pParent)
    , m_sCond(_T(""))
    , m_sInput(_T(""))
    , m_sOutput(_T(""))
    , m_sUrl(_T(""))
{
    m_lUserID = -1;
    m_iDeviceIndex = -1;
}

CDlgISAPIPassthrough::~CDlgISAPIPassthrough()
{
}

void CDlgISAPIPassthrough::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_CMD_TYPE, m_cmbCmdType);
    DDX_Text(pDX, IDC_EDIT_COND, m_sCond);
    DDX_Text(pDX, IDC_EDIT_INPUT, m_sInput);
    DDX_Text(pDX, IDC_EDIT_OUTPUT, m_sOutput);
    DDX_Text(pDX, IDC_EDIT_URL, m_sUrl);
}


BEGIN_MESSAGE_MAP(CDlgISAPIPassthrough, CDialog)
    ON_BN_CLICKED(IDC_BTN_COMMAND, &CDlgISAPIPassthrough::OnClickedBtnCommand)
END_MESSAGE_MAP()


// CDlgISAPIPassthrough 消息处理程序


void CDlgISAPIPassthrough::OnClickedBtnCommand()
{
    // TODO:  在此添加控件通知处理程序代码
    UpdateData(TRUE);

    int iCmdType = m_cmbCmdType.GetCurSel();
    
    NET_EHOME_PTXML_PARAM struPTXML = { 0 };
    struPTXML.pRequestUrl = m_sUrl.GetBuffer(0);
    struPTXML.dwRequestUrlLen = m_sUrl.GetLength();
    struPTXML.pCondBuffer = m_sCond.GetBuffer(0);
    struPTXML.dwCondSize = m_sCond.GetLength();
    struPTXML.pInBuffer = m_sInput.GetBuffer(0);
    struPTXML.dwInSize = m_sInput.GetLength();
    char sOutput[1024 * 10] = { 0 };
    struPTXML.pOutBuffer = sOutput;
    struPTXML.dwOutSize = sizeof(sOutput);

    if (iCmdType == 0) //GET
    {
        if (NET_ECMS_GetPTXMLConfig(m_lUserID, &struPTXML))
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, 1, "NET_ECMS_GetPTXMLConfig");
        }
        else
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, 1, "NET_ECMS_GetPTXMLConfig");
            return;
        }
    }
    else if (iCmdType == 1) //PUT
    {
        if (NET_ECMS_PutPTXMLConfig(m_lUserID, &struPTXML))
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, 1, "NET_ECMS_PutPTXMLConfig");
        }
        else
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, 1, "NET_ECMS_PutPTXMLConfig");
            return;
        }
    }
    else if (iCmdType == 2) //POST
    {
        if (NET_ECMS_PostPTXMLConfig(m_lUserID, &struPTXML))
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, 1, "NET_ECMS_PostPTXMLConfig");
        }
        else
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, 1, "NET_ECMS_PostPTXMLConfig");
            return;
        }
    }
    else if (iCmdType == 3) //DELETE
    {
        if (NET_ECMS_DeletePTXMLConfig(m_lUserID, &struPTXML))
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_SUCC_T, 1, "NET_ECMS_DeletePTXMLConfig");
        }
        else
        {
            g_pMainDlg->AddLog(m_iDeviceIndex, OPERATION_FAIL_T, 1, "NET_ECMS_DeletePTXMLConfig");
            return;
        }
    }
    else
    {
        AfxMessageBox("请选择请求类型");
        return;
    }

    m_sOutput = sOutput;

    UpdateData(FALSE);
}


BOOL CDlgISAPIPassthrough::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  在此添加额外的初始化
    m_iDeviceIndex = g_pMainDlg->GetCurDeviceIndex();
    if (m_iDeviceIndex < 0)
    {
        AfxMessageBox("请选择一个设备");
        return TRUE;
    }
    m_lUserID = g_struDeviceInfo[m_iDeviceIndex].lLoginID;
    if (m_lUserID < 0)
    {
        AfxMessageBox("请先登陆设备");
        return TRUE;
    }

    return TRUE;  // return TRUE unless you set the focus to a control
    // 异常:  OCX 属性页应返回 FALSE
}
