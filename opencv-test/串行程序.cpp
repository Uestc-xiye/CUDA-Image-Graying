#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>
#include <iostream>
#include <string>
#include <cassert>
#include <vector>
#include <math.h>
#include <time.h>
#include <io.h>
#include <chrono>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs/legacy/constants_c.h"

#include <cuda.h>
#include <cuda_runtime.h>
#include <cuda_runtime_api.h>

using namespace cv;
using namespace std;

#define THREAD_NUM 256

//����ת���Ҷ�ͼ��
void rgb2grayincpu(unsigned char* const d_in, unsigned char* const d_out, uint imgheight, uint imgwidth) {
	//ʹ������ѭ��Ƕ��ʵ��x�����y����ı任
	for (int i = 0; i < imgheight; i++) {
		for (int j = 0; j < imgwidth; j++) {
			d_out[i * imgwidth + j] = 0.299f * d_in[(i * imgwidth + j) * 3]
				+ 0.587f * d_in[(i * imgwidth + j) * 3 + 1]
				+ 0.114f * d_in[(i * imgwidth + j) * 3 + 2];
		}
	}
}


int Initfunc(string inputfilename, double& cpusumtime) {

	/*ͼƬ����Ԥ����*/
	//����ͼƬ
	Mat srcImg = imread(inputfilename);
	FILE* fp;//��������ʱ���ļ�

	//��ȡͼƬ����ֵ
	int imgHeight = srcImg.rows;
	int imgWidth = srcImg.cols;

	Mat grayImg(imgHeight, imgWidth, CV_8UC1, Scalar(0));    //����Ҷ�ͼ
	int hist[256];    //�Ҷ�ֱ��ͼͳ������
	memset(hist, 0, 256 * sizeof(int));    //�ԻҶ�ֱ��ͼ�����ʼ��Ϊ0


	/*CPU���п�ʼ*/
	//���лҶȻ�
	//��ʱ��ʼ
	auto cpustart = chrono::system_clock::now();
	//����������
	rgb2grayincpu(srcImg.data, grayImg.data, imgHeight, imgWidth);

	vector<int> compression_params;
	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);

	//��ʱ����
	auto cpuend = chrono::system_clock::now();
	//����ʱ���
	auto cpuduration = chrono::duration_cast<chrono::microseconds>(cpuend - cpustart);
	double cput = cpuduration.count();
	//΢��ת��Ϊ��
	double cputime = cput / 1000000;
	cpusumtime += cputime;
	//��ӡ����ִ��ʱ��
	cout << setiosflags(ios::fixed) << setprecision(10) << "cpu  exec time�� " << cputime << " s" << endl;
	//printf("cpu  exec time is %.10lg s\n", cputime / 1000000);

	/*����Ҷ�ͼƬ*/
	try {
		int len = inputfilename.length();
		cout << "inputfilename.length:" << len << endl;
		string str = "./GrayPicture/";
		imwrite(str + inputfilename.substr(10, len - 14) + "_to_gray.png", grayImg, compression_params);
		cout << str + inputfilename.substr(10, len - 14) + "_to_gray.png" << endl;

		//��GrayPicture�ļ����У����ɻҶȱ任��Ľ��ͼƬ
	}
	catch (runtime_error& ex) {
		fprintf(stderr, "ͼ��ת����PNG��ʽ��������%s\n", ex.what());
		return 1;
	}
	return 0;
}

//������ȡͼƬ
void getFiles(string path, vector<string>& files) {
	//�ļ����
	intptr_t hFile = 0;
	//�ļ���Ϣ
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("\\*").c_str(), &fileinfo)) != -1) {
		do {
			//�����Ŀ¼,����֮
			//�������,�����б�
			if ((fileinfo.attrib & _A_SUBDIR)) {
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					getFiles(p.assign(path).append("\\").append(fileinfo.name), files);
			}
			else {
				files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}


int main() {
	//ͼƬ�ļ�·��������Ŀ�ļ��µ�Picture�ļ�������
	string filePath = "./Picture";
	vector<string> files;
	//��ȡͼƬ�ļ�
	getFiles(filePath, files);
	//��ȡͼƬ����
	int size = files.size();
	//���ͼƬ����
	cout << "ͼƬ������" << size << endl;

	double cpusumtime = 0;
	for (int i = 0; i < size; i++) {
		cout << "�� " << i + 1 << "/" << size << " ��ͼƬ" << endl;
		cout << files[i].c_str() << endl;
		Initfunc(files[i].c_str(), cpusumtime);
		cout << endl;
	}

	cout << "cpusumtime��" << cpusumtime << " s" << endl;

	return 0;
}
