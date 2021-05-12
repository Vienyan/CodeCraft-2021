#pragma once
#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <string>
#include "FileOperation.h"

using namespace std;
class Process {
public:
	int T;
	int K;
	void setT(int t, int k) { this->T = t; this->K = k; }

	vector<int> putVMsort;
	int putVMindex = 0;

	vector<Server> myServer;//已购买的现有的服务器
	vector<Server> myServer2;//原TodayBuyServer，即今天购买的服务器
	map<int, VM> myVM;//现有的虚拟机
	vector<Record> myRecordTodaySort;
	vector<pair<int, int>> mmapForServer;
	//map<int, int> mmapForServer;
	vector<int> VMneedModiInServerID;
	vector<pair<VM, int>> buyTogether;


	Process(void);
	void handleProcess(vector<Answer>& ans, vector<Server>& candidateServer, map<string, VM>& candidateVM, vector<vector<Record>>& myRecord);
	void everyDayNeedToDo(vector<Server>& candidateServer, map<string, VM>& candidateVM, vector<vector<Record>>& myRecord, int DayIndex,int T);
	void everyDayNeedToDo2(vector<Server>& candidateServer, map<string, VM>& candidateVM, vector<vector<Record>>& myRecord, int DayIndex, int T,double& todayNeedType);
	void migrationVM(Answer& ans);
	bool findAvailableServer(Answer& ans, VM& VMnew);
	void buyNewServer(vector<Server>& candidateServer, VM& VMnew, int putIndex, Answer& ans,int& buyCnt);
	void buyNewServer2(vector<Server>& candidateServer, Answer& ans, int& buyCnt, double todayNeedType);
	void deleteVM(VM& VMneedToDel, vector<Server>& myServer);
	void updatePeerDay(Answer& ans);
	void output(vector<Answer>& ans, int day_idx);
	void output2(vector<Answer>& ans, string fileName, int day_idx);
	vector<int> choseServer(bool isTowNode,int needCore,int needMem,vector<Server> &myServer);

	//add
	void ReadFile(vector<vector<Record>>&myRecord, int day_idx);
};

#endif