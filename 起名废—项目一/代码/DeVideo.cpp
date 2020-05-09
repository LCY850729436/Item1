#include<iostream>
#include<opencv2/opencv.hpp>
#include<opencv2\imgproc\types_c.h>
#include<opencv2/imgcodecs/legacy/constants_c.h>
#include<string>
#include<sstream>
#include<fstream>
#include<math.h>
#include<Windows.h>
#include<direct.h>
#include<cstdlib>

using namespace std;
using namespace cv;

//将视频解码为图片
string VideoToImage()
{
	cout << "请输入要解码的视频的路径：";
	string VideoPath;
	cin >> VideoPath;
	string str1 = "ffmpeg -i ";
	string str2 = " DecodeImage//%8d.jpg";
	string finalInstruct = str1 + VideoPath + str2;
	return finalInstruct;
}

//在提取的中心小正方形的边界上每隔周长个像素提取一个点的坐标，求所提取四个点的平均坐标（即为小正方形的大致中心）
Point Center_cal(vector<vector<Point> > contours, int i)
{
	int centerx = 0, centery = 0, n = contours[i].size();
	centerx = (contours[i][n / 4].x + contours[i][n * 2 / 4].x + contours[i][3 * n / 4].x + contours[i][n - 1].x) / 4;
	centery = (contours[i][n / 4].y + contours[i][n * 2 / 4].y + contours[i][3 * n / 4].y + contours[i][n - 1].y) / 4;
	Point point1 = Point(centerx, centery);
	return point1;
}

//对图像进行透视变换和得到定位点
void LocateAndWarpQRcode(Mat InImage, Mat &OutImage, Point QRPoints[4])
{
	Mat src = InImage;
	Mat src_gray;
	Mat src_all = src.clone();
	Mat threshold_output;
	vector<vector<Point> > contours, contours2;
	vector<Vec4i> hierarchy;
	//预处理
	cvtColor(src, src_gray, CV_BGR2GRAY);
	blur(src_gray, src_gray, Size(3, 3)); //模糊，去除毛刺
	threshold(src_gray, threshold_output, 100, 255, THRESH_OTSU);
	//寻找轮廓 
	//第一个参数是输入图像 2值化的
	//第二个参数是内存存储器，FindContours找到的轮廓放到内存里面。
	//第三个参数是层级，**[Next, Previous, First_Child, Parent]** 的vector
	//第四个参数是类型，采用树结构
	//第五个参数是节点拟合模式，这里是全部寻找
	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));

	//轮廓筛选
	int c = 0, ic = 0, area = 0;
	int parentIdx = -1;
	for (int i = 0; i< contours.size(); i++)
	{
		//hierarchy[i][2] != -1 表示不是最外面的轮廓
		if (hierarchy[i][2] != -1 && ic == 0)
		{
			parentIdx = i;
			ic++;
		}
		else if (hierarchy[i][2] != -1)
		{
			ic++;
		}
		//最外面的清0
		else if (hierarchy[i][2] == -1)
		{
			ic = 0;
			parentIdx = -1;
		}
		//找到定位点信息
		if (ic >= 2)
		{
			contours2.push_back(contours[parentIdx]);
			ic = 0;
			parentIdx = -1;
		}
	}
	if (contours2.size() != 4)//如果定位点不是4个，则认为定位失败
	{
		for (int i = 0;i < 4;i++)
		{
			QRPoints[i].x = 0;
			QRPoints[i].y = 0;
		}
	}
	else
	{
		Point point[4];
		for (int i = 0; i < contours2.size(); i++)
		{
			point[i] = Center_cal(contours2, i);
		}



		//按左下角为0，左上角为1，右上角为2，右下角为3对point排序,再存到QRPoints里
		//左上角的横纵坐标和最小，右下角横纵坐标和最大，因此先将这两个点确定
		int sum[4] = { 0 };
		for (int i = 0;i < 4;i++)sum[i] = point[i].x + point[i].y;
		int min = 999999, max = 0;
		for (int i = 0;i < 4;i++)
		{
			if (min > sum[i])min = sum[i];
			if (max < sum[i])max = sum[i];
		}
		for (int i = 0;i < 4;i++)
		{
			if (sum[i] == min)QRPoints[1] = point[i];
			if (sum[i] == max)QRPoints[3] = point[i];
		}
		//接下来确定左下角和右上角
		for (int i = 0;i < 4;i++)
		{
			if (QRPoints[3].x - point[i].x > 200 && point[i].y - QRPoints[1].y > 200)QRPoints[0] = point[i];
			if (point[i].x - QRPoints[1].x > 200 && QRPoints[3].y - point[i].y > 200)QRPoints[2] = point[i];
		}

		vector<Point2f>src_corners(4);
		for (int i = 0;i < 4;i++)src_corners[i] = QRPoints[i];


		//为方便解码，若遇到不是矩形的情况，将其进行放大为矩形
		//对左边进行调整
		if (QRPoints[0].x < QRPoints[1].x)QRPoints[1].x = QRPoints[0].x;
		else QRPoints[0].x = QRPoints[1].x;

		//对右边进行调整
		if (QRPoints[2].x > QRPoints[3].x)QRPoints[3].x = QRPoints[2].x;
		else QRPoints[2].x = QRPoints[3].x;

		//对上边进行调整
		if (QRPoints[1].y < QRPoints[2].y)QRPoints[2].y = QRPoints[1].y;
		else QRPoints[1].y = QRPoints[2].y;

		//对底边进行调整
		if (QRPoints[0].y > QRPoints[3].y)QRPoints[3].y = QRPoints[0].y;
		else QRPoints[0].y = QRPoints[3].y;

		//对图片进行透视变换
		vector<Point2f>dst_corners(4);
		for (int i = 0;i < 4;i++)dst_corners[i] = QRPoints[i];

		//根据四个定位点的中心坐标求出二维码的四个顶点坐标
		double addlen = double(QRPoints[2].x - QRPoints[1].x) / 47 * 3.6;
		QRPoints[0].x -= addlen;
		QRPoints[0].y += addlen;
		QRPoints[1].x -= addlen;
		QRPoints[1].y -= addlen;
		QRPoints[2].x += addlen;
		QRPoints[2].y -= addlen;
		QRPoints[3].x += addlen;
		QRPoints[3].y += addlen;

		Mat resultImg;
		Mat warpmatrix = getPerspectiveTransform(src_corners, dst_corners);
		warpPerspective(InImage, resultImg, warpmatrix, InImage.size(), CV_INTER_LINEAR);

		OutImage = resultImg;
	}
}

//判断该张图片有没有定位点，如果有返回true，否则返回false
bool JudegLocatedPoint(Point QRPoints[4])
{
	int flag = 0;
	for (int i = 0;i < 4;i++)
		if (QRPoints[i].x != 0 && QRPoints[i].y != 0)flag++;
	if (flag == 4)return true;
	else return false;
}

//根据每张图片的实际情况来为每张图片定义一个黑白临界值
double GetValue(Mat image, Point QRPoints[4])
{
	Mat grey;
	cvtColor(image, grey, CV_BGR2GRAY);
	//设置标准像素值，且其值取二维码四个顶点处的像素值的平均值

	double value1 = grey.at<uchar>(QRPoints[0].y - 10, QRPoints[0].x + 10);
	double value2 = grey.at<uchar>(QRPoints[1].y + 10, QRPoints[1].x + 10);
	double value3 = grey.at<uchar>(QRPoints[2].y + 10, QRPoints[2].x - 10);
	double value4 = grey.at<uchar>(QRPoints[3].y - 10, QRPoints[3].x - 10);
	double standerdValue = (value1 + value2 + value3 + value4) / 4;

	return standerdValue;
}

//解码时，检验每个色块代表黑色还是白色，若为黑色返回0，若为白色返回1
int Color_Check(Mat image, int x, int y, double rows, double cols, int standerdValue)
{

	int grayValue[100000];
	int count = 0;
	for (int i = x;i<x + rows;i++)
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
		if (grayValue[i] <= (standerdValue + 100))blackNum++;
		else whiteNum++;
	}
	//返回颜色占比大于50%的颜色对应的值
	double checkValue = 0;
	if (blackNum / count >= 0.5)return 0;
	else return 1;
}
//判断该张图片是奇数张还是偶数张,奇数则返回0，偶数返回1
int JudgeParity(Mat image, Point QRPoints[4], int standerdValue)
{
	Mat grey;
	cvtColor(image, grey, CV_BGR2GRAY);
	//获得要解码的二维码每个色块的高和长
	double rows = double(QRPoints[0].y - QRPoints[1].y) / 53;
	double cols = double(QRPoints[2].x - QRPoints[1].x) / 53;
	int X = QRPoints[1].y;
	int Y = QRPoints[1].x;

	//flag为0则为奇数，为1则为偶数
	int flag = 0;
	double whiteNum = 0;
	double blackNum = 0;
	for (double j = Y + 8 * cols;j < Y + 15 * cols;j = j + cols)
	{
		if (Color_Check(grey, X, j, rows, cols, standerdValue) == 1)whiteNum++;
		else blackNum++;
	}
	if (whiteNum / 7 > 0.5)flag = 0;
	else flag = 1;

	return flag;
}

//二维码解码
void QRcode_Decode(Mat image, Point QRPoints[4], vector<int>&decodeByte, int standerdValue)
{
	Mat grey;
	cvtColor(image, grey, CV_BGR2GRAY);
	//获得要解码的二维码每个色块的高和长
	double rows = double(QRPoints[0].y - QRPoints[1].y) / 53;
	double cols = double(QRPoints[2].x - QRPoints[1].x) / 53;
	int X = QRPoints[1].y;
	int Y = QRPoints[1].x;

	//先将所有的二进制数据存入一个int类容器里
	//由于double到int的强制转换会丢失一部分数据，所以在解码是强制每行，每列只解码固定的次数
	//以避免多余解码
	int count1 = 0;
	for (double j = Y + 15 * cols;j < Y + 45 * cols;j = j + cols)
	{
		decodeByte.push_back(Color_Check(grey, X, j, rows, cols, standerdValue));
		count1++;
		if (count1 >= 30)break;
	}

	int counti = 0;
	for (double i = X + rows;i < X + 8 * rows;i = i + rows)
	{
		int countj = 0;
		for (double j = Y + 8 * cols;j < Y + 45 * cols;j = j + cols)
		{
			decodeByte.push_back(Color_Check(grey, i, j, rows, cols, standerdValue));
			countj++;
			if (countj >= 37)break;
		}
		counti++;
		if (counti >= 7)break;
	}

	counti = 0;
	for (double i = X + 8 * rows;i < X + 45 * rows;i = i + rows)
	{
		int countj = 0;
		for (double j = Y;j < Y + 53 * cols;j = j + cols)
		{
			decodeByte.push_back(Color_Check(grey, i, j, rows, cols, standerdValue));
			countj++;
			if (countj >= 53)break;
		}
		counti++;
		if (counti >= 37)break;
	}

	counti = 0;
	for (double i = X + 45 * rows;i < X + 53 * rows;i = i + rows)
	{
		int countj = 0;
		for (double j = Y + 8 * cols;j < Y + 45 * cols;j = j + cols)
		{
			decodeByte.push_back(Color_Check(grey, i, j, rows, cols, standerdValue));
			countj++;
			if (countj >= 37)break;
		}
		counti++;
		if (counti >= 8)break;
	}
}

//将解码出来的二进制数据转化为对应字符串并存到txt文件里
void ByteToStr(vector<int>&decodeByte)
{
	//校验文件，存储奇偶校验的结果
	ofstream vout("vout.bin", ios::out | ios::binary);
	//再对int类容器里的数据进行还原
	ofstream fout("decode.bin", ios::out | ios::binary);
	int flag = 1;//判断是否读到了结束标志，即连续读到18个0
				 //每次读取9个数据，包括8位数据和1位奇偶校验码
	for (int i = 0;i < decodeByte.size() && flag == 1;i = i + 9)
	{
		int byte[9] = { 0 };
		int count = 0;
		for (int j = 0;j < 9;j++)
		{
			if (j + i >= decodeByte.size())break;
			byte[j] = decodeByte[j + i];
			if (byte[j] == 0)count++;
		}
		//检测到9个0之后检测后面的9个数是不是也为0，若是，则表明到了数据末尾
		if (count == 9)
		{
			if (i + 9 >= decodeByte.size())break;
			for (int j = 9;j < 18;j++)if (decodeByte[i + j] == 0)count++;

			if (count == 18)flag = 0;
			else count = 0;
		}
		if (flag == 1)
		{
			int num = 0;
			int CountNum = 0;//记录这9位数据中数据位中1的个数
			for (int j = 7;j >= 0;j--)
			{
				num = num + byte[j] * pow(2, 7 - j);
				if (byte[j] == 1)CountNum++;
			}
			unsigned char ch = (unsigned char)num;
			fout << ch;
			if (CountNum % 2 == 0)
			{
				if (byte[8] == 1)vout << (unsigned char)255;
				else vout << (unsigned char)0;
			}
			else
			{
				if (byte[8] == 0)vout << (unsigned char)255;
				else vout << (unsigned char)0;
			}
			if (i + 9 >= decodeByte.size())break;
		}
	}
}

int main()
{
	string Instruct;
	Instruct = VideoToImage();
	WinExec("cmd", SW_NORMAL);
	LPCSTR instruct = Instruct.c_str();
	system(instruct);

	cout << endl << "视频解码完成，开始获取解码后的图片" << endl;

	char buffer[MAX_PATH];
	_getcwd(buffer, MAX_PATH);//获取当前程序的绝对地址
	string str = buffer;
	string str1 = "\\*.jpg";
	string DirextPath = str + str1;
	//定义ImagePath存储所有图片路径
	vector<string> ImagePath;
	glob(DirextPath, ImagePath, true);
	if (ImagePath.size() == 0)cout << "no image!" << endl;
	//没有图片则不进行解码
	else
	{
		//定义一个大的Mat用于存储所有的图片
		vector<Mat> images(ImagePath.size());
		for (int i = 0;i < ImagePath.size();i++)images[i] = imread(ImagePath[i]);

		cout << endl << "已获取所有图片，开始对所有图片进行解码" << endl;


		//开始对解码出来的所有图片进行解码

		vector<int> decodeByte;//暂存解码出来的二进制数据

		vector<Mat> OneImageGroup;//暂存一张图片的所有重复帧

		vector<vector<Point>> QRPointsGroup;//暂存重复帧里每张图片的定位点

		int lastParity = 0;//暂存前一张图片的奇偶数来判断是否为重复帧

		for (int i = 0;i < images.size();i++)
		{
			Point QRPoints[4];

			Mat finalImage;//存储透视变换后的图像

			LocateAndWarpQRcode(images[i], finalImage, QRPoints);



			//如果存在定位点，则进行分组解码
			if (JudegLocatedPoint(QRPoints))
			{
				int standerdValue = GetValue(finalImage, QRPoints);

				int thisParity = JudgeParity(finalImage, QRPoints, standerdValue);

				//如果该张图片与上一张图片的奇偶判定一样，则放进一个图里面
				if (thisParity == lastParity)
				{
					OneImageGroup.push_back(finalImage);
					vector<Point> points;
					for (int i = 0;i < 4;i++)points.push_back(QRPoints[i]);
					QRPointsGroup.push_back(points);
				}

				//如果不相等，则说明要开始解码下一张不同的图片，并对上一个分组进行解码
				else
				{
					lastParity = thisParity;
					//选取分组里面中心一点的图片进行解码
					int ImgNum = 0;
					if (OneImageGroup.size() == 1)ImgNum = 0;
					else ImgNum = OneImageGroup.size() / 2;

					Point De_QRPoints[4];
					for (int j = 0;j < 4;j++)
						De_QRPoints[j] = QRPointsGroup[ImgNum][j];

					QRcode_Decode(OneImageGroup[ImgNum], De_QRPoints, decodeByte, standerdValue);

					//解码后清空该容器，用于存储下一组图片
					vector<Mat>().swap(OneImageGroup);
					vector<vector<Point>>().swap(QRPointsGroup);
					OneImageGroup.push_back(finalImage);
					vector<Point> points;
					for (int i = 0;i < 4;i++)points.push_back(QRPoints[i]);
					QRPointsGroup.push_back(points);
				}
			}

		}
		//对最后一组进行解码
		int ImgNum = 0;
		if (OneImageGroup.size() == 1)ImgNum = 0;
		else ImgNum = OneImageGroup.size() / 2;
		Point De_QRPoints[4];
		for (int j = 0;j < 4;j++)
			De_QRPoints[j] = QRPointsGroup[ImgNum][j];
		int standerdValue = GetValue(OneImageGroup[ImgNum], De_QRPoints);
		QRcode_Decode(OneImageGroup[ImgNum], De_QRPoints, decodeByte, standerdValue);

		//将二进制数据转化为文本存储
		ByteToStr(decodeByte);

		cout << endl << "已完成解码，已经生成decode.bin和vout.bin文件" << endl;
	}

	system("pause");

	return 0;
}