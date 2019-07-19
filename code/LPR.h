#pragma once

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>

#define template_len 13
#define license_len 7

using namespace cv;
using namespace std;

class LPR
{
public:
	LPR();
	LPR(String path);
	void load(String path);//����ͼ��
	void showSrc();//��ʾԭͼ
	void gaussFilter();//��˹�˲�
	void sobel();//sobel����
	void thresholding();//��ֵ��
	void closeOperation();//������
	void removeLittle();//��̬����
	void getMaxArea();//���������
	void affine();//����任
	void reThreshold();//�ٴζ�ֵ��
	void recognition();//Ѱ���ַ�
	void processing();//����
	~LPR();

private:
	Mat srcImage;//ԭͼ
	Mat gaussImage;//��˹�˲����ͼ��
	Mat sobelImage;//sobel������ͼ��
	Mat binImage;//��ֵ��ͼ��
	Mat reBinImage;//�ٴζ�ֵ����ͼ��
	Mat closeImage;//�������ͼ��
	Mat markedImage;//���
	Mat plateImage;//��������
	Mat charMarkedImage;//�ַ����
	String license;//����
	vector<Point> maxArea;//�������
	RotatedRect minRect;//��С��Ӿ���
	Point2f P[4];//���εĶ���
	Mat NUM[template_len];//�ַ�ƥ��ģ��
	Mat charMat[license_len];//��ȡ���ַ�
};
