// DlgIPSelect.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "EHomeDemo.h"
#include "DlgIPSelect.h"
#include "afxdialogex.h"

#include <WinSock2.h>
#include <Iphlpapi.h>

#pragma comment(lib,"iphlpapi.lib")

// DlgIPSelect �Ի���

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


// DlgIPSelect ��Ϣ�������


BOOL DlgIPSelect::OnInitDialog()
{
    CDialog::OnInitDialog();

    UpdateData(FALSE);

    GetLocalIP();

    return TRUE;
}

void DlgIPSelect::OnBnClickedOk()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    UpdateData(TRUE);

    CDialog::OnOK();
}

void DlgIPSelect::GetLocalIP()
{
    //PIP_ADAPTER_INFO�ṹ��ָ��洢����������Ϣ
    BYTE *pArray = NULL;
    PIP_ADAPTER_INFO pIpAdapterInfoTemp = NULL;

    pIpAdapterInfoTemp = new(std::nothrow) IP_ADAPTER_INFO();
    if (pIpAdapterInfoTemp == NULL)
    {
        return;
    }
    PIP_ADAPTER_INFO pIpAdapterInfo = pIpAdapterInfoTemp;
    //�õ��ṹ���С,����GetAdaptersInfo����
    unsigned long stSize = sizeof(IP_ADAPTER_INFO);
    //����GetAdaptersInfo����,���pIpAdapterInfoָ�����;����stSize��������һ��������Ҳ��һ�������
    int nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
    if (ERROR_BUFFER_OVERFLOW == nRel)
    {
        //����������ص���ERROR_BUFFER_OVERFLOW
        //��˵��GetAdaptersInfo�������ݵ��ڴ�ռ䲻��,ͬʱ�䴫��stSize,��ʾ��Ҫ�Ŀռ��С
        //��Ҳ��˵��ΪʲôstSize����һ��������Ҳ��һ�������

        //���������ڴ�ռ������洢����������Ϣ
        pArray = new(std::nothrow) BYTE[stSize];
        if (pArray == NULL)
        {
            //�ͷ�ԭ��������ڴ�
            if (pIpAdapterInfoTemp)
            {
                delete pIpAdapterInfoTemp;
                pIpAdapterInfoTemp = NULL;
            }

            return;
        }
        pIpAdapterInfo = (IP_ADAPTER_INFO*)pArray;

        //�ٴε���GetAdaptersInfo����,���pIpAdapterInfoָ�����
        nRel = GetAdaptersInfo(pIpAdapterInfo, &stSize);
    }

    if (ERROR_SUCCESS == nRel)
    {
        m_cmIP.AddString("0.0.0.0");
        //���������Ϣ
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

    //�ͷ��ڴ�ռ�
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
