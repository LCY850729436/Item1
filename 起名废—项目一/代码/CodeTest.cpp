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

using namespace std;
using namespace cv;

//将字符的各个字符转化为8位二进制数据
void StrToByte(int a[8], int ch)
{
	int n, i, j = 0;
	n = ch;
	i = n;
	while (i)
	{
		a[j] = i % 2;
		i /= 2;
		j++;

	}
	for (;j < 8;j++)if (a[j] == -1)a[j] = 0;
}


//将文本里的所有字符转化为8位二进制数据，并在最后添加18位全为0的数据作为结束标志位
void TxtToByte(vector<int>&Text, vector<int>&BinaryByte)
{
	for (int i = 0;i < Text.size();i++)
	{
		int a[8] = { -1 };
		StrToByte(a, Text[i]);
		for (int j = 7;j >= 0;j--)BinaryByte.push_back(a[j]);
		//每8位增加一个奇偶校验码，采用奇校验
		int count = 0;//记录1的个数
		for (int j = 0;j < 8;j++)if (a[j] == 1)count++;
		if (count % 2 == 0)BinaryByte.push_back(1);//1的个数为偶数，奇偶校验码为1
		else BinaryByte.push_back(0);//1的个数为奇数，奇偶校验码为0
		if (i > 60000)break;//文件过大，完全读入会生成很多不能编码为视频的图片
	}
	for (int i = 0;i < 18;i++)BinaryByte.push_back(0);
}


//绘制二维码的像素(扩大为正常像素的12*12倍)
void WritePixel(int rows, int cols, Mat image, int color)
{
	for (int i = rows;i < rows + 12;i++)
		for (int j = cols;j < cols + 12;j++)
			image.at<uchar>(i, j) = color;
}

//绘制“回“字
void WritePoint(int x, int y, Mat image)
{
	for (int i = x;i < x + 84;i++)
		for (int j = y;j < y + 12;j++)
			image.at<uchar>(j, i) = 0;
	for (int i = x;i < x + 84;i++)
		for (int j = y + 72;j < y + 84;j++)
			image.at<uchar>(j, i) = 0;
	for (int i = x;i < x + 84;i++)
		for (int j = y + 12;j < y + 24;j++)
		{
			if (i >= x + 12 && i < x + 72)image.at<uchar>(j, i) = 255;
			else image.at<uchar>(j, i) = 0;
		}
	for (int i = x;i < x + 84;i++)
		for (int j = y + 60;j < y + 72;j++)
		{
			if (i >= x + 12 && i < x + 72)image.at<uchar>(j, i) = 255;
			else image.at<uchar>(j, i) = 0;
		}
	for (int i = x;i < x + 84;i++)
		for (int j = y + 24;j < y + 60;j++)
		{
			if (i >= x + 12 && i < x + 24 || i >= x + 60 && i < x + 72)image.at<uchar>(j, i) = 255;
			else image.at<uchar>(j, i) = 0;
		}
}

//绘制二维码的四个“回”字顶点
void WritePto(Mat image)
{
	WritePoint(196, 146, image);//左上
	WritePoint(748, 146, image);//右上
	WritePoint(196, 698, image);//左下
	WritePoint(748, 698, image);//右下
}


//绘制二维码,0编码为黑色，1编码为白色
void DrawQRcode()
{
	string FilePath;

	vector<int>BinaryByte;
	vector<int>Text;
	cout << "请输入要编码的文本地址：";
	cin >> FilePath;

	const char* file = FilePath.c_str();
	FILE * fr;
	char path[100];
	sprintf_s(path, 100, file, (1));
	fopen_s(&fr, path, "rb");
	char  temp[1];
	int byte = 0;
	while (!feof(fr)) {
		temp[0] = fgetc(fr);
		unsigned char Temp = (unsigned char)temp[0];
		Text.push_back((int)Temp);
		byte++;
	}

	TxtToByte(Text, BinaryByte);
	int size = BinaryByte.size();//防止因输出多余二进制流数据的色块
	int num = 0;//计算对多少二进制数据编码
	int count = 0;//用于计算编码合适结束
	int imageNum = 1;//用于计算输出的图片数
	int x, y;//定义二维码结束绘制时的坐标
	while (size != 0)
	{
		//生成900*1000像素的纯白底色图用于绘制二维码
		Mat image(900, 1000, CV_8UC1, 255);

		//绘制四个定位点
		WritePto(image);

		//进行编码开始绘制二维码
		//在第一行开头设置一个7位同步码来解决重复帧的问题
		for (int j = 292;j < 376;j = j + 12)
		{
			if (imageNum % 2 == 1)WritePixel(146, j, image, 255);//如果张数为单数，则全为白色
			else WritePixel(146, j, image, 0);//如果为双数，则全为黑色
		}
		for (int j = 376;j < 736 && num < size;j = j + 12)
		{
			if (BinaryByte[count] == 0)WritePixel(146, j, image, 0);//黑
			else WritePixel(146, j, image, 255);//白
			count++;
			num++;
			x = 146;
			y = j;
		}
		size = size - num;
		num = 0;
		for (int i = 158;i < 242 && num < size;i = i + 12)//行
			for (int j = 292;j < 736 && num < size;j = j + 12)//列
			{
				if (BinaryByte[count] == 0)WritePixel(i, j, image, 0);//黑
				else WritePixel(i, j, image, 255);//白
				count++;
				num++;
				x = i;
				y = j;
			}

		for (int i = 242;i < 686 && num < size;i = i + 12)//行
			for (int j = 196;j < 832 && num < size;j = j + 12)//列
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
		for (int i = 686;i < 782 && num < size;i = i + 12)//行
			for (int j = 292;j < 736 && num < size;j = j + 12)//列
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
		string str = "Image\\";
		string imageName = str + res + ".jpg";
		//所有数据全部绘制进二维码后，若二维码没被填满则将其用黑色和白色交替填满便于定位识别
		if (size == 0)
		{
			y = y + 12;
			//将没填满的一行填满
			if (x < 242 || x >= 686)
				for (int i = x, j = y;j < 736;j = j + 12)
				{
					if ((j / 12) % 2 == 0)
					{
						if ((i / 12) % 2 == 0)WritePixel(i, j, image, 0);
						else WritePixel(i, j, image, 255);
					}
					else
					{
						if ((i / 12) % 2 == 0)WritePixel(i, j, image, 255);
						else WritePixel(i, j, image, 0);
					}
				}
			else for (int i = x, j = y;j < 832;j = j + 12)
			{
				if ((j / 12) % 2 == 0)
				{
					if ((i / 12) % 2 == 0)WritePixel(i, j, image, 0);
					else WritePixel(i, j, image, 255);
				}
				else
				{
					if ((i / 12) % 2 == 0)WritePixel(i, j, image, 255);
					else WritePixel(i, j, image, 0);
				}
			}
			x = x + 12;
			//绘制剩下的区域
			for (x; x < 242; x = x + 12)//行
				for (int j = 292;j < 736;j = j + 12)//列
				{
					if ((j / 12) % 2 == 0)
					{
						if ((x / 12) % 2 == 0)WritePixel(x, j, image, 0);
						else WritePixel(x, j, image, 255);
					}
					else
					{
						if ((x / 12) % 2 == 0)WritePixel(x, j, image, 255);
						else WritePixel(x, j, image, 0);
					}
				}//灰白交替
			for (x; x < 686; x = x + 12)//行
				for (int j = 196; j < 832; j = j + 12)//列
				{
					if ((j / 12) % 2 == 0)
					{
						if ((x / 12) % 2 == 0)WritePixel(x, j, image, 0);
						else WritePixel(x, j, image, 255);
					}
					else
					{
						if ((x / 12) % 2 == 0)WritePixel(x, j, image, 255);
						else WritePixel(x, j, image, 0);
					}
				}//灰白交替
			for (x; x < 782; x = x + 12)//行
				for (int j = 292;j < 736;j = j + 12)//列
				{
					if ((j / 12) % 2 == 0)
					{
						if ((x / 12) % 2 == 0)WritePixel(x, j, image, 0);
						else WritePixel(x, j, image, 255);
					}
					else
					{
						if ((x / 12) % 2 == 0)WritePixel(x, j, image, 255);
						else WritePixel(x, j, image, 0);
					}
				}//灰白交替
		}
		imwrite(imageName, image);
		imageNum++;
	}

}

//图像编码为视频
string ImageToVideo()
{
	string finalInstrcut;


	cout << "请输入编码后的视频名称：";
	string Name;
	cin >> Name;
	cout << "请输入编码后视频允许的最大长度：";
	string VideoLen;
	cin >> VideoLen;

	string str1 = "ffmpeg -f image2 -r 20 -i ";
	string str2 = " -vcodec mpeg4 -t ";
	string str3 = " Video\\";
	finalInstrcut = str1 + "Image\\%d.jpg " + str2 + VideoLen + str3 + Name;
	return finalInstrcut;
}

int main()
{

	//用于存储解码后的二进制数据流
	vector<int>decodeByte;
	//将要编码的字符全部转化为二维码图片
	DrawQRcode();

	cout << endl << "已将二进制文件编码为图片，请按照提示输入相关信息" << endl;
	cout << endl;

	//将生成的二维码图片转化为视频
	string Instruct;
	Instruct = ImageToVideo();
	WinExec("cmd", SW_NORMAL);
	LPCSTR instruct = Instruct.c_str();
	system(instruct);

	cout << endl << "已完成视频编码";


	return 0;
}