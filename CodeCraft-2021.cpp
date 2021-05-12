// CodeCraft-2021.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include "Server.h"
#include "VM.h"
#include "FileOperation.h"
#include "Process.h"
 #include "time.h"
 #include <ctime>
// #include "FileOperation.cpp"
// #include "Process.cpp"

using namespace std;

int main()
{
	//clock_t startTime, endTime;
	//startTime = clock();//计时开始
	//srand((unsigned)time(NULL));

	// TODO:read standard input
	vector<Server> candidateServer;
	map<string,VM> candidateVM;
	vector<vector<Record>> myRecord;
	FileOperation myFileOperation;
	//myFileOperation.ReadFile2(candidateServer, candidateVM, myRecord, "training-1.txt");//读文件
	myFileOperation.ReadFile(candidateServer, candidateVM, myRecord);//读cin
	vector<Answer> ans;

	// TODO:process
	Process myProcess;
	myProcess.setT(myFileOperation.T, myFileOperation.K);
	myProcess.handleProcess(ans, candidateServer, candidateVM, myRecord);

	// TODO:write standard output
	// TODO:fflush(stdout);

	//endTime = clock();//计时结束
	//cout << "The run time is: " << (double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << endl;

	return 0;
}
