#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdint>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include "circuit/circuit.h"
#include "circuit/share.h"
#include "../core/httplib.h"
#include "../core/OEP.h"
#include "../core/relation.h"
#include "../core/PSI.h"
#include "../core/party.h"
#include "../core/RNG.h"

using namespace std;
using namespace SECYAN;

vector<vector<uint64_t>> jiaoji_result;

vector<uint64_t> readDataFromFile(const string& filename, int flag) {
    ifstream ifs;
    ifs.open(filename, ios::in);
    vector<uint64_t> target = { 0 };
    
    if (!ifs.is_open()) {
        cout << "read " << filename << " false" << endl;
        return target;
    }
    
    vector<uint64_t> target_1, target_2;
    vector<string> item;
    string temp;
    
    while (getline(ifs, temp)) {
        item.push_back(temp);
    }
    
    for (auto it = item.begin(); it != item.end(); it++) {
        cout << *it << endl;
        
        istringstream istr(*it);
        string str;
        int count = 1;
        
        while (istr >> str) {
            if (count == 1) {
                int r = atoi(str.c_str());
                target_1.push_back(r);
            }
            else if (count == 2) {
                int r = atoi(str.c_str());
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

void sortWithIndex(std::vector<uint64_t>& data0, std::vector<uint64_t>& data1) {
    // 创建一个包含索引的配对数组
    std::vector<std::pair<uint64_t, uint64_t>> pairedData;
    for (size_t i = 0; i < data0.size(); ++i) {
        pairedData.emplace_back(data0[i], data1[i]);
    }

    // 使用 std::sort 和自定义比较函数进行排序
    std::sort(pairedData.begin(), pairedData.end());

    // 更新原始数组
    for (size_t i = 0; i < pairedData.size(); ++i) {
        data0[i] = pairedData[i].first;
        data1[i] = pairedData[i].second;
    }
}

//vector<vector<uint64_t>> test_one_psi()
void test_one_psi()
{
	auto role = gParty.GetRole();
	auto bc = gParty.GetCircuit(S_BOOL);
	clock_t start;
 	start = clock();

	vector<uint64_t> Alicedata = readDataFromFile("Alicedata.txt", 1);
	vector<uint64_t> Bobdata = readDataFromFile("Bobdata.txt", 1);
	// todo 验证（ZK）
	Alicedata.erase(unique(Alicedata.begin(), Alicedata.end()), Alicedata.end());
	Alicedata.erase(Alicedata.begin());
    Bobdata.erase(unique(Bobdata.begin(), Bobdata.end()), Bobdata.end());
	Bobdata.erase(Bobdata.begin());
    vector<uint64_t> AliceSet(Alicedata.begin(), Alicedata.end());
	vector<uint64_t> BobSet(Bobdata.begin(), Bobdata.end());
	
	PSI *psi = (role == SERVER) ?
		new PSI(AliceSet, Alicedata.size(), Bobdata.size(), PSI::Alice) :
		new PSI(BobSet, Alicedata.size(), Bobdata.size(), PSI::Bob);
	vector<uint32_t> out = psi->Intersect();

	auto s_in = bc->PutSharedSIMDINGate(out.size(), out.data(), 1);
	auto s_out = bc->PutOUTGate(s_in, ALL);
	uint32_t *a;
	uint32_t b, c;
	gParty.ExecCircuit();
	s_out->get_clear_value_vec(&a, &b, &c);
	
	int num = accumulate(a, a + out.size(), 0);
 	
	if(role == SERVER){
  		auto table=psi->Get_cuckooTable();
  		vector<uint64_t> intersectData(num);
  		int count=0;
  		for(int i=0;i<(int)table.size();i++){
   			if(a[i]==1){
    				intersectData[count]=table[i];
    				count++;
   			}
  		}

  		gParty.OTSend(intersectData, intersectData);

  		sort(intersectData.begin(), intersectData.begin() + num);
  
  		vector<uint64_t> Alicedata0=readDataFromFile("Alicedata.txt", 1);
  		Alicedata0.erase(Alicedata0.begin());
  		vector<uint64_t> Alicedata1=readDataFromFile("Alicedata.txt", 2);
  		Alicedata1.erase(Alicedata1.begin());
		sortWithIndex(Alicedata0, Alicedata1);

   		vector<uint64_t> end_receivedata0;
 		vector<uint64_t> end_receivedata1;
  		gParty.Recv(end_receivedata0);
  		gParty.Recv(end_receivedata1);

      	int abc_count=0;
		int k = 0;
		for (size_t i = 0; i < Alicedata0.size(); ++i) {
    		// 在 end_receivedata0 中查找 Alicedata0[i]
    		auto it = std::find(end_receivedata0.begin() + k, end_receivedata0.end(), Alicedata0[i]);
    		if (it != end_receivedata0.end()) {
        		size_t index = std::distance(end_receivedata0.begin(), it);
        		// 将匹配的元素及其对应的值添加到 jiaoji_result 中
        		jiaoji_result.push_back({Alicedata0[i], Alicedata1[i], end_receivedata1[index]});
        		abc_count++;
				k = index + 1;
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
  		
		sort(receivedData.begin(), receivedData.begin() + num);
  		
		vector<uint64_t> Bobdata0=readDataFromFile("Bobdata.txt", 1);
  		Bobdata0.erase(Bobdata0.begin());
  		vector<uint64_t> Bobdata1=readDataFromFile("Bobdata.txt", 2);
  		Bobdata1.erase(Bobdata1.begin());
  
		vector<uint64_t> end_senddata0, end_senddata1;
        int end_data_count = 0;
		
		sortWithIndex(Bobdata0, Bobdata1);

		// 从Bobdata0中找到与receivedData中相同的元素，并将其索引对应的Bobdata0和Bobdata1加入end_senddata0和end_senddata1
		int k = 0;
		for (const auto& value : receivedData) {
    		auto it = std::find(Bobdata0.begin() + k, Bobdata0.end(), value);
    		if (it != Bobdata0.end()) {
        		size_t index = std::distance(Bobdata0.begin(), it);
        		end_senddata0.push_back(Bobdata0[index]);
        		end_senddata1.push_back(Bobdata1[index]);
        		end_data_count++;
        		k = index + 1; 
    		}
		}
		// todo 加噪声（差分隐私）
		gParty.Send(end_senddata0);
  		gParty.Send(end_senddata1);
  		cout<<"Bob transmitted the data to Alice successfully"<<endl;
 	}
 	
	float duration = 1000.0 * (clock() - start) / CLOCKS_PER_SEC;
    cout << "intersection: " << duration << " ms" << endl;
    gParty.Reset();
 
 	delete[] a;
 	delete psi;
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


