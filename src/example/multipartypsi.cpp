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
function<run_query_m> query_funcs[QTOTAL] = {run_Q3_m, run_Q10_m, run_Q18_m, run_Q8_m, run_Q9_m};

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
	query_funcs[qn](ds, true, rp, de);
    gParty.Tick("Running time");
    auto cost = gParty.GetCommCostAndResetStats();
    cout << "Communication cost: " << cost / 1024 / 1024.0 << " MB" << endl;
    cout << "Finished!" << endl;
    return 0;
}