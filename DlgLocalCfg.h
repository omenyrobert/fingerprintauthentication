#pragma once


// CDlgLocalCfg dialog

class CDlgLocalCfg : public CDialog
{
    DECLARE_DYNAMIC(CDlgLocalCfg)

public:
    CDlgLocalCfg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CDlgLocalCfg();

    CComboBox m_comCmsAccess;
    CComboBox m_comAlarmAccess;
    CComboBox m_comStreamAccess;

// Dialog Data
    enum { IDD = IDD_DLG_LOCAL_CFG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnOk();
    afx_msg void OnBtnGetSecureAccessClicked();
    afx_msg void OnBtnSetSecureAccessClicked();

    CString m_strLogPath;
    long m_iLogLevel;
    CString m_strSMSIP;
    CString m_strAMSIP;
    BOOL m_bAutoDel;
    afx_msg void OnBnClickedBtnCmsAlarm();
    DWORD m_dwKeepLive;
    afx_msg void OnBnClickedBtnRegCfg();
    DWORD m_dwCount;
    CString m_csAlarmServerIP;
    int m_iAlarmTcpPort;
    int m_iAlarmUdpPort;
    CString m_csPicServerIP;
    int m_iPicServerPort;

    int m_iAlarmType;
    int m_iPicType;
};
