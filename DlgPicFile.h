#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CDlgPicFile �Ի���

class CDlgPicFile : public CDialog
{
    DECLARE_DYNAMIC(CDlgPicFile)

public:
    CDlgPicFile(CWnd* pParent = NULL);   // ��׼���캯��
    virtual ~CDlgPicFile();

// �Ի�������
    enum { IDD = IDD_DIALOG_PIC_FILE };

    BOOL CheckInitParam();
    static UINT GetFileThread(LPVOID pParam);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnSearchPicFile();
    virtual BOOL OnInitDialog();
    CListCtrl m_lstPicFile;
    CTime m_dateStart;
    CTime m_timeStart;
    CTime m_DateStop;
    CTime m_timeStop;
    CComboBox m_cmbPicType;

    LONG m_iDeviceIndex;
    LONG m_lLoginID;
    BOOL m_bSearching;
    BOOL m_bQuit;
    LONG m_iChanIndex;
    LONG m_lFileHandle;
    HANDLE m_hFileThread;
    DWORD m_dwFileNum;
    NET_EHOME_FINDCOND m_struFindCond;
    NET_EHOME_FINDDATA m_struFindData;
    int m_byTimeType;
};
