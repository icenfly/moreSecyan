#include <vector>
#include <string>
#include "../core/relation.h"
#include "circuit/circuit.h"
#include "circuit/share.h"
#include <iostream>
#include <chrono>
#include "TPCH.h"
#include "unistd.h"

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

void run_Q3(DataSize ds, bool printResult)
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
	orders.RevealAnnotToOwner();
	if (printResult)
		orders.Print();
}

void run_Q3_m(DataSize ds, bool printResult, bool resultProtection, bool dualExecution)
{
	if(!dualExecution){
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
	else{
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
			bool verified = true;
			Relation c_orders_copy = c_orders;
			c_orders_copy.RemoveZeroAnnotatedTuples();
			Relation s_orders_copy = s_orders;
			s_orders_copy.RemoveZeroAnnotatedTuples();
			if (gParty.GetRole() == CLIENT)
			{
				vector<uint64_t> send_filtered_packedTuples = c_orders_copy.PackTuples();
				gParty.Send(send_filtered_packedTuples);
			}
			else
			{	
				// Then receive the packed tuples
				vector<uint64_t> filtered_packedTuples;
				gParty.Recv(filtered_packedTuples);

				vector<uint64_t> s_filtered_packedTuples = s_orders_copy.PackTuples();
				
				if (s_filtered_packedTuples.size() != filtered_packedTuples.size()) {
					cout << "Result verification failed! Dual Execution results have different structures." << endl;
					cout << "Server filtered relation size: " << s_filtered_packedTuples.size() << endl;
					cout << "Client filtered relation size: " << filtered_packedTuples.size() << endl;
					verified = false;
				} else {
					// Compare content of what would be printed
					for (uint32_t i = 0; i < s_filtered_packedTuples.size(); i++) {
						if (s_filtered_packedTuples[i] != filtered_packedTuples[i]) {
							cout << "Result verification failed! Dual Execution results differ at position " << i << endl;
							cout << "Server value: " << s_filtered_packedTuples[i] << endl;
							cout << "Client value: " << filtered_packedTuples[i] << endl;
							verified = false;
							break;
						}
					}
				}
			}
			if (verified){
				if(resultProtection){
					cout << "c_orders.Print_Avg_ResultProtection(\"AVG(orders.annotation)\")" << endl;
					c_orders.Print_Avg_ResultProtection("AVG(orders.annotation)");
					cout << "s_orders.Print_Avg_ResultProtection(\"AVG(orders.annotation)\")" << endl;
					s_orders.Print_Avg_ResultProtection("AVG(orders.annotation)");
				}
				else{
					cout << "c_orders.Print()" << endl;
					c_orders.Print();
					cout << "s_orders.Print()" << endl;
					s_orders.Print();
				}
				cout << "Result verification passed!" << endl;
			}
		}
	}
}
