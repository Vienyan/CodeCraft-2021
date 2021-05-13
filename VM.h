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
	//固有参数
	string type;
	int coreSize;
	int memorySize;
	bool isBinode;

	//实例化之后的参数
	int ID;
	int inServerID = 0;
	//int inServerIDtoday = -1;
	char AorB;
};

#endif