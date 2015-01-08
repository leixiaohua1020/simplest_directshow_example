/**
 * 最简单的基于DirectShow的播放器-图形界面版
 * Simplest DirectShow Player GUI
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序是一个最简单的基于DirectShow的图形界面播放器。
 * 适合初学者学习DirectShow。
 *
 * This example is the simplest Player based on DirectShow.
 * And it has a simple Graphical User Interface.
 * Suitable for the beginner of DirectShow.
 */

#include "stdafx.h"
#include "playerGUI.h"
#include "playerGUIDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
//	virtual BOOL PreTranslateMessage(MSG* pMsg);
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CplayerGUIDlg 对话框


CplayerGUIDlg::CplayerGUIDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CplayerGUIDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CplayerGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CURTIME, m_curtime);
	DDX_Control(pDX, IDC_DURATION, m_duration);
	DDX_Control(pDX, IDC_PROGRESS, m_progress);
	DDX_Control(pDX, IDC_URL, m_url);
}

BEGIN_MESSAGE_MAP(CplayerGUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(ID_START, &CplayerGUIDlg::OnBnClickedStart)
	ON_BN_CLICKED(ID_PAUSE, &CplayerGUIDlg::OnBnClickedPause)
	ON_BN_CLICKED(ID_STOP, &CplayerGUIDlg::OnBnClickedStop)
	ON_BN_CLICKED(ID_ABOUT, &CplayerGUIDlg::OnBnClickedAbout)
	ON_BN_CLICKED(IDC_FILEBROWSE, &CplayerGUIDlg::OnBnClickedFilebrowse)
	ON_WM_DROPFILES()
	ON_COMMAND(ID_WEBSITE, &CplayerGUIDlg::OnWebsite)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(ID_FULLSCREEN, &CplayerGUIDlg::OnBnClickedFullscreen)
	ON_BN_CLICKED(ID_INFO, &CplayerGUIDlg::OnBnClickedInfo)
	ON_MESSAGE(WM_GRAPHNOTIFY, &CplayerGUIDlg::OnGraphNotify)
END_MESSAGE_MAP()


// CplayerGUIDlg 消息处理程序

BOOL CplayerGUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_progress.SetRange(0,100);
	SystemClear();

	// Init COM
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)){
		AfxMessageBox(_T("Error - Can't init COM."));
		return FALSE;
	}
	hr=CoCreateInstance(CLSID_FilterGraph, NULL,CLSCTX_INPROC_SERVER,IID_IGraphBuilder, (void **)&pGraph);
	if (FAILED(hr)){
		AfxMessageBox(_T("Error - Can't create Filter Graph."));
		return FALSE;
	}
	//  Query Interface
	hr|=pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
	hr|=pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
	hr|=pGraph->QueryInterface(IID_IBasicVideo, (void **)&pVideo);
	hr|=pGraph->QueryInterface(IID_IBasicAudio, (void **)&pAudio);
	hr|=pGraph->QueryInterface(IID_IVideoWindow, (void **)&pWindow);
	hr|=pGraph->QueryInterface(IID_IMediaSeeking, (void **)&pSeeking);
	if (FAILED(hr)){
		AfxMessageBox(_T("Error - Can't Query Interface."));
		return FALSE;
	}


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CplayerGUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CplayerGUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CplayerGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CplayerGUIDlg::OnBnClickedStart()
{
	CStringA cstr_urla;
	CStringW cstr_urlw;
	HRESULT hr;

	//Render
#ifdef _UNICODE
	m_url.GetWindowText(cstr_urlw);
#else
	USES_CONVERSION;
	m_url.GetWindowText(cstr_urla);
	cstr_urlw.Format(L"%s",A2W(cstr_urla));
#endif
	if(cstr_urlw.IsEmpty()){
		AfxMessageBox(_T("Input URL is NULL!"));
		return;
	}

	hr = pGraph->RenderFile(cstr_urlw, NULL);
	if(FAILED(hr)){
		AfxMessageBox(_T("Can't open input file!"));
		return;
	}

	//Set Window
	HWND screen_hwnd=NULL;
	RECT windowRect;
	screen_hwnd = this->GetDlgItem(IDC_SCREEN)->GetSafeHwnd(); 
	::GetClientRect(screen_hwnd, &windowRect);

	pWindow->put_Visible(OAFALSE);
	pWindow->put_Owner((OAHWND)screen_hwnd);
	pWindow->put_Left(0);
	pWindow->put_Top(0);
	pWindow->put_Width(windowRect.right - windowRect.left);
	pWindow->put_Height(windowRect.bottom - windowRect.top);
	pWindow->put_WindowStyle(WS_CHILD|WS_CLIPCHILDREN|WS_CLIPSIBLINGS|WS_THICKFRAME);
	pWindow->put_MessageDrain((OAHWND) screen_hwnd);//Receive Message
	pWindow->put_Visible(OATRUE);

	HWND dlg_hwnd=NULL;
	dlg_hwnd = this->GetSafeHwnd(); 
	pEvent->SetNotifyWindow((OAHWND)dlg_hwnd, WM_GRAPHNOTIFY, 0);

	// Run
	hr = pControl->Run();

	playerstate=STATE_PLAY;
	SetBtn(STATE_PLAY);
	SetTimer(1,1000,NULL);
}


void CplayerGUIDlg::OnBnClickedPause()
{
	HRESULT hr;
	if(playerstate==STATE_PLAY){
		hr=pControl->Pause();
		playerstate=STATE_PAUSE;
		GetDlgItem(ID_PAUSE)->SetWindowText(_T("Resume"));
	}else if(playerstate==STATE_PAUSE){
		hr=pControl->Run();
		playerstate=STATE_PLAY;
		GetDlgItem(ID_PAUSE)->SetWindowText(_T("Pause"));
	}

}


void CplayerGUIDlg::OnBnClickedStop()
{
	long long position = 0;
	HRESULT hr;
	hr = pSeeking->SetPositions(&position, AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame, 
		0, AM_SEEKING_NoPositioning);
	KillTimer(1);
	hr=pControl->Stop();
	
	// Enumerate the filters And remove them
	IEnumFilters *pEnum = NULL;
	hr = pGraph->EnumFilters(&pEnum);
	if (SUCCEEDED(hr))
	{
		IBaseFilter *pFilter = NULL;
		while (S_OK == pEnum->Next(1, &pFilter, NULL))
		{
			// Remove the filter.
			pGraph->RemoveFilter(pFilter);
			// Reset the enumerator.
			pEnum->Reset();
			pFilter->Release();
		}
		pEnum->Release();
	}

	SystemClear();
}


void CplayerGUIDlg::OnBnClickedAbout()
{
	CAboutDlg dlg;
	dlg.DoModal();
}


void CplayerGUIDlg::OnBnClickedFilebrowse()
{
	CString FilePathName;
	//Filter String
	CString strfilter;
	strfilter.Append(_T("Common media formats|*.avi;*.wmv;*.wmp;*.wm;*.asf;*.rm;*.ram;*.rmvb;*.ra;*.mpg;*.mpeg;*.mpe;*.m1v;*.m2v;*.mpv2;"));
	strfilter.Append(_T("*.mp2v;*.dat;*.mp4;*.m4v;*.m4p;*.vob;*.ac3;*.dts;*.mov;*.qt;*.mr;*.3gp;*.3gpp;*.3g2;*.3gp2;*.swf;*.ogg;*.wma;*.wav;"));
	strfilter.Append(_T("*.mid;*.midi;*.mpa;*.mp2;*.mp3;*.m1a;*.m2a;*.m4a;*.aac;*.mkv;*.ogm;*.m4b;*.tp;*.ts;*.tpr;*.pva;*.pss;*.wv;*.m2ts;*.evo;"));
	strfilter.Append(_T("*.rpm;*.realpix;*.rt;*.smi;*.smil;*.scm;*.aif;*.aiff;*.aifc;*.amr;*.amv;*.au;*.acc;*.dsa;*.dsm;*.dsv;*.dss;*.pmp;*.smk;*.flic|"));
	strfilter.Append(_T("Windows Media Video(*.avi;*wmv;*wmp;*wm;*asf)|*.avi;*.wmv;*.wmp;*.wm;*.asf|"));
	strfilter.Append(_T("Windows Media Audio(*.wma;*wav;*aif;*aifc;*aiff;*mid;*midi;*rmi)|*.wma;*.wav;*.aif;*.aifc;*.aiff;*.mid;*.midi;*.rmi|"));
	strfilter.Append(_T("Real(*.rm;*ram;*rmvb;*rpm;*ra;*rt;*rp;*smi;*smil;*.scm)|*.rm;*.ram;*.rmvb;*.rpm;*.ra;*.rt;*.rp;*.smi;*.smil;*.scm|"));
	strfilter.Append(_T("MPEG Video(*.mpg;*mpeg;*mpe;*m1v;*m2v;*mpv2;*mp2v;*dat;*mp4;*m4v;*m4p;*m4b;*ts;*tp;*tpr;*pva;*pss;*.wv;)|"));
	strfilter.Append(_T("*.mpg;*.mpeg;*.mpe;*.m1v;*.m2v;*.mpv2;*.mp2v;*.dat;*.mp4;*.m4v;*.m4p;*.m4b;*.ts;*.tp;*.tpr;*.pva;*.pss;*.wv;|"));
	strfilter.Append(_T("MPEG Audio(*.mpa;*mp2;*m1a;*m2a;*m4a;*aac;*.m2ts;*.evo)|*.mpa;*.mp2;*.m1a;*.m2a;*.m4a;*.aac;*.m2ts;*.evo|"));
	strfilter.Append(_T("DVD(*.vob;*ifo;*ac3;*dts)|*.vob;*.ifo;*.ac3;*.dts|MP3(*.mp3)|*.mp3|CD Tracks(*.cda)|*.cda|"));
	strfilter.Append(_T("Quicktime(*.mov;*qt;*mr;*3gp;*3gpp;*3g2;*3gp2)|*.mov;*.qt;*.mr;*.3gp;*.3gpp;*.3g2;*.3gp2|"));
	strfilter.Append(_T("Flash Files(*.flv;*swf;*.f4v)|*.flv;*.swf;*.f4v|Playlist(*.smpl;*.asx;*m3u;*pls;*wvx;*wax;*wmx;*mpcpl)|*.smpl;*.asx;*.m3u;*.pls;*.wvx;*.wax;*.wmx;*.mpcpl|"));
	strfilter.Append(_T("Others(*.ivf;*au;*snd;*ogm;*ogg;*fli;*flc;*flic;*d2v;*mkv;*pmp;*mka;*smk;*bik;*ratdvd;*roq;*drc;*dsm;*dsv;*dsa;*dss;*mpc;*divx;*vp6;*.ape;*.flac;*.tta;*.csf)"));
	strfilter.Append(_T("|*.ivf;*.au;*.snd;*.ogm;*.ogg;*.fli;*.flc;*.flic;*.d2v;*.mkv;*.pmp;*.mka;*.smk;*.bik;*.ratdvd;*.roq;*.drc;*.dsm;*.dsv;*.dsa;*.dss;*.mpc;*.divx;*.vp6;*.ape;*.amr;*.flac;*.tta;*.csf|"));
	strfilter.Append(_T("All Files(*.*)|*.*||"));

	LPCTSTR lpszfilter=strfilter;
	CFileDialog dlg(TRUE,NULL,NULL,NULL,lpszfilter);///TRUE为OPEN对话框，FALSE为SAVE AS对话框 
	if(dlg.DoModal()==IDOK) {
		FilePathName=dlg.GetPathName();
		m_url.SetWindowText(FilePathName);
	}
}

void CplayerGUIDlg::SystemClear()
{
	SetBtn(STATE_PREPARE);
	playerstate=STATE_PREPARE;
	m_curtime.SetWindowText(_T("00:00:00"));
	m_duration.SetWindowText(_T("00:00:00"));
	m_progress.SetPos(0);


	//Background
	CPaintDC dc(this);
	CRect   rect;   
	GetDlgItem(IDC_SCREEN)->GetClientRect(&rect);   
	CDC   dcMem;   
	dcMem.CreateCompatibleDC(&dc);   
	CBitmap   bmpBackground;   
	bmpBackground.LoadBitmap(IDB_WELCOME);   
	//IDB_BITMAP是你自己的图对应的ID   
	BITMAP   bitmap;   
	bmpBackground.GetBitmap(&bitmap);   
	CBitmap   *pbmpOld=dcMem.SelectObject(&bmpBackground);   
	dc.StretchBlt(0,0,rect.Width(),rect.Height(),&dcMem,0,0,   
		bitmap.bmWidth,bitmap.bmHeight,SRCCOPY);   
	Invalidate(TRUE);
}

void CplayerGUIDlg::SetBtn(PlayerState state)
{
	if(state==STATE_PREPARE){
		GetDlgItem(ID_START)->EnableWindow(TRUE);
		GetDlgItem(ID_PAUSE)->EnableWindow(FALSE);
		GetDlgItem(ID_STOP)->EnableWindow(FALSE);
		GetDlgItem(ID_FULLSCREEN)->EnableWindow(FALSE);
		GetDlgItem(ID_INFO)->EnableWindow(FALSE);
	}else{
		GetDlgItem(ID_START)->EnableWindow(FALSE);
		GetDlgItem(ID_PAUSE)->EnableWindow(TRUE);
		GetDlgItem(ID_STOP)->EnableWindow(TRUE);
		GetDlgItem(ID_FULLSCREEN)->EnableWindow(TRUE);
		GetDlgItem(ID_INFO)->EnableWindow(TRUE);
	}
}


void CplayerGUIDlg::OnDropFiles(HDROP hDropInfo)
{
	CDialogEx::OnDropFiles(hDropInfo);
	LPTSTR pFilePathName =(LPTSTR)malloc(MAX_URL_LENGTH);
	::DragQueryFile(hDropInfo, 0, pFilePathName,MAX_URL_LENGTH);  // 获取拖放文件的完整文件名，最关键！
	m_url.SetWindowText(pFilePathName);
	::DragFinish(hDropInfo);   // 注意这个不能少，它用于释放Windows 为处理文件拖放而分配的内存
	free(pFilePathName);
}


void CplayerGUIDlg::OnWebsite()
{
	ShellExecuteA( NULL,"open","http://blog.csdn.net/leixiaohua1020",NULL,NULL,SW_SHOWNORMAL);
}


void CplayerGUIDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// Release resource
	if(pVideo)
		pVideo->Release();
	if(pAudio)
		pAudio->Release();
	if(pWindow)
		pWindow->Release();
	if(pSeeking)
		pSeeking->Release();

	if(pControl)
	pControl->Release();
	if(pEvent)
	pEvent->Release();
	if(pGraph)
	pGraph->Release();

	CoUninitialize();
}




void CplayerGUIDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1){
		CString curtimestr,durationstr;
		long long curtime;
		long long duration;
		int tns, thh, tmm, tss;
		int progress;
		//ms
		pSeeking->GetCurrentPosition(&curtime);
		if(curtime!=0){
			//change to second
			tns = curtime/10000000;
			thh  = tns / 3600;
			tmm  = (tns % 3600) / 60;
			tss  = (tns % 60);
			curtimestr.Format(_T("%02d:%02d:%02d"),thh,tmm,tss);
			m_curtime.SetWindowText(curtimestr);
		}
		pSeeking->GetDuration(&duration);
		if(duration!=0){
			tns = duration/10000000;
			thh  = tns / 3600;
			tmm  = (tns % 3600) / 60;
			tss  = (tns % 60);
			durationstr.Format(_T("%02d:%02d:%02d"),thh,tmm,tss);
			m_duration.SetWindowText(durationstr);

			progress=curtime*100/duration;
			m_progress.SetPos(progress);
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}


void CplayerGUIDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar->GetSafeHwnd() == m_progress.GetSafeHwnd()){
		float pos_bar=0.0;
		long long duration=0.0;
		long long pos_time=0.0;
		if(nSBCode==SB_THUMBPOSITION){
			pos_bar=(float)nPos/100.0;
			pSeeking->GetDuration(&duration);
			pos_time=pos_bar*duration;

			long long position = (long long)(pos_time);
			HRESULT hr = pSeeking->SetPositions(&position, AM_SEEKING_AbsolutePositioning | AM_SEEKING_SeekToKeyFrame, 
				0, AM_SEEKING_NoPositioning);
		}
	}
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CplayerGUIDlg::OnBnClickedFullscreen()
{
	pWindow->put_FullScreenMode(OATRUE);
}


void CplayerGUIDlg::OnBnClickedInfo()
{
	CStringW infostrw;
	infostrw.AppendFormat(L"========Video Information========\r\n");
	long video_w=0,video_h=0;
	REFTIME avgtimeperframe=0;
	float framerate=0;
	pVideo->get_VideoWidth(&video_w);
	pVideo->get_VideoHeight(&video_h);
	pVideo->get_AvgTimePerFrame(&avgtimeperframe);
	framerate=1/avgtimeperframe;
	infostrw.AppendFormat(L"Video Width:%d\r\nVideo Height:%d\r\nVideo Framerate:%f\r\n"
		,video_w,video_h,framerate);

	infostrw.AppendFormat(L"=======Filters in FilterGpragh=======\r\n");

	IEnumFilters *pFilterEnum=NULL;
	if(FAILED(pGraph->EnumFilters(&pFilterEnum))){
		pFilterEnum->Release();
		return ;
	}
	pFilterEnum->Reset();
	IBaseFilter * filter = NULL;
	ULONG fetchCount = 0;
	//Pin Info
	while (SUCCEEDED(pFilterEnum->Next(1, &filter, &fetchCount)) && fetchCount){
		if (!filter){
			continue;
		}
		FILTER_INFO FilterInfo;
		if (FAILED(filter->QueryFilterInfo(&FilterInfo))){
			continue;
		}
		infostrw.AppendFormat(L"[%s]\r\n",FilterInfo.achName);
		filter->Release();
	}
	pFilterEnum->Release();
	infostrw.AppendFormat(L"===========================\r\n");

#ifdef _UNICODE
	AfxMessageBox(infostrw,MB_ICONINFORMATION);
#else
	USES_CONVERSION;
	CStringA infostra;
	infostra.Format("%s",W2A(infostrw));
	AfxMessageBox(infostra,MB_ICONINFORMATION);
#endif

}

//Exit Full Screen mode when push "ESC"
BOOL CplayerGUIDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN){
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE){
			// Restore form fullscreen mode
			pWindow->put_FullScreenMode(OAFALSE);

			return 1;
		}
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

// Deal with the graph events 
LRESULT CplayerGUIDlg::OnGraphNotify(WPARAM inWParam, LPARAM inLParam)
{
	LONG   eventCode = 0, eventParam1 = 0, eventParam2 = 0;
	while (SUCCEEDED(pEvent->GetEvent(&eventCode, &eventParam1, &eventParam2, 0)))
	{ 
		// Spin through the events
		pEvent->FreeEventParams(eventCode, eventParam1, eventParam2);
		switch (eventCode)
		{
		case EC_COMPLETE:{
			OnBnClickedStop();
			break;
			}
		case EC_USERABORT:
		case EC_ERRORABORT:{
			OnBnClickedStop();
			break;
			}
		default:
			break;
		}
	}
	return 0;
}