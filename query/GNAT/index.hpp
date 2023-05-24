#pragma once
#include "db.hpp"

class index_t
{
public:
	db_t* db = nullptr;

	double dist(int x, int y)
	{
		//++dist_call_cnt;
		return db->dist(x, y);
	}
	double dist(int x,float *y)
	{
	    //++dist_call_cnt;
        return db->dist(x,y);
	}
public:
	long long dist_call_cnt = 0;
	index_t(){}
	index_t(db_t* db) :db(db) {}
	virtual void build() = 0;
};
