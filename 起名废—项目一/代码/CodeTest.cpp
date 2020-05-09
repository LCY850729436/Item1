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

//���ַ��ĸ����ַ�ת��Ϊ8λ����������
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


//���ı���������ַ�ת��Ϊ8λ���������ݣ�����������18λȫΪ0��������Ϊ������־λ
void TxtToByte(vector<int>&Text, vector<int>&BinaryByte)
{
	for (int i = 0;i < Text.size();i++)
	{
		int a[8] = { -1 };
		StrToByte(a, Text[i]);
		for (int j = 7;j >= 0;j--)BinaryByte.push_back(a[j]);
		//ÿ8λ����һ����żУ���룬������У��
		int count = 0;//��¼1�ĸ���
		for (int j = 0;j < 8;j++)if (a[j] == 1)count++;
		if (count % 2 == 0)BinaryByte.push_back(1);//1�ĸ���Ϊż������żУ����Ϊ1
		else BinaryByte.push_back(0);//1�ĸ���Ϊ��������żУ����Ϊ0
		if (i > 60000)break;//�ļ�������ȫ��������ɺܶ಻�ܱ���Ϊ��Ƶ��ͼƬ
	}
	for (int i = 0;i < 18;i++)BinaryByte.push_back(0);
}


//���ƶ�ά�������(����Ϊ�������ص�12*12��)
void WritePixel(int rows, int cols, Mat image, int color)
{
	for (int i = rows;i < rows + 12;i++)
		for (int j = cols;j < cols + 12;j++)
			image.at<uchar>(i, j) = color;
}

//���ơ��ء���
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

//���ƶ�ά����ĸ����ء��ֶ���
void WritePto(Mat image)
{
	WritePoint(196, 146, image);//����
	WritePoint(748, 146, image);//����
	WritePoint(196, 698, image);//����
	WritePoint(748, 698, image);//����
}


//���ƶ�ά��,0����Ϊ��ɫ��1����Ϊ��ɫ
void DrawQRcode()
{
	string FilePath;

	vector<int>BinaryByte;
	vector<int>Text;
	cout << "������Ҫ������ı���ַ��";
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
	int size = BinaryByte.size();//��ֹ�������������������ݵ�ɫ��
	int num = 0;//����Զ��ٶ��������ݱ���
	int count = 0;//���ڼ��������ʽ���
	int imageNum = 1;//���ڼ��������ͼƬ��
	int x, y;//�����ά���������ʱ������
	while (size != 0)
	{
		//����900*1000���صĴ��׵�ɫͼ���ڻ��ƶ�ά��
		Mat image(900, 1000, CV_8UC1, 255);

		//�����ĸ���λ��
		WritePto(image);

		//���б��뿪ʼ���ƶ�ά��
		//�ڵ�һ�п�ͷ����һ��7λͬ����������ظ�֡������
		for (int j = 292;j < 376;j = j + 12)
		{
			if (imageNum % 2 == 1)WritePixel(146, j, image, 255);//�������Ϊ��������ȫΪ��ɫ
			else WritePixel(146, j, image, 0);//���Ϊ˫������ȫΪ��ɫ
		}
		for (int j = 376;j < 736 && num < size;j = j + 12)
		{
			if (BinaryByte[count] == 0)WritePixel(146, j, image, 0);//��
			else WritePixel(146, j, image, 255);//��
			count++;
			num++;
			x = 146;
			y = j;
		}
		size = size - num;
		num = 0;
		for (int i = 158;i < 242 && num < size;i = i + 12)//��
			for (int j = 292;j < 736 && num < size;j = j + 12)//��
			{
				if (BinaryByte[count] == 0)WritePixel(i, j, image, 0);//��
				else WritePixel(i, j, image, 255);//��
				count++;
				num++;
				x = i;
				y = j;
			}

		for (int i = 242;i < 686 && num < size;i = i + 12)//��
			for (int j = 196;j < 832 && num < size;j = j + 12)//��
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
		for (int i = 686;i < 782 && num < size;i = i + 12)//��
			for (int j = 292;j < 736 && num < size;j = j + 12)//��
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
		string str = "Image\\";
		string imageName = str + res + ".jpg";
		//��������ȫ�����ƽ���ά�������ά��û�����������ú�ɫ�Ͱ�ɫ�����������ڶ�λʶ��
		if (size == 0)
		{
			y = y + 12;
			//��û������һ������
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
			//����ʣ�µ�����
			for (x; x < 242; x = x + 12)//��
				for (int j = 292;j < 736;j = j + 12)//��
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
				}//�Ұ׽���
			for (x; x < 686; x = x + 12)//��
				for (int j = 196; j < 832; j = j + 12)//��
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
				}//�Ұ׽���
			for (x; x < 782; x = x + 12)//��
				for (int j = 292;j < 736;j = j + 12)//��
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
				}//�Ұ׽���
		}
		imwrite(imageName, image);
		imageNum++;
	}

}

//ͼ�����Ϊ��Ƶ
string ImageToVideo()
{
	string finalInstrcut;


	cout << "�������������Ƶ���ƣ�";
	string Name;
	cin >> Name;
	cout << "������������Ƶ�������󳤶ȣ�";
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

	//���ڴ洢�����Ķ�����������
	vector<int>decodeByte;
	//��Ҫ������ַ�ȫ��ת��Ϊ��ά��ͼƬ
	DrawQRcode();

	cout << endl << "�ѽ��������ļ�����ΪͼƬ���밴����ʾ���������Ϣ" << endl;
	cout << endl;

	//�����ɵĶ�ά��ͼƬת��Ϊ��Ƶ
	string Instruct;
	Instruct = ImageToVideo();
	WinExec("cmd", SW_NORMAL);
	LPCSTR instruct = Instruct.c_str();
	system(instruct);

	cout << endl << "�������Ƶ����";


	return 0;
}