#include "pch.h"
#include "framework.h"
#include "afxdialogex.h"
#include "DC_LicensePlateRecognitionSys.h"
#include "DC_LicensePlateRecognitionSysDlg.h"
#include <io.h>
#include <string>
#include <opencv2\ml.hpp>
#include <opencv2\opencv.hpp>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <fcntl.h> 
#include "stdio.h"
#include <iostream> 
#include<iostream>
#include<cmath> 
#include <algorithm>
#include <atlimage.h>  // 包含ATL的头文件
#include <algorithm> // 包含 reverse 算法
#include <numeric>   // 包含 accumulate 算法

#define MODEL_NUM 65  //模板图像数量

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
using namespace std;
using namespace cv;
using namespace ml;

int g_slider_pos = 0; //位置
bool isStop = false; //是否暂停
int hueEnhanceFactor = 1;  // 初始色度增强因子
int ContrastValue = 1;
int BrightValue = 0;
VideoCapture cap;
Mat OriginalImg;
CStatic m_ChepaiPic;//图片控件1

//数字0-9对应48-57 字母A-Z对应65-90
Mat NUM[65][3];//字符匹配模板	
string modelPath = "model_new\\";//模板图像路径
String word[65] = { "0","1","2","3","4","5","6","7","8","9",
"A","B","C","D","E","F","G","H","J","K","L","M","N","P","Q","R","S","7","U","V","W","X","Y","Z",
"藏","川","鄂","甘","赣","贵","桂","黑","沪","吉","冀","津","晋","京","辽","鲁","蒙","闽","宁",
"青","琼","陕","苏","皖","湘","京","渝","豫","粤","云","浙" };//"T"->"7", "新"->"京"

tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Chepai_pic, m_ChepaiPic);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CDCLicensePlateRecognitionSysDlg 对话框



CDCLicensePlateRecognitionSysDlg::CDCLicensePlateRecognitionSysDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DC_LICENSEPLATERECOGNITIONSYS_DIALOG, pParent)
	, result(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDCLicensePlateRecognitionSysDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	/*DDX_Text(pDX, IDC_CARNUM, result);*/
}

BEGIN_MESSAGE_MAP(CDCLicensePlateRecognitionSysDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CDCLicensePlateRecognitionSysDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_LOAD_PIC, &CDCLicensePlateRecognitionSysDlg::OnBnClickedLoadPic)
	ON_BN_CLICKED(IDC_OPENCAM, &CDCLicensePlateRecognitionSysDlg::OnBnClickedOpencam)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_PLATEPOS, &CDCLicensePlateRecognitionSysDlg::OnBnClickedPlatepos)
	ON_BN_CLICKED(IDCANCEL, &CDCLicensePlateRecognitionSysDlg::OnBnClickedCancel)
	ON_STN_CLICKED(IDC_CARNUM, &CDCLicensePlateRecognitionSysDlg::OnStnClickedCarnum)
	ON_BN_CLICKED(IDC_VideoPlayer, &CDCLicensePlateRecognitionSysDlg::OnBnClickedVideoplayer)
	ON_BN_CLICKED(IDC_LOAD_VIDEO, &CDCLicensePlateRecognitionSysDlg::OnBnClickedLoadVideo)
END_MESSAGE_MAP()


// CDCLicensePlateRecognitionSysDlg 消息处理程序

BOOL CDCLicensePlateRecognitionSysDlg::OnInitDialog()
{

	::AllocConsole();//打开控件台资源
	FILE *fp;
	freopen_s(&fp, "CONOUT$", "w+t", stdout);


	CDialogEx::OnInitDialog();
	// 将“关于...”菜单项添加到系统菜单中。
	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CDCLicensePlateRecognitionSysDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
void CDCLicensePlateRecognitionSysDlg::OnPaint()
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
HCURSOR CDCLicensePlateRecognitionSysDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CDCLicensePlateRecognitionSysDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}

//加载图片
void CDCLicensePlateRecognitionSysDlg::OnBnClickedLoadPic()
{
	
	CString PathName;
	CString filter;
	filter = "所有文件(*.bmp,*.png,*.gif,*.tiiff)|*.bmp;*.png;*.jpg;*.gif;*.tiff| BMP(*.bmp)|*.bmp| JPG(*.jpg)|*.jpg|PNG(*.png)|*.png|GIF(*.gif)|*.gif| TIFF(*.tiff)|*.tiff||";
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter, NULL);
	if (dlg.DoModal() == IDOK)
	{
		//打开对话框获取图像信息
		PathName = dlg.GetPathName();     //获取文件路径名   如D:\pic\abc.bmp
		dc_FilePath = CT2A(PathName);//CString转string,如果需要，const char *strTemp = FilePath.c_str();
		ATL::CImage myImage;
		myImage.Load(PathName);
		CRect rect;
		CWnd *pWnd = GetDlgItem(IDC_PICTURE);  //(这是在此资源创建的类的内部, 若是在外部, 可先通过获得CMainFrame的指针, 再通过pMianFrame->GetDlgItem(IDCk_MY_PIC)来获取)
		CDC *pDC = pWnd->GetDC();
		pWnd->GetClientRect(&rect);		pDC->SetStretchBltMode(STRETCH_HALFTONE);
		myImage.Draw(pDC->m_hDC, rect);
		ReleaseDC(pDC);
		myImage.Destroy();
		//在Picture控件中显示图片
	}

	for (int i = 0; i < 65; i++) {
		for (int j = 0; j < 3; j++) {
			String imgPath = modelPath + word[i] + "\\" + word[i] + "-" + word[j] + ".jpg";
			cout << imgPath << endl;
			NUM[i][j] = imread(imgPath);
			if (NUM[i][j].empty())
			{
				AfxMessageBox(_T("图像读取失败，请检查图像名、路径！！！！！！"));
				return;
			}
		}
	}


}

//车牌定位
void CDCLicensePlateRecognitionSysDlg::OnBnClickedPlatepos()
{
	OriginalImg = imread(dc_FilePath, IMREAD_COLOR);
	process();


}

void CDCLicensePlateRecognitionSysDlg::process()
{
	double  BeginTime = (double)getTickCount();		//识别开始时间点，返回从操作系统启动所经过（elapsed）的毫秒数
	ImageWidth = OriginalImg.rows;					//获取图片的宽
	ImageHeight = OriginalImg.cols;                 //获取图像的长
	double length, area, rectArea;					//定义轮廓周长、面积、外界矩形面积
	CvRect rect;									//外界矩形
	CvBox2D box, boxTemp;							//外接矩形
	double axisLongTemp = 0.0, axisShortTemp = 0.0;	//矩形的长边和短边
	float  angle = 0, angleTemp = 0;
	double rectDegree = 0.0;						//矩形度=外界矩形面积/轮廓面积
	double long2Short = 1.0;						//体态比=长边/短边
	double axisLong = 0.0, axisShort = 0.0;			//矩形的长边和短边
	bool   TestPlantFlag = 0;						//车牌是否检测成功标志位

	Mat BinaryImg = OriginalImg.clone();            //克隆原始图像
	int i, j;
	for (i = 0; i < ImageWidth; i++)				//通过颜色分量将图片进行二值化处理
	{
		for (j = 0; j < ImageHeight; j++)
		{
			pixelB = BinaryImg.at<Vec3b>(i, j)[0]; //动态地址访问像素，获取图片各个通道的值
			pixelG = BinaryImg.at<Vec3b>(i, j)[1];
			pixelR = BinaryImg.at<Vec3b>(i, j)[2];

			if ((abs(pixelB - B) < 95 && abs(pixelG - G) < DifMax && abs(pixelR - R) < DifMax) && (float)pixelB / pixelG > 1.1 && (float)pixelB / pixelR > 1.1)
			{	//将各个通道的值和各个通道阈值进行比较
				BinaryImg.at<Vec3b>(i, j)[0] = 255;	//符合颜色阈值范围内的设置成白色
				BinaryImg.at<Vec3b>(i, j)[1] = 255;
				BinaryImg.at<Vec3b>(i, j)[2] = 255;
			}
			else
			{
				BinaryImg.at<Vec3b>(i, j)[0] = 0;	//不符合颜色阈值范围内的设置为黑色
				BinaryImg.at<Vec3b>(i, j)[1] = 0;
				BinaryImg.at<Vec3b>(i, j)[2] = 0;
			}
		}
	}
	//imshow("基于颜色信息二值化", BinaryImg);
	//imwrite("1Color_Binary.jpg", BinaryImg);

	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3)); //设置形态学处理窗的大小

	dilate(BinaryImg, BinaryImg, element);	//进行多次膨胀操作
	dilate(BinaryImg, BinaryImg, element);
	dilate(BinaryImg, BinaryImg, element);
	dilate(BinaryImg, BinaryImg, element);
	dilate(BinaryImg, BinaryImg, element);
	//imshow("多次膨胀后", BinaryImg);
	//imwrite("2pengzhang.jpg", BinaryImg);

	erode(BinaryImg, BinaryImg, element);	//进行多次腐蚀操作
	erode(BinaryImg, BinaryImg, element);
	erode(BinaryImg, BinaryImg, element);
	erode(BinaryImg, BinaryImg, element);
	erode(BinaryImg, BinaryImg, element);
	//imshow("多次腐蚀后", BinaryImg);
	//imwrite("3fushi.jpg", BinaryImg);

	cvtColor(BinaryImg, BinaryImg, CV_BGR2GRAY);	//将形态学处理之后的图像转化为灰度图像
	threshold(BinaryImg, BinaryImg, 100, 255, THRESH_BINARY); //灰度图像二值化

	CvMemStorage* storage = cvCreateMemStorage(0);//用来创建一个内存存储器，来统一管理各种动态对象的内存,需要cvReleaseImage(&storage);释放内存
	CvSeq* seq = 0;     //创建一个序列,CvSeq本身就是一个可以增长的序列，不是固定的序列
	//cvCreateSeq():参数1：该序列的标志？？；参数2：序列头的大小；参数3：与参数1相关；参数4：容器
	CvSeq* tempSeq = cvCreateSeq(CV_SEQ_ELTYPE_POINT, sizeof(CvSeq), sizeof(CvPoint), storage);

	IplImage* BwImg;                //定义IplImage指针
	BwImg = &(IplImage(BinaryImg)); //用MAT强制转换为IplImage指针类型
	int cnt = cvFindContours(BwImg, storage, &seq, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	for (tempSeq = seq; tempSeq != NULL; tempSeq = tempSeq->h_next)
	{
		area = cvContourArea(tempSeq);       //获取轮廓面积，连通域轮廓线所包含的区域的面积
		if (area > 1000 && area < 50000)     //矩形区域面积大小判断
		{
			rect = cvBoundingRect(tempSeq, 1);//update=1: 计算矩形边界，而且将结果写入到轮廓头的 rect 域中 header.
			CvPoint2D32f pt[4];//有两个float x,y;			
			boxTemp = cvMinAreaRect2(tempSeq, 0);  //获取轮廓的矩形,可得四个顶点坐标，车牌倾斜角度
			cvBoxPoints(boxTemp, pt);              //获取矩形四个顶点坐标，存到pt中
			//注意夹角 angleTemp 是水平轴逆时针旋转，与碰到的第一个边（不管是高还是宽）的夹角。
			angleTemp = boxTemp.angle;             //得到车牌倾斜角度
			axisLongTemp = sqrt(pow(pt[1].x - pt[0].x, 2) + pow(pt[1].y - pt[0].y, 2));  //计算长轴（勾股定理）
			axisShortTemp = sqrt(pow(pt[2].x - pt[1].x, 2) + pow(pt[2].y - pt[1].y, 2)); //计算短轴（勾股定理）
			if (axisShortTemp > axisLongTemp)   //短轴大于长轴，交换数据
			{
				double widthtemp = axisLongTemp;
				axisLongTemp = axisShortTemp;
				axisShortTemp = widthtemp;
			}
			else
			{
				angleTemp += 90;
			}
			rectArea = axisLongTemp * axisShortTemp;  //计算矩形的面积
			rectDegree = area / rectArea;     //计算矩形度（比值越接近1说明越接近矩形）

			long2Short = axisLongTemp / axisShortTemp; //计算长宽比
			if (long2Short > 2.2 && long2Short < 3.8 && rectDegree > 0.63 && rectDegree < 1.37 && rectArea > 2000 && rectArea < 50000)
			{
				Mat CutPlant = OriginalImg.clone();
				TestPlantFlag = true;             //检测车牌区域成功
				IplImage* ReImg;                  //定义提取车牌框图像
				ReImg = &(IplImage(CutPlant)); //强制类型转换
				for (int i = 0; i < 4; ++i)       //划线框出车牌区域
				{
					cvLine(ReImg, cvPointFrom32f(pt[i]), cvPointFrom32f(pt[((i + 1) % 4) ? (i + 1) : 0]), CV_RGB(255, 0, 0));
				}
				//cvShowImage("提取车牌结果图", ReImg);    //显示最终结果图
				Mat ReImgMat = cv::cvarrToMat(ReImg);
				imwrite("提取车牌结果图.jpg", ReImgMat);

				CString thePath = CString("提取车牌结果图.jpg");
				ATL::CImage myImage;
				myImage.Load(thePath);
				CRect rect;
				CWnd* pWnd = GetDlgItem(IDC_PICTURE);  //(这是在此资源创建的类的内部, 若是在外部, 可先通过获得CMainFrame的指针, 再通过pMianFrame->GetDlgItem(IDCk_MY_PIC)来获取)
				CDC* pDC = pWnd->GetDC();
				pWnd->GetClientRect(&rect);		pDC->SetStretchBltMode(STRETCH_HALFTONE);
				myImage.Draw(pDC->m_hDC, rect);
				ReleaseDC(pDC);
				myImage.Destroy();


				box = boxTemp;
				angle = angleTemp;
				axisLong = axisLongTemp;
				axisShort = axisShortTemp;
			}
		}
	}
	if (!TestPlantFlag)
	{
		cout << "车牌区域提取失败！！！！" << endl;
		return;
	}

	//图片矫正
	Mat RotractImg(ImageWidth, ImageHeight, CV_8UC1, Scalar(0, 0, 0));//把像素值改为0，倾斜矫正图片
	Point2f center = box.center;     //获取车牌中心坐标
	Mat M2 = getRotationMatrix2D(center, angle, 1);//主要用于获得图像绕着 某一点的旋转矩阵 ，计算旋转加缩放的变换矩阵 
	warpAffine(OriginalImg, RotractImg, M2, OriginalImg.size(), 1, 0, Scalar(0));  //仿射变换--旋转和缩放，进行倾斜矫正
	//imwrite("6baizheng.jpg", RotractImg);
	//imshow("倾斜矫正后图片", RotractImg);

	//获取车牌所在区域
	Rect recttemp(center.x - axisLong / 2, center.y - axisShort / 2, axisLong, axisShort);//定义车牌所在矩形区域
	if (recttemp.x < 0) recttemp.x = 0;
	else if (recttemp.x >= RotractImg.rows) recttemp.x = RotractImg.rows - 1;
	if (recttemp.y < 0) recttemp.y = 0;
	else if (recttemp.y >= RotractImg.cols) recttemp.y = RotractImg.cols - 1;
	Mat PlantRGBImg = RotractImg(recttemp);  //截取车牌彩色照片
	resize(PlantRGBImg, PlantRGBImg, Size(500, 500 * axisShort / axisLong), (0, 0), (0, 0), INTER_AREA);
	//imshow("截取车牌变换尺寸后", PlantRGBImg);
	imwrite("chepai.jpg", PlantRGBImg);

	//二值化车牌
	Mat PlantBinImg;
	cvtColor(PlantRGBImg, PlantBinImg, CV_BGR2GRAY);
	threshold(PlantBinImg, PlantBinImg, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
	imwrite("7erzhihua.jpg", PlantBinImg);
	//imshow("车牌二值图像", PlantBinImg);

	//去除边框
	for (int i = 0; i < PlantBinImg.rows; i++)
		for (int j = 0; j < PlantBinImg.cols; j++)
			if (i < 20 || i > PlantBinImg.rows - 20 || j < 10 || j > PlantBinImg.cols - 10 || (j > 145 && j < 175))
				PlantBinImg.at<uchar>(i, j) = 0;


	Mat PlantBinImgTemp = PlantBinImg.clone();
	element = getStructuringElement(MORPH_RECT, Size(5, 6)); //设置形态学核的大小
	erode(PlantBinImgTemp, PlantBinImgTemp, element);
	//imshow("车牌腐蚀后图像", PlantBinImgTemp);
	//imwrite("8fushi2.jpg", PlantBinImgTemp);
	element = getStructuringElement(MORPH_RECT, Size(10, 3));
	dilate(PlantBinImgTemp, PlantBinImgTemp, element);
	//imwrite("9pengzhang2.jpg", PlantBinImgTemp);
	//imshow("车牌膨胀后图像", PlantBinImgTemp);

	Mat PlantBinImgNot = PlantBinImgTemp < 100;    //字符颜色取反
	//imwrite("10yansequfan2.jpg", PlantBinImgNot);
	//imshow("车牌字符颜色反转", PlantBinImgNot);
	IplImage* BinImg;//定义IplImage指针,推出前需要 释放内存
	BinImg = &(IplImage(PlantBinImgTemp)); //将MAT强制转换为IplImage指针类型
	cnt = cvFindContours(BinImg, storage, &seq, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);//寻找车牌二值图像中的轮廓
	Mat NewCopyImg(PlantBinImgTemp.rows, PlantBinImgTemp.cols, CV_8UC1, CvScalar(255, 255, 255));  //提取出来的字符复制到该图片
	IplImage* NewBinImg;
	for (tempSeq = seq; tempSeq != NULL; tempSeq = tempSeq->h_next)
	{
		rect = cvBoundingRect(tempSeq, 1);   //用矩形框选出字符
		if (rect.width >= 8 && rect.width <= 100 && rect.height >= 8 && rect.height <= 150)
		{
			BinImg = &(IplImage(PlantBinImgNot));		//形态学处理后提取字符的图像
			NewBinImg = &(IplImage(NewCopyImg));        //提取字符后复制生成的新图像
			cvSetImageROI(BinImg, rect);                //提取矩形的字符区域
			cvSetImageROI(NewBinImg, rect);             //提取新图像的字符复制区域
			cvCopy(BinImg, NewBinImg);                  //复制提取出来的字符到新图像指定区域
		}
	}

	Mat CharacterCutImg = NewCopyImg.clone();
	//imshow("提取字符图", CharacterCutImg);
	imwrite("CharacterCutImg.jpg", CharacterCutImg);
	element = getStructuringElement(MORPH_RECT, Size(4, 6)); //设置形态学处理窗的大小
	erode(CharacterCutImg, CharacterCutImg, element);        //对字符图进行水平膨胀处理	
	Mat CharacterCutNot = CharacterCutImg < 100;            //对字符分割图取反
	//imshow("分割准备图", CharacterCutNot);
	imwrite("CharacterCutNot.jpg", CharacterCutNot);

	char* outText;
	api->Init(".\\traineddata", "ChePai");
	// Open input image with leptonica library
	Pix* image = pixRead("CharacterCutImg.jpg");
	api->SetImage(image);
	// Get OCR result
	outText = api->GetUTF8Text();
	printf("output:%s\n", outText);
	api->Clear();

	IplImage* CharacterImg;									//定义IplImage指针
	CharacterImg = &(IplImage(CharacterCutNot));			//将MAT强制转换为IplImage指针类型
	cnt = cvFindContours(CharacterImg, storage, &seq, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);//寻找车牌二值图像中的轮廓
	int RectCnt = 0;   //提取的字符个数
	CvRect CharacterRect[10];
	for (tempSeq = seq; tempSeq != NULL; tempSeq = tempSeq->h_next)
	{
		area = cvContourArea(tempSeq);       //获取轮廓面积
		rect = cvBoundingRect(tempSeq, 1);   //用矩形框选出字符
		CvPoint PotLT, PotHW;            //提取矩形框的左上角坐标，长宽数据
		PotLT.x = rect.x;
		PotLT.y = rect.y;
		PotHW.x = rect.x + rect.width;
		PotHW.y = rect.y + rect.height;

		CharacterRect[RectCnt] = rect;   //记录每个字符矩形
		RectCnt++;       //字符个数统计		
		cvDrawRect(&(IplImage(CharacterCutImg)), PotLT, PotHW, CvScalar(0, 0, 255));//画矩形，框出提取到的字符
	}
	if (RectCnt < 7)
	{
		cout << "字符提取失败！！！！！！" << endl;
		return;
	}


	int p = 0, q = 0;		//将记录的字符矩形进行升序排序
	CvRect Temprect;		//目的是将提取到的字符按照顺序进行保存
	for (p = 0; p < RectCnt; p++)
	{
		for (q = 1; q < RectCnt - p; q++)
		{
			if (CharacterRect[q - 1].x > CharacterRect[q].x)
			{
				Temprect = CharacterRect[q];
				CharacterRect[q] = CharacterRect[q - 1];
				CharacterRect[q - 1] = Temprect;
			}
		}
	}
	//将提取到的字符进行依次保存
	int FirstTemp = 0;
	int n = 0;
	Mat roiImg;
	for (p = RectCnt - 1; p >= RectCnt - 7; p--)
	{
		if (p >= RectCnt - 6)
		{
			if (CharacterRect[p].width < 60 && p >= RectCnt - 6) //如果字符宽度小于60
			{
				CharacterRect[p].x = CharacterRect[p].x - (60 - CharacterRect[p].width) / 2;
				CharacterRect[p].width = 60;  //将宽度拓宽到60
			}
		}
		else if (RectCnt > 7)
		{
			FirstTemp = CharacterRect[RectCnt - 7].x - CharacterRect[0].x + CharacterRect[RectCnt - 7].width;
			CharacterRect[RectCnt - 7].x = CharacterRect[0].x;
			CharacterRect[RectCnt - 7].width = FirstTemp;

			CharacterRect[RectCnt - 7].y = CharacterRect[3].y;
			CharacterRect[RectCnt - 7].height = CharacterRect[3].height;
		}
		char file_img[100];
		if (CharacterRect[p].x + CharacterRect[p].width > PlantBinImg.cols) CharacterRect[p].width = PlantBinImg.cols - CharacterRect[p].x - 1;
		if (CharacterRect[p].y + CharacterRect[p].height > PlantBinImg.rows) CharacterRect[p].height = PlantBinImg.rows - CharacterRect[p].y - 1;
		if (CharacterRect[p].x < 0)
		{
			CharacterRect[p].width += CharacterRect[p].x;
			CharacterRect[p].x = 0;
		}
      	roiImg = ~PlantBinImg(CharacterRect[p]);//依次保存各个字符到目标文件夹
		sprintf_s(file_img, "result\\%d.jpg", n++);
		imwrite(file_img, roiImg);

		if (p == RectCnt - 7) break;
	}

	cvReleaseMemStorage(&storage);//释放内存


	CString cstr;
	string str[7];
	int k = 0;
	Mat testImg;
	string charPath = "result\\";//字符图像路径
	string character_path;

	//Mat NUM[MODEL_NUM];//字符匹配模板	
	//for (int i = 0; i <= MODEL_NUM; i++) {
	//	imgPath = modelPath + to_string(i) + ".jpg";
	//	NUM[i] = imread(imgPath);
	//	if (NUM[i].empty())
	//	{
	//		AfxMessageBox(_T("图像读取失败，请检查图像名、路径！！！！！！"));
	//		return;
	//	}
	//}



	string plate[7];
	for (int j = 6; j >= 0; j--)
	{
		character_path = charPath + to_string(j) + ".jpg";//开始依次读分割后的字符图像
		testImg = imread(character_path);

		float error[MODEL_NUM] = { 0 };
		for (int i = 0; i < 65; i++)
		{
			Mat picture = testImg.clone();
			resize(picture, picture, Size(60, 100), (0, 0), (0, 0), INTER_AREA);
			float error_sum = 0;
			float error_temp = 0;
			for (int o = 0; o < 3; o++)
			{
				resize(NUM[i][o], NUM[i][o], Size(60, 100), (0, 0), (0, 0), INTER_AREA);

				for (int row = 0; row < NUM[i][o].rows; row++)
				{
					for (int col = 0; col < NUM[i][o].cols; col++)
					{
						error_temp = picture.at<uchar>(row, col) - NUM[i][o].at<uchar>(row, col);
						error_sum += abs(error_temp);//关键比较值
					}
				}

				float error_part = error_sum / (picture.rows * picture.cols * 255);
				if (o == 0) error[i] = error_part;
				if (error_part < error[i]) error[i] = error_part;
			}
			//cout<< j <<" " << word[i] << ":" << error[i] << endl;
		}


		int start = 0, end = MODEL_NUM - 1;

		if (j == 6)
		{
			start = 36;
		}
		else if (j == 5)
		{
			start = 10;
			end = 33;
		}
		else
		{
			end = 33;
		}

		float min_error = error[start];
		int Index = start;

		for (int i = start; i <= end; i++)
		{
			if (min_error >= error[i])
			{
				min_error = error[i];
				Index = i;
			}
		}

		plate[j] = word[Index];

	}

	String chePai;
	for (int i = 6; i >= 0; i--)
		chePai += plate[i];
	cout << chePai << endl;

	//显示车牌图片
	CString Path_chepaiPic = _T("chepai.jpg");
	ATL::CImage myImage;
	myImage.Load(Path_chepaiPic);

	// 获取IDC_Chepai_pic控件的DC
	CWnd* pWnd = GetDlgItem(IDC_Chepai_pic);
	CDC* pDC = pWnd->GetDC();

	// 获取控件的客户区域
	
	LPRECT lpRect = reinterpret_cast<LPRECT>(&rect);
	pWnd->GetClientRect(lpRect);
	
	
	// 将图片绘制到控件上
	myImage.Draw(pDC->m_hDC, rect.x, rect.y, rect.width, rect.height, 0, 0, myImage.GetWidth(), myImage.GetHeight());



	//显示车牌识别结果
	CString Path_chepaiResult = _T("CharacterCutImg.jpg");
	ATL::CImage myImage2;
	myImage2.Load(Path_chepaiResult);

	CWnd* pWnd2 = GetDlgItem(IDC_Chepai_result);	// 释放DC
	CDC* pDC2 = pWnd2->GetDC();
	LPRECT lpRect2 = reinterpret_cast<LPRECT>(&rect);
	pWnd2->GetClientRect(lpRect2);
	myImage2.Draw(pDC2->m_hDC, rect.x, rect.y, rect.width, rect.height, 0, 0, myImage.GetWidth(), myImage.GetHeight());


	//释放指针
	pWnd->ReleaseDC(pDC);
	pWnd2->ReleaseDC(pDC2);


	//输出模板识别结果
	
	// 获取 IDC_ModelResult 控件的指针
	CListBox* pListBox = static_cast<CListBox*>(GetDlgItem(IDC_ModelResult));

	// 清空 List Box
	pListBox->ResetContent();
	//输出结果
	CString cstr_chePai(chePai.c_str());//类型转换
	pListBox->AddString(cstr_chePai);
	
	//输出文字识别结果
	CListBox* pListBox2 = static_cast<CListBox*>(GetDlgItem(IDC_CharacterResult));
	// 清空 List Box
	pListBox2->ResetContent();
	CString cstr_wenZi(outText);//类型转换
	pListBox2->AddString(cstr_wenZi);
}

// 将 OpenCV 的 Mat 转换为 CImage
CImage Mat2CImage(const cv::Mat& mat) {
	// 检查输入矩阵是否为空
	if (mat.empty()) {
		AfxMessageBox(_T("输入图像为空！"));
		return CImage();  // 返回一个空的CImage对象
	}

	// 确保输入矩阵是 3 通道的 BGR 图像
	if (mat.channels() != 3) {
		AfxMessageBox(_T("输入的需要是3通道图像"));
		return CImage();
	}

	// 创建 CImage 对象
	CImage cimage;
	cimage.Create(mat.cols, mat.rows, 24);  // 24表示RGB图像，8位三通道

	// 复制图像数据
	uchar* destBits = (uchar*)cimage.GetBits();
	int destPitch = cimage.GetPitch();
	uchar* srcBits = mat.data;
	int srcPitch = mat.step;

	for (int i = 0; i < mat.rows; ++i) {
		memcpy(destBits, srcBits, mat.cols * 3);  // 3表示每个像素有3个通道
		destBits += destPitch;
		srcBits += srcPitch;
	}

	return cimage;
}

//VideoCapture capture(0);
void CDCLicensePlateRecognitionSysDlg::OnBnClickedOpencam()
{
	//// TODO: 在此添加控件通知处理程序代码
	//capture.open(0); //打开摄像头
	//// 设置计时器,每3ms触发一次事件
	//SetTimer(1, 3, NULL);
}

void CDCLicensePlateRecognitionSysDlg::OnTimer(UINT_PTR nIDEvent)
{
	/*CRect rect;
	CDC *pDC;
	HDC hDC;
	CWnd *pwnd;

	pwnd = GetDlgItem(IDC_PICTURE);
	pDC = pwnd->GetDC();
	hDC = pDC->GetSafeHdc();
	pwnd->GetClientRect(&rect);

	static int i;
	static Mat Fream;
	capture >> Fream;
	IplImage* m_Frame;
	m_Frame = &(IplImage(Fream));
	CvvImage m_CvvImage;
	m_CvvImage.CopyOf(m_Frame, 1);
	if (true)
	{
		m_CvvImage.DrawToHDC(hDC, &rect);
	}*/

	CDialogEx::OnTimer(nIDEvent);
}




void CDCLicensePlateRecognitionSysDlg::OnBnClickedCancel()
{
	FreeConsole();//释放控制台资源
	CDialogEx::OnCancel();
}


void CDCLicensePlateRecognitionSysDlg::OnStnClickedCarnum()    
{
	// TODO: 在此添加控件通知处理程序代码
}


void onTrackbarSlide(int pos, void*)//滑块的回调函数
{
	cap.set(CAP_PROP_POS_FRAMES, pos); //设置当前帧
}
void onHueEnhanceTrackbarSlide(int pos, void*) {
	// 在这里添加滑块值改变时的逻辑
	hueEnhanceFactor = pos;  // 这里的处理可能需要根据具体情况调整
}


void pauseTrackbarSlide(int pos, void*) //暂停回调函数
{
	if (pos == 1)
	{
		isStop = true;
	}
	else if (pos == 0)
	{
		isStop = false;
	}
}



void CDCLicensePlateRecognitionSysDlg::OnBnClickedVideoplayer()
{
	// TODO: 在此添加控件通知处理程序代码
	namedWindow("视频播放器", WINDOW_NORMAL);

	cap.open(m_strVideoPath);//视频读取结构
	if (!cap.isOpened())//如果视频不能正常打开则返回
	{
		AfxMessageBox(_T("无法打开视频文件"));
	}

	//获取总帧数
	int framenum = (int)cap.get(CAP_PROP_FRAME_COUNT);

	createTrackbar("暂停", "视频播放器", 0, 1, pauseTrackbarSlide);
	createTrackbar("进度：", "视频播放器", &g_slider_pos, framenum, onTrackbarSlide);

	Mat src;
	Mat dst;
	while (1)
	{
		while (!isStop) //播放
		{
			double alpha = ContrastValue;//控制对比度 预设值为 1 即为原图像
			double beta = BrightValue;//控制亮度 预设值0 ，即为原视频亮度
			dst = Mat::zeros(src.size(), src.type());
			cap >> src;
			if (src.empty()) break;
			//设置进度条自动滚动
			int current_pos = (int)cap.get(CAP_PROP_POS_FRAMES);
			setTrackbarPos("进度：", "视频播放器", current_pos); //设置进度条位置
			setTrackbarPos("暂停", "视频播放器", isStop); //设置进度条位置
			//opencv修改亮度和对比度
			src.convertTo(dst, src.type(), alpha, beta);
			if (current_pos % 20 == 0) {
				OriginalImg = dst.clone();
				process();
			}
			imshow("视频播放器", dst);
			switch (waitKey(33))
			{
			case ' ':isStop = 1 - isStop;
				break;
			default:break;
			}
		}
		switch (waitKey(33))
		{
			case ' ':isStop = 1 - isStop;
					break;
			default:break;
		}
		setTrackbarPos("暂停", "视频播放器", isStop); //设置进度条位置
	}
}


void CDCLicensePlateRecognitionSysDlg::OnBnClickedLoadVideo()//读取视频路径
{
	// TODO: 在此添加控件通知处理程序代码
	CString videoPath;
	CString filter;
	filter = _T("视频文件(*.mp4,*.avi,*.mov)|*.mp4;*.avi;*.mov||");

	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, filter, NULL);
	if (dlg.DoModal() == IDOK)
	{
		// 获取视频文件路径
		videoPath = dlg.GetPathName();

		// 将路径存储在变量中，例如类的成员变量 m_strVideoPath
		m_strVideoPath = CT2A(videoPath); // CString 转换为 std::string
		std::replace(m_strVideoPath.begin(), m_strVideoPath.end(), '\\', '/');
		
	}

	for (int i = 0; i < 65; i++) {
		for (int j = 0; j < 3; j++) {
			String imgPath = modelPath + word[i] + "\\" + word[i] + "-" + word[j] + ".jpg";
			cout << imgPath << endl;
			NUM[i][j] = imread(imgPath);
			if (NUM[i][j].empty())
			{
				AfxMessageBox(_T("图像读取失败，请检查图像名、路径！！！！！！"));
				return;
			}
		}
	}
}
