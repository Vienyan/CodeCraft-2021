#include <iostream>
#include <vector>
#include<string>
#include <sstream>
#include <fstream>
#include "FileOperation.h"
using namespace std;

FileOperation::FileOperation(void){
	
}

void FileOperation::ReadFile(vector<Server>& candidateServer, map<string,VM>& candidateVM, vector<vector<Record>>& myRecord)
{
	int n;

	//服务器类型
	cin >> n;
	cin.get();
	candidateServer.resize(n);
	for (int i = 0; i < n; ++i) {
		string input = "";
		getline(cin, input);
		input = input.substr(1, input.length() - 2);
		stringstream ss(input);
		string temp;

		getline(ss, temp, ',');
		candidateServer[i].type = temp;
		getline(ss, temp, ',');
		candidateServer[i].coreSize = stoi(temp);
		candidateServer[i].coreSizeHalf = candidateServer[i].coreSize / 2;
		getline(ss, temp, ',');
		candidateServer[i].memorySize = stoi(temp);
		candidateServer[i].memorySizeHalf = candidateServer[i].memorySize / 2;
		getline(ss, temp, ',');
		candidateServer[i].hardWareCost = stoi(temp);
		getline(ss, temp, ',');
		candidateServer[i].energyCost = stoi(temp);
	}
	
	//虚拟机类型
	cin >> n;
	cin.get();
	for (int i = 0; i < n; ++i) {
		string input = "";
		getline(cin, input);
		input = input.substr(1, input.length() - 2);
		stringstream ss(input);
		string temp;

		getline(ss, temp, ',');
		string tempName = temp;
		getline(ss, temp, ',');
		candidateVM[tempName].coreSize = stoi(temp);
		getline(ss, temp, ',');
		candidateVM[tempName].memorySize = stoi(temp);
		getline(ss, temp, ',');
		if (stoi(temp) == 1) {
			candidateVM[tempName].isBinode = true;
		}else{
			candidateVM[tempName].isBinode = false;
		}
	}

	//T天中前K天的请求序列
	cin >> this->T >> this->K;
	myRecord.resize(this->T);
	for (int i = 0; i < this->K; ++i) {
		int m = 0;
		cin >> m;
		cin.get();
		myRecord[i].resize(m);
		for (int j = 0; j < m; ++j) {
			string input = "";
			getline(cin, input);
			input = input.substr(1, input.length() - 2);
			stringstream ss(input);
			string temp;
			getline(ss, temp, ',');
			if (temp == "add") {
				myRecord[i][j].AorD = true;
				getline(ss, temp, ',');
				myRecord[i][j].typeName = temp.substr(1, temp.length() - 1);
				getline(ss, temp, ',');
				myRecord[i][j].ID = stoi(temp);
			}
			else {
				myRecord[i][j].AorD = false;
				getline(ss, temp, ',');
				myRecord[i][j].ID = stoi(temp);
			}
		}
	}

}


void FileOperation::ReadFile2(vector<Server>& candidateServer, map<string, VM>& candidateVM, vector<vector<Record>>& myRecord, string fileName)
{
	//string fileName = "training-2.txt";
	ifstream inFile(fileName, ios::in);

	int n;

	//服务器类型
	string lineStr;
	getline(inFile, lineStr);
	n = stoi(lineStr);
	candidateServer.resize(n);
	for (int i = 0; i < n; ++i) {
		string input = "";
		getline(inFile, input);
		input = input.substr(1, input.length() - 2);
		stringstream ss(input);
		string temp;

		getline(ss, temp, ',');
		candidateServer[i].type = temp;
		getline(ss, temp, ',');
		candidateServer[i].coreSize = stoi(temp);
		candidateServer[i].coreSizeHalf = candidateServer[i].coreSize / 2;
		getline(ss, temp, ',');
		candidateServer[i].memorySize = stoi(temp);
		candidateServer[i].memorySizeHalf = candidateServer[i].memorySize / 2;
		getline(ss, temp, ',');
		candidateServer[i].hardWareCost = stoi(temp);
		getline(ss, temp, ',');
		candidateServer[i].energyCost = stoi(temp);
	}

	//虚拟机类型
	getline(inFile, lineStr);
	n = stoi(lineStr);
	for (int i = 0; i < n; ++i) {
		string input = "";
		getline(inFile, input);
		input = input.substr(1, input.length() - 2);
		stringstream ss(input);
		string temp;

		getline(ss, temp, ',');
		string tempName = temp;
		getline(ss, temp, ',');
		candidateVM[tempName].coreSize = stoi(temp);
		getline(ss, temp, ',');
		candidateVM[tempName].memorySize = stoi(temp);
		getline(ss, temp, ',');
		if (stoi(temp) == 1) {
			candidateVM[tempName].isBinode = true;
		}
		else {
			candidateVM[tempName].isBinode = false;
		}
	}

	//T天中前K天的请求序列
	getline(inFile, lineStr);
	stringstream tmp_ss(lineStr);
	string tmp_str;
	getline(tmp_ss, tmp_str, ' ');
	this->T = stoi(tmp_str);
	getline(tmp_ss, tmp_str, ' ');
	this->K = stoi(tmp_str);
	myRecord.resize(this->T);
	for (int i = 0; i < this->T; ++i) {
		int m = 0;
		getline(inFile, lineStr);
		m = stoi(lineStr);
		myRecord[i].resize(m);
		for (int j = 0; j < m; ++j) {
			string input = "";
			getline(inFile, input);
			input = input.substr(1, input.length() - 2);
			stringstream ss(input);
			string temp;
			getline(ss, temp, ',');
			if (temp == "add") {
				myRecord[i][j].AorD = true;
				getline(ss, temp, ',');
				myRecord[i][j].typeName = temp.substr(1, temp.length() - 1);
				getline(ss, temp, ',');
				myRecord[i][j].ID = stoi(temp);
			}
			else {
				myRecord[i][j].AorD = false;
				getline(ss, temp, ',');
				myRecord[i][j].ID = stoi(temp);
			}
		}
	}

}
