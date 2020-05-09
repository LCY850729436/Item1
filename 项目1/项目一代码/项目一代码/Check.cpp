#include<iostream>
#include<string>
#include<fstream>
#include<vector>
#include<math.h>

using namespace std;

//将字符的各个字符转化为8位二进制数据
void StrToByte(int a[8], char ch)
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
void TxtToByte(vector<char>&Text, vector<int>&BinaryByte)
{
	for (int i = 0;i < Text.size();i++)
	{
		int a[8] = { -1 };
		StrToByte(a, Text[i]);
		for (int j = 7;j >= 0;j--)BinaryByte.push_back(a[j]);
	}
}

int main()
{
	string FilePath;
	vector<int>InByte;//存储编码二进制文件数据
	vector<int>OutByte;//存储解码后的二进制文件数据

	vector<char>TextIn;
	vector<char>TextOut;
	cout << "请输入编码二进制文件地址：";
	cin >> FilePath;
	ifstream fin(FilePath);
	char ch;//用于暂存从文件读出来的单个字符
	fin >> noskipws;//读取文件时不忽略空格和换行
	while (fin >> ch)TextIn.push_back(ch);
	TxtToByte(TextIn, InByte);
	fin.close();
	cout << TextIn.size() << endl;
	cout << InByte.size() << endl;

	cout << "请输入解码二进制文件地址：";
	cin >> FilePath;
	ifstream finOut(FilePath);
	finOut >> noskipws;//读取文件时不忽略空格和换行
	while (finOut >> ch)TextOut.push_back(ch);
	TxtToByte(TextOut, OutByte);
	finOut.close();
	cout << TextOut.size() << endl;
	cout << OutByte.size() << endl;

	cout << "请输入对比文件名字：";
	cin >> FilePath;
	ofstream fout(FilePath);
	int count = 0;
	for (int i = 0;i < InByte.size() && i < OutByte.size();i += 8)
	{
		int a[8] = { 0 };
		for (int j = 0;j < 8;j++)if (InByte[i + j] == OutByte[i + j])a[j] = 1;
		int num = 0;
		for (int j = 7;j >= 0;j--)num = num + a[j] * pow(2, 7 - j);
		char ch = char(num);
		fout << ch;
		count+=8;
	}
	for (int i = count;i < InByte.size();i = i + 8)
	{
		int num = 1;
		char ch = char(num);
		fout << ch;
	}
	fout.close();
	
	

	return 0;
}