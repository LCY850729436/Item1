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

// 二值化阈值
#define GRAY_THRESH 150

// 直线上点的个数
#define HOUGH_VOTE 50


//将字符的各个字符转化为8位二进制数据
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

//将文本里的所有字符转化为8位二进制数据
void TxtToByte(string str, vector<int>&BinaryByte)
{
	for (int i = 0;i < str.size();i++)
	{
		int a[8] = { -1 };
		StrToByte(a, str[i]);
		for (int j = 7;j >= 0;j--)BinaryByte.push_back(a[j]);
	}
}


//绘制二维码的像素(扩大为正常像素的16倍)
void WritePixel(int rows, int cols, Mat image,int color)
{
	for (int i = rows;i < rows + 16;i++)
		for (int j = cols;j < cols + 16;j++)
			image.at<uchar>(i, j) = color;
}

//绘制“回“字
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

//绘制二维码的三个“回”字顶点
void WritePto(Mat image)
{
	WritePoint(196, 146, image);//左上
	WritePoint(692, 146, image);//右上
	WritePoint(196, 642, image);//左下
}


//绘制二维码,0编码为黑色，1编码为白色
void DrawQRcode()
{
	string str;
	vector<int>BinaryByte;
	cout << "请输入要编码的字符：";
	cin >> str;
	TxtToByte(str, BinaryByte);
	int size = BinaryByte.size();//防止因输出多余二进制流数据的色块
	int num = 0;//计算对多少二进制数据编码
	int count = 0;//用于计算编码合适结束
	int imageNum = 1;//用于计算输出的图片数
	int x, y;//定义二维码结束绘制时的坐标
	while (size != 0)
	{
		//生成900*1000像素的纯白底色图用于绘制二维码
		Mat image(900, 1000, CV_8UC1, 255);

		//绘制三个定位点
		WritePto(image);

		//进行编码开始绘制二维码
		for (int i = 146;i < 274 && num < size;i = i + 16)//行
			for (int j = 324;j < 668 && num < size;j = j + 16)//列
			{
				if (BinaryByte[count] == 0)WritePixel(i, j, image, 0);//黑
				else WritePixel(i, j, image, 255);;//白
				count++;
				num++;
				x = i;
				y = j;
			}
		size = size - num;
		num = 0;
		for (int i = 274;i < 626 && num < size;i = i + 16)//行
			for (int j = 196;j < 804 && num < size;j = j + 16)//列
			{
				if (BinaryByte[count] == 0)WritePixel(i, j, image, 0);//黑
				else WritePixel(i, j, image, 255);;//白
				count++;
				num++;
				x = i;
				y = j;
			}
		size = size - num;
		num = 0;
		for (int i = 626;i < 754 && num < size;i = i + 16)//行
			for (int j = 324;j < 804 && num < size;j = j + 16)//列
			{
				if (BinaryByte[count] == 0)WritePixel(i, j, image, 0);//黑
				else WritePixel(i, j, image, 255);;//白
				count++;
				num++;
				x = i;
				y = j;
			}
		size = size - num;
		num = 0;
		//将int转化为字符串型数字便于为输出的图片命名
		string res;
		stringstream ss;          
		ss << imageNum; 
		ss >> res;
		string imageName = res + ".jpg";
		//所有数据全部绘制进二维码后，若二维码没被填满则将其用其他颜色填满便于定位识别
		if (size == 0)
		{
			y = y + 16;
			//将没填满的一行填满
			if (x < 274)for (int i = x, j = y;j < 668;j = j + 16)WritePixel(i, j, image, 85);
			else for (int i = x, j = y;j < 804;j = j + 16)WritePixel(i, j, image, 85);
			x = x + 16;
			//绘制剩下的区域
			for (x; x < 274; x = x + 16)//行
				for (int j = 324;j < 668;j = j + 16)//列
					WritePixel(x, j, image, 85);//灰
			for (x; x < 626; x = x + 16)//行
				for (int j = 196; j < 804; j = j + 16)//列
					WritePixel(x, j, image, 85);//灰
			for (x; x < 754; x = x + 16)//行
				for (int j = 324; j < 804; j = j + 16)//列
					WritePixel(x, j, image, 85);//灰
		}
		imwrite(imageName, image);
		imageNum++;
	}

}


//图像矫正
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
	imwrite("矫正.jpg", dstImg);
	//imwrite("imageText_D.jpg",dstImg);
	waitKey(0);

	return dstImg;
}


//找到所提取轮廓的中心点
Point Center_cal(vector<vector<Point> > contours, int i)
{
	int centerx = 0, centery = 0, n = contours[i].size();
	//在提取的小正方形的边界上每隔周长个像素提取一个点的坐标，求所提取四个点的平均坐标（即为小正方形的大致中心）
	centerx = (contours[i][n / 4].x + contours[i][n * 2 / 4].x + contours[i][3 * n / 4].x + contours[i][n - 1].x) / 4;
	centery = (contours[i][n / 4].y + contours[i][n * 2 / 4].y + contours[i][3 * n / 4].y + contours[i][n - 1].y) / 4;
	Point point1 = Point(centerx, centery);
	return point1;
}


//定位二维码，求出二维码的四个顶点的坐标
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
	//imshow("预处理后：",threshold_output);
	//寻找轮廓 
	//第一个参数是输入图像 2值化的
	//第二个参数是内存存储器，FindContours找到的轮廓放到内存里面。
	//第三个参数是层级，**[Next, Previous, First_Child, Parent]** 的vector
	//第四个参数是类型，采用树结构
	//第五个参数是节点拟合模式，这里是全部寻找
	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));
	//CHAIN_APPROX_NONE全体,CV_CHAIN_APPROX_SIMPLE,,,RETR_TREE    RETR_EXTERNAL    RETR_LIST   RETR_CCOMP

	int c = 0, ic = 0, k = 0, area = 0;

	//程序的核心筛选
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
			area = contourArea(contours[i]);//得出一个二维码定位角的面积，以便计算其边长（area_side）（数据覆盖无所谓，三个定位角中任意一个数据都可以）
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
	area = contourArea(contours2[1]);//为什么这一句和前面一句计算的面积不一样呢
	int area_side = cvRound(sqrt(double(area)));
	for (int i = 0; i<contours2.size(); i++)
	{
		line(drawing2, point[i%contours2.size()], point[(i + 1) % contours2.size()], color, area_side / 4, 8);
	}



	//接下来要框出这整个二维码
	Mat gray_all, threshold_output_all;
	vector<vector<Point> > contours_all;
	vector<Vec4i> hierarchy_all;
	cvtColor(drawing2, gray_all, CV_BGR2GRAY);


	threshold(gray_all, threshold_output_all, 45, 255, THRESH_BINARY);

	//表示只寻找最外层轮廓
	findContours(threshold_output_all, contours_all, hierarchy_all, RETR_EXTERNAL, CHAIN_APPROX_NONE, Point(0, 0));//RETR_EXTERNAL表示只寻找最外层轮廓

																												   //求最小包围矩形，斜的也可以哦
	RotatedRect rectPoint = minAreaRect(contours_all[0]);
	Point2f fourPoint2f[4];

	//将rectPoint变量中存储的坐标值放到 fourPoint的数组中
	rectPoint.points(fourPoint2f);

	//为方便解码，若遇到不是矩形的情况，将其进行放大为矩形
	//对左边进行调整

	if (fourPoint2f[0].x < fourPoint2f[1].x)fourPoint2f[1].x = fourPoint2f[0].x;
	else fourPoint2f[0].x = fourPoint2f[1].x;
	//对右边进行调整
	if (fourPoint2f[2].x > fourPoint2f[3].x)fourPoint2f[3].x = fourPoint2f[2].x;
	else fourPoint2f[2].x = fourPoint2f[3].x;
	//对上边进行调整
	if (fourPoint2f[1].y < fourPoint2f[2].y)fourPoint2f[2].y = fourPoint2f[1].y;
	else fourPoint2f[1].y = fourPoint2f[2].y;
	//对底边进行调整
	if (fourPoint2f[0].y > fourPoint2f[3].y)fourPoint2f[3].y = fourPoint2f[0].y;
	else fourPoint2f[0].y = fourPoint2f[3].y;

	for (int i = 0;i < 4;i++)QRPoints[i] = fourPoint2f[i];
}

//解码时，检验每个色块代表黑色还是白色或是代表停止的灰色，若为黑色返回0，若为白色返回1，否则为-1；
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
	//返回颜色占比大于50%的颜色对应的值
	double checkValue = 0;
	if (blackNum / count >= 0.5)return 0;
	else if (whiteNum / count >= 0.5)return 1;
	else return -1;
}

//二维码解码
void QRcode_Decode(Mat image,Point2f QRPoints2f[4])
{
	Mat grey;
	cvtColor(image, grey, CV_BGR2GRAY);
	Point QRPoints[4];
	//float类型转化为int型
	for (int i = 0;i < 4;i++)
	{
		QRPoints[i].x = (int)QRPoints2f[i].x;
		QRPoints[i].y = (int)QRPoints2f[i].y;
	}
	cout << "四个顶点坐标为：" << endl;
	for (int i = 0;i < 4;i++)
	{
		cout << "(" << QRPoints[i].x << "," << QRPoints[i].y << ")" << endl;
	}
	//获得要解码的二维码每个色块的高和长
	int rows = (QRPoints[0].y - QRPoints[1].y) / 38;
	int cols = (QRPoints[2].x - QRPoints[1].x) / 38;
	int X = QRPoints[1].y;
	int Y = QRPoints[1].x;
	//开始解码二维码，并输出到txt里
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
	//存储二维码四个顶点的坐标
	Point2f QRPoints[4];
	LocateQRcode(QRPoints);

	Mat image = imread("5.jpg");
	QRcode_Decode(image, QRPoints);

	
	
	system("pause");

	return 0;
}