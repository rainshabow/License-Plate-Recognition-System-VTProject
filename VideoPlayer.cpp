#include "stdio.h"
#include <iostream> 
#include<opencv2/opencv.hpp>  
#include<iostream>
#include<cmath> 
using namespace cv;
using namespace std;

int g_slider_pos = 0; //位置
bool isStop = false; //是否暂停
bool isHist = false; //是否直方图均衡

int medianFilter = 0;
bool isGauss = false;
int ContrastValue = 1;
int BrightValue = 0;
int hueEnhanceFactor = 1;  // 初始色度增强因子
VideoCapture cap;
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


int main()
{
	namedWindow("视频播放器", WINDOW_NORMAL);

	cap.open("C:/Users/71092/Desktop/车牌识别视频素材 - 1.车牌识别视频素材(Av528769674,P1).mp4");//视频读取结构
	if (!cap.isOpened())//如果视频不能正常打开则返回
	{
		cout << "cannot open video!" << endl;
		return 0;
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
			
			imshow("视频播放器", dst);
			waitKey(33);
		}
		while (isStop) //暂停
		{
			waitKey(0);
			break;
		}
	}
}