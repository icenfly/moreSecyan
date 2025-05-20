#pragma once

#include "../core/relation.h"
#include <string>
#include <cstdint>
#include <vector>

using namespace SECYAN;

enum RelationName
{
    CUSTOMER,
    ORDERS,
    LINEITEM,
    PART,
    SUPPLIER,
    PARTSUPP,
    RTOTAL
};

enum QueryName
{
    Q3,
    Q10,
    Q18,
    Q8,
    Q9,
    QTOTAL
};

enum DataSize
{
    _1MB,
    _3MB,
    _10MB,
    _33MB,
    _100MB,
    DTOTAL
};

using run_query = void(DataSize, bool);
run_query run_Q3;

using run_query_m = void(DataSize, bool, bool, bool);
run_query_m run_Q3_m;