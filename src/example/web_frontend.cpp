#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <sstream>
#include "../core/httplib.h"
#include "TPCH.h"
#include "../core/relation.h"

using namespace std;
using namespace SECYAN;

// 导入multipartypsi.cpp中的函数声明
extern void run_Q3(DataSize ds, bool printResult, bool resultProtection);
extern Relation::RelationInfo GetRI(RelationName rn, QueryName qn, DataSize ds, e_role owner);
extern std::string GetFilePath(RelationName rn, DataSize ds);

// 捕获输出的函数
class OutputCapture {
private:
    std::stringstream buffer;
    std::streambuf* old_cout;
    bool active;
    
public:
    OutputCapture() : active(false) {
        // 在构造函数中不立即捕获输出
    }
    
    void start() {
        if (!active) {
            old_cout = std::cout.rdbuf(buffer.rdbuf());
            active = true;
        }
    }
    
    void stop() {
        if (active) {
            std::cout.rdbuf(old_cout);
            active = false;
        }
    }
    
    std::string getOutput() {
        std::string result = buffer.str();
        buffer.str(""); // 清空缓冲区
        return result;
    }
    
    ~OutputCapture() {
        // 确保在析构时恢复标准输出
        if (active) {
            std::cout.rdbuf(old_cout);
        }
    }
};

int main() {
    httplib::Server svr;
    
    // 提供静态文件
    svr.set_mount_point("/", "./web");
    
    // API端点处理PSI查询
    svr.Post("/api/run_psi", [](const httplib::Request& req, httplib::Response& res) {
        int irole = 0; // 默认为服务器角色
        int iqn = 0;   // 默认为Q3查询
        int ids = 0;   // 默认为1MB数据大小
        bool rp = false; // 默认不启用结果保护
        
        // 从请求中获取参数
        if (req.has_param("role")) {
            irole = std::stoi(req.get_param_value("role"));
        }
        if (req.has_param("query")) {
            iqn = std::stoi(req.get_param_value("query"));
        }
        if (req.has_param("datasize")) {
            ids = std::stoi(req.get_param_value("datasize"));
        }
        if (req.has_param("resultprotection")) {
            rp = req.get_param_value("resultprotection") == "1";
        }
        
        // 验证参数
        if (irole != 0 && irole != 1) {
            res.set_content("角色错误！必须是0(服务器)或1(客户端)", "text/plain; charset=utf-8");
            return;
        }
        
        if (iqn < 0 || iqn >= 5) {
            res.set_content("查询选择错误！必须是0-4之间的数字", "text/plain; charset=utf-8");
            return;
        }
        
        if (ids < 0 || ids >= 5) {
            res.set_content("数据大小选择错误！必须是0-4之间的数字", "text/plain; charset=utf-8");
            return;
        }
        
        e_role role = (e_role)irole;
        QueryName qn = (QueryName)iqn;
        DataSize ds = (DataSize)ids;
        
        // 捕获输出
        OutputCapture capture;
        std::string output;
        
        try {
            // 开始捕获输出
            capture.start();
            
            cout << "建立连接中..." << endl;
            gParty.Init("127.0.0.1", 7766, role);
            cout << "连接成功！" << endl;
            
            cout << "开始运行查询..." << endl;
            gParty.Tick("运行时间");
            
            // 目前只实现了Q3查询
            if (qn == Q3) {
                run_Q3(ds, true, rp);
            } else {
                cout << "暂不支持的查询类型，目前只实现了Q3查询。" << endl;
            }
            
            gParty.Tick("运行时间");
            auto cost = gParty.GetCommCostAndResetStats();
            cout << "通信成本: " << cost / 1024 / 1024.0 << " MB" << endl;
            cout << "完成！" << endl;
            
            // 停止捕获并获取输出
            capture.stop();
            output = capture.getOutput();
            
            // 返回捕获的输出
            res.set_content(output, "text/plain; charset=utf-8");
        } catch (const std::exception& e) {
            // 确保停止捕获
            capture.stop();
            output = capture.getOutput();
            
            // 添加错误信息
            output += "\n错误: " + std::string(e.what());
            res.set_content(output, "text/plain; charset=utf-8");
        }
    });
    
    // 启动服务器
    std::cout << "启动Web服务器在 http://localhost:8080" << std::endl;
    svr.listen("localhost", 8080);
    
    return 0;
} 