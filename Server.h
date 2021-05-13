#pragma once
#ifndef __SERVER_H__
#define __SERVER_H__

#include <string>
#include <map>
using namespace std;
class Server {
public:
	//���������в���
	string type;
	int coreSize;
	int coreSizeHalf;
	int memorySize;
	int memorySizeHalf;
	int hardWareCost;
	int energyCost;

	//ʵ����֮��Ĳ���
	int ID;//�ڼ�̨������������ǽ���չ���ģ�û���ü���ţ�����ID=-1, -2, -3����-1̨����-2̨����-3̨
	int coreAre, coreBre;
	int memAre, memBre;

	//������Ҫ�õ��Ĳ�������Ŀǰû��
	map<int,char> VMinServer;		//Ŀǰ�÷�����������Щ�����,�����A�ڵ��B�ڵ�
	//bool Power;						//�Ƿ��ڿ���״̬

};

#endif