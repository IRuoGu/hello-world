#include "stdafx.h"
#include "LPR.h"

bool comp(RotatedRect a, RotatedRect b)
{
	return a.center.x < b.center.x;
}

float PR(Mat &src, Mat &dst, int num)
{
	int temp = 0;
	float result = 0;
// 	imshow("src", src);
// 	imshow("dst", dst);
// 	waitKey(0);

	for (int i = 0; i < src.rows; i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			if (src.ptr(i)[j] == dst.ptr(i)[j])
			{
				temp++;
			}
		}
	}
	result = (float)temp / (src.rows*src.cols);
	return result;
}

void Output(int num)
{
	if (num < 10)
	{
		cout << num << " ";
	}
	else
	{
		if (num == 10)
		{
			cout << "D" << " ";
		}
		else if (num == 11)
		{
			cout << "��" << " ";
		}
		else if (num == 12)
		{
			cout << "H" << " ";
		}
		else
		{
			cout << "*" << " ";
		}
	}
}
LPR::LPR()
{

}

LPR::LPR(String path)
{
	this->load(path);
}

void LPR::load(String path)
{
	//��ȡ����ͼƬ
	srcImage = imread(path);
	if (srcImage.empty())
	{
		cout << "�����·��!" << endl;
/*		exit(-1);*/
	}
	cvtColor(srcImage, markedImage, CV_16S);
	//��������ͼ��
	for (int i = 0; i < template_len; i++)
	{
		stringstream stream;
		stream << "pictures/num_";
		stream << i;
		stream << ".bmp";
		String name = stream.str();
		NUM[i] = imread(name);
		if (NUM[i].empty())
		{
			cout << "δ�ܶ�ȡ" << name << endl;
		}
		threshold(NUM[i], NUM[i], 0, 255, CV_THRESH_BINARY);
	}
}

void LPR::showSrc()
{
	//��ʾԭͼ
	imshow("ԭͼ",srcImage);
}

void LPR::gaussFilter()
{
	//��˹�˲�
	GaussianBlur(srcImage, gaussImage, Size(3, 3), 0);
/*	imshow("��Ч��ͼ����˹�˲�", gaussImage);*/
}

void LPR::sobel()
{
	//sobel����
	//���� grad_x �� grad_y ����  
	Mat grad_x, grad_y;
	Mat abs_grad_x, abs_grad_y;

	//�� X�����ݶ�  
	Sobel(gaussImage, grad_x, CV_16S, 1, 0, 3, 1, 1, BORDER_DEFAULT);
	convertScaleAbs(grad_x, abs_grad_x);
/*	imshow("��Ч��ͼ�� X����Sobel", abs_grad_x);*/

	//��Y�����ݶ�  
	Sobel(gaussImage, grad_y, CV_16S, 0, 1, 3, 1, 1, BORDER_DEFAULT);
	convertScaleAbs(grad_y, abs_grad_y);
/*	imshow("��Ч��ͼ��Y����Sobel", abs_grad_y);*/

	//�ϲ��ݶ�(����)  
	addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, sobelImage);
/*	imshow("��Ч��ͼ�����巽��Sobel", sobelImage);*/
}

void LPR::thresholding()
{
	//��ɫ�ռ�ת������
	cvtColor(sobelImage, binImage, CV_RGB2GRAY);

	//threshold ������ͨ�������Ҷ�ͼ�е㣬��ͼ����Ϣ��ֵ������������ͼƬֻ�ж���ɫֵ��
	threshold(binImage, binImage, 180, 255, CV_THRESH_BINARY);
/*	imshow("��Ч��ͼ����ֵ������", binImage);*/
}

void LPR::closeOperation()
{
	//�����
	Mat element = getStructuringElement(MORPH_RECT, Size(17, 17));
	//������̬ѧ����
	morphologyEx(binImage, closeImage, MORPH_CLOSE, element);//����С�ڵ� 
/*	imshow("��Ч��ͼ��������", closeImage);*/
}

void LPR::removeLittle()
{
	//��ʴ����
	Mat ele = getStructuringElement(MORPH_RECT, Size(5, 5));//getStructuringElement����ֵ�����ں˾���
	erode(closeImage, closeImage, ele);//erode����ֱ�ӽ��и�ʴ����
/*	imshow("��Ч��ͼ��ȥ��С����", closeImage);*/

	//���Ͳ���
	ele = getStructuringElement(MORPH_RECT, Size(17, 5));//getStructuringElement����ֵ�����ں˾���
	dilate(closeImage, closeImage, ele);//dilate����ֱ�ӽ������Ͳ���
/*	imshow("��Ч��ͼ��������", closeImage);*/
}

void LPR::getMaxArea()
{
	//ȡ����
	vector< vector< Point> > contours;
	findContours(closeImage,
		contours, // a vector of contours
		CV_RETR_EXTERNAL, // ��ȡ�ⲿ����
		CV_CHAIN_APPROX_NONE); // all pixels of each contours
	int max = 0;
	for (size_t i = 0; i < contours.size(); i++)
	{
		vector<Point> p;
		p = contours[i];
		if (p.size() > max)
		{
			max = p.size();
			maxArea = p;
		}
	}
	//�����������
	for (size_t i = 0; i < maxArea.size(); i++)
	{
		circle(markedImage, maxArea[i], 1, Scalar(240, 255, 25));
	}

	//������С��Ӿ���
	minRect = minAreaRect(maxArea);
	minRect.points(P);//P-���εĶ��� 
	for (int j = 0; j <= 3; j++)
	{
		line(markedImage, P[j], P[(j + 1) % 4], Scalar(255), 2);
	}
/*	imshow("��Ч��ͼ�����", markedImage);*/
}

void LPR::affine()
{
	//����任
	Point2f srcTri[3];
	Point2f dstTri[3];
	Mat rot_mat(2, 3, CV_32FC1);
	Mat warp_mat(2, 3, CV_32FC1);

	//�������������������任
	//����
	int tep = markedImage.cols;
	for (size_t i = 0; i < maxArea.size(); i++)
	{
		if (maxArea[i].x < tep)
		{
			tep = maxArea[i].x;
			srcTri[0] = maxArea[i];
		}
	}
	//����
	tep = 0;
	for (size_t i = 0; i < maxArea.size(); i++)
	{
		if (maxArea[i].y > tep)
		{
			tep = maxArea[i].y;
			srcTri[1] = maxArea[i];
		}
	}
	//����
	tep = 0;
	for (size_t i = 0; i < maxArea.size(); i++)
	{
		if (maxArea[i].x > tep)
		{
			tep = maxArea[i].x;
			srcTri[2] = maxArea[i];
		}
	}

	//Scalar(blue,green,red)
	circle(markedImage, srcTri[0], 5, Scalar(255));
	circle(markedImage, srcTri[1], 5, Scalar(0, 255));
	circle(markedImage, srcTri[2], 5, Scalar(0, 0, 255));

/*	imshow("����ǡ���������", markedImage);*/


	dstTri[0] = Point2f(0, 0);
	dstTri[1] = Point2f(0, minRect.size.height);
	dstTri[2] = Point2f(minRect.size.width, minRect.size.height);

	//�������任����
	warp_mat = getAffineTransform(srcTri, dstTri);
	//�Լ���ͼ�ν��з���任����
	warpAffine(gaussImage, plateImage, warp_mat, minRect.size);
/*	imshow("��Ч��ͼ������任", plateImage);*/

}

void LPR::reThreshold()
{
	//�ٴζ�ֵ��
	threshold(plateImage, reBinImage, 180, 255, CV_THRESH_BINARY_INV);
/*	imshow("�ٴζ�ֵ��", reBinImage);*/
}

void LPR::recognition()
{

	cvtColor(reBinImage, reBinImage, COLOR_BGR2GRAY);
	cvtColor(plateImage, charMarkedImage, CV_16S);

	threshold(reBinImage, reBinImage, 48, 255, CV_THRESH_BINARY_INV);

	//�������
	Mat ele = getStructuringElement(MORPH_RECT, Size(3, 5));//getStructuringElement����ֵ�����ں˾���
	dilate(reBinImage, reBinImage, ele);
/*	imshow("����", reBinImage);*/

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(reBinImage, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

	vector<RotatedRect> RectArr(license_len);
	int a = 0;
	//�����ɰ�Χ���ֵ���С����
	for (vector<vector<Point>>::iterator It = contours.begin(); It < contours.end(); It++)
	{                        
		Point2f vertex[4];
		RotatedRect rect = minAreaRect(*It);
		if (rect.size.area() > 200)
		{
			RectArr[a] = rect;
			RectArr[a].points(vertex);
			a++;
			for (int j = 0; j < 4; j++)
				line(charMarkedImage, vertex[j], vertex[(j + 1) % 4], Scalar(0, 0, 255), 1);
		}
	}

	imshow("����", charMarkedImage);

	Mat warp_mat(2, 3, CV_32FC1);
	sort(RectArr.begin(), RectArr.end(), comp);//����������������

	cout << "���ƣ�";
	for (int i = 0; i < RectArr.size(); i++)
	{
		Point2f vertex[4];
		RectArr[i].points(vertex);
		//����任
		Point2f srcTri[3];
		Point2f dstTri[3];
		srcTri[0] = vertex[0];
		srcTri[1] = vertex[1];
		srcTri[2] = vertex[2];
		dstTri[0] = Point2f(0, 400);
		dstTri[1] = Point2f(0, 0);
		dstTri[2] = Point2f(400, 0);
		Mat warp_mat = getAffineTransform(srcTri, dstTri);
		//�Լ���ͼ�ν��з���任����
		warpAffine(plateImage, charMat[i], warp_mat, Size(400, 400));

// 		stringstream stream;//		�ַ���
// 		stream << i + 1;
// 		String name = stream.str();

		//��ֵ��
		threshold(charMat[i], charMat[i], 150, 255, CV_THRESH_BINARY_INV);

		float maxRate = 0, tempRate = 0;
		int mostLike;
		for (int j = 0; j < template_len; j++)
		{
			tempRate = PR(charMat[i], NUM[j], j);
/*			cout << setprecision(2) << tempRate<<" ";//���ƥ����*/
			if (tempRate > maxRate)
			{
				maxRate = tempRate;
				mostLike = j;
			}
		}
		Output(mostLike);

/*		imwrite(name+".bmp", charMat[i]);//������ȡ���ַ�*/
	}
	cout << endl;
}

void LPR::processing()
{
	//��˹�˲�
	gaussFilter();
	//sobel����
	sobel();
	//��ֵ��
	thresholding();
	//������
	closeOperation();
	//ȥ��С����
	removeLittle();
	//ȡ�������
	getMaxArea();
	//����任
	affine();
	//�ٴζ�ֵ��
	reThreshold();
	//ʶ���ַ�
	recognition();
}


LPR::~LPR()
{
	destroyAllWindows();
}

