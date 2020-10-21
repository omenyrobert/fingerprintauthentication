#pragma once


// DlgIPSelect 对话框

class DlgIPSelect : public CDialog
{
	DECLARE_DYNAMIC(DlgIPSelect)

public:
	DlgIPSelect(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~DlgIPSelect();

// 对话框数据
	enum { IDD = IDD_DLG_IP_SELECT };


    void GetLocalIP();

protected:
    virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();

    int m_nPort;
    CString m_csIP;

private:
    CComboBox m_cmIP;
};
