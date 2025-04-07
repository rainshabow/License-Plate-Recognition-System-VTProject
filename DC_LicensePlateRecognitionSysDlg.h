#include <string>


// CDCLicensePlateRecognitionSysDlg 对话框
class CDCLicensePlateRecognitionSysDlg : public CDialogEx
{
// 构造
public:
	CDCLicensePlateRecognitionSysDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DC_LICENSEPLATERECOGNITIONSYS_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	unsigned char pixelB, pixelG, pixelR;  //各通道值
	unsigned char DifMax = 60;             //基于颜色区分的阈值
	unsigned char B = 160, G = 60, R = 60; //各通道的阈值设定，针对与蓝色车牌
public:
//	Mat OriginalImg;
public:
	std::string dc_FilePath;		//记录载入的图片路径
	std::string m_strVideoPath;		//记录载入的视频路径
	int	ImageWidth, ImageHeight;	//输入图像的长何宽


public:
//	void drawCImg(IplImage& plmg);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	CStatic m_ChepaiPic; // 图片控件1
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedLoadPic();
	afx_msg void OnBnClickedOpencam();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedPlatepos();
	CString result;
	afx_msg void OnBnClickedCancel();
	afx_msg void OnStnClickedCarnum();
	afx_msg void OnBnClickedVideoplayer();
	void process();
	afx_msg void OnBnClickedLoadVideo();
};
