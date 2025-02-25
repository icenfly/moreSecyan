#include <cstdint>
#include <string>
#include <functional>
#include "TPCH.h"
#include <vector>
#include "../core/relation.h"
#include <iostream>
#include <chrono>
#include <sstream>
#include <thread>
#include <mutex>
#include "../core/httplib.h"

using namespace std;
using namespace SECYAN;

// 简单的JSON解析和生成类
class SimpleJSON {
public:
    // 从字符串解析JSON
    static std::map<std::string, std::string> parse(const std::string& json_str) {
        std::map<std::string, std::string> result;
        size_t pos = 0;
        
        // 跳过开头的 {
        pos = json_str.find('{', pos);
        if (pos == std::string::npos) return result;
        pos++;
        
        while (pos < json_str.length()) {
            // 跳过空白
            while (pos < json_str.length() && isspace(json_str[pos])) pos++;
            if (pos >= json_str.length() || json_str[pos] == '}') break;
            
            // 解析键
            if (json_str[pos] != '"') {
                pos = json_str.find('"', pos);
                if (pos == std::string::npos) break;
            }
            pos++; // 跳过 "
            
            size_t key_start = pos;
            pos = json_str.find('"', pos);
            if (pos == std::string::npos) break;
            
            std::string key = json_str.substr(key_start, pos - key_start);
            pos++; // 跳过 "
            
            // 跳过 :
            pos = json_str.find(':', pos);
            if (pos == std::string::npos) break;
            pos++;
            
            // 跳过空白
            while (pos < json_str.length() && isspace(json_str[pos])) pos++;
            
            // 解析值
            std::string value;
            if (pos < json_str.length()) {
                if (json_str[pos] == '"') {
                    // 字符串值
                    pos++; // 跳过 "
                    size_t value_start = pos;
                    pos = json_str.find('"', pos);
                    if (pos == std::string::npos) break;
                    
                    value = json_str.substr(value_start, pos - value_start);
                    pos++; // 跳过 "
                } else if (isdigit(json_str[pos]) || json_str[pos] == '-') {
                    // 数字值
                    size_t value_start = pos;
                    while (pos < json_str.length() && 
                           (isdigit(json_str[pos]) || json_str[pos] == '.' || 
                            json_str[pos] == '-' || json_str[pos] == 'e' || 
                            json_str[pos] == 'E' || json_str[pos] == '+')) {
                        pos++;
                    }
                    value = json_str.substr(value_start, pos - value_start);
                } else if (json_str.substr(pos, 4) == "true") {
                    value = "true";
                    pos += 4;
                } else if (json_str.substr(pos, 5) == "false") {
                    value = "false";
                    pos += 5;
                } else if (json_str.substr(pos, 4) == "null") {
                    value = "null";
                    pos += 4;
                }
            }
            
            result[key] = value;
            
            // 跳过逗号
            pos = json_str.find_first_of(",}", pos);
            if (pos == std::string::npos) break;
            if (json_str[pos] == '}') break;
            pos++; // 跳过 ,
        }
        
        return result;
    }
    
    // 生成JSON字符串
    static std::string stringify(const std::map<std::string, std::string>& data) {
        std::stringstream ss;
        ss << "{";
        
        bool first = true;
        for (const auto& pair : data) {
            if (!first) ss << ",";
            first = false;
            
            ss << "\"" << pair.first << "\":";
            
            // 检查值是否是数字、布尔值或null
            if (pair.second == "true" || pair.second == "false" || pair.second == "null" ||
                (pair.second.length() > 0 && (isdigit(pair.second[0]) || pair.second[0] == '-'))) {
                ss << pair.second;
            } else {
                ss << "\"" << pair.second << "\"";
            }
        }
        
        ss << "}";
        return ss.str();
    }
};

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

std::string filename[] = {
	"customer.tbl",
	"orders.tbl",
	"lineitem.tbl",
	"part.tbl",
	"supplier.tbl",
	"partsupp.tbl"};

std::string datapath[] = {
	"../../../data/1MB/",
	"../../../data/3MB/",
	"../../../data/10MB/",
	"../../../data/33MB/",
	"../../../data/100MB/"};

// 用于存储查询结果的全局变量
std::mutex result_mutex;
std::string query_results;
std::string running_time;
double comm_cost = 0.0;

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
	return datapath[ds] + filename[rn];
}

// 自定义输出流，用于捕获查询结果
class CaptureStream : public std::stringstream {
public:
    CaptureStream() : std::stringstream() {}
    
    template<typename T>
    CaptureStream& operator<<(const T& value) {
        std::stringstream::operator<<(value);
        return *this;
    }
    
    // 重载endl操作符
    CaptureStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
        manip(*this);
        return *this;
    }
};

// 捕获查询结果的输出流
CaptureStream resultStream;

void run_Q3(DataSize ds, bool printResult, bool resultProtection)
{
	vector<string> o_groupBy = {"o_orderkey", "o_orderdate", "o_shippriority"};
	auto cust_ri = GetRI(CUSTOMER, Q3, ds, SERVER);
	Relation::AnnotInfo cust_ai = {true, true};
	Relation customer(cust_ri, cust_ai);
	auto filePath = GetFilePath(CUSTOMER, ds);
	customer.LoadData(filePath.c_str(), "q3_annot");

	auto orders_ri = GetRI(ORDERS, Q3, ds, CLIENT);
	Relation::AnnotInfo orders_ai = {true, true};
	Relation orders(orders_ri, orders_ai);
	filePath = GetFilePath(ORDERS, ds);
	orders.LoadData(filePath.c_str(), "q3_annot");
	//orders.Print();

	auto lineitem_ri = GetRI(LINEITEM, Q3, ds, SERVER);
	Relation::AnnotInfo lineitem_ai = {false, true};
	Relation lineitem(lineitem_ri, lineitem_ai);
	filePath = GetFilePath(LINEITEM, ds);
	lineitem.LoadData(filePath.c_str(), "q3_annot");
	lineitem.Aggregate();

	orders.SemiJoin(customer, "o_custkey", "c_custkey");
	//orders.PrintTableWithoutRevealing("orders semijoin customer");
	// PSI: 95.4267ms
	// Two OEPs: 53.8459ms
	// AnnotMul: 4.4265ms

	orders.SemiJoin(lineitem, "o_orderkey", "l_orderkey");
	//orders.PrintTableWithoutRevealing("orders semijoin lineitem");
	// PSI: 162.877ms
	// Two OEPs: 25.8395ms
	// AnnotMul: 51.1201ms

	orders.Aggregate(o_groupBy);

	if (printResult){
		if(resultProtection){
			orders.RevealAnnotToOwner();
			// 将结果输出到捕获流
			std::streambuf* oldCoutStreamBuf = std::cout.rdbuf();
			std::cout.rdbuf(resultStream.rdbuf());
			
			orders.Print_Avg_ResultProtection("AVG(orders.annotation)");
			
			// 恢复标准输出
			std::cout.rdbuf(oldCoutStreamBuf);
		}
		else{
			orders.RevealAnnotToOwner();
			// 将结果输出到捕获流
			std::streambuf* oldCoutStreamBuf = std::cout.rdbuf();
			std::cout.rdbuf(resultStream.rdbuf());
			
			orders.Print();
			
			// 恢复标准输出
			std::cout.rdbuf(oldCoutStreamBuf);
		}
	}
}

// 运行查询的函数，用于Web服务器调用
bool run_query(int role, int query_type, int data_size, bool result_protection, 
               const std::string& server_address, int server_port) {
    try {
        e_role user_role = (e_role)role;
        QueryName qn = (QueryName)query_type;
        DataSize ds = (DataSize)data_size;
        
        // 清空之前的结果
        {
            std::lock_guard<std::mutex> lock(result_mutex);
            resultStream.str("");
            query_results = "";
            running_time = "";
            comm_cost = 0.0;
        }
        
        // 初始化连接
        gParty.Init(server_address, server_port, user_role);
        
        // 开始计时
        gParty.Tick("Running time");
        
        // 根据查询类型运行相应的查询
        if (qn == Q3) {
            run_Q3(ds, true, result_protection);
        } else {
            // 其他查询类型的实现可以在这里添加
            return false;
        }
        
        // 结束计时并获取运行时间
        auto elapsed = gParty.Tick("Running time");
        running_time = elapsed;
        
        // 获取通信成本
        comm_cost = gParty.GetCommCostAndResetStats() / 1024.0 / 1024.0;
        
        // 保存查询结果
        {
            std::lock_guard<std::mutex> lock(result_mutex);
            query_results = resultStream.str();
        }
        
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error running query: " << e.what() << std::endl;
        return false;
    }
}

// Web服务器函数
void start_web_server(int port) {
    httplib::Server svr;
    
    // 设置静态文件目录
    svr.set_mount_point("/", "./web");
    
    // 处理查询请求
    svr.Post("/run_query", [](const httplib::Request& req, httplib::Response& res) {
        // 解析JSON请求
        auto json = SimpleJSON::parse(req.body);
        
        int role = std::stoi(json["role"]);
        int query_type = std::stoi(json["query_type"]);
        int data_size = std::stoi(json["data_size"]);
        bool result_protection = json["result_protection"] == "true";
        std::string server_address = json["server_address"];
        int server_port = std::stoi(json["server_port"]);
        
        // 运行查询
        bool success = run_query(role, query_type, data_size, result_protection, 
                                server_address, server_port);
        
        // 构建响应
        std::map<std::string, std::string> responseData;
        if (success) {
            std::lock_guard<std::mutex> lock(result_mutex);
            responseData["status"] = "success";
            responseData["results"] = query_results;
            responseData["running_time"] = running_time;
            responseData["comm_cost"] = std::to_string(comm_cost);
        } else {
            responseData["status"] = "error";
            responseData["error"] = "查询执行失败";
        }
        
        std::string response = SimpleJSON::stringify(responseData);
        
        res.set_content(response, "application/json");
    });
    
    // 启动服务器
    std::cout << "Web服务器启动在 http://localhost:" << port << std::endl;
    svr.listen("0.0.0.0", port);
}

int main(int argc, char **argv)
{
    // 检查是否有命令行参数
    if (argc > 1 && std::string(argv[1]) == "--web") {
        // 启动Web服务器模式
        int port = 8080;
        if (argc > 2) {
            port = std::stoi(argv[2]);
        }
        start_web_server(port);
        return 0;
    }
    
    // 传统命令行模式
    int irole, iqn, ids;
    string address = "127.0.0.1";
    uint16_t port = 7766;
    e_role role = SERVER;

    cout << "Who are you? [0. Server, 1. Client]: ";
    cin >> irole;
    if (irole != 0 && irole != 1)
    {
        cerr << "Role error!" << endl;
        exit(1);
    }
    role = (e_role)irole;

    cout << "Establishing connection... ";
    gParty.Init(address, port, role);
    cout << "Finished!" << endl;

    QueryName qn;
    DataSize ds;
    cout << "Which query to run? [0. Q3, 1. Q10, 2. Q18, 3. Q8, 4. Q9]: ";
    cin >> iqn;
    if (iqn < 0 || iqn >= 5)
    {
        cerr << "Query selection error!" << endl;
        exit(1);
    }
    qn = (QueryName)iqn;

    cout << "Which TPCH data size to use? [0. 1MB, 1. 3MB, 2. 10MB, 3. 33MB, 4. 100MB]: ";
    cin >> ids;
    if (ids < 0 || ids >= 5)
    {
        cerr << "Data size selection error!" << endl;
        exit(1);
    }
    ds = (DataSize)ids;

	bool rp = false;
	cout << "Do you want to enable result protection? [0. No, 1. Yes]: ";
	cin >> rp;

    cout << "Start running query..." << endl;
    gParty.Tick("Running time");
	run_Q3(ds, true, rp);
    gParty.Tick("Running time");
    auto cost = gParty.GetCommCostAndResetStats();
    cout << "Communication cost: " << cost / 1024 / 1024.0 << " MB" << endl;
    cout << "Finished!" << endl;
    return 0;
}