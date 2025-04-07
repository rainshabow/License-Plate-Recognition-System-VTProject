#include "stdio.h"
#include <iostream> 
#include<opencv2/opencv.hpp>  
#include<iostream>
#include<cmath> 
using namespace cv;
using namespace std;

int g_slider_pos = 0; //λ��
bool isStop = false; //�Ƿ���ͣ
bool isHist = false; //�Ƿ�ֱ��ͼ����

int medianFilter = 0;
bool isGauss = false;
int ContrastValue = 1;
int BrightValue = 0;
int hueEnhanceFactor = 1;  // ��ʼɫ����ǿ����
VideoCapture cap;
void onTrackbarSlide(int pos, void*)//����Ļص�����
{
	cap.set(CAP_PROP_POS_FRAMES, pos); //���õ�ǰ֡
}

void onHueEnhanceTrackbarSlide(int pos, void*) {
	// ��������ӻ���ֵ�ı�ʱ���߼�
	hueEnhanceFactor = pos;  // ����Ĵ��������Ҫ���ݾ����������
}

void pauseTrackbarSlide(int pos, void*) //��ͣ�ص�����
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
	namedWindow("��Ƶ������", WINDOW_NORMAL);

	cap.open("C:/Users/71092/Desktop/����ʶ����Ƶ�ز� - 1.����ʶ����Ƶ�ز�(Av528769674,P1).mp4");//��Ƶ��ȡ�ṹ
	if (!cap.isOpened())//�����Ƶ�����������򷵻�
	{
		cout << "cannot open video!" << endl;
		return 0;
	}

	//��ȡ��֡��
	int framenum = (int)cap.get(CAP_PROP_FRAME_COUNT);

	createTrackbar("��ͣ", "��Ƶ������", 0, 1, pauseTrackbarSlide);
	createTrackbar("���ȣ�", "��Ƶ������", &g_slider_pos, framenum, onTrackbarSlide);

	Mat src;
	Mat dst;
	while (1)
	{
		while (!isStop) //����
		{
			double alpha = ContrastValue;//���ƶԱȶ� Ԥ��ֵΪ 1 ��Ϊԭͼ��
			double beta = BrightValue;//�������� Ԥ��ֵ0 ����Ϊԭ��Ƶ����
			dst = Mat::zeros(src.size(), src.type());
			cap >> src;
			if (src.empty()) break;
			//���ý������Զ�����
			int current_pos = (int)cap.get(CAP_PROP_POS_FRAMES);
			setTrackbarPos("���ȣ�", "��Ƶ������", current_pos); //���ý�����λ��
			
			imshow("��Ƶ������", dst);
			waitKey(33);
		}
		while (isStop) //��ͣ
		{
			waitKey(0);
			break;
		}
	}
}