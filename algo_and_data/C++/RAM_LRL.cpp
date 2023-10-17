#include <iostream>
#include <cmath>
#include <cassert>              
#include <fstream>
#include <ctime>

//#define COSSIM //���������ƶ�����
#define COS_PARM 1.0 / 3

#define MAX_USER_SIZE 200
#define MAX_RES_SIZE 5

using namespace std;

int USERSIZE, RESSIZE;

int cap[MAX_RES_SIZE], total_cap = 0.0;
double norm_cap[MAX_RES_SIZE];


double budget = 12000;
double socialWelfare = 0.0, totalBids = 0.0;

double start_time, end_time;	//�����㷨��ִ��ʱ�� 

struct REQ {
    int id;
    int isactive;
    double bid;
    int res[MAX_RES_SIZE];//requirement
    double norm_res[MAX_RES_SIZE];
    double cos;
    double val;//bib/res
    double pay;
};

REQ user[MAX_USER_SIZE];


//�ļ��������� 
void readData(string filename) {
    //c_str()����string���Ͷ�Ӧ��charָ��,Ϊ�˼���C�е��ַ���(char����) 
    ifstream f(filename.c_str());
    string buffer;
    //�ɹ����ļ��򷵻�true 
    assert(f.is_open());        //������Ե�����Ϊfalse, ����ֹ����ִ��
    //����һ���ļ�����, ��Ϊ��, ��.eof()����Ϊtrue, ����������������eof()Ϊfalse 
    while (!getline(f, buffer).eof()) {
    	//�����û���������Դ�������� 
        if (buffer[0] == 'p') {
            //%*c��ʾ�������ַ�, ֱ�Ӻ��Ե��� 
            sscanf(buffer.c_str(), "%*c %d %d", &USERSIZE, &RESSIZE);
        }
        //�����û��ĳ��� 
        else if (buffer[0] == 'b') {
            int idx;
            double bid;
            sscanf(buffer.c_str(), "%*c %d %lf", &idx, &bid);
        	user[idx - 1].id = idx;
        	user[idx - 1].isactive = 1;
        	user[idx - 1].bid = bid;
        	totalBids += bid;
        }
        //�����û�������������Դ������ 
		else if (buffer[0] == 'd') {
			int idx, j, num;	//�û�idx�ĵ�j����Դ������num�� 
			sscanf(buffer.c_str(), "%*c %d %d %d", &idx, &j, &num);
			user[idx - 1].res[j - 1] = num;
		}
        //�������������������Դ������ 
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
        		norm_cap[idx] = 1.0;
//        		total_cap += capacity;
        		idx++;
			}
		}
    }
}


//�Ƿ񻹴��ڻ�Ծ�û�
int hasActive(REQ user[],int len)
{
    int i=len;
    for(i=0; i<len; i++)
    {
        if(user[i].isactive==1)
            return 1;
    }
    return 0;
}

//������֧��
double totalPay(REQ users[])
{
    int i;
    double sum=0;
    for(i=0; i<USERSIZE; i++)
        sum+=users[i].pay;
    return sum;
}

//�����ܷ�����Դ
void totalAlloc(REQ users[], int sum[])
{
    int i,j;
    for(j=0; j<RESSIZE; j++)
        sum[j]=0;
    for(i=0; i<USERSIZE; i++)
        if(users[i].pay>0)
        {
            for(j=0; j<RESSIZE; j++)
                sum[j]+=users[i].res[j];
        }
}

int resOverflow(REQ user[], int res[], int cap[])
{
    int i,j;
    for(i=0; i<RESSIZE; i++)
    {
        res[i]=0;
    }
    //���㵱ǰ��Ծ�û��������Ƿ񳬹�ϵͳ�����������������������
    for(i=0; i<USERSIZE; i++)
    {
        if(user[i].isactive==1)
        {
            for(j=0; j<RESSIZE; j++)
                res[j]+=user[i].res[j];
        }

    }
    for(i=0; i<RESSIZE; i++)
    {
        if(res[i]>cap[i])
            return 1;
    }
    return 0;
}

//�����������������ƶ�
double cossim(double va[], double vb[])
{
    int i; 
    double res, sum=0.0, norma=0.0, normb=0.0;
    for(i=0; i<RESSIZE; i++)
    {
        sum+=va[i]*vb[i];
        norma+=va[i]*va[i];
        normb+=vb[i]*vb[i];
    }
    res = sum / ((sqrt(norma)) * (sqrt(normb)));
    return res;
}

//����������ģ
double norm(int v[])
{
    int i;
    double res, norm = 0.0;
    for(i=0; i<RESSIZE; i++)
    {
//    	norm += v[i] * 1.0 / cap[i];
        norm += pow(v[i] * 1.0 / cap[i], 2);
//        norm += v[i] * v[i];
    }
    res = sqrt((double)norm);
    return res;
}

void get_norm_res(REQ users[]) {
	int i, j;
	for (i = 0; i < USERSIZE; i++) {
		for (j = 0; j < RESSIZE; j++) {
			users[i].norm_res[j] = (double)users[i].res[j] / cap[j];
		}
	}
}

void getVal(REQ users[])
{
    int i;
    for(i=0; i<USERSIZE; i++)
    {
//#ifdef COSSIM
        //�������ƶ�
        double cos;
        if (COS_PARM == 0) 
        	cos = 1;
        else 
        	cos= cossim(user[i].norm_res, norm_cap);
		user[i].cos = cos; 
        user[i].val = (user[i].bid * pow(cos, COS_PARM)) / norm(user[i].res);
//#else
//        //���������ƶ�
//        user[i].cos = 1;
//        user[i].val = user[i].bid/norm(user[i].res);
//#endif
    }
}

void showRes() {
	printf("budget = %d, totalBids = %lf, capacity =", budget, totalBids);
	for (int i = 0; i < RESSIZE; i++) {
		printf(" %d", cap[i]);
	}
	printf("\n");
}

void showAlloc(REQ users[], int sum[]) {
	printf("�û���֧����%f, ��ḣ����%lf\n", totalPay(user), socialWelfare);
	printf("��Դ�����ʣ�");
	for (int j = 0; j < RESSIZE; j++) {
		printf("%.3f ", (double)sum[j] / cap[j]);
	}
	
}

void showUser(int id) {
	for (int i = 0; i < USERSIZE; i++) {
		if (user[i].id == id) 
			cout << "---------------------------------------------------------------------------------------------" << endl;
		printf("id=%d, val = %f, cos = %f bid = %f ,req=",user[i].id,user[i].val,user[i].cos,user[i].bid);
		for (int j = 0; j < RESSIZE; j++) {
			printf(" %d", user[i].res[j]);
		}
		printf(" ,pay = %f\n", user[i].pay);
		if (user[i].id == id) 
			cout << "---------------------------------------------------------------------------------------------" << endl;
	}
}

double show_user_i_utility(int id) {
	for (int i = 0; i < USERSIZE; i++) {
   		if (user[i].id == id) {
   			printf("id=%d���û�����Ϊ��%.1lf\n", id, user[i].bid);
   			printf("id=%d���û�֧���ķ���Ϊ��%.1lf\n", id, user[i].pay);
		}
	} 
}

int main()
{
	printf("RAM-LRL�㷨��\n");
//	readData("example.txt");
	readData("data.txt");
    int i = 0, j, feas = 0, sum[RESSIZE] = {0};
    double p = 0, temppay = 0;
    REQ temp;
//    showRes();
    //get user norm_res
    start_time = clock();
    get_norm_res(user);
    //obtain val
    getVal(user);
    
    //sort according to the val
    for(i=0; i<USERSIZE; i++)
    {
        for(j=0; j<USERSIZE-i-1; j++)
        {
            if(user[j].val<user[j+1].val)
            {
                temp=user[j];
                user[j]=user[j+1];
                user[j+1]=temp;
            }
        }
    }

    do
    {
        //increase p
//        p += 5;
        p += 10;
//		p += 0.001;

        //remove inactive user
        for(i=0; i<USERSIZE; i++)
        {
            if(user[i].val<p)
            {
                if((user[i].pay==0)&&(user[i].isactive!=0))
                {
                    user[i].isactive=0;
//                    printf("p= %f  user %d alloc failed\n",p,user[i].id );
                }
				
            }
        }


        //�����Ծ�û������󳬹�ϵͳ�������������
        if(resOverflow(user,sum,cap))
            continue;
        temppay=0;
        //��Ծ�û�������С��ϵͳ���������Գ��Է���
        for(i=0; i<USERSIZE; i++)
        {
            if((user[i].isactive==1))
            {
                temppay+=p * norm(user[i].res);
            }
        }
        //����յ���Ǯ������budget���������ۣ��߳����Լ۱��û����������ɹ�
        if(temppay>=budget)
        {
            for(i=0; i<USERSIZE; i++)
            {
                if(user[i].isactive==1)
                {
                    user[i].pay = (p * norm(user[i].res)) / pow(user[i].cos, COS_PARM);
                    user[i].isactive=0;
//                    printf("p= %f  user %d alloc access\n",p,user[i].id );
                    socialWelfare += user[i].bid;
                }
            }
            feas=1;
        }
    }
    while((feas==0)&&(hasActive(user,USERSIZE)));
    end_time = clock();
    printf("\nȫ�ּ۸� %f\n",p);
    if(feas==0)
        printf("failed\n\n");
    else
    {
        printf("success\n\n");

		
		
        totalAlloc(user,sum);
        
        showAlloc(user, sum);
    }
    
	printf("\nִ��ʱ�䣺%.3fs\n\n", (end_time - start_time) / CLOCKS_PER_SEC);
	
    showUser(98);
    
    cout << "--------------------------------" << endl;
   	show_user_i_utility(98);
    return 0;
}

































