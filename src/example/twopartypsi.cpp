#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <set>
#include <stdexcept> 
#include <vector>
#include <algorithm>
#include <cstdint>
#include "TPCH.h"
#include "circuit/circuit.h"
#include "circuit/share.h"
#include "../core/httplib.h"
#include "../core/OEP.h"
#include "../core/relation.h"
#include "../core/PSI.h"
#include "../core/party.h"
#include "../core/RNG.h"

using namespace std;

size_t NumRows[RTOTAL][DTOTAL] = {
	{150, 450, 1500, 4950, 15000},
	{1500, 4500, 15000, 49500, 150000},
	{6005, 17973, 60175, 198344, 595215},
	{200, 600, 2000, 6600, 20000},
	{10, 30, 100, 330, 1000},
	{800, 2400, 8000, 26400, 80000}};

std::vector<std::string> AttrNames[RTOTAL][QTOTAL] = {
	{{"c_custkey"}, {"c_custkey", "c_name", "c_nationkey"}, {"c_custkey", "c_name"}, {"c_custkey"}, {}},
	{{"o_custkey", "o_orderkey", "o_orderdate", "o_shippriority"}, {"o_custkey", "o_orderkey"}, {"o_custkey", "o_orderkey", "o_orderdate", "o_totalprice"}, {"o_orderkey", "o_custkey", "o_year"}, {"o_orderkey", "o_year"}},
	{{"l_orderkey"}, {"l_orderkey"}, {"l_orderkey"}, {"l_orderkey", "l_suppkey", "l_partkey"}, {"l_orderkey", "l_suppkey", "l_partkey"}},
	{{}, {}, {}, {"p_partkey"}, {"p_partkey"}},
	{{}, {}, {}, {"s_suppkey"}, {"s_suppkey"}},
	{{}, {}, {}, {}, {"ps_partkey", "ps_suppkey"}}};

std::vector<Relation::DataType> AttrTypes[RTOTAL][QTOTAL] = {
	{{Relation::INT}, {Relation::INT, Relation::STRING, Relation::INT}, {Relation::INT, Relation::STRING}, {Relation::INT}, {}},
	{{Relation::INT, Relation::INT, Relation::DATE, Relation::STRING}, {Relation::INT, Relation::INT}, {Relation::INT, Relation::INT, Relation::DATE, Relation::DECIMAL}, {Relation::INT, Relation::INT, Relation::INT}, {Relation::INT, Relation::INT}},
	{{Relation::INT}, {Relation::INT}, {Relation::INT}, {Relation::INT, Relation::INT, Relation::INT}, {Relation::INT, Relation::INT, Relation::INT}},
	{{}, {}, {}, {Relation::INT}, {Relation::INT}},
	{{}, {}, {}, {Relation::INT}, {Relation::INT}},
	{{}, {}, {}, {}, {Relation::INT, Relation::INT}}};

std::string filenames[] = {
	"customer.tbl",
	"orders.tbl",
	"lineitem.tbl",
	"part.tbl",
	"supplier.tbl",
	"partsupp.tbl"};

std::string datapaths[] = {
	"../../../data/1MB/",
	"../../../data/3MB/",
	"../../../data/10MB/",
	"../../../data/33MB/",
	"../../../data/100MB/"};

inline Relation::RelationInfo GetRI(RelationName rn, QueryName qn, DataSize ds, e_role owner)
{
	Relation::RelationInfo ri = {
		owner,
		false,
		AttrNames[rn][qn],
		AttrTypes[rn][qn],
		NumRows[rn][ds],
		false};
	return ri;
}
inline std::string GetFilePath(RelationName rn, DataSize ds)
{
	return datapaths[ds] + filenames[rn];
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

void readData(string filename, vector<char*>& columnNames, vector<vector<char*>>& data) {
    int columnNum;

    ifstream inputFile(filename); // 打开文件

    if (!inputFile.is_open()) {
        cerr << "无法打开文件: " << filename << endl;
        return; // 如果文件打开失败，则返回
    }

    string line;

    // 读取 columnNum
    getline(inputFile, line);
    try {
        columnNum = stoi(line); // 将读取的字符串转换为整数
    } catch (const invalid_argument& e) {
        cerr << "Error: 第一行不是有效的数字: " << line << endl;
        inputFile.close();
        return;
    }

    // 读取 columnNames
    getline(inputFile, line);
    stringstream ss_columnNames(line); // 使用stringstream来分割字符串
    string columnName;
    while (getline(ss_columnNames, columnName, '|')) {
        char* cstr = new char[columnName.length() + 1];
        strcpy(cstr, columnName.c_str());
        columnNames.push_back(cstr);
    }

    // 读取 data
    while (getline(inputFile, line)) {
        stringstream ss_data(line); // 同样使用stringstream来分割每一行数据
        string cell;
        vector<char*> rowData;
        while (getline(ss_data, cell, '|')) {
            char* cstr = new char[cell.length() + 1];
            strcpy(cstr, cell.c_str());
            rowData.push_back(cstr);
        }
        data.push_back(rowData);
    }

    inputFile.close(); // 关闭文件
}

void readSetFromData(vector<vector<char*>> data, vector<char*> colNames, string specColumn, vector<uint64_t>& set) {
    int columnIndex = -1; // 初始化列索引为 -1，表示未找到

    // 1. 查找 specColumn 在 colNames 中的索引
    for (int i = 0; i < colNames.size(); ++i) {
        if (string(colNames[i]) == specColumn) {
            columnIndex = i; // 找到列名，记录索引
            break; // 找到即可跳出循环
        }
    }

    // 检查是否找到指定的列名
    if (columnIndex == -1) {
        cerr << "Error: 指定的列名 '" << specColumn << "' 未在列名列表中找到." << endl;
    }

    std::set<uint64_t> uniqueSet; // 使用 set 自动去重和排序

    // 2. 遍历 data，提取指定列的数据并转换为 uint64_t
    for (const auto& row : data) {
        // 确保当前行有足够的列数，防止越界访问
        if (columnIndex < row.size()) {
            string cellValueStr = row[columnIndex];
            try {
                // 将字符串转换为 uint64_t
                uint64_t cellValue = stoull(cellValueStr);
                uniqueSet.insert(cellValue); // 插入 set，自动去重和排序
            } catch (const invalid_argument& e) {
                cerr << "Warning: 无法将值 '" << cellValueStr << "' 转换为 uint64_t (无效参数). 行数据将被忽略." << endl;
                continue; // 忽略当前行数据，继续下一行
            } catch (const out_of_range& e) {
                cerr << "Warning: 值 '" << cellValueStr << "' 超出 uint64_t 的范围. 行数据将被忽略." << endl;
                continue; // 忽略当前行数据，继续下一行
            }
        } else {
            cerr << "Warning: 数据行长度小于列索引，可能数据不完整. 行数据将被忽略." << endl;
        }
    }

    // 3. 将 set 转换为 vector
    set.assign(uniqueSet.begin(), uniqueSet.end());
}

void join(Party& gParty, RelationName rn, DataSize ds, string querykey)
{
	auto role = gParty.GetRole();
	auto bc = gParty.GetCircuit(S_BOOL);
	clock_t start;
 	start = clock();

    vector<char*> AlicecolNames;
    vector<char*> BobcolNames;
    vector<vector<char*>> AliceData;
    vector<vector<char*>> BobData;
    readData(GetFilePath(rn, ds), AlicecolNames, AliceData);
    readData(GetFilePath(rn, ds), BobcolNames, BobData);

    vector<uint64_t> AliceSet;
	vector<uint64_t> BobSet;
    readSetFromData(AliceData, AlicecolNames, querykey, AliceSet);
    readSetFromData(BobData, BobcolNames, querykey, BobSet);

	PSI *psi = (role == SERVER) ?
		new PSI(AliceSet, AliceSet.size(), BobSet.size(), PSI::Alice) :
		new PSI(BobSet, AliceSet.size(), BobSet.size(), PSI::Bob);
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

        vector<vector<char*>> AliceIntersectData;
        int columnIndex = -1; // 初始化列索引为 -1，表示未找到

        // 1. 查找 querykey 在 AlicecolNames 中的位置 columnIndex
        for (int i = 0; i < AlicecolNames.size(); ++i) {
            if (string(AlicecolNames[i]) == querykey) {
                columnIndex = i; // 找到列名，记录索引
                break; // 找到即可跳出循环
            }
        }

        // 2. 根据 columnIndex 从 BobData 中提取数据
        for(int i = 0; i < AliceData.size(); i++){
            if(find(intersectData.begin(), intersectData.end(), string(AliceData[i][columnIndex])) != intersectData.end()){
                AliceIntersectData.push_back(AliceData[i]);
            }
        }

   		vector<uint64_t> end_receivedata0;
 		vector<uint64_t> end_receivedata1;
        vector<vector<uint64_t>> BobIntersectData;
  		gParty.Recv(end_receivedata0);
        /*
        for(int i = 0; i < end_receivedata0[0]; i++){
            gParty.Recv(end_receivedata1);
            BobIntersectData.push_back(end_receivedata1);
        }*/

        int intersectCount = 0;
        vector<vector<char*>> intersectResult;


      	cout<<"Intersect Result"<<endl;
        for(auto col : AlicecolNames){
            cout<<col<<" ";
        }
        for(auto col : BobcolNames){
            cout<<col<<" ";
        }
        cout<<endl;
        for(int i = 0; i < end_receivedata0[0]; i++){
            for(auto col : AliceIntersectData[i]){
                cout<<col<<" ";
            }/*
            for(auto col : BobIntersectData[i]){
                cout<<col<<" ";
            }*/
            cout<<endl;
        }
      	cout<<"total:   "<<intersectCount<<endl;
        for(int i = 0; i < intersectCount; i++){
      	cout << "The intersection is successfully solved!" << endl;
	    }
    }
	if(role==CLIENT){
  		vector<uint32_t> selectBits(num, 1);
  		vector<uint64_t> receivedData = gParty.OTRecv(selectBits);
  		
		sort(receivedData.begin(), receivedData.begin() + num);
  
		vector<uint64_t> end_senddata0;
        vector<vector<char*>> end_senddata1;
        int end_data_count = 0;
	
        int columnIndex = -1; // 初始化列索引为 -1，表示未找到

        // 1. 查找 querykey 在 BobcolNames 中的位置 columnIndex
        for (int i = 0; i < BobcolNames.size(); ++i) {
            if (string(BobcolNames[i]) == querykey) {
                columnIndex = i; // 找到列名，记录索引
                break; // 找到即可跳出循环
            }
        }

        // 2. 根据 columnIndex 从 BobData 中提取数据
        for(int i = 0; i < BobData.size(); i++){
            if(find(receivedData.begin(), receivedData.end(), string(BobData[i][columnIndex])) != receivedData.end()){
                end_senddata1.push_back(BobData[i]);
                end_data_count ++;
            }
        }

		// todo 加噪声（差分隐私）
        end_senddata0.push_back(end_data_count);
        /*
		gParty.Send(end_senddata0);
        for(int i = 0; i < end_data_count; i++){
            gParty.Send(end_senddata1[i]);
        }*/
  		cout<<"Bob transmitted the data to Alice successfully"<<endl;
 	}
 	
	float duration = 1000.0 * (clock() - start) / CLOCKS_PER_SEC;
    cout << "intersection: " << duration << " ms" << endl;
    gParty.Reset();
 
 	delete[] a;
 	delete psi;
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
        /*
        for (const auto& row : jiaoji_result) { 
            response_stream << row[0] << "\t\t" << row[1] << "\t\t" << row[2] << "\n"; 
            
        }*/
        
        res.set_content(response_stream.str(), "text/plain");
    
        
    }
    
void handle_options(const httplib::Request& req, httplib::Response& res) {
        // 为OPTIONS请求设置
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    }

// 初始化函数，与 mine.cpp 中的初始化部分对应
void initializeParty(Party& gParty, int argc, char* argv[]) {
    // 第一个参数是SERVER 或 CLIENT，第二个参数是customer.tbl 或 orders.tbl 或 lineitem.tbl 或 part.tbl 或 supplier.tbl 或 partsupp.tbl
    if (argc < 3) {
        cerr << "Usage: <program> SERVER|CLIENT <datafile>" << endl;
        exit(1);
    }
    string role = argv[1];
    if (role == "SERVER") {
        gParty.Init("localhost", 12345, SERVER);
    }
    else if (role == "CLIENT") {
        gParty.Init("localhost", 12345, CLIENT);
    }
    else{
        cerr << "Invalid role. Use SERVER or CLIENT." << endl;
        exit(1);
    }

    string filename = argv[2];
    // input with CUSTOMER, ORDERS, LINEITEM, PART, SUPPLIER, PARTSUPP, and check them with enum RelationName
    if (filename != "CUSTOMER" && filename != "ORDERS" && filename != "LINEITEM" && filename != "PART" && filename != "SUPPLIER" && filename != "PARTSUPP") {
        cerr << "Invalid data file. Use one of: CUSTOMER, ORDERS, LINEITEM, PART, SUPPLIER, PARTSUPP" << endl;
        exit(1);
    }

    RelationName rn;
    if (filename == "CUSTOMER") rn = CUSTOMER;
    else if (filename == "ORDERS") rn = ORDERS;
    else if (filename == "LINEITEM") rn = LINEITEM;
    else if (filename == "PART") rn = PART;
    else if (filename == "SUPPLIER") rn = SUPPLIER;
    else if (filename == "PARTSUPP") rn = PARTSUPP;

    string datasize = argv[3];
    if (datasize != "1MB" && datasize != "3MB" && datasize != "10MB" && datasize != "33MB" && datasize != "100MB") {
        cerr << "Invalid data size. Use one of: 1MB, 3MB, 10MB, 33MB, 100MB" << endl;
        exit(1);
    }

    DataSize ds;
    if (datasize == "1MB") ds = _1MB;
    else if (datasize == "3MB") ds = _3MB;
    else if (datasize == "10MB") ds = _10MB;
    else if (datasize == "33MB") ds = _33MB;
    else if (datasize == "100MB") ds = _100MB;

    string querykey = argv[4];
    if (querykey == "") {
        cerr << "Invalid query key. Please Specify." << endl;
        exit(1);
    }
    
    join(gParty, rn, ds, querykey);

    httplib::Server server1;
    // 处理OPTIONS请求
    server1.Options("/", handle_options);

    // 处理 GET 请求
    server1.Get("/", handle_request);

    int port1 = 8080;
   
    if(role == "SERVER"){
        std::cout << "Listening for requests..." << std::endl;
        server1.listen("localhost", port1);	
    }
}
    
int main(int argc, char* argv[]) {
    Party gParty;
    // 初始化
    initializeParty(gParty, argc, argv);

    // 关闭通信
    gParty.Reset();
    return 0;
}
