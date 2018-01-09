
// OpenCV_picDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "OpenCV_pic.h"
#include "OpenCV_picDlg.h"
#include "afxdialogex.h"
#include "img_effect.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace cv;
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


// COpenCV_picDlg 对话框



COpenCV_picDlg::COpenCV_picDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(COpenCV_picDlg::IDD, pParent)
	, m_sFile(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void COpenCV_picDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDT_FILE, m_sFile);
}

BEGIN_MESSAGE_MAP(COpenCV_picDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_FILE, &COpenCV_picDlg::OnBnClickedBtnFile)
END_MESSAGE_MAP()


// COpenCV_picDlg 消息处理程序

BOOL COpenCV_picDlg::OnInitDialog()
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
	CRect rc;

	GetDlgItem(IDC_STA_PIC)->GetWindowRect(&rc);
	cv::namedWindow("Pic", CV_WINDOW_NORMAL);
	cvResizeWindow("Pic", rc.Width(), rc.Height());

	HWND hWnd = (HWND)cvGetWindowHandle("Pic");
	HWND hParent = ::GetParent(hWnd);
	::SetParent(hWnd, GetDlgItem(IDC_STA_PIC)->m_hWnd);
	::ShowWindow(hParent, SW_HIDE);


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void COpenCV_picDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void COpenCV_picDlg::OnPaint()
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
HCURSOR COpenCV_picDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void COpenCV_picDlg::OnBnClickedBtnFile()
{
	CFileDialog  Dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("JPG Files(*.jpg)|*.jpg|JPEG Files(*.jpeg)|*.jpeg|BMP Files(*.bmp)|*.bmp|PNG Files(*.png)|*.png|All Files(*.*)|*.*||"));
	//打开文件
	if (Dlg.DoModal() == IDOK) {//是否打开成功
		m_sFile = Dlg.GetPathName();
		m_Image = imread((LPCSTR)(CStringA)(m_sFile));  //CString to string
		Mat dst;

		// blur(m_Image, dst, Size(5, 5));
		// boxFilter(m_Image, dst, -1, Size(5, 5));

		applySketchEffect(m_Image, dst, Size(19, 19), 0);
		imwrite("C:/Users/joegn/Pictures/effect-sketch.jpg", dst);
		applyCartoonEffect(m_Image, dst, 2, 7, 9, 9, 7);
		imwrite("C:/Users/joegn/Pictures/effect-cartoon.jpg", dst);
		applyOilPaintingEffect(m_Image, dst, 3, 10);
		imwrite("C:/Users/joegn/Pictures/effect-oil-painting.jpg", dst);
		applyOldPhotoEffect(m_Image, dst);
		imwrite("C:/Users/joegn/Pictures/effect-old-photo.jpg", dst);
		applyNegativePhotoEffect(m_Image, dst);
		imwrite("C:/Users/joegn/Pictures/effect-negative.jpg", dst);
		applyReliefEffect(m_Image, dst);
		imwrite("C:/Users/joegn/Pictures/effect-relief.jpg", dst);
		applySmeltColorEffect(m_Image, dst);
		imwrite("C:/Users/joegn/Pictures/effect-smelt-color.jpg", dst);
		applyIceColorEffect(m_Image, dst);
		imwrite("C:/Users/joegn/Pictures/effect-ice-color.jpg", dst);
		
		imshow("Pic", dst);

		UpdateData(false);
	}


}