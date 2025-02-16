#include <iostream>
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

Relation fileLoadserver(string filename, vector<uint64_t>& keys, uint32_t& size) {
    RelationName rn;
    Relation::RelationInfo ri = GetRI(CUSTOMER, Q3, _1MB, SERVER);
    Relation::AnnotInfo ai = {true, true};
    Relation relation(ri, ai);
    auto filePath = GetFilePath(CUSTOMER, _1MB);
	relation.LoadData(filePath.c_str(), "q3_annot");
    
    keys = relation.Project(AttrNames[CUSTOMER][Q3][0]);
    sort(keys.begin(), keys.end());
    keys.erase(unique(keys.begin(), keys.end()), keys.end());
    size = keys.size();
    return relation;
}

Relation fileLoadclient(string filename, vector<uint64_t>& keys, uint32_t& size) {
    RelationName rn;
    Relation::RelationInfo ri = GetRI(ORDERS, Q3, _1MB, CLIENT);
    Relation::AnnotInfo ai = {true, true};
    Relation relation(ri, ai);
    auto filePath = GetFilePath(ORDERS, _1MB);
	relation.LoadData(filePath.c_str(), "q3_annot");
    
    keys = relation.Project(AttrNames[ORDERS][Q3]);
    sort(keys.begin(), keys.end());
    keys.erase(unique(keys.begin(), keys.end()), keys.end());
    size = keys.size();
    return relation;
}

// 服务端逻辑，与 mine.cpp 中的服务端部分对应
void serverLogic(Party& gParty, uint32_t *realout, Relation& aliceRelation) {
    for(int i=0;i<(int)realout.size();i++){
        if(realout[i]==1){
            cout << 1 << endl;
        }
    }
}

// 客户端逻辑，与 mine.cpp 中的客户端部分对应
void clientLogic(Party& gParty, uint32_t *realout, Relation& bobRelation) {
    for(int i=0;i<(int)realout.size();i++){
        if(realout[i]==1){
            cout << 1 << endl;
        }
    }
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
    if (find(begin(filenames), end(filenames), filename) == end(filenames)) {
        cerr << "Invalid data file. Use one of: customer.tbl, orders.tbl, lineitem.tbl, part.tbl, supplier.tbl, partsupp.tbl." << endl;
        exit(1);
    }

    vector<uint64_t> aliceKeys;
    vector<uint64_t> bobKeys;
    uint32_t aliceSize;
    uint32_t bobSize;
    auto bc = gParty.GetCircuit(S_BOOL);

    vector<uint32_t> out;

    if(role == "SERVER"){
        vector<uint32_t> as = {aliceSize};
        vector<uint32_t> bs;
        gParty.Send(as);
        gParty.Recv(bs);
        PSI psi(aliceKeys, aliceSize, bs[0], PSI::Alice);     
        out = psi.Intersect();
    }
    else{
        vector<uint32_t> as;
        vector<uint32_t> bs = {bobSize};
        gParty.Recv(as);
        gParty.Send(bs);
        PSI psi(bobKeys, as[0], bobSize, PSI::Bob);
        out = psi.Intersect();
    }

	auto s_in = bc->PutSharedSIMDINGate(out.size(), out.data(), 1);
	auto s_out = bc->PutOUTGate(s_in, ALL);
	uint32_t *realout;
	uint32_t b, c;
	gParty.ExecCircuit();
	s_out->get_clear_value_vec(&realout, &b, &c);

    if (role == "SERVER") {
        Relation aliceRelation = fileLoadserver(filename, aliceKeys, aliceSize);
        serverLogic(gParty, realout, aliceRelation);
    } else {
        Relation bobRelation = fileLoadclient(filename, bobKeys, bobSize);
        clientLogic(gParty, realout, bobRelation);
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