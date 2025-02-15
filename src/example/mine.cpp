#include "circuit/circuit.h"
#include "circuit/share.h"
#include <vector>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include<iomanip>
#include<fstream>
#include<sstream>
#include <algorithm>
#include <cstdint>

#include <cassert>
#include <cstdlib>
#include <ctime>
#include "../core/httplib.h"
#include "../core/OEP.h"
#include "../core/relation.h"
#include "../core/PSI.h"
#include "../core/party.h"
#include "../core/RNG.h"

using namespace std;
using namespace SECYAN;

vector<vector<uint64_t>> jiaoji_result;

vector<uint64_t> readAlicedata(int flag) {
    ifstream ifs;  //创建流对象
    ifs.open("Alicedata.txt", ios::in);
    vector<uint64_t> target = { 0 };
    //判断文件是否打开
    if (!ifs.is_open()) {
        cout << "read Alicedata false" << endl;
        return target;
    }
    vector<uint64_t> target_1,target_2;
    vector<string> item;    //用于存放文件中的一行数据
    string temp;   //把文件中的一行数据作为字符串放入容器中
    while (getline(ifs, temp))   //利用getline（）读取每一行，并放入到 item 中
    {
        item.push_back(temp);
    }
    for (auto it = item.begin(); it != item.end(); it++)
    {
        cout << *it << endl;

        istringstream istr(*it); //把一行数据分为单个数据
        string str;
        int count = 1;        //统计一行数据中单个数据个数

        //获取文件中的第 1、2 列数据
        while (istr >> str)                      //以空格为界，把istringstream中数据取出放入到依次s中
        {
            //获取第1列数据
            if (count == 1)
            {
                int r = atoi(str.c_str());       //数据类型转换，将string类型转换成float,如果字符串不是有数字组成，则字符被转化为 0

                target_1.push_back(r);
            }
            //获取第2列数据
            else if (count == 2)
            {
                int r = atoi(str.c_str());       //数据类型转换，将string类型转换成float

                target_2.push_back(r);
            }
            count++;
        }
    }
    if (flag == 1)
        return target_1;
    else if (flag == 2) 
        return target_2;
    else
        return target;
}

vector<uint64_t> readBobdata(int flag) {
    ifstream ifs;  //创建流对象
    ifs.open("Bobdata.txt", ios::in);
    vector<uint64_t> target = { 0 };
    //判断文件是否打开
    if (!ifs.is_open()) {
        cout << "read Bobdata false" << endl;
        return target;
    }
    vector<uint64_t> target_1, target_2;
    vector<string> item;    //用于存放文件中的一行数据
    string temp;   //把文件中的一行数据作为字符串放入容器中
    while (getline(ifs, temp))   //利用getline（）读取每一行，并放入到 item 中
    {
        item.push_back(temp);
    }
    for (auto it = item.begin(); it != item.end(); it++)
    {
        cout << *it << endl;

        istringstream istr(*it); //把一行数据分为单个数据
        string str;
        int count = 1;        //统计一行数据中单个数据个数

        //获取文件中的第 1、2 列数据
        while (istr >> str)                      //以空格为界，把istringstream中数据取出放入到依次s中
        {
            //获取第1列数据
            if (count == 1)
            {
                int r = atoi(str.c_str());       //数据类型转换，将string类型转换成float,如果字符串不是有数字组成，则字符被转化为 0

                target_1.push_back(r);
            }
            //获取第2列数据
            else if (count == 2)
            {
                int r = atoi(str.c_str());       //数据类型转换，将string类型转换成float

                target_2.push_back(r);
            }
            count++;
        }
    }
    if (flag == 1)
        return target_1;
    else if (flag == 2)
        return target_2;
    else
        return target;
}

vector<uint64_t> paixu(vector<uint64_t> a,int len){
 	for(int i=0;i<len-1;i++){
  		for(int j=0;j<len-i-1;j++){
   			if(a[j]>a[j+1]){
    				uint64_t temp=a[j];
    				a[j]=a[j+1];
    				a[j+1]=temp;
   			}			
  		}
 
 	}
 	return a;
}
//vector<vector<uint64_t>> test_one_psi()
void test_one_psi()
{
	auto role = gParty.GetRole();
	auto bc = gParty.GetCircuit(S_BOOL);
	clock_t start, end;
 	float duration;
 	start = clock();
	vector<uint64_t> Alicedata = readAlicedata(1);
	vector<uint64_t> Bobdata = readBobdata(1);
	auto alast = unique(Alicedata.begin(), Alicedata.end());
   	Alicedata.erase(alast, Alicedata.end());  
    	Alicedata.erase(Alicedata.begin());
    	int M=Alicedata.size();
    	auto blast = unique(Bobdata.begin(), Bobdata.end());
    	Bobdata.erase(blast, Bobdata.end());  
    	Bobdata.erase(Bobdata.begin());
    	int N=Bobdata.size();
    	vector<uint64_t> AliceSet(M);
    	vector<uint64_t> BobSet(N);
	for (int i = 0; i < M; i++)
		AliceSet[i] = Alicedata[i];
	for (int i = 0; i < N; i++)
		BobSet[i] = Bobdata[i];
	PSI *psi;
	if (role == SERVER){
	        cout<<"this is alice"<<endl;
	        for(int i=0;i<M;i++){
                        cout<<AliceSet[i]<<" ";
                }
                cout<<endl;
		psi = new PSI(AliceSet, M, N, PSI::Alice);
	}
	else{
	        for(int i=0;i<N;i++){
                        cout<<BobSet[i]<<" ";
                }
                cout<<endl;
		psi = new PSI(BobSet, M, N, PSI::Bob);
	}
	vector<uint32_t> out = psi->Intersect();
	// auto out = psi->Intersect();
	cout<<"out result"<<endl;
 	for (int i = 0; i < (int)out.size(); i++){
  		cout<<out[i]<<" ";
 	}
 	cout<<endl;
	auto s_in = bc->PutSharedSIMDINGate(out.size(), out.data(), 1);
	auto s_out = bc->PutOUTGate(s_in, ALL);
	uint32_t *a;
	uint32_t b, c;
	gParty.ExecCircuit();
	s_out->get_clear_value_vec(&a, &b, &c);
	cout<<"real result"<<endl;
 	for(int i=0;i<(int)out.size();i++){
     		cout<<a[i]<<" "; 
 	}
 	cout<<endl;
	
	int num = 0;
	for (int i = 0; i < (int)out.size(); i++)
		num += a[i];
	cout<<num<<endl;
	vector<uint64_t> end_senddata0;
 	vector<uint64_t> end_senddata1;
 	vector<uint64_t> end_receivedata0;
 	vector<uint64_t> end_receivedata1;
 	int end_data_count=0;
 	
 	vector<vector<uint64_t>> jiaoji_result;
 	
	if(role == SERVER){
  		auto table=psi->Get_cuckooTable();
  		vector<uint64_t> intersectData(num);
  		int count=0;
  		cout<<(int)table.size()<<endl;
  		for(int i=0;i<(int)table.size();i++){
   			if(a[i]==1){
    				intersectData[count]=table[i];
    				count++;
    
   			}
  		}
  		cout<<"intersection"<<endl;
  		for(int i=0;i<num;i++){
   			cout<<intersectData[i]<<" ";
 		}
  		cout<<endl; 
  		gParty.OTSend(intersectData, intersectData);
  		vector<uint64_t> re_intersectData=paixu(intersectData,num);
  
  		vector<uint64_t> Alicedata0=readAlicedata(1);
  		Alicedata0.erase(Alicedata0.begin());
  		vector<uint64_t> Alicedata1=readAlicedata(2);
  		Alicedata1.erase(Alicedata1.begin());
  
  		for(int i=0;i<Alicedata0.size()-1;i++){
   			for(int j=0;j<Alicedata0.size()-i-1;j++){
    				if(Alicedata0[j]>Alicedata0[j+1]){
     					uint64_t temp=Alicedata0[j];
     					Alicedata0[j]=Alicedata0[j+1];
     					Alicedata0[j+1]=temp;
     
     					temp=Alicedata1[j];
     					Alicedata1[j]=Alicedata1[j+1];
     					Alicedata1[j+1]=temp;
    				}
   			} 
 
		}
		int q=0;
  		int k=0;
  
  		gParty.Recv(end_receivedata0);
  		gParty.Recv(end_receivedata1);
      		int abc_count=0;
      
      		cout<<"A\t\tB\t\tC\t\t"<<endl;
      		for(int i=0;i<Alicedata0.size();i++){
       			for(int j=0;j<end_receivedata0.size();j++){
       	 			if(Alicedata0[i]==end_receivedata0[j]){
         				vector<uint64_t> row;
         				row.push_back(Alicedata0[i]);
     					row.push_back(Alicedata1[i]);
     					row.push_back(end_receivedata1[j]);
     					jiaoji_result.push_back(row);
         				abc_count++;
        			}
        			if(Alicedata0[i]<end_receivedata0[j]){
         				break;
        			}
       			}	
      
      		}
      
      		cout<<"Alice_intersect_table"<<endl;
      		cout<<"A\tB\tC\t"<<endl;
      		for (const auto& row : jiaoji_result) {
          		for (int value : row) {
              			std::cout << value << '\t';
          		}
          		std::cout << '\n';
      		}
      		cout<<"total:   "<<abc_count<<endl;
      		cout << "The intersection is successfully solved!" << endl;
	}
	if(role==CLIENT){
  		vector<uint32_t> selectBits(num, 1);
  		vector<uint64_t> receivedData = gParty.OTRecv(selectBits);
  		vector<uint64_t> re_receivedData=paixu(receivedData,num);
  		vector<uint64_t> Bobdata0=readBobdata(1);
  		Bobdata0.erase(Bobdata0.begin());
  		vector<uint64_t> Bobdata1=readBobdata(2);
  		Bobdata1.erase(Bobdata1.begin());
  
  		for(int i=0;i<Bobdata0.size()-1;i++){
   			for(int j=0;j<Bobdata0.size()-i-1;j++){
    				if(Bobdata0[j]>Bobdata0[j+1]){
     					uint64_t temp=Bobdata0[j];
     					Bobdata0[j]=Bobdata0[j+1];
     					Bobdata0[j+1]=temp;
     
     					temp=Bobdata1[j];
     					Bobdata1[j]=Bobdata1[j+1];
     					Bobdata1[j+1]=temp;
    				}			
   			} 
 
  		}
  		int q=0;
  		int k=0;
  
  		cout<<"Bob_intersect_table"<<endl;
  		while(q<num&&k<Bobdata0.size()){
   			if(Bobdata0[k]<re_receivedData[q]){
    				k++;
   
   			}
   			else if(Bobdata0[k]==re_receivedData[q]){
    
    				cout<<Bobdata0[k]<<"\t\t"<<Bobdata1[k]<<endl;
    				end_senddata0.push_back(Bobdata0[k]);
    				end_senddata1.push_back(Bobdata1[k]);
    				k++;
    				end_data_count++;
   
   			}
   			else{
    				q++;    
   			}   
  		}
		
		gParty.Send(end_senddata0);
  		gParty.Send(end_senddata1);
  		cout<<"Bob transmitted the data to Alice successfully"<<endl;
 	}
 	end=clock();
 	duration = 1000.0 * (end - start) / CLOCKS_PER_SEC;
 	cout << "intersection: " << duration << " ms" << endl;
 	gParty.Reset();
 
 
 	delete[] a;
 	delete psi;
 	//return jiaoji_result;
}
void writeRandomDataToFile(const std::string& filename,int seed,int m,int flag) {
    std::ofstream outputFile(filename);

    // 检查文件是否成功打开
    if (!outputFile.is_open()) {
        std::cerr << "Error: Unable to open file " << filename << std::endl;
        return;
    }

    // 使用当前时间作为随机数生成器的种子
    std::srand(seed);

    // 写入数据
    if(flag==1)
    	outputFile << "A" << " " << "B" << "\n";
    else
    	outputFile << "A" << " " << "C" << "\n";
    for (int i = 0; i < m; ++i) {
        // 生成两个随机数据
        int randomData1 = std::rand()%700000+1;
        int randomData2 = std::rand()%700000+1;

        // 写入到文件中
        outputFile << randomData1 << " " << randomData2 << "\n";
    }

    // 关闭文件
    outputFile.close();
}

void handle_request(const httplib::Request& req, httplib::Response& res) {
// 设置 CORS 头
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
    // 处理来自前端的 HTTP 请求
    std::cout << "Received request: " << req.method << " " << req.path << std::endl;
    std::ostringstream response_stream; 
    auto role = gParty.GetRole();
  
    response_stream << "A\t\tB\t\tC\n"; // 循环输出数据行 
    for (const auto& row : jiaoji_result) { 
        response_stream << row[0] << "\t\t" << row[1] << "\t\t" << row[2] << "\n"; 
        
    }
    
    res.set_content(response_stream.str(), "text/plain");

    
}

void handle_options(const httplib::Request& req, httplib::Response& res) {
    // 为OPTIONS请求设置
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type");
}

int main(int argc, char **)
{
 	srand(14131);
 	string address = "127.0.0.1";
 	uint16_t port = 7766;
 	e_role role = SERVER;
 	//role = CLIENT;

 	if (argc > 1)
  		role = (e_role)(1 - role);

 	gParty.Init(address, port, role);
 
 
 	std::string filename1 = "Alicedata.txt";
 	std::string filename2 = "Bobdata.txt";
     	// 调用函数写入数据
    	writeRandomDataToFile(filename1,1234560,10000,1);
     
 	writeRandomDataToFile(filename2,7418905,20000,2);

 	test_one_psi();

 	httplib::Server server1;
 	// 处理OPTIONS请求
        server1.Options("/", handle_options);

     	// 处理 GET 请求
     	server1.Get("/", handle_request);
 
     	int port1 = 8080;
	
     	if(role==SERVER){
     		std::cout << "Listening for requests..." << std::endl;
     		server1.listen("localhost", port1);
     		
	}
 	return 0;
}


