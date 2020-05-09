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

//����Ƶ����ΪͼƬ
string VideoToImage()
{
	cout << "������Ҫ�������Ƶ��·����";
	string VideoPath;
	cin >> VideoPath;
	string str1 = "ffmpeg -i ";
	string str2 = " DecodeImage//%8d.jpg";
	string finalInstruct = str1 + VideoPath + str2;
	return finalInstruct;
}

//����ȡ������С�����εı߽���ÿ���ܳ���������ȡһ��������꣬������ȡ�ĸ����ƽ�����꣨��ΪС�����εĴ������ģ�
Point Center_cal(vector<vector<Point> > contours, int i)
{
	int centerx = 0, centery = 0, n = contours[i].size();
	centerx = (contours[i][n / 4].x + contours[i][n * 2 / 4].x + contours[i][3 * n / 4].x + contours[i][n - 1].x) / 4;
	centery = (contours[i][n / 4].y + contours[i][n * 2 / 4].y + contours[i][3 * n / 4].y + contours[i][n - 1].y) / 4;
	Point point1 = Point(centerx, centery);
	return point1;
}

//��ͼ�����͸�ӱ任�͵õ���λ��
void LocateAndWarpQRcode(Mat InImage, Mat &OutImage, Point QRPoints[4])
{
	Mat src = InImage;
	Mat src_gray;
	Mat src_all = src.clone();
	Mat threshold_output;
	vector<vector<Point> > contours, contours2;
	vector<Vec4i> hierarchy;
	//Ԥ����
	cvtColor(src, src_gray, CV_BGR2GRAY);
	blur(src_gray, src_gray, Size(3, 3)); //ģ����ȥ��ë��
	threshold(src_gray, threshold_output, 100, 255, THRESH_OTSU);
	//Ѱ������ 
	//��һ������������ͼ�� 2ֵ����
	//�ڶ����������ڴ�洢����FindContours�ҵ��������ŵ��ڴ����档
	//�����������ǲ㼶��**[Next, Previous, First_Child, Parent]** ��vector
	//���ĸ����������ͣ��������ṹ
	//����������ǽڵ����ģʽ��������ȫ��Ѱ��
	findContours(threshold_output, contours, hierarchy, CV_RETR_TREE, CHAIN_APPROX_NONE, Point(0, 0));

	//����ɸѡ
	int c = 0, ic = 0, area = 0;
	int parentIdx = -1;
	for (int i = 0; i< contours.size(); i++)
	{
		//hierarchy[i][2] != -1 ��ʾ���������������
		if (hierarchy[i][2] != -1 && ic == 0)
		{
			parentIdx = i;
			ic++;
		}
		else if (hierarchy[i][2] != -1)
		{
			ic++;
		}
		//���������0
		else if (hierarchy[i][2] == -1)
		{
			ic = 0;
			parentIdx = -1;
		}
		//�ҵ���λ����Ϣ
		if (ic >= 2)
		{
			contours2.push_back(contours[parentIdx]);
			ic = 0;
			parentIdx = -1;
		}
	}
	if (contours2.size() != 4)//�����λ�㲻��4��������Ϊ��λʧ��
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



		//�����½�Ϊ0�����Ͻ�Ϊ1�����Ͻ�Ϊ2�����½�Ϊ3��point����,�ٴ浽QRPoints��
		//���Ͻǵĺ����������С�����½Ǻ���������������Ƚ���������ȷ��
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
		//������ȷ�����½Ǻ����Ͻ�
		for (int i = 0;i < 4;i++)
		{
			if (QRPoints[3].x - point[i].x > 200 && point[i].y - QRPoints[1].y > 200)QRPoints[0] = point[i];
			if (point[i].x - QRPoints[1].x > 200 && QRPoints[3].y - point[i].y > 200)QRPoints[2] = point[i];
		}

		vector<Point2f>src_corners(4);
		for (int i = 0;i < 4;i++)src_corners[i] = QRPoints[i];


		//Ϊ������룬���������Ǿ��ε������������зŴ�Ϊ����
		//����߽��е���
		if (QRPoints[0].x < QRPoints[1].x)QRPoints[1].x = QRPoints[0].x;
		else QRPoints[0].x = QRPoints[1].x;

		//���ұ߽��е���
		if (QRPoints[2].x > QRPoints[3].x)QRPoints[3].x = QRPoints[2].x;
		else QRPoints[2].x = QRPoints[3].x;

		//���ϱ߽��е���
		if (QRPoints[1].y < QRPoints[2].y)QRPoints[2].y = QRPoints[1].y;
		else QRPoints[1].y = QRPoints[2].y;

		//�Եױ߽��е���
		if (QRPoints[0].y > QRPoints[3].y)QRPoints[3].y = QRPoints[0].y;
		else QRPoints[0].y = QRPoints[3].y;

		//��ͼƬ����͸�ӱ任
		vector<Point2f>dst_corners(4);
		for (int i = 0;i < 4;i++)dst_corners[i] = QRPoints[i];

		//�����ĸ���λ����������������ά����ĸ���������
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

//�жϸ���ͼƬ��û�ж�λ�㣬����з���true�����򷵻�false
bool JudegLocatedPoint(Point QRPoints[4])
{
	int flag = 0;
	for (int i = 0;i < 4;i++)
		if (QRPoints[i].x != 0 && QRPoints[i].y != 0)flag++;
	if (flag == 4)return true;
	else return false;
}

//����ÿ��ͼƬ��ʵ�������Ϊÿ��ͼƬ����һ���ڰ��ٽ�ֵ
double GetValue(Mat image, Point QRPoints[4])
{
	Mat grey;
	cvtColor(image, grey, CV_BGR2GRAY);
	//���ñ�׼����ֵ������ֵȡ��ά���ĸ����㴦������ֵ��ƽ��ֵ

	double value1 = grey.at<uchar>(QRPoints[0].y - 10, QRPoints[0].x + 10);
	double value2 = grey.at<uchar>(QRPoints[1].y + 10, QRPoints[1].x + 10);
	double value3 = grey.at<uchar>(QRPoints[2].y + 10, QRPoints[2].x - 10);
	double value4 = grey.at<uchar>(QRPoints[3].y - 10, QRPoints[3].x - 10);
	double standerdValue = (value1 + value2 + value3 + value4) / 4;

	return standerdValue;
}

//����ʱ������ÿ��ɫ������ɫ���ǰ�ɫ����Ϊ��ɫ����0����Ϊ��ɫ����1
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
	//������ɫռ�ȴ���50%����ɫ��Ӧ��ֵ
	double checkValue = 0;
	if (blackNum / count >= 0.5)return 0;
	else return 1;
}
//�жϸ���ͼƬ�������Ż���ż����,�����򷵻�0��ż������1
int JudgeParity(Mat image, Point QRPoints[4], int standerdValue)
{
	Mat grey;
	cvtColor(image, grey, CV_BGR2GRAY);
	//���Ҫ����Ķ�ά��ÿ��ɫ��ĸߺͳ�
	double rows = double(QRPoints[0].y - QRPoints[1].y) / 53;
	double cols = double(QRPoints[2].x - QRPoints[1].x) / 53;
	int X = QRPoints[1].y;
	int Y = QRPoints[1].x;

	//flagΪ0��Ϊ������Ϊ1��Ϊż��
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

//��ά�����
void QRcode_Decode(Mat image, Point QRPoints[4], vector<int>&decodeByte, int standerdValue)
{
	Mat grey;
	cvtColor(image, grey, CV_BGR2GRAY);
	//���Ҫ����Ķ�ά��ÿ��ɫ��ĸߺͳ�
	double rows = double(QRPoints[0].y - QRPoints[1].y) / 53;
	double cols = double(QRPoints[2].x - QRPoints[1].x) / 53;
	int X = QRPoints[1].y;
	int Y = QRPoints[1].x;

	//�Ƚ����еĶ��������ݴ���һ��int��������
	//����double��int��ǿ��ת���ᶪʧһ�������ݣ������ڽ�����ǿ��ÿ�У�ÿ��ֻ����̶��Ĵ���
	//�Ա���������
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

//����������Ķ���������ת��Ϊ��Ӧ�ַ������浽txt�ļ���
void ByteToStr(vector<int>&decodeByte)
{
	//У���ļ����洢��żУ��Ľ��
	ofstream vout("vout.bin", ios::out | ios::binary);
	//�ٶ�int������������ݽ��л�ԭ
	ofstream fout("decode.bin", ios::out | ios::binary);
	int flag = 1;//�ж��Ƿ�����˽�����־������������18��0
				 //ÿ�ζ�ȡ9�����ݣ�����8λ���ݺ�1λ��żУ����
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
		//��⵽9��0֮��������9�����ǲ���ҲΪ0�����ǣ��������������ĩβ
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
			int CountNum = 0;//��¼��9λ����������λ��1�ĸ���
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

	cout << endl << "��Ƶ������ɣ���ʼ��ȡ������ͼƬ" << endl;

	char buffer[MAX_PATH];
	_getcwd(buffer, MAX_PATH);//��ȡ��ǰ����ľ��Ե�ַ
	string str = buffer;
	string str1 = "\\*.jpg";
	string DirextPath = str + str1;
	//����ImagePath�洢����ͼƬ·��
	vector<string> ImagePath;
	glob(DirextPath, ImagePath, true);
	if (ImagePath.size() == 0)cout << "no image!" << endl;
	//û��ͼƬ�򲻽��н���
	else
	{
		//����һ�����Mat���ڴ洢���е�ͼƬ
		vector<Mat> images(ImagePath.size());
		for (int i = 0;i < ImagePath.size();i++)images[i] = imread(ImagePath[i]);

		cout << endl << "�ѻ�ȡ����ͼƬ����ʼ������ͼƬ���н���" << endl;


		//��ʼ�Խ������������ͼƬ���н���

		vector<int> decodeByte;//�ݴ��������Ķ���������

		vector<Mat> OneImageGroup;//�ݴ�һ��ͼƬ�������ظ�֡

		vector<vector<Point>> QRPointsGroup;//�ݴ��ظ�֡��ÿ��ͼƬ�Ķ�λ��

		int lastParity = 0;//�ݴ�ǰһ��ͼƬ����ż�����ж��Ƿ�Ϊ�ظ�֡

		for (int i = 0;i < images.size();i++)
		{
			Point QRPoints[4];

			Mat finalImage;//�洢͸�ӱ任���ͼ��

			LocateAndWarpQRcode(images[i], finalImage, QRPoints);



			//������ڶ�λ�㣬����з������
			if (JudegLocatedPoint(QRPoints))
			{
				int standerdValue = GetValue(finalImage, QRPoints);

				int thisParity = JudgeParity(finalImage, QRPoints, standerdValue);

				//�������ͼƬ����һ��ͼƬ����ż�ж�һ������Ž�һ��ͼ����
				if (thisParity == lastParity)
				{
					OneImageGroup.push_back(finalImage);
					vector<Point> points;
					for (int i = 0;i < 4;i++)points.push_back(QRPoints[i]);
					QRPointsGroup.push_back(points);
				}

				//�������ȣ���˵��Ҫ��ʼ������һ�Ų�ͬ��ͼƬ��������һ��������н���
				else
				{
					lastParity = thisParity;
					//ѡȡ������������һ���ͼƬ���н���
					int ImgNum = 0;
					if (OneImageGroup.size() == 1)ImgNum = 0;
					else ImgNum = OneImageGroup.size() / 2;

					Point De_QRPoints[4];
					for (int j = 0;j < 4;j++)
						De_QRPoints[j] = QRPointsGroup[ImgNum][j];

					QRcode_Decode(OneImageGroup[ImgNum], De_QRPoints, decodeByte, standerdValue);

					//�������ո����������ڴ洢��һ��ͼƬ
					vector<Mat>().swap(OneImageGroup);
					vector<vector<Point>>().swap(QRPointsGroup);
					OneImageGroup.push_back(finalImage);
					vector<Point> points;
					for (int i = 0;i < 4;i++)points.push_back(QRPoints[i]);
					QRPointsGroup.push_back(points);
				}
			}

		}
		//�����һ����н���
		int ImgNum = 0;
		if (OneImageGroup.size() == 1)ImgNum = 0;
		else ImgNum = OneImageGroup.size() / 2;
		Point De_QRPoints[4];
		for (int j = 0;j < 4;j++)
			De_QRPoints[j] = QRPointsGroup[ImgNum][j];
		int standerdValue = GetValue(OneImageGroup[ImgNum], De_QRPoints);
		QRcode_Decode(OneImageGroup[ImgNum], De_QRPoints, decodeByte, standerdValue);

		//������������ת��Ϊ�ı��洢
		ByteToStr(decodeByte);

		cout << endl << "����ɽ��룬�Ѿ�����decode.bin��vout.bin�ļ�" << endl;
	}

	system("pause");

	return 0;
}