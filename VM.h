#pragma once
#ifndef __VM_H__
#define __VM_H__

#include <string>

using namespace std;


//struct VMpara
//{
//	int coreSize;
//	int memorySize;
//	bool binode;
//
//	int ID;
//};

class VM {
public:
	//���в���
	string type;
	int coreSize;
	int memorySize;
	bool isBinode;

	//ʵ����֮��Ĳ���
	int ID;
	int inServerID = 0;
	//int inServerIDtoday = -1;
	char AorB;
};

#endif