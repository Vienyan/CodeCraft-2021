#pragma once
#ifndef __FILEOPERATION_H__
#define __FILEOPERATION_H__

#include "Server.h"
#include "VM.h"

#include <string>
#include <vector>
#include <map>

using namespace std;

struct Migration
{
	int VMid;
	int SERid;
	char SERnode;
};

struct Answer {
	int purchase = 0;
	map<string, int> needToBuy;
	//vector<string> needToBuy;
	int migration = 0;
	
	vector<Migration> MigraData;
	vector<pair<int, char>> putVM;
};

struct Record {
	bool AorD;
	string typeName;
	int ID;
};

class FileOperation {
public:
	int T;
	int K;

	FileOperation(void);
	void ReadFile(vector<Server> &candidateServer, map<string, VM>&candidateVM, vector<vector<Record>>&myRecord);
	void ReadFile2(vector<Server>& candidateServer, map<string, VM>& candidateVM, vector<vector<Record>>& myRecord, string fileName);

};
#endif