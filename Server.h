#pragma once
#ifndef __SERVER_H__
#define __SERVER_H__

#include <string>
#include <map>
using namespace std;
class Server {
public:
	//服务器固有参数
	string type;
	int coreSize;
	int coreSizeHalf;
	int memorySize;
	int memorySizeHalf;
	int hardWareCost;
	int energyCost;

	//实例化之后的参数
	int ID;//第几台服务器，如果是今天刚购买的，没来得及编号，则是ID=-1, -2, -3：第-1台，第-2台，第-3台
	int coreAre, coreBre;
	int memAre, memBre;

	//可能需要用到的参数，但目前没有
	map<int,char> VMinServer;		//目前该服务器上有哪些虚拟机,存放在A节点或B节点
	//bool Power;						//是否处于开机状态

};

#endif