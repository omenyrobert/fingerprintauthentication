// DlgIPSelect.cpp : 实现文件
//

#include "stdafx.h"
#include "EHomeDemo.h"
#include "DlgIPSelect.h"
#include "afxdialogex.h"

#include <WinSock2.h>
#include <Iphlpapi.h>

#pragma comment(lib,"iphlpapi.lib")

// DlgIPSelect 对话框

IMPLEMENT_DYNAMIC(DlgIPSelect, CDialog)

DlgIPSelect::DlgIPSelect(CWnd* pParent /*=NULL*/)
	: CDialog(DlgIPSelect::IDD, pParent)
{
    m_nPort = 7660;
}

DlgIPSelect::~DlgIPSelect()
{
}

void DlgIPSelect::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COM_IP, m_cmIP);
    DDX_Text(pDX, IDC_EDT_PORT, m_nPort);
    DDX_Text(pDX, IDC_COM_IP, m_csIP);
}


BEGIN_MESSAGE_MAP(DlgIPSelect, CDialog)
    ON_BN_CLICKED(IDOK, &DlgIPSelect::OnBnClickedOk)
END_MESSAGE_MAP()


// DlgIPSelect 消息处理程序


BOOL DlgIPSelect::OnInitDialog()
{
    CDialog::OnInitDialog();

    UpdateData(FALSE);

    GetLocalIP();

    return TRUE;
}

void DlgIPSelect::OnBnClickedOk()
{
    // TODO:  在此添加控件通知处理程序代码
    UpdateData(TRUE);

    CDialog::OnOK();
}

void DlgIPSelect::GetLocalIP()
{
    //PIP_ADAPTER_INFO结构体指针存储本机网卡信息
    BYTE *pArray = NULL;
    PIP_ADAPTER_INFO pIpAdapterInfoTemp = NULL;

    pIpAdapterInfoTemp = new(std::nothrow) IP_ADAPTER_INFO();
    if (pIpAdapterInfoTemp == NULL)
    {
        return;
    }
    PIP_ADAPTER_INFO pIpAdapterInfo = pIpAdapterInfoTemp;
    //得到结构体大小,用于GetAdaptersInfo参数
    unsigned long stSize = sizeof(IP_ADAPTER_INFO);
    //调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量;其中stSize参数既是一个输入量也是一个输出量
    int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
    if (ERROR_BUFFER_OVERFLOW == nRel)
    {
        //如果函数返回的是ERROR_BUFFER_OVERFLOW
        //则说明GetAdaptersInfo参数传递的内存空间不够,同时其传出stSize,表示需要的空间大小
        //这也是说明为什么stSize既是一个输入量也是一个输出量

        //重新申请内存空间用来存储所有网卡信息
        pArray = new(std::nothrow) BYTE[stSize];
        if (pArray == NULL)
        {
            //释放原先申请的内存
            if (pIpAdapterInfoTemp)
            {
                delete pIpAdapterInfoTemp;
                pIpAdapterInfoTemp = NULL;
            }

            return;
        }
        pIpAdapterInfo = (IP_ADAPTER_INFO*)pArray;

        //再次调用GetAdaptersInfo函数,填充pIpAdapterInfo指针变量
        nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
    }

    if (ERROR_SUCCESS == nRel)
    {
        m_cmIP.AddString("0.0.0.0");
        //输出网卡信息
        while (pIpAdapterInfo)
        {
            IP_ADDR_STRING *pIpAddrString = &(pIpAdapterInfo->IpAddressList);
            do
            {
                CString csIp;
                csIp.Format("%s", pIpAddrString->IpAddress.String);
                m_cmIP.AddString(csIp);
                pIpAddrString = pIpAddrString->Next;
            } while (pIpAddrString);
            pIpAdapterInfo = pIpAdapterInfo->Next;
        }
    }

    //释放内存空间
    if (pIpAdapterInfoTemp)
    {
        delete pIpAdapterInfoTemp;
        pIpAdapterInfoTemp = NULL;
    }

    if (pArray)
    {
        delete[]pArray;
        pArray = NULL;
    }

    m_cmIP.SetCurSel(0);

}
