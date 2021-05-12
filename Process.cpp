#include "Process.h"
#include <iostream>
#include <unordered_map>
#include <stdlib.h>
#include <time.h> 
#include <iterator>
#include <set>
#include <algorithm>
#include <fstream>
#include <sstream>


const double thresholdRateMax = 0.93;		//迁移的利用率阈值
const double rateReduceSpeed = 7500;		//控制利用率阈值的下降速度
const double energyTransformMax = 0.06;		//控制因电费高而迁移的服务器数量
const double typeRate = 0.85;			//划分cpu型和内存型的分界点
const int serverSizeRate = 100;				//设定服务器的容量大小在前期的比重，越大则越容易购买容量大的服务器
const int SizeUseTime = 150;				//用这个来控制服务器大小对购买偏好的影响时间，这个越大，则容量的影响下降越快，越快消失
const double energyExpend = 1.2;			//前期电费的比重扩大
const double energyReduce = 0.3;			//后期电费比重减小

Process::Process(void){
	//构造函数

	this->putVMindex = 0;
	putVMsort.clear();
}

//处理请求的主函数
void Process::handleProcess(vector<Answer>& ans, vector<Server>& candidateServer, 
	map<string, VM>& candidateVM, vector<vector<Record>>& myRecord){
	ans.resize(this->T);//初始化答案的规格，多少天就多少条记录

	//vector<Server> myServer;//已购买的现有的服务器
	//map<int, VM> myVM;//现有的虚拟机
	//vector<Server> todayBuyServer;
	//vector<Record> myRecordTodaySort;
	////map<int, int> mmapForServer;
	//vector<pair<int, int>> mmapForServer;
	//vector<int> VMneedModiInServerID;
	//vector<pair<VM, int>> buyTogether;

	//处理掉一天，再读取新的一天，直至T天
	int day_idx = this->K;
	for (int i = 0; i < T; ++i) {
		//开始第i天的操作，下面是每天要声明的变量
		myServer2.clear();
		myServer2.push_back(Server());//为了下标一致，先放置一台空的
		myRecordTodaySort.clear();
		mmapForServer.clear();//为了对服务器的购买优先级所做的排序
		//为了解决我们的购买顺序跟判题器的所认为的购买顺序不同，引入以下变量，每天结束的时候更新
		VMneedModiInServerID.clear();
		//存虚拟机ID：这些虚拟机所对应的服务器是新买的，所以在今天结束之后，需要对今天购买的服务器重新编号，所以这些虚拟机所对应的服务器需要改
		int buyCnt = 1;//今天买的第几台服务器

		//下面每一天开始操作前，要做的预备计算
		putVMsort.clear();
		this->putVMindex = 0;
		double todayNeedType = 0.0;
		//everyDayNeedToDo(candidateServer, candidateVM, myRecord, i, myRecordTodaySort,T);
		everyDayNeedToDo2(candidateServer, candidateVM, myRecord, i, T, todayNeedType);
		ans[i].putVM.resize(putVMsort.size());

		//服务器迁移操作
		migrationVM(ans[i]);

		//开始当天的第j个订单，即myRecord[i][j]是第i天的第j条要处理的记录
		buyTogether.clear();
		for (int j = 0; j < myRecordTodaySort.size(); ++j) {
			if (myRecordTodaySort[j].AorD) {//如果是add虚拟机
				//获取要添加的虚拟机的信息，存在VMnew中
				VM VMnew = candidateVM[myRecordTodaySort[j].typeName];
				VMnew.type = myRecordTodaySort[j].typeName;
				VMnew.ID = myRecordTodaySort[j].ID;
				bool hasPutVM = false;//是否放置成功的标志位

				//先找以前买的服务器看能不能用
				hasPutVM = findAvailableServer(ans[i],VMnew);
				if (hasPutVM == false) {
					buyTogether.push_back(make_pair(VMnew, putVMsort[this->putVMindex++]));
				}
				if (buyTogether.size() >= 3 || j == myRecordTodaySort.size() - 1 || myRecordTodaySort[j+1].AorD==false) {
					if (!buyTogether.empty()) {
						buyNewServer2(candidateServer, ans[i], buyCnt, todayNeedType);
						buyTogether.clear();
					}
				}
			}
			else {//如果是del虚拟机
				if (j== myRecordTodaySort.size()-1 && !buyTogether.empty()) {
					buyNewServer2(candidateServer, ans[i], buyCnt, todayNeedType);
					buyTogether.clear();
				}
				VM VMneedToDel = myVM[myRecordTodaySort[j].ID];
				myVM.erase(myRecordTodaySort[j].ID);
				if (VMneedToDel.inServerID < 0) {
					//如果要删的虚拟机的服务器是今天刚买的
					deleteVM(VMneedToDel, myServer2);
				}
				else {
					//如果要删的虚拟机的服务器不是今天刚买的
					deleteVM(VMneedToDel, myServer);
				}
			}
		}

		//今天结束，将今天买的服务器重新编号与现有的合并，然后更新今天加入的虚拟机的所在服务器编号，对ans也要更新
		updatePeerDay(ans[i]);

		//当你的程序完整输出了一天的决策信息后，请务必清空输出缓存区
		fflush(stdout);

		//处理cin
		this->output(ans, i);
		this->ReadFile(myRecord, day_idx++);

		////处理txt
		//this->output2(ans, "result.txt", i);
	}
}


void Process::everyDayNeedToDo(vector<Server>& candidateServer, map<string, VM>& candidateVM, vector<vector<Record>>& myRecord, int DayIndex, int T)
{
	//对可购买的服务器清单进行排序，确定购买的优先级
	//for (int j = 0; j < candidateServer.size(); ++j) {
	//	int cost = candidateServer[j].hardWareCost + candidateServer[j].energyCost * (T + 1 - DayIndex);
	//	mmapForServer[cost] = j;
	//}
	mmapForServer.resize(candidateServer.size());
	int idx = 0;

	for (int j = 0; j < candidateServer.size(); ++j) {
		int cost = candidateServer[j].hardWareCost + candidateServer[j].energyCost * (T + 1 - DayIndex) * (energyExpend - energyReduce * DayIndex / T);
		//mmapForServer[cost] = j;
		int size = (serverSizeRate - SizeUseTime* DayIndex / T)*(candidateServer[j].coreSize + candidateServer[j].memorySize);
		if(size >0){
			cost = cost - size;
		}	
		mmapForServer[idx++] = make_pair(cost, j);
	}
	sort(mmapForServer.begin(), mmapForServer.end());


	//对今天的虚拟机请求进行排序
	vector<Record> todayRecord = myRecord[DayIndex];
	myRecordTodaySort.resize(todayRecord.size());
	int Test_putVMcnt = 0; //这个参数为了调试，后期可以删
	int i = 0;
	int DelCnt = 0;

	vector<pair<int, int>> SortBySize;
	for (int j = 0; j < todayRecord.size(); ++j) {
		if (todayRecord[j].AorD == true) {//add
			++Test_putVMcnt;
			VM VMnew = candidateVM[todayRecord[j].typeName];
			int VMSize = 3*VMnew.coreSize + VMnew.memorySize;//3是权重系数
			SortBySize.push_back(make_pair(VMSize, j));

			if (j == todayRecord.size() - 1) {
				sort(SortBySize.begin(), SortBySize.end(), greater<pair<int, int>>{});
				for (int k = 0; k < SortBySize.size(); ++k) {
					myRecordTodaySort[i++] = todayRecord[SortBySize[k].second];
					putVMsort.push_back(SortBySize[k].second - DelCnt);
				}
				SortBySize.clear();
			}

		}
		else {//del
			sort(SortBySize.begin(), SortBySize.end(), greater<pair<int, int>>{});
			for (int k = 0; k < SortBySize.size(); ++k) {
				myRecordTodaySort[i++] = todayRecord[SortBySize[k].second];
				putVMsort.push_back(SortBySize[k].second - DelCnt);
			}
			SortBySize.clear();
			++DelCnt;
			myRecordTodaySort[j] = todayRecord[j];
			++i;
		}
	}
}

void Process::everyDayNeedToDo2(vector<Server>& candidateServer, map<string, VM>& candidateVM, 
	vector<vector<Record>>& myRecord, int DayIndex, int T,double& todayNeedType)
{
	//对可购买的服务器清单进行排序，确定购买的优先级(前期电费支出的比重应该增强,后期比重减轻，前期更偏买大的，后期偏买小的)

	//for (int j = 0; j < candidateServer.size(); ++j) {
	//	int cost = candidateServer[j].hardWareCost + candidateServer[j].energyCost * (T + 1 - DayIndex) * (1.2 - 0.3 * DayIndex/T);
	//	mmapForServer[cost] = j;
	//}
	mmapForServer.resize(candidateServer.size());
	int idx = 0;

	for (int j = 0; j < candidateServer.size(); ++j) {
		int cost = candidateServer[j].hardWareCost + candidateServer[j].energyCost * (T + 1 - DayIndex) * (energyExpend - energyReduce * DayIndex / T);
		//mmapForServer[cost] = j;
		int size = (serverSizeRate - SizeUseTime* DayIndex / T)*(candidateServer[j].coreSize + candidateServer[j].memorySize);
		if(size >0){
			cost = cost - size;
		}
		mmapForServer[idx++] = make_pair(cost, j);
	}
	sort(mmapForServer.begin(), mmapForServer.end());
	
	//对今天的虚拟机请求进行排序
	vector<Record> todayRecord = myRecord[DayIndex];
	myRecordTodaySort.resize(todayRecord.size());
	int Test_putVMcnt = 0; //这个参数为了调试，后期可以删
	int i = 0;
	int DelCnt = 0;
	bool tt = true;

	vector<pair<int, int>> SortBySize;
	//vector<pair<int, int>> SortBySize2;
	//unordered_map<int, bool> used;
	//vector<bool> used(todayRecord.size(), false);
	int todayNeedCpu = 0;
	int todayNeedMem = 0;

	for (int j = 0; j < todayRecord.size(); ++j) {
		if (todayRecord[j].AorD == true) {//add
			++Test_putVMcnt;
			VM VMnew = candidateVM[todayRecord[j].typeName];
			int VMSize = VMnew.coreSize - VMnew.memorySize;//3是权重系数
			//int VMSize2 = VMnew.memorySize - VMnew.coreSize;
			todayNeedCpu += VMnew.coreSize;
			todayNeedMem += VMnew.memorySize;
			SortBySize.push_back(make_pair(VMSize, j));
			//SortBySize2.push_back(make_pair(VMSize2, j));

			int k1 = 0, k2 = SortBySize.size() - 1;
			if (j == todayRecord.size() - 1) {
				sort(SortBySize.begin(), SortBySize.end(), greater<pair<int, int>>{});
				for (int k = 0; k < SortBySize.size(); ++k) {
					int kk;
					if (k % 2 == 0) {
						kk = k1++;
					}
					else {
						kk = k2--;
					}
					myRecordTodaySort[i++] = todayRecord[SortBySize[kk].second];
					putVMsort.push_back(SortBySize[kk].second - DelCnt);
				}
				SortBySize.clear();
			}

		}
		else {//del
			//todayNeedCpu -= myVM[todayRecord[j].ID].coreSize;
			//todayNeedMem -= myVM[todayRecord[j].ID].memorySize;

			sort(SortBySize.begin(), SortBySize.end(), greater<pair<int, int>>{});
			int k1 = 0, k2 = SortBySize.size() - 1;
			for (int k = 0; k < SortBySize.size(); ++k) {
				int kk;
				if (k % 2 == 0) {
					kk = k1++;
				}
				else {
					kk = k2--;
				}
				myRecordTodaySort[i++] = todayRecord[SortBySize[kk].second];
				putVMsort.push_back(SortBySize[kk].second - DelCnt);
			}
			SortBySize.clear();
			++DelCnt;
			myRecordTodaySort[j] = todayRecord[j];
			++i;
		}
	}
	//预测一下今天所需要的服务器类型
	todayNeedType = double(todayNeedCpu) / todayNeedMem;
}

void Process::migrationVM(Answer& ans)
{
	//把迁移信息写入到ans中
	//ans.migration是迁移的次数
	//ans.MigraData是迁移的信息，其中
	//ans.MigraData[i].VMid是Int类型，表示需要迁移的虚拟机的ID;
	//ans.MigraData[i].SERid是Int类型，表示需要迁移的虚拟机的ID;
	//ans.MigraData[i].SERnode;是char类型，表示㤇迁移的虚拟机节点，A或者B，如果是双节点布置，则为C
	//i是今天第i条要迁移的记录

	//myServer是现有服务器，myVM是现有虚拟机

	//限制：迁移虚拟机数量≤当前虚拟机数量的百分之三，向下取整
	int succeed_count = 0;
	//double thresholdRate = 0.93 - (double)myServer.size() / 1350.0 * 0.01;
	double thresholdRate = thresholdRateMax - (double)myVM.size() / rateReduceSpeed * 0.01;
	int max_energy_trans = myServer.size() * energyTransformMax;			//限制迁移电费最大的遍历服务器数

	int max_count = int(myVM.size()*0.03);
	if (max_count < 1)
		return;

	//准备数据结构
	Migration migra;
	vector<pair<double,int>> used_rate_mp2;
	used_rate_mp2.resize(myServer.size());
	vector<pair<int, int>> used_score_vm;

	if (max_count>10){

		vector<pair<double, int>> used_energy_mp;		//电费的排行
		used_energy_mp.resize(myServer.size());
		int idx_energy=0;
		for (int i = 0; i < myServer.size(); i++) {
			used_energy_mp[idx_energy++] = make_pair(myServer[i].energyCost, myServer[i].ID);
		}
		//先对电费排序,从小到大
		sort(used_energy_mp.begin(), used_energy_mp.end());

		int search_server = 0;
		for (auto it = used_energy_mp.rbegin(); it != used_energy_mp.rend(); it++) {
			int server_id = (*it).second;

			//如果选择的服务器超过了限制
			if (search_server >= max_energy_trans) {
				break;
			}
			search_server++;			//记录搜索了多少个服务器

			//虚拟机排序
			used_score_vm.clear();
			for (auto vm_it = myServer[server_id].VMinServer.begin(); vm_it != myServer[server_id].VMinServer.end(); vm_it++) {
				int vm_id = vm_it->first;
				int used_score = myVM[vm_id].coreSize + myVM[vm_id].memorySize;
				used_score_vm.push_back({ used_score,vm_id });
			}
			sort(used_score_vm.begin(), used_score_vm.end());

			for (auto it_vm = used_score_vm.begin(); it_vm != used_score_vm.end(); it_vm++) {

				int vm_id = (*it_vm).second;
				int need_cpu = myVM[vm_id].coreSize;
				int need_mem = myVM[vm_id].memorySize;
				bool is_double = myVM[vm_id].isBinode;

				bool succeed = false;
				bool flag = false;

				//向电费低的服务器尝试迁移
				for (int j = 0; j < used_energy_mp.size(); j++) {
					int new_server_id = used_energy_mp[j].second;

					if (server_id == new_server_id) {
						flag = true;
						break;
					}
					//终止条件
					if (succeed_count >= max_count)
						return;

					//判断剩余容量是否足够
					if (is_double) {	//双节点
						if (myServer[new_server_id].coreAre >= need_cpu / 2 && myServer[new_server_id].coreBre >= need_cpu / 2 &&
							myServer[new_server_id].memAre >= need_mem / 2 && myServer[new_server_id].memBre >= need_mem / 2) {

							//迁移双节点虚拟机任务
							int add_cpu = need_cpu / 2;
							int add_mem = need_mem / 2;
							myServer[server_id].coreAre += add_cpu;
							myServer[server_id].coreBre += add_cpu;
							myServer[server_id].memAre += add_mem;
							myServer[server_id].memBre += add_mem;
							myServer[server_id].VMinServer.erase(vm_id);

							myServer[new_server_id].coreAre -= add_cpu;
							myServer[new_server_id].coreBre -= add_cpu;
							myServer[new_server_id].memAre -= add_mem;
							myServer[new_server_id].memBre -= add_mem;
							myServer[new_server_id].VMinServer[vm_id] = 'C';
							myVM[vm_id].AorB = 'C';
							myVM[vm_id].inServerID = new_server_id;

							succeed = true;
						}
					}
					else {				//单节点
						if (myServer[new_server_id].coreAre >= need_cpu && myServer[new_server_id].memAre >= need_mem) {

							//迁移单节点虚拟机任务
							if (myVM[vm_id].AorB == 'A') {
								myServer[server_id].coreAre += need_cpu;
								myServer[server_id].memAre += need_mem;
							}
							else {
								myServer[server_id].coreBre += need_cpu;
								myServer[server_id].memBre += need_mem;
							}
							myServer[server_id].VMinServer.erase(vm_id);

							myServer[new_server_id].coreAre -= need_cpu;
							myServer[new_server_id].memAre -= need_mem;
							myServer[new_server_id].VMinServer[vm_id] = 'A';
							myVM[vm_id].AorB = 'A';
							myVM[vm_id].inServerID = new_server_id;

							succeed = true;
						}
						else if (myServer[new_server_id].coreBre >= need_cpu && myServer[new_server_id].memBre >= need_mem) {

							//迁移单节点虚拟机任务
							if (myVM[vm_id].AorB == 'A') {
								myServer[server_id].coreAre += need_cpu;
								myServer[server_id].memAre += need_mem;
							}
							else {
								myServer[server_id].coreBre += need_cpu;
								myServer[server_id].memBre += need_mem;
							}
							myServer[server_id].VMinServer.erase(vm_id);

							myServer[new_server_id].coreBre -= need_cpu;
							myServer[new_server_id].memBre -= need_mem;
							myServer[new_server_id].VMinServer[vm_id] = 'B';
							myVM[vm_id].AorB = 'B';
							myVM[vm_id].inServerID = new_server_id;

							succeed = true;
						}
					}

					if (succeed) {
						//记录一条迁移信息
						migra.VMid = vm_id;
						migra.SERid = myVM[vm_id].inServerID;
						migra.SERnode = myVM[vm_id].AorB;
						ans.MigraData.push_back(migra);
						ans.migration = ans.MigraData.size();
						succeed_count += 1;
						break;
					}

				}
			}
		}		
	}

	//再计算平衡程度
	int idx_rate = 0;			//重置
	for (int i = 0; i < myServer.size(); i++) {
		double rate = (1.0 - double(myServer[i].coreAre + myServer[i].coreBre) / double(myServer[i].coreSize)) + \
			(1.0 - double(myServer[i].memAre + myServer[i].memBre) / double(myServer[i].memorySize));
		rate = rate / 2;
		used_rate_mp2[idx_rate++] = make_pair(rate, myServer[i].ID);
	}
	sort(used_rate_mp2.begin(), used_rate_mp2.end());

	//对利用率最低的服务器内的虚拟机逐个迁移
	for (auto& it : used_rate_mp2) {
		//如果选择的服务器超过了限制
		if (it.first>thresholdRate) {
			return;
		}
		if (it.first <= 0) {
			continue;
		}
		int server_id = it.second;

		//优先迁移资源占用大的虚拟机
		used_score_vm.clear();
		for (auto vm_it = myServer[server_id].VMinServer.begin(); vm_it != myServer[server_id].VMinServer.end(); vm_it++) {
			int vm_id = vm_it->first;
			int used_score = myVM[vm_id].coreSize + myVM[vm_id].memorySize;
			used_score_vm.push_back({ used_score,vm_id });
		}
		sort(used_score_vm.begin(), used_score_vm.end());

		//向利用率高的服务器尝试迁移
		for (auto it_vm = used_score_vm.rbegin(); it_vm != used_score_vm.rend(); it_vm++) {
			int vm_id = (*it_vm).second;
			int need_cpu = myVM[vm_id].coreSize;
			int need_mem = myVM[vm_id].memorySize;
			bool is_double = myVM[vm_id].isBinode;

			bool succeed = false;
			bool flag = false;
			for (int j = used_rate_mp2.size() - 1; j >= 0; --j) {
				int new_server_id = used_rate_mp2[j].second;

				if (server_id == new_server_id) {
					flag = true;
					break;
				}
				//终止条件
				if (succeed_count >= max_count)
					return;

				//判断剩余容量是否足够
				if (is_double) {	//双节点
					if (myServer[new_server_id].coreAre >= need_cpu / 2 && myServer[new_server_id].coreBre >= need_cpu / 2 && 
						myServer[new_server_id].memAre >= need_mem / 2 && myServer[new_server_id].memBre >= need_mem / 2) {

						//迁移双节点虚拟机任务
						int add_cpu = need_cpu / 2;
						int add_mem = need_mem / 2;

						myServer[server_id].coreAre += add_cpu;
						myServer[server_id].coreBre += add_cpu;
						myServer[server_id].memAre += add_mem;
						myServer[server_id].memBre += add_mem;
						myServer[server_id].VMinServer.erase(vm_id);

						myServer[new_server_id].coreAre -= add_cpu;
						myServer[new_server_id].coreBre -= add_cpu;
						myServer[new_server_id].memAre -= add_mem;
						myServer[new_server_id].memBre -= add_mem;
						myServer[new_server_id].VMinServer[vm_id] = 'C';
						myVM[vm_id].AorB = 'C';
						myVM[vm_id].inServerID = new_server_id;

						succeed = true;
					}
				}
				else {				//单节点
					if (myServer[new_server_id].coreAre >= need_cpu && myServer[new_server_id].memAre >= need_mem) {

						//迁移单节点虚拟机任务
						if (myVM[vm_id].AorB == 'A') {
							myServer[server_id].coreAre += need_cpu;
							myServer[server_id].memAre += need_mem;
						}
						else {
							myServer[server_id].coreBre += need_cpu;
							myServer[server_id].memBre += need_mem;
						}
						myServer[server_id].VMinServer.erase(vm_id);

						myServer[new_server_id].coreAre -= need_cpu;
						myServer[new_server_id].memAre -= need_mem;
						myServer[new_server_id].VMinServer[vm_id] = 'A';
						myVM[vm_id].AorB = 'A';
						myVM[vm_id].inServerID = new_server_id;

						succeed = true;
					}
					else if (myServer[new_server_id].coreBre >= need_cpu && myServer[new_server_id].memBre >= need_mem) {
						//迁移单节点虚拟机任务
						if (myVM[vm_id].AorB == 'A') {
							myServer[server_id].coreAre += need_cpu;
							myServer[server_id].memAre += need_mem;
						}
						else {
							myServer[server_id].coreBre += need_cpu;
							myServer[server_id].memBre += need_mem;
						}
						myServer[server_id].VMinServer.erase(vm_id);

						myServer[new_server_id].coreBre -= need_cpu;
						myServer[new_server_id].memBre -= need_mem;
						myServer[new_server_id].VMinServer[vm_id] = 'B';
						myVM[vm_id].AorB = 'B';
						myVM[vm_id].inServerID = new_server_id;

						succeed = true;
					}
				}

				if (succeed) {
					//记录一条迁移信息
					migra.VMid = vm_id;
					migra.SERid = myVM[vm_id].inServerID;
					migra.SERnode = myVM[vm_id].AorB;
					ans.MigraData.push_back(migra);
					ans.migration = ans.MigraData.size();

					succeed_count += 1;
					break;
				}
			}
		}
	}

}



//在现有服务器中，找是否放得下VMnew虚拟机
bool Process::findAvailableServer(Answer& ans, VM& VMnew) {
	bool hasPutVM = false;
	int result = -1;			//存放计算出的是哪个适合

	if (VMnew.isBinode){
		//双节点
		int needCoreEach = VMnew.coreSize / 2;
		int needMemEach = VMnew.memorySize / 2;
		//返回的第一个元素代表选择的是第几号服务器，第二个元素代表gap
		vector<int> result_list1 = choseServer(VMnew.isBinode,VMnew.coreSize,VMnew.memorySize,myServer);
		vector<int> result_list2 = choseServer(VMnew.isBinode,VMnew.coreSize,VMnew.memorySize,myServer2);

		int gap1 = result_list1[1];
		int gap2 = result_list2[1];
		//只要比较这两者的最小值就可以了
		if (result_list1[0] == -1) {
			result = result_list2[0];
		}
		else if (result_list2[0] == -1) {
			result = result_list1[0];
		}
		else {
			result = gap1 < gap2 ? result_list1[0] : result_list2[0];
		}
		if (result != -1){
			//选择的是server里的服务器
			if (gap1<gap2){
				myServer[result].VMinServer[VMnew.ID] = 'C';
				myServer[result].coreAre -= needCoreEach;
				myServer[result].coreBre -= needCoreEach;
				myServer[result].memAre -= needMemEach;
				myServer[result].memBre -= needMemEach;

				VMnew.inServerID = myServer[result].ID;
				ans.putVM[putVMsort[this->putVMindex++]] = (make_pair(myServer[result].ID, 'C'));

				hasPutVM = true;
				VMnew.AorB = 'C';
				myVM[VMnew.ID] = VMnew;
			}else{
				//选择的是server2里的服务器
				myServer2[result].VMinServer[VMnew.ID] = 'C';
				myServer2[result].coreAre -= needCoreEach;
				myServer2[result].coreBre -= needCoreEach;
				myServer2[result].memAre -= needMemEach;
				myServer2[result].memBre -= needMemEach;
				
				VMnew.inServerID = myServer2[result].ID;
				ans.putVM[putVMsort[this->putVMindex++]] = (make_pair(myServer2[result].ID, 'C'));
				VMneedModiInServerID.push_back(VMnew.ID);

				hasPutVM = true;
				VMnew.AorB = 'C';
				myVM[VMnew.ID] = VMnew;
			}
		}
	}
	else {
		//单节点
		int needCore = VMnew.coreSize;
		int needMem = VMnew.memorySize;

		vector<int> result_list1 = choseServer(VMnew.isBinode,needCore,needMem,myServer);
		vector<int> result_list2 = choseServer(VMnew.isBinode,needCore,needMem,myServer2);

		//只要比较这两者的最小值就可以了
		int gap1 = result_list1[1];
		int gap2 = result_list2[1];
		if (result_list1[0] == -1) {
			result = result_list2[0];
		}
		else if (result_list2[0] == -1) {
			result = result_list1[0];
		}
		else {
			result = gap1 < gap2 ? result_list1[0] : result_list2[0];
		}

		//找到合适的服务器
		if (result != -1){
			//是myServer里面的服务器
			if (gap1<gap2){
				//查看是哪个服务器的哪个节点----A节点
				if (result % 2 == 0){
					int s = result / 2;
					myServer[s].VMinServer[VMnew.ID] = 'A';
					myServer[s].coreAre -= needCore;
					myServer[s].memAre -= needMem;

					VMnew.inServerID = myServer[s].ID;
					ans.putVM[putVMsort[this->putVMindex++]] = (make_pair(myServer[s].ID, 'A'));

					hasPutVM = true;
					VMnew.AorB = 'A';
					myVM[VMnew.ID] = VMnew;
				}else{
					//B节点
					int s = (result-1)/ 2;
					myServer[s].VMinServer[VMnew.ID] = 'B';
					myServer[s].coreBre -= needCore;
					myServer[s].memBre -= needMem;

					VMnew.inServerID = myServer[s].ID;
					ans.putVM[putVMsort[this->putVMindex++]] = (make_pair(myServer[s].ID, 'B'));
					
					hasPutVM = true;
					VMnew.AorB = 'B';
					myVM[VMnew.ID] = VMnew;
				}
			}else{
				//取的是myServer2的服务器
				if (result % 2 == 0){
					int s = result / 2;
					myServer2[s].VMinServer[VMnew.ID] = 'A';
					myServer2[s].coreAre -= needCore;
					myServer2[s].memAre -= needMem;

					VMnew.inServerID = myServer2[s].ID;
					ans.putVM[putVMsort[this->putVMindex++]] = (make_pair(myServer2[s].ID, 'A'));
					VMneedModiInServerID.push_back(VMnew.ID);

					hasPutVM = true;
					VMnew.AorB = 'A';
					myVM[VMnew.ID] = VMnew;
				}else{
					//B节点
					int s = (result - 1) / 2;
					myServer2[s].VMinServer[VMnew.ID] = 'B';
					myServer2[s].coreBre -= needCore;
					myServer2[s].memBre -= needMem;

					VMnew.inServerID = myServer2[s].ID;
					ans.putVM[putVMsort[this->putVMindex++]] = (make_pair(myServer2[s].ID, 'B'));
					VMneedModiInServerID.push_back(VMnew.ID);
					
					hasPutVM = true;
					VMnew.AorB = 'B';
					myVM[VMnew.ID] = VMnew;
				}
			}
		}
	}
	return hasPutVM;
}

//购买新的服务器
void Process::buyNewServer(vector<Server>& candidateServer, VM& VMnew, int putIndex, Answer& ans, int& buyCnt) {
	int k = 0;
	for (auto& it :mmapForServer) {
		k = it.second;
		if (VMnew.isBinode){
			if (VMnew.coreSize/2 <= candidateServer[k].coreSizeHalf && VMnew.memorySize/2 <= candidateServer[k].memorySizeHalf) {
				break; 
			}
		}else{
			if (VMnew.coreSize <= candidateServer[k].coreSizeHalf && VMnew.memorySize <= candidateServer[k].memorySizeHalf) {
				break;
			}
		}
	}

	ans.purchase++;//这个参数其实多余了，没什么卵用
	Server tempServer;
	VMneedModiInServerID.push_back(VMnew.ID);
	tempServer = candidateServer[k];
	tempServer.ID = -buyCnt;
	if (VMnew.isBinode) {//双节点
		tempServer.VMinServer[VMnew.ID] = 'C';
		tempServer.coreAre = tempServer.coreBre = tempServer.coreSizeHalf - VMnew.coreSize / 2;
		tempServer.memAre = tempServer.memBre = tempServer.memorySizeHalf - VMnew.memorySize / 2;
		ans.putVM[putIndex] = (make_pair(tempServer.ID, 'C'));
		VMnew.AorB = 'C';
	}
	else {//单节点，默认放在A
		tempServer.VMinServer[VMnew.ID] = 'A';
		tempServer.coreAre = tempServer.coreSizeHalf - VMnew.coreSize;
		tempServer.coreBre = tempServer.coreSizeHalf;
		tempServer.memAre = tempServer.memorySizeHalf - VMnew.memorySize;
		tempServer.memBre = tempServer.memorySizeHalf;
		ans.putVM[putIndex] = (make_pair(tempServer.ID, 'A'));
		VMnew.AorB = 'A';
	}
	VMnew.inServerID = tempServer.ID;
	myVM[VMnew.ID] = VMnew;
	buyCnt++;
	myServer2.push_back(tempServer);
}

void Process::buyNewServer2(vector<Server>& candidateServer, Answer& ans, int& buyCnt, double todayNeedType) {
	bool CanPutAll = false;
	int k = 0;
	int n = buyTogether.size();
	//根据todayNeedType判断今天更需要cpu大的还是内存大的
	double add_mem = 1.0;
	double add_cpu = 1.0;
	if (todayNeedType<typeRate){
		//更需要内存大一些的
		add_mem = add_mem + typeRate - todayNeedType;
	}else{
		add_cpu = add_cpu + todayNeedType - typeRate;
	}

	if (n == 1) {
		VM VMnew = buyTogether[0].first;
		for (auto& it:mmapForServer) {
			k = it.second;
			if (VMnew.isBinode) {
				if (add_cpu * VMnew.coreSize / 2 <= candidateServer[k].coreSizeHalf && add_mem * VMnew.memorySize / 2 <= candidateServer[k].memorySizeHalf) {
					CanPutAll = true;
					break;
				}
			}
			else {
				if (add_cpu * VMnew.coreSize <= candidateServer[k].coreSizeHalf && add_mem * VMnew.memorySize <= candidateServer[k].memorySizeHalf) {
					CanPutAll = true;
					break;
				}
			}
		}
	}
	else if (n == 2) {
		bool isBinodeA = buyTogether[0].first.isBinode;
		bool isBinodeB = buyTogether[1].first.isBinode;
		int c1 = buyTogether[0].first.isBinode ? buyTogether[0].first.coreSize >> 1 : buyTogether[0].first.coreSize;
		int m1 = buyTogether[0].first.isBinode ? buyTogether[0].first.memorySize >> 1 : buyTogether[0].first.memorySize;
		int c2 = buyTogether[1].first.isBinode ? buyTogether[1].first.coreSize >> 1 : buyTogether[1].first.coreSize;
		int m2 = buyTogether[1].first.isBinode ? buyTogether[1].first.memorySize >> 1 : buyTogether[1].first.memorySize;
		int c = 0, m = 0;
		if (isBinodeA == false && isBinodeB == false) {
			c = max(c1, c2);
			m = max(m1, m2);
		}
		else {
			c = c1 + c2;
			m = m1 + m2;
		}

		for (auto& it:mmapForServer) {
			k = it.second;
			if (add_cpu * c <= candidateServer[k].coreSizeHalf && add_mem * m <= candidateServer[k].memorySizeHalf) {
				CanPutAll = true;
				break;
			}
		}
	}
	else {//n==3
		bool isBinodeA = buyTogether[0].first.isBinode;
		bool isBinodeB = buyTogether[1].first.isBinode;
		bool isBinodeC = buyTogether[2].first.isBinode;
		int c1 = buyTogether[0].first.isBinode ? buyTogether[0].first.coreSize >> 1 : buyTogether[0].first.coreSize;
		int m1 = buyTogether[0].first.isBinode ? buyTogether[0].first.memorySize >> 1 : buyTogether[0].first.memorySize;
		int c2 = buyTogether[1].first.isBinode ? buyTogether[1].first.coreSize >> 1 : buyTogether[1].first.coreSize;
		int m2 = buyTogether[1].first.isBinode ? buyTogether[1].first.memorySize >> 1 : buyTogether[1].first.memorySize;
		int c3 = buyTogether[2].first.isBinode ? buyTogether[2].first.coreSize >> 1 : buyTogether[2].first.coreSize;
		int m3 = buyTogether[2].first.isBinode ? buyTogether[2].first.memorySize >> 1 : buyTogether[2].first.memorySize;

		int c, m;
		if (isBinodeA && isBinodeB && isBinodeC) {
			c = c1 + c2 + c3;
			m = m1 + m2 + m3;
		}
		else if(isBinodeA==true && isBinodeB==false&& isBinodeC==false){
			c = max(c1 + c2, c1 + c3);
			m = max(m1 + m2, m1 + m3);
		}
		else if (isBinodeA == false && isBinodeB == true && isBinodeC == false) {
			c = max(c2 + c1, c2 + c3);
			m = max(m2 + m1, m2 + m3);
		}
		else if (isBinodeA == false && isBinodeB == false && isBinodeC == true) {
			c = max(c3 + c1, c3 + c2);
			m = max(m3 + m1, m3 + m2);
		}
		else{
			c = c1+c2+c3;
			m = m1+m2+m3;
		}

		for (auto& it : mmapForServer) {
			k = it.second;
			if (add_cpu * c <= candidateServer[k].coreSizeHalf && add_mem * m <= candidateServer[k].memorySizeHalf) {
				CanPutAll = true;
				break;
			}
		}
	}

	if (CanPutAll) {
		ans.purchase++;//这个参数其实多余了，没什么卵用
		Server tempServer;
		tempServer = candidateServer[k];
		tempServer.ID = -buyCnt;
		tempServer.coreAre = tempServer.coreBre = tempServer.coreSizeHalf;
		tempServer.memAre = tempServer.memBre = tempServer.memorySizeHalf;
		for (auto& d : buyTogether) {
			VM VMnew = d.first;
			int putIndex = d.second;
			VMneedModiInServerID.push_back(VMnew.ID);
			if (VMnew.isBinode) {//双节点
				tempServer.VMinServer[VMnew.ID] = 'C';

				int coreNeedT = VMnew.coreSize >> 1;
				int memNeedT = VMnew.memorySize >> 1;
				tempServer.coreAre -= coreNeedT;
				tempServer.coreBre -= coreNeedT;
				tempServer.memAre -= memNeedT;
				tempServer.memBre -= memNeedT;
				
				ans.putVM[putIndex] = (make_pair(tempServer.ID, 'C'));
				VMnew.AorB = 'C';
			}
			else {//单节点
				if ((tempServer.coreAre > tempServer.coreBre && tempServer.memAre>= VMnew.memorySize) || 
					(tempServer.coreAre == tempServer.coreBre && tempServer.memAre >= tempServer.memBre)) {
					tempServer.VMinServer[VMnew.ID] = 'A';
					tempServer.coreAre -= VMnew.coreSize;
					tempServer.memAre -= VMnew.memorySize;
					ans.putVM[putIndex] = (make_pair(tempServer.ID, 'A'));
					VMnew.AorB = 'A';
				}
				else {
					tempServer.VMinServer[VMnew.ID] = 'B';
					tempServer.coreBre -= VMnew.coreSize;
					tempServer.memBre -= VMnew.memorySize;
					ans.putVM[putIndex] = (make_pair(tempServer.ID, 'B'));
					VMnew.AorB = 'B';
				}

			}
			VMnew.inServerID = tempServer.ID;
			myVM[VMnew.ID] = VMnew;
		}
		buyCnt++;
		myServer2.push_back(tempServer);
	}
	else {
		if (n == 2) {
			buyNewServer(candidateServer, buyTogether[0].first, buyTogether[0].second, ans, buyCnt);
			buyNewServer(candidateServer, buyTogether[1].first, buyTogether[1].second, ans, buyCnt);
		}
		else { //n==3
			int tt = 5;
			int tempIndex = 0;
			int ms = 0;
			for (auto& d : buyTogether) {
				VM VMnew = d.first;
				int s = VMnew.coreSize + VMnew.memorySize;
				if (s > ms) {
					ms = s;
					tt = tempIndex;
				}
				++tempIndex;
			}
			//tt = 0;
			buyNewServer(candidateServer, buyTogether[tt].first, buyTogether[tt].second, ans, buyCnt);
			auto it_buyTogether = buyTogether.begin();
			advance(it_buyTogether, tt);
			buyTogether.erase(it_buyTogether);
			buyNewServer2(candidateServer, ans, buyCnt, todayNeedType);
		}
	}
}

//删除虚拟机，释放所占用的资源
void Process::deleteVM(VM& VMneedToDel, vector<Server>& myServer){
	auto it = myServer[abs(VMneedToDel.inServerID)].VMinServer.find(VMneedToDel.ID);
	if (it != myServer[abs(VMneedToDel.inServerID)].VMinServer.end()) {
		myServer[abs(VMneedToDel.inServerID)].VMinServer.erase(it);
	}

	if (VMneedToDel.isBinode) {
		//双节点
		int needCoreEach = VMneedToDel.coreSize / 2;
		int needMemEach = VMneedToDel.memorySize / 2;
		myServer[abs(VMneedToDel.inServerID)].coreAre += needCoreEach;
		myServer[abs(VMneedToDel.inServerID)].coreBre += needCoreEach;
		myServer[abs(VMneedToDel.inServerID)].memAre += needMemEach;
		myServer[abs(VMneedToDel.inServerID)].memBre += needMemEach;
	}
	else {
		//单节点
		if (VMneedToDel.AorB == 'A') {
			myServer[abs(VMneedToDel.inServerID)].coreAre += VMneedToDel.coreSize;
			myServer[abs(VMneedToDel.inServerID)].memAre += VMneedToDel.memorySize;
		}
		else {
			myServer[abs(VMneedToDel.inServerID)].coreBre += VMneedToDel.coreSize;
			myServer[abs(VMneedToDel.inServerID)].memBre += VMneedToDel.memorySize;
		}
	}
}

//每日更新
void Process::updatePeerDay(Answer& ans)
{
	unordered_map<int, int> temp2true;
	map<string, vector<Server>> tempMap;
	for (int j = 1; j < myServer2.size(); ++j) {
		Server tempServer = myServer2[j];
		tempMap[tempServer.type].push_back(tempServer);
	}
	//将今天买的服务器按照判题器的尿性，重新编号并与现有的合并，写入购买的答案
	for (auto it = tempMap.begin(); it != tempMap.end(); ++it) {
		for (int k = 0; k < it->second.size(); ++k) {
			temp2true[it->second[k].ID] = myServer.size(); //buyOrderToday从1开始
			it->second[k].ID = myServer.size();
			myServer.push_back(it->second[k]);
		}
		ans.needToBuy[it->first] = it->second.size();
	}
	//然后修改今天加入的虚拟机的所在服务器编号
	for (int j = 0; j < VMneedModiInServerID.size(); ++j) {
		int tempID = VMneedModiInServerID[j];
		auto it = myVM.find(tempID);
		if (it != myVM.end()) {
			int tt = it->second.inServerID;
			it->second.inServerID = temp2true[tt];
		}
	}
	//已写入的ansPutVM也要改
	for (int j = 0; j < ans.putVM.size(); ++j) {
		int tt = ans.putVM[j].first;
		if (tt < 0) {
			ans.putVM[j].first = temp2true[tt];
		}
	}
}

//答案输出
void Process::output(vector<Answer>& ans, int day_idx)
{
	cout << "(purchase, " << ans[day_idx].needToBuy.size() << ")" << endl;
	for (auto it = ans[day_idx].needToBuy.begin(); it != ans[day_idx].needToBuy.end(); ++it) {
		cout << "(" << it->first << ", " << it->second << ")" << endl;
	}
	cout << "(migration, " << ans[day_idx].MigraData.size() << ")" << endl;
	for (int j = 0; j < ans[day_idx].MigraData.size(); j++) {
		if (ans[day_idx].MigraData[j].SERnode == 'C') {
			cout << "(" << ans[day_idx].MigraData[j].VMid << ", " << ans[day_idx].MigraData[j].SERid << ")" << endl;
		}
		else {
			cout << "(" << ans[day_idx].MigraData[j].VMid << ", " << ans[day_idx].MigraData[j].SERid << ", " << 
				ans[day_idx].MigraData[j].SERnode << ")" << endl;
		}
	}
	for (int j = 0; j < ans[day_idx].putVM.size(); ++j) {
		if (ans[day_idx].putVM[j].second == 'C') {//双节点
			cout << "(" << ans[day_idx].putVM[j].first << ")" << endl;
		}
		else {
			cout << "(" << ans[day_idx].putVM[j].first << ", " << ans[day_idx].putVM[j].second << ")" << endl;
		}
	}

	
}

//输出第day_idx天的数据
void Process::output2(vector<Answer>& ans, string fileName, int day_idx)
{
	ofstream out;
	out.open(fileName, ios::app);	//以追加写入方式打开文件
	out << "(purchase, " << ans[day_idx].needToBuy.size() << ")" << endl;
	for (auto it = ans[day_idx].needToBuy.begin(); it != ans[day_idx].needToBuy.end(); ++it) {
		out << "(" << it->first << ", " << it->second << ")" << endl;
	}
	out << "(migration, " << ans[day_idx].MigraData.size() << ")" << endl;
	for (int j = 0; j < ans[day_idx].MigraData.size(); j++) {
		if (ans[day_idx].MigraData[j].SERnode == 'C') {
			out << "(" << ans[day_idx].MigraData[j].VMid << ", " << ans[day_idx].MigraData[j].SERid << ")" << endl;
		}
		else {
			out << "(" << ans[day_idx].MigraData[j].VMid << ", " << ans[day_idx].MigraData[j].SERid << ", " << 
				ans[day_idx].MigraData[j].SERnode << ")" << endl;
		}
	}
	for (int j = 0; j < ans[day_idx].putVM.size(); ++j) {
		if (ans[day_idx].putVM[j].second == 'C') {//双节点
			out << "(" << ans[day_idx].putVM[j].first << ")" << endl;
		}
		else {
			out << "(" << ans[day_idx].putVM[j].first << ", " << ans[day_idx].putVM[j].second << ")" << endl;
		}
	}
}

vector<int> Process::choseServer(bool isTowNode,int needCore,int needMem,vector<Server> &myServer){
    //注意这里传进来的不管是单节点还是双节点，都是虚拟机本身需要的cpu和内存
    //如果是双节点
    int min_gap = 999999;
    int index = -1;
    vector<int> result;
    if (isTowNode){
        int needCoreEach = needCore >>1;
        int needMemEach = needMem >>1;

        for(int k = 0;k<myServer.size();k++){
            if (needCoreEach <= myServer[k].coreAre && needCoreEach <= myServer[k].coreBre && 
				needMemEach <= myServer[k].memAre && needMemEach <= myServer[k].memBre){
                //第k个服务器放得下该虚拟机，则计算贴合程度
                //计算A节点的贴合程度
                int gap1 = myServer[k].memAre - needMemEach + 3 * (myServer[k].coreAre - needCoreEach);
				//如果不是虚拟机和服务器不是同一类型的,增加一个罚数
				double useRadio_aCpu = (myServer[k].coreSizeHalf - myServer[k].coreAre + needCoreEach) / double(myServer[k].coreSizeHalf);
				double useRadio_aMem = (myServer[k].memorySizeHalf - myServer[k].memAre + needMemEach) / double(myServer[k].memorySizeHalf);
				gap1 = gap1 * ( 5 * abs(useRadio_aCpu - useRadio_aMem) / 0.1);
                //计算B点的贴合程度
				double useRadio_bCpu = (myServer[k].coreSizeHalf - myServer[k].coreBre + needCoreEach) / double(myServer[k].coreSizeHalf);
				double useRadio_bMem = (myServer[k].memorySizeHalf - myServer[k].memBre + needMemEach) / double(myServer[k].memorySizeHalf);
                int gap2 = myServer[k].memBre - needMemEach + 3* (myServer[k].coreBre - needCoreEach);
				gap2 = gap2 * (5 * abs(useRadio_bCpu - useRadio_bMem) / 0.1);

				if (myServer[k].coreAre + myServer[k].coreBre == myServer[k].coreSize && 
					myServer[k].memAre + myServer[k].memBre == myServer[k].memorySize) {
					//空服务器的惩罚系数应该跟电费有关
					gap1 = gap1 * 5 +  myServer[k].energyCost;
					gap2 = gap2 * 5 +  myServer[k].energyCost;
				}
                //计算两个节点贴合最小值
                int min_suit = gap1<gap2?gap1:gap2;
                if (min_suit<min_gap){
                    min_gap = min_suit;
                    index = k;
                }
            }
        }
    }else{

        for(int k = 0;k<myServer.size();k++){
            if (needCore <= myServer[k].coreAre && needMem <= myServer[k].memAre) {
                //A节点放得下，计算A节点的贴合程度
                int gap = myServer[k].memAre -  needMem + 3 * (myServer[k].coreAre - needCore);
				double useRadio_aCpu = (myServer[k].coreSizeHalf - myServer[k].coreAre + needCore) / double(myServer[k].coreSizeHalf);
				double useRadio_aMem = (myServer[k].memorySizeHalf - myServer[k].memAre + needMem) / double(myServer[k].memorySizeHalf);
				gap = gap * ( 5 * abs(useRadio_aCpu - useRadio_aMem) / 0.1);
				if (myServer[k].coreAre + myServer[k].coreBre == myServer[k].coreSize && 
					myServer[k].memAre + myServer[k].memBre == myServer[k].memorySize) {
					gap = gap * 5 + myServer[k].energyCost;
				}
                if (gap<min_gap){
                    min_gap = gap;
                    index = 2*k;
                }
            }
            if (needCore <= myServer[k].coreBre && needMem <= myServer[k].memBre) {
                //B节点放得下,计算B节点的贴合程度
                int gap = myServer[k].memBre - needMem + 3 * (myServer[k].coreBre - needCore);
				double useRadio_bCpu = (myServer[k].coreSizeHalf - myServer[k].coreBre + needCore) / double(myServer[k].coreSizeHalf);
				double useRadio_bMem = (myServer[k].memorySizeHalf - myServer[k].memBre + needMem) / double(myServer[k].memorySizeHalf);
				gap = gap * (5 * abs(useRadio_bCpu - useRadio_bMem) / 0.1);

				if (myServer[k].coreAre + myServer[k].coreBre == myServer[k].coreSize && 
					myServer[k].memAre + myServer[k].memBre == myServer[k].memorySize) {
					gap = gap * 5 +  myServer[k].energyCost;
				}
                if (gap<min_gap){
                    min_gap = gap;
                    index = 2*k+1;
                }
            }
        }
    }
    
    if (index != -1){
        result.push_back(index);
        result.push_back(min_gap);
    }else{
        result.push_back(-1);
        result.push_back(min_gap);
    }
    return result;
}

//追加接收第day_idx天的请求序列
void Process::ReadFile(vector<vector<Record>>&myRecord, int day_idx) {
	if (day_idx >= this->T)
		return;

	int m = 0;
	cin >> m;
	cin.get();
	myRecord[day_idx].resize(m);
	for (int j = 0; j < m; ++j) {
		string input = "";
		getline(cin, input);
		input = input.substr(1, input.length() - 2);
		stringstream ss(input);
		string temp;
		getline(ss, temp, ',');
		if (temp == "add") {
			myRecord[day_idx][j].AorD = true;
			getline(ss, temp, ',');
			myRecord[day_idx][j].typeName = temp.substr(1, temp.length() - 1);
			getline(ss, temp, ',');
			myRecord[day_idx][j].ID = stoi(temp);
		}
		else {
			myRecord[day_idx][j].AorD = false;
			getline(ss, temp, ',');
			myRecord[day_idx][j].ID = stoi(temp);
		}
	}
}