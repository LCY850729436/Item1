#include<iostream>
#include<opencv2/opencv.hpp>
#include<opencv2\imgproc\types_c.h>
#include<opencv2/imgcodecs/legacy/constants_c.h>
#include<string>
#include<sstream>
#include<fstream>

using namespace std;
using namespace cv;

RNG rng(12345);
//Scalar colorful = CV_RGB(rng.uniform(0,255),rng.uniform(0,255),rng.uniform(0,255));

// ��ֵ����ֵ
#define GRAY_THRESH 150

// ֱ���ϵ�ĸ���
#define HOUGH_VOTE 50


//���ַ��ĸ����ַ�ת��Ϊ8λ����������
void StrToByte(int a[8],char ch)
{
	int n, i, j = 0;
	n = (int)ch;
	i = n;
	while (i)
	{
		a[j] = i % 2;
		i /= 2;
		j++;

	}
	for (;j < 8;j++)if (a[j] == -1)a[j] = 0;
}

//���ı���������ַ�ת��Ϊ8λ����������
void TxtToByte(string str, vector<int>&BinaryByte)
{
	for (int i = 0;i < str.size();i++)
	{
		int a[8] = { -1 };
		StrToByte(a, str[i]);
		for (int j = 7;j >= 0;j--)BinaryByte.push_back(a[j]);
	}
}


//���ƶ�ά�������(����Ϊ�������ص�16��)
void WritePixel(int rows, int cols, Mat image,int color)
{
	for (int i = rows;i < rows + 16;i++)
		for (int j = cols;j < cols + 16;j++)
			image.at<uchar>(i, j) = color;
}

//���ơ��ء���
void WritePoint(int x, int y, Mat image)
{
	for (int i = x;i < x + 112;i++)
		for (int j = y;j < y + 16;j++)
			image.at<uchar>(j, i) = 0;
	for (int i = x;i < x + 112;i++)
		for (int j = y + 96;j < y + 112;j++)
			image.at<uchar>(j, i) = 0;
	for (int i = x;i < x + 112;i++)
		for (int j = y + 16;j < y + 32;j++)
		{
			if (i >= x + 16 && i < x + 96)image.at<uchar>(j, i) = 255;
			else image.at<uchar>(j, i) = 0;
		}
	for (int i = x;i < x + 112;i++)
		for (int j = y + 80;j < y + 96;j++)
		{
			if (i >= x + 16 && i < x + 96)image.at<uchar>(j, i) = 255;
			else image.at<uchar>(j, i) = 0;
		}
	for (int i = x;i < x + 112;i++)
		for (int j = y + 32;j < y + 80;j++)
		{
			if (i >= x + 16 && i < x + 32 || i >= x + 80 && i < x + 96)image.at<uchar>(j, i) = 255;
			else image.at<uchar>(j, i) = 0;
		}
}

//���ƶ�ά����������ء��ֶ���
void WritePto(Mat image)
{
	WritePoint(196, 146, image);//����
	WritePoint(692, 146, image);//����
	WritePoint(196, 642, image);//����
}


//���ƶ�ά��,0����Ϊ��ɫ��1����Ϊ��ɫ
void DrawQRcode()
{
	string str;
	vector<int>BinaryByte;
	cout << "������Ҫ������ַ���";
	cin >> str;
	TxtToByte(str, BinaryByte);
	int size = BinaryByte.size();//��ֹ�������������������ݵ�ɫ��
	int num = 0;//����Զ��ٶ��������ݱ���
	int count = 0;//���ڼ��������ʽ���
	int imageNum = 1;//���ڼ��������ͼƬ��
	int x, y;//�����ά���������ʱ������
	while (size != 0)
	{
		//����900*1000���صĴ��׵�ɫͼ���ڻ��ƶ�ά��
		Mat image(900, 1000, CV_8UC1, 255);

		//����������λ��
		WritePto(image);

		//���б��뿪ʼ���ƶ�ά��
		for (int i = 146;i < 274 && num < size;i = i + 16)//��
			for (int j = 324;j < 668 && num < size;j = j + 16)//��
			{
				if (BinaryByte[count] == 0)WritePixel(i, j, image, 0);//��
				else WritePixel(i, j, image, 255);;//��
				count++;
				num++;
				x = i;
				y = j;
			}
		size = size - num;
		num = 0;
		for (int i = 274;i < 626 && num < size;i = i + 16)//��
			for (int j = 196;j < 804 && num < size;j = j + 16)//��
			{
				if (BinaryByte[count] == 0)WritePixel(i, j, image, 0);//��
				else WritePixel(i, j, image, 255);;//��
				count++;
				num++;
				x = i;
				y = j;
			}
		size = size - num;
		num = 0;
		for (int i = 626;i < 754 && num < size;i = i + 16)//��
			for (int j = 324;j < 804 && num < size;j = j + 16)//��
			{
				if (BinaryByte[count] == 0)WritePixel(i, j, image, 0);//��
				else WritePixel(i, j, image, 255);;//��
				count++;
				num++;
				x = i;
				y = j;
			}
		size = size - num;
		num = 0;
		//��intת��Ϊ�ַ��������ֱ���Ϊ�����ͼƬ����
		string res;
		stringstream ss;          
		ss << imageNum; 
		ss >> res;
		string imageName = res + ".jpg";
		//��������ȫ�����ƽ���ά�������ά��û������������������ɫ�������ڶ�λʶ��
		if (size == 0)
		{
			y = y + 16;
			//��û������һ������
			if (x < 274)for (int i = x, j = y;j < 668;j = j + 16)WritePixel(i, j, image, 85);
			else for (int i = x, j = y;j < 804;j = j + 16)WritePixel(i, j, image, 85);
			x = x + 16;
			//����ʣ�µ�����
			for (x; x < 274; x = x + 16)//��
				for (int j = 324;j < 668;j = j + 16)//��
					WritePixel(x, j, image, 85);//��
			for (x; x < 626; x = x + 16)//��
				for (int j = 196; j < 804; j = j + 16)//��
					WritePixel(x, j, image, 85);//��
			for (x; x < 754; x = x + 16)//��
				for (int j = 324; j < 804; j = j + 16)//��
					WritePixel(x, j, image, 85);//��
		}
		imwrite(imageName, image);
		imageNum++;
	}

}


//ͼ�����
Mat Image_Correct()
{
	//Read a single-channel image
	const char* filename = "4.jpg";
	Mat srcImg = imread(filename, CV_LOAD_IMAGE_GRAYSCALE);

	Point center(srcImg.cols / 2, srcImg.rows / 2);

	//Expand image to an optimal size, for faster processing speed
	//Set widths of borders in four directions
	//If borderType==BORDER_CONSTANT, fill the borders with (0,0,0)
	Mat padded;
	int opWidth = getOptimalDFTSize(srcImg.rows);
	int opHeight = getOptimalDFTSize(srcImg.cols);
	copyMakeBorder(srcImg, padded, 0, opWidth - srcImg.rows, 0, opHeight - srcImg.cols, BORDER_CONSTANT, Scalar::all(0));

	Mat planes[] = { Mat_<float>(padded), Mat::zeros(padded.size(), CV_32F) };
	Mat comImg;
	//Merge into a double-channel image
	merge(planes, 2, comImg);

	//Use the same image as input and output,
	//so that the results can fit in Mat well
	dft(comImg, comImg);

	//Compute the magnitude
	//planes[0]=Re(DFT(I)), planes[1]=Im(DFT(I))
	//magnitude=sqrt(Re^2+Im^2)
	split(comImg, planes);
	magnitude(planes[0], planes[1], planes[0]);

	//Switch to logarithmic scale, for better visual results
	//M2=log(1+M1)
	Mat magMat = planes[0];
	magMat += Scalar::all(1);
	log(magMat, magMat);

	//Crop the spectrum
	//Width and height of magMat should be even, so that they can be divided by 2
	//-2 is 11111110 in binary system, operator & make sure width and height are always even
	magMat = magMat(Rect(0, 0, magMat.cols & -2, magMat.rows & -2));

	//Rearrange the quadrants of Fourier image,
	//so that the origin is at the center of image,
	//and move the high frequency to the corners
	int cx = magMat.cols / 2;
	int cy = magMat.rows / 2;

	Mat q0(magMat, Rect(0, 0, cx, cy));
	Mat q1(magMat, Rect(0, cy, cx, cy));
	Mat q2(magMat, Rect(cx, cy, cx, cy));
	Mat q3(magMat, Rect(cx, 0, cx, cy));

	Mat tmp;
	q0.copyTo(tmp);
	q2.copyTo(q0);
	tmp.copyTo(q2);

	q1.copyTo(tmp);
	q3.copyTo(q1);
	tmp.copyTo(q3);

	//Normalize the magnitude to [0,1], then to[0,255]
	normalize(magMat, magMat, 0, 1, CV_MINMAX);
	Mat magImg(magMat.size(), CV_8UC1);
	magMat.convertTo(magImg, CV_8UC1, 255, 0);
	//imwrite("imageText_mag.jpg",magImg);

	//Turn into binary image
	threshold(magImg, magImg, GRAY_THRESH, 255, CV_THRESH_BINARY);
	//imwrite("imageText_bin.jpg",magImg);

	//Find lines with Hough Transformation
	vector<Vec2f> lines;
	float pi180 = (float)CV_PI / 180;
	Mat linImg(magImg.size(), CV_8UC3);
	HoughLines(magImg, lines, 1, pi180, HOUGH_VOTE, 0, 0);
	int numLines = lines.size();
	for (int l = 0; l<numLines; l++)
	{
		float rho = lines[l][0], theta = lines[l][1];
		Point pt1, pt2;
		double a = cos(theta), b = sin(theta);
		double x0 = a * rho, y0 = b * rho;
		pt1.x = cvRound(x0 + 1000 * (-b));
		pt1.y = cvRound(y0 + 1000 * (a));
		pt2.x = cvRound(x0 - 1000 * (-b));
		pt2.y = cvRound(y0 - 1000 * (a));
		line(linImg, pt1, pt2, Scalar(255, 0, 0), 3, 8, 0);
	}
	//imwrite("imageText_line.jpg",linImg);
	if (lines.size() == 3) {
		cout << "found three angels:" << endl;
		cout << lines[0][1] * 180 / CV_PI << endl << lines[1][1] * 180 / CV_PI << endl << lines[2][1] * 180 / CV_PI << endl << endl;
	}

	//Find the proper angel from the three found angels
	float angel = 0;
	float piThresh = (float)CV_PI / 90;
	float pi2 = CV_PI / 2;
	for (int l = 0; l<numLines; l++)
	{
		float theta = lines[l][1];
		if (abs(theta) < piThresh || abs(theta - pi2) < piThresh)
			continue;
		else {
			angel = theta;
			break;
		}
	}

	//Calculate the rotation angel
	//The image has to be square,
	//so that the rotation angel can be calculate right
	angel = angel<pi2 ? angel : angel - CV_PI;
	if (angel != pi2) {
		float angelT = srcImg.rows*tan(angel) / srcImg.cols;
		angel = atan(angelT);
	}
	float angelD = angel * 180 / (float)CV_PI;
	cout << "the rotation angel to be applied:" << endl << angelD << endl << endl;

	//Rotate the image to recover
	Mat rotMat = getRotationMatrix2D(center, angelD, 1.0);
	Mat dstImg = Mat::ones(srcImg.size(), CV_8UC3);
	warpAffine(srcImg, dstImg, rotMat, srcImg.size(), 1, 0, Scalar(255, 255, 255));
	imwrite("����.jpg", dstImg);
	//imwrite("imageText_D.jpg",dstImg);
	waitKey(0);

	return dstImg;
}


//�ҵ�����ȡ���������ĵ�
Point Center_cal(vector<vector<Point> > contours, int i)
{
	int centerx = 0, centery = 0, n = contours[i].size();
	//����ȡ��С�����εı߽���ÿ���ܳ���������ȡһ��������꣬������ȡ�ĸ����ƽ�����꣨��ΪС�����εĴ������ģ�
	centerx = (contours[i][n / 4].x + contours[i][n * 2 / 4].x + contours[i][3 * n / 4].x + contours[i][n - 1].x) / 4;
	centery = (contours[i][n / 4].y + contours[i][n * 2 / 4].y + contours[i][3 * n / 4].y + contours[i][n - 1].y) / 4;
	Point point1 = Point(centerx, centery);
	return point1;
}


//��λ��ά�룬�����ά����ĸ����������
void LocateQRcode(Point2f QRPoints[4])
{
	Mat src; Mat src_gray;

	src = imread("5.jpg", 1);
	Mat src_all = src.clone();

	cvtColor(src, src_gray, CV_BGR2GRAY);
	//  src_gray = Scalar::all(255) - src_gray;
	blur(src_gray, src_gray, Size(3, 3));
	equalizeHist(src_gray, src_gray);

	Scalar color = Scalar(1, 1, 255);
	Mat threshold_output;
	vector<vector<Point> > contours, contours2;
	vector<Vec4i> hierarchy;
	Mat drawing = Mat::zeros(src.size(), CV_8UC3);
	Mat drawing2 = Mat::zeros(src.size(), CV_8UC3);
	threshold(src_gray, threshold_output, 112, 255, THRESH_BINARY);
	//Canny(src_gray,threshold_output,136,196,3);
	//imshow("Ԥ�����",threshold_output);
	//Ѱ������ 
	//��һ������������ͼ�� 2ֵ����
	//�ڶ����������ڴ�洢����FindContours�ҵ��������ŵ��ڴ����档
	//�����������ǲ㼶��**[Next, Previous, First_Child, Parent]** ��vector
	//���ĸ����������ͣ��������ṹ
	//����������ǽڵ����ģʽ��������ȫ��Ѱ��
	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));
	//CHAIN_APPROX_NONEȫ��,CV_CHAIN_APPROX_SIMPLE,,,RETR_TREE    RETR_EXTERNAL    RETR_LIST   RETR_CCOMP

	int c = 0, ic = 0, k = 0, area = 0;

	//����ĺ���ɸѡ
	int parentIdx = -1;
	for (int i = 0; i< contours.size(); i++)
	{
		if (hierarchy[i][2] != -1 && ic == 0)
		{
			parentIdx = i;
			ic++;
		}
		else if (hierarchy[i][2] != -1)
		{
			ic++;
		}
		else if (hierarchy[i][2] == -1)
		{
			ic = 0;
			parentIdx = -1;
		}

		if (ic >= 2)
		{
			contours2.push_back(contours[parentIdx]);
			drawContours(drawing, contours, parentIdx, CV_RGB(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)), 1, 8);
			ic = 0;
			parentIdx = -1;
			area = contourArea(contours[i]);//�ó�һ����ά�붨λ�ǵ�������Ա������߳���area_side�������ݸ�������ν��������λ��������һ�����ݶ����ԣ�
		}
		//cout<<"i= "<<i<<" hierarchy[i][2]= "<<hierarchy[i][2]<<" parentIdx= "<<parentIdx<<" ic= "<<ic<<endl;
	}


	for (int i = 0; i<contours2.size(); i++)
		drawContours(drawing2, contours2, i, CV_RGB(rng.uniform(100, 255), rng.uniform(100, 255), rng.uniform(100, 255)), -1, 4, hierarchy[k][2], 0, Point());


	Point point[100];
	for (int i = 0; i<contours2.size(); i++)
	{
		point[i] = Center_cal(contours2, i);
	}
	area = contourArea(contours2[1]);//Ϊʲô��һ���ǰ��һ�����������һ����
	int area_side = cvRound(sqrt(double(area)));
	for (int i = 0; i<contours2.size(); i++)
	{
		line(drawing2, point[i%contours2.size()], point[(i + 1) % contours2.size()], color, area_side / 4, 8);
	}



	//������Ҫ�����������ά��
	Mat gray_all, threshold_output_all;
	vector<vector<Point> > contours_all;
	vector<Vec4i> hierarchy_all;
	cvtColor(drawing2, gray_all, CV_BGR2GRAY);


	threshold(gray_all, threshold_output_all, 45, 255, THRESH_BINARY);

	//��ʾֻѰ�����������
	findContours(threshold_output_all, contours_all, hierarchy_all, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point(0, 0));//RETR_EXTERNAL��ʾֻѰ�����������

																												   //����С��Χ���Σ�б��Ҳ����Ŷ
	RotatedRect rectPoint = minAreaRect(contours_all[0]);
	Point2f fourPoint2f[4];

	//��rectPoint�����д洢������ֵ�ŵ� fourPoint��������
	rectPoint.points(fourPoint2f);

	//Ϊ������룬���������Ǿ��ε������������зŴ�Ϊ����
	//����߽��е���

	if (fourPoint2f[0].x < fourPoint2f[1].x)fourPoint2f[1].x = fourPoint2f[0].x;
	else fourPoint2f[0].x = fourPoint2f[1].x;
	//���ұ߽��е���
	if (fourPoint2f[2].x > fourPoint2f[3].x)fourPoint2f[3].x = fourPoint2f[2].x;
	else fourPoint2f[2].x = fourPoint2f[3].x;
	//���ϱ߽��е���
	if (fourPoint2f[1].y < fourPoint2f[2].y)fourPoint2f[2].y = fourPoint2f[1].y;
	else fourPoint2f[1].y = fourPoint2f[2].y;
	//�Եױ߽��е���
	if (fourPoint2f[0].y > fourPoint2f[3].y)fourPoint2f[3].y = fourPoint2f[0].y;
	else fourPoint2f[0].y = fourPoint2f[3].y;

	for (int i = 0;i < 4;i++)QRPoints[i] = fourPoint2f[i];
}

//����ʱ������ÿ��ɫ������ɫ���ǰ�ɫ���Ǵ���ֹͣ�Ļ�ɫ����Ϊ��ɫ����0����Ϊ��ɫ����1������Ϊ-1��
int Color_Check(Mat image, int x,int y,int rows,int cols)
{
	int grayValue[100000];
	int count = 0;
	for(int i=x;i<x+rows;i++)
		for (int j = y;j < y + cols;j++)
		{
			grayValue[count] = image.at<uchar>(i, j);
			count++;
		}
	double blackNum = 0;
	double whiteNum = 0;
	double elseColor = 0;
	for (int i = 0;i < count;i++)
	{
		if (grayValue[i] <= 50 )blackNum++;
		else if (grayValue[i] >= 180)whiteNum++;
		else elseColor++;
	}
	//������ɫռ�ȴ���50%����ɫ��Ӧ��ֵ
	double checkValue = 0;
	if (blackNum / count >= 0.5)return 0;
	else if (whiteNum / count >= 0.5)return 1;
	else return -1;
}

//��ά�����
void QRcode_Decode(Mat image,Point2f QRPoints2f[4])
{
	Mat grey;
	cvtColor(image, grey, CV_BGR2GRAY);
	Point QRPoints[4];
	//float����ת��Ϊint��
	for (int i = 0;i < 4;i++)
	{
		QRPoints[i].x = (int)QRPoints2f[i].x;
		QRPoints[i].y = (int)QRPoints2f[i].y;
	}
	cout << "�ĸ���������Ϊ��" << endl;
	for (int i = 0;i < 4;i++)
	{
		cout << "(" << QRPoints[i].x << "," << QRPoints[i].y << ")" << endl;
	}
	//���Ҫ����Ķ�ά��ÿ��ɫ��ĸߺͳ�
	int rows = (QRPoints[0].y - QRPoints[1].y) / 38;
	int cols = (QRPoints[2].x - QRPoints[1].x) / 38;
	int X = QRPoints[1].y;
	int Y = QRPoints[1].x;
	//��ʼ�����ά�룬�������txt��
	ofstream fout("decode.txt");
	for (int i = X;i < X + 8 * rows;i = i + rows)
		for (int j = Y + 8 * cols;j < Y + 30 * cols;j = j + cols)
			fout << Color_Check(grey, i, j, rows, cols);
	for (int i = X + 8 * rows;i < X + 30 * rows;i = i + rows)
		for (int j = Y;j < Y + 38 * cols;j = j + cols)
			fout << Color_Check(grey, i, j, rows, cols);
	for (int i = X + 30 * rows;i < X + 38 * rows;i = i + rows)
		for (int j = Y + 8 * cols;j < Y + 30 * cols;j = j + cols)
			fout << Color_Check(grey, i, j, rows, cols);
}


int main()
{

	DrawQRcode();
	//�洢��ά���ĸ����������
	Point2f QRPoints[4];
	LocateQRcode(QRPoints);

	Mat image = imread("5.jpg");
	QRcode_Decode(image, QRPoints);

	
	
	system("pause");

	return 0;
}