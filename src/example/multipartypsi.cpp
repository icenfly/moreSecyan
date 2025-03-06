#include <cstdint>
#include <string>
#include <functional>
#include "TPCH.h"
#include <vector>
#include "../core/relation.h"
#include <iostream>
#include <chrono>

using namespace std;
using namespace SECYAN;

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
			orders.Print_Avg_ResultProtection("AVG(orders.annotation)");
		}
		else{
			orders.RevealAnnotToOwner();
			orders.Print();
		}
	}
}

void run_Q3_DE(DataSize ds, bool printResult, bool resultProtection)
{
	vector<string> o_groupBy = {"o_orderkey", "o_orderdate", "o_shippriority"};

	auto s_cust_ri = GetRI(CUSTOMER, Q3, ds, SERVER);
	Relation::AnnotInfo s_cust_ai = {true, true};
	Relation s_customer(s_cust_ri, s_cust_ai);
	auto s_customer_filePath = GetFilePath(CUSTOMER, ds);
	s_customer.LoadData(s_customer_filePath.c_str(), "q3_annot");

	auto c_cust_ri = GetRI(CUSTOMER, Q3, ds, CLIENT);
	Relation::AnnotInfo c_cust_ai = {true, true};
	Relation c_customer(c_cust_ri, c_cust_ai);
	auto c_customer_filePath = GetFilePath(CUSTOMER, ds);
	c_customer.LoadData(c_customer_filePath.c_str(), "q3_annot");

	auto c_orders_ri = GetRI(ORDERS, Q3, ds, CLIENT);
	Relation::AnnotInfo c_orders_ai = {true, true};
	Relation c_orders(c_orders_ri, c_orders_ai);
	auto c_orders_filePath = GetFilePath(ORDERS, ds);
	c_orders.LoadData(c_orders_filePath.c_str(), "q3_annot");
	//orders.Print();

	auto s_orders_ri = GetRI(ORDERS, Q3, ds, SERVER);
	Relation::AnnotInfo s_orders_ai = {true, true};
	Relation s_orders(s_orders_ri, s_orders_ai);
	auto s_orders_filePath = GetFilePath(ORDERS, ds);
	s_orders.LoadData(s_orders_filePath.c_str(), "q3_annot");

	auto s_lineitem_ri = GetRI(LINEITEM, Q3, ds, SERVER);
	Relation::AnnotInfo s_lineitem_ai = {false, true};
	Relation s_lineitem(s_lineitem_ri, s_lineitem_ai);
	auto s_lineitem_filePath = GetFilePath(LINEITEM, ds);
	s_lineitem.LoadData(s_lineitem_filePath.c_str(), "q3_annot");
	s_lineitem.Aggregate();

	auto c_lineitem_ri = GetRI(LINEITEM, Q3, ds, CLIENT);
	Relation::AnnotInfo c_lineitem_ai = {false, true};
	Relation c_lineitem(c_lineitem_ri, c_lineitem_ai);
	auto c_lineitem_filePath = GetFilePath(LINEITEM, ds);
	c_lineitem.LoadData(c_lineitem_filePath.c_str(), "q3_annot");
	c_lineitem.Aggregate();

	c_orders.SemiJoin(s_customer, "o_custkey", "c_custkey");

	c_orders.SemiJoin(s_lineitem, "o_orderkey", "l_orderkey");

	c_orders.Aggregate(o_groupBy);

	s_orders.SemiJoin(c_customer, "o_custkey", "c_custkey");

	s_orders.SemiJoin(c_lineitem, "o_orderkey", "l_orderkey");

	s_orders.Aggregate(o_groupBy);

	if(printResult){
		c_orders.RevealAnnotToOwner();
		s_orders.RevealAnnotToOwner();
		if (c_orders.Equal(s_orders)){
			if(resultProtection){
				cout << "Dual Execution: orders as client: " << endl;
				c_orders.Print_Avg_ResultProtection("AVG(orders.annotation)");
				cout << "Dual Execution: orders as server: " << endl;
				s_orders.Print_Avg_ResultProtection("AVG(orders.annotation)");
			}
			else{
				cout << "Dual Execution: orders as client: " << endl;
				c_orders.Print();
				cout << "Dual Execution: orders as server: " << endl;
				s_orders.Print();
			}
			cout << "Result Verification passed!" << endl;
		}
		else{
			cout << "Result Verification failed! Abort printResult!" << endl;
		}
	}
}

int main(int argc, char **)
{
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

	bool de = false;
	cout << "Do you want to enable dual execution? [0. No, 1. Yes]: ";
	cin >> de;

    cout << "Start running query..." << endl;
    gParty.Tick("Running time");
	if(de){
		run_Q3_DE(ds, true, rp);
	}
	else{
		run_Q3(ds, true, rp);
	}
    gParty.Tick("Running time");
    auto cost = gParty.GetCommCostAndResetStats();
    cout << "Communication cost: " << cost / 1024 / 1024.0 << " MB" << endl;
    cout << "Finished!" << endl;
    return 0;
}