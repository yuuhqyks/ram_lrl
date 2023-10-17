#include <iostream>
#include <cmath>
#include <cassert>              
#include <fstream>
#include <algorithm>
#include <ctime>

#define MAX_USER_SIZE 300
#define MAX_RES_SIZE 10

using namespace std;

int USER_SIZE, RES_SIZE;

int cap[MAX_RES_SIZE], tmp_cap[MAX_RES_SIZE], alloc_cap[MAX_RES_SIZE];

double social_welfare = 0.0, totalPay = 0.0;
bool first_alloc = true;

double start_time, end_time;	//计算算法的执行时间

struct REQ {
    int id;
    double bid;
    int res[MAX_RES_SIZE];//requirement
    bool exist;	//为false表示排除该用户 
    double res_density;
    bool alloc_success;
    double pay;
};

REQ user[MAX_USER_SIZE], tmp_user[MAX_USER_SIZE];

bool cmp(const REQ &r1, const REQ &r2) {
	return r1.res_density > r2.res_density;
}


//文件读入数据 
void readData(string filename) {
    //c_str()返回string类型对应的char指针,为了兼容C中的字符串(char数组) 
    ifstream f(filename.c_str());
    string buffer;
    //成功打开文件则返回true 
    assert(f.is_open());        //如果断言的条件为false, 则终止程序执行
    //读入一行文件内容, 若为空, 则.eof()函数为true, 若该行中有内容则eof()为false 
    while (!getline(f, buffer).eof()) {
    	//读入用户数量与资源种类数量 
        if (buffer[0] == 'p') {
            //%*c表示跳过该字符, 直接忽略掉！ 
            sscanf(buffer.c_str(), "%*c %d %d", &USER_SIZE, &RES_SIZE);
        }
        //读入用户的出价 
        else if (buffer[0] == 'b') {
            int idx;
            double bid;
            sscanf(buffer.c_str(), "%*c %d %lf", &idx, &bid);
        	user[idx - 1].id = idx;
        	user[idx - 1].bid = bid;
        	user[idx - 1].exist = true;
        	user[idx - 1].alloc_success = false;
        }
        //读入用户对所有种类资源的需求 
		else if (buffer[0] == 'd') {
			int idx, j, num;	//用户idx的第j种资源数量是num个 
			sscanf(buffer.c_str(), "%*c %d %d %d", &idx, &j, &num);
			user[idx - 1].res[j - 1] = num;
		}
        //读入服务器所有种类资源的容量 
        else if (buffer[0] == 'r') {
        	int idx = 0, pos1 = buffer.find(" "), pos2;
        	while (pos1 != -1) {
        		buffer.replace(pos1, 1, "r");
        		pos2 = buffer.find(" ");
        		string st;
        		if (pos2 != -1) 
        			st = buffer.substr(pos1+1, pos2-pos1-1);
				else 
					st = buffer.substr(pos1+1, buffer.length()-1-pos1);
				pos1 = pos2;
        		int capacity = stoi(st);
        		cap[idx] = capacity;
        		idx++;
			}
		}
    }
}

//用fr = 1 / Cr计算用户资源密度 
void calc_density() {
	for (int i = 0; i < USER_SIZE; i++) {
		double tmp = 0.0;
		for (int j = 0; j < RES_SIZE; j++) {
			double fr = 1.0 / cap[j];
//			double fr = 1.0;
			tmp += fr * user[i].res[j];
		}
		user[i].res_density = user[i].bid / sqrt(tmp);
	}
}

void copy_2_tmp() {
	for (int i = 0; i < USER_SIZE; i++) {
		tmp_user[i] = user[i];
	}
	for (int j = 0; j < RES_SIZE; j++) {
		tmp_cap[j] = cap[j];
	}
}

void recover_tmp() {
	for (int i = 0; i < USER_SIZE; i++) {
		tmp_user[i].alloc_success = false;
	}
	for (int j = 0; j < RES_SIZE; j++) {
		tmp_cap[j] = cap[j];
	}
}

void monotonic_alloc() {
	for (int i = 0; i < USER_SIZE; i++) {
		if (!user[i].exist) continue;
		bool flag = true;
		int capacity[RES_SIZE];
		for (int j = 0; j < RES_SIZE; j++) {
			capacity[j] = tmp_cap[j] - tmp_user[i].res[j];
			if (capacity[j] < 0) {
				flag = false;
				break;
			}
		}
		if (flag) {
			if (first_alloc) {
				social_welfare += tmp_user[i].bid;
				user[i].alloc_success = true;
				for (int j = 0; j < RES_SIZE; j++) {
					alloc_cap[j] += tmp_user[i].res[j];
				}
			}
			tmp_user[i].alloc_success = true;
			for (int j = 0; j < RES_SIZE; j++) {
				tmp_cap[j] = capacity[j];
			}
		}
	}
	first_alloc = false;
}

void monotonic_pay() {
	for (int i = 0; i < USER_SIZE; i++) {
		user[i].pay = 0.0;
		if (user[i].alloc_success) {
			int l = -1;
			recover_tmp();
			user[i].exist = false;
			monotonic_alloc();
			user[i].exist = true;
			for (int j = 0; j < USER_SIZE; j++) {
				if (tmp_user[j].res_density >= user[i].res_density) 
					continue;
				if (!user[j].alloc_success && tmp_user[j].alloc_success) {
					l = j;
					break;
				}
			}
			if (l != -1) {
				double tmp = user[i].bid / user[i].res_density;
				user[i].pay = tmp_user[l].res_density * tmp;
			}
		}
	}
}


void showInfo() {
//	cout << "初始时各种资源的容量：";
//	for (int j = 0; j < RES_SIZE; j++) {
//		cout << " " << cap[j]; 
//	}
	cout << endl;
//	cout << "已分配的各种资源的容量：";

//	cout << endl << endl;
	for (int i = 0; i < USER_SIZE; i++) {
		totalPay += user[i].pay;
//		printf("id=%d, bid = %f ,req=",user[i].id, user[i].bid);
//		for (int j = 0; j < RES_SIZE; j++) {
//			printf(" %d", user[i].res[j]);
//		}
//		printf(" res_density = %lf, alloc_success = %d, pay = %lf\n", user[i].res_density, user[i].alloc_success, user[i].pay);
	}
//	cout << endl << endl;
	
	printf("单调算法加临界值支付\n\n");
	
	cout << "用户总支付：" << totalPay << " 社会福利：" << social_welfare;
	
	printf("\n资源利用率："); 
	for (int j = 0; j < RES_SIZE; j++) {
		printf("%.3lf ", (double)alloc_cap[j] / cap[j]);  
	}
	printf("\n执行时间：%.3fs\n", (end_time - start_time) / CLOCKS_PER_SEC);
	
}


int main() {
//	readData("example.txt");
	readData("data.txt");
	start_time = clock();
	calc_density();
	sort(user, user + USER_SIZE, cmp);
	copy_2_tmp();
	monotonic_alloc();
	monotonic_pay();
	end_time = clock();
	
	showInfo();
	return 0; 
}

















