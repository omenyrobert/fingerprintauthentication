#pragma once
#include "afxwin.h"

#include "HCEHomeStream.h"
#include "wavesound.h"
#include "SoundIn.h"


#define AUDIOTALKTYPE_G722       0
#define AUDIOTALKTYPE_G711_MU    1
#define AUDIOTALKTYPE_G711_A     2
#define AUDIOTALKTYPE_MP2L2      5
#define AUDIOTALKTYPE_G726       6
#define AUDIOTALKTYPE_AAC        7
#define AUDIOTALKTYPE_PCM        8


#define LISTEN_PORT_BASE    7500

#define AUDENCSIZE            1280
#define AUDDECSIZE            80

#define G711_AUDENCSIZE        320
#define G711_AUDDECSIZE        160

#define G726_AUDENCSIZE     640
#define G726_AUDDECSIZE        80 

#define BITS_PER_SAMPLE        16
#define CHANNEL                1
#define SAMPLES_PER_SECOND    16000

#define BIT_RATE_16000        16000
#define AUDIOBUF            (160*40L)

#define MAX_SOUND_OUT        20

typedef struct  STRUCT_TALK_MR
{
    BYTE byFlag;
    BYTE byAudioType;
    BYTE byIndex;
    BYTE res;
    STRUCT_TALK_MR()
    {
        byFlag = 0;
        byAudioType = 0;
        byIndex = 0;
        res = 0;
    }
}TALK_MR, *LPTALK_MR;

// CDlgAudioTalk dialog



class CDlgAudioTalk : public CDialog
{
    DECLARE_DYNAMIC(CDlgAudioTalk)

public:
    CDlgAudioTalk(CWnd* pParent = NULL);   // standard constructor
    virtual ~CDlgAudioTalk();

// Dialog Data
    enum { IDD = IDD_DLG_AUDIO_TALK };

    NET_EHOME_VOICETALK_PARA    m_struVoiceTalkPara;
    LONG m_lVoiceTalkHandle;

protected:

    virtual BOOL OnInitDialog();

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
protected:
    static DWORD WINAPI SendVoiceDataThread (LPVOID lpArg);
    HANDLE    m_hVoiceTransmit;
    BOOL    m_bExitThread;
public:
    void    CheckInitParam();
public:
    afx_msg void OnBnClickedBtnStartVoicetalkListen();
    afx_msg void OnBnClickedBtnReqAudioTalk();
    afx_msg void OnBnClickedBtnStartPush();
    afx_msg void OnBnClickedBtnStopPush();
    afx_msg void OnBnClickedBtnStopListen();

    afx_msg void OnBnClickedBtnStartVoicetalk();
    afx_msg void OnBnClickedBtnStopVoicetalk();
    CComboBox m_combCBDataType;
    afx_msg void OnBnClickedBtnStartTransmit();
    afx_msg void OnBnClickedBtnStopTransmit();
    CComboBox m_cmbVoiceChan;

    afx_msg void OnBnClickedBtnAudiotalkTest();

    static BOOL CALLBACK onDeviceAudioTalkConnectCB(LONG lAudioTalkHandle, NET_EHOME_VOICETALK_NEWLINK_CB_INFO *pNewLinkCBMsg, void *pUserData);
    static BOOL CALLBACK AudioTalkStreamCallback(LONG iAudioTalkHandle, NET_EHOME_VOICETALK_DATA_CB_INFO *pVoiceTalkCBMsg, void *pUserData);

    CString m_csIP;
    int m_nPort;

    CComboBox m_cmbLocalType;

    int m_nLocalType;

private:

    int m_iSessionID;

    LONG m_lAudioTalkHandle;
    int  m_iAudioEncType;

    BOOL m_bCMSAudioTalk;
    LONG m_lListenHandle;

    BOOL            m_bWaveDeal;

    TALK_MR         m_talkMr;

    WAVEFORMATEX    m_struWaveFormat;
#ifndef WIN64
    CWaveSound      m_SoundOut;
    CSoundIn        m_SoundIn;
#endif
    BOOL            m_bOpenWavOut;
    BOOL            m_bOpenWavIn;

    HMMIO           m_hFile;
    MMCKINFO        m_MMCKInfoData;
    MMCKINFO        m_MMCKInfoParent;
    MMCKINFO        m_MMCKInfoChild;

    LPBYTE  m_pRenderBuf;        //buffer
    int     m_rIndexRV;          //data that has already been read
    int     m_ReceiveIndexRV;    //received data for m_pRenderBuf
    int     m_nBufNo;
    BOOL    m_bOpenPlayThread;

//     void*   m_pEncoder;     //g722 encoder
//     void*   m_pG726Enc;     //g726±àÂë¾ä±ú
//     void*   m_pG726EncM;    //g726±àÂëÄ£¿é¾ä±ú
//     BOOL    m_bReset;       //g726Ê×Ö¡ÖØÖÃ

    DWORD   m_dwBufSize;
    DWORD   m_dwBufNum;

//     BYTE    m_byG711DecBuf[G711_AUDDECSIZE*2];
//     BYTE    m_byDecBuf[AUDENCSIZE];
//     BYTE    m_byEncBuf[AUDDECSIZE];
//     BYTE    m_byEncBuf711[G711_AUDDECSIZE];
//     UINT16  m_wG711AudioTemp[AUDENCSIZE/2];

    HANDLE  m_hExitEvent;
    HANDLE  m_hHaveRDataRV;


    BOOL m_bRecord;
    BOOL m_bOpenTalk;

public:

    BOOL SendDataToDevice(char *buf, DWORD dwSize);
    static CDlgAudioTalk *s_pAudioTalkDlg;
    void InputAudioData(LPTALK_MR lpTalkMR);
    static UINT PlayAudioThread(LPVOID pParam);
    BOOL CopyAudioData(PBYTE lpTemp, DWORD dwReadLength, int nIndex);
    BOOL PutIntoBuf(char *lpTemp, int Bytes, LPTALK_MR lpTalkMR);
    void ExitPlayAudio(BYTE *lpTemp, BYTE *lpPlayBuf, void *pDecoder, void *pG726Dec, void *pG726DecM, BYTE byIndex);

    BOOL OpenAudioIn();
    BOOL OpenAudioOut();

    CRITICAL_SECTION m_csAudioBuf;
};
