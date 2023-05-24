#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <iterator>
#include <iostream>
#include "../Node.hpp"
using namespace std;
#ifdef CALC_DIST_COUNT
extern double dcmp;
#endif
class db_t
{
public:
    virtual double memory_used()  = 0;
	virtual size_t size() const = 0;
	virtual void read(string path) = 0;
	virtual double dist(int x, int y) const = 0;
	virtual double dist(int x,float* y)=0;
};

class double_db_t : public db_t
{
public:
	int dimension = 0;
	vector<vector<float>> objs; //all data
public:
	size_t size() const
	{
		return objs.size();
	}
	double memory_used()
	{
	    return 1.0*sizeof(float)*dimension*objs.size()+4;
	}
	void read(string path);
	void read(vector<Node> &dataset);
	double dist(int x,float* y)
	{
	    double dist = 0.0;
	    //cout<<dimension<<" "<<y.size()<<"\n";
	    #ifdef CALC_DIST_COUNT
	    dcmp++;
	    #endif
        for (int i = 0; i < dimension; i++) {
            double dx=objs[x][i] - y[i];
            dist += dx*dx;
        }
        dist = sqrt(dist);
        return dist;
	}
	double dist(int x,int y)
	{
        double dist = 0.0;
	    //cout<<dimension<<" "<<y.size()<<"\n";
	    #ifdef CALC_DIST_COUNT
	    dcmp++;
	    #endif
        for (int i = 0; i < dimension; i++) {
            double dx=objs[x][i] - objs[y][i];
            dist += dx*dx;
        }
        dist = sqrt(dist);
        return dist;
	}
};

class L1_db_t : public double_db_t
{
public:
	double dist(int x, int y) const;
};

class L2_db_t : public double_db_t
{
public:
	double dist(int x, int y) const;
};

class L5_db_t : public double_db_t
{
public:
	double dist(int x, int y) const;
};

class Linf_db_t : public double_db_t
{
public:
	double dist(int x, int y) const;
};

class str_db_t : public db_t
{
	vector<string> objs;
public:
	size_t size() const
	{
		return objs.size();
	}
	void read(string path);
	double dist(int x, int y) const;
};

void double_db_t::read(string path)
{
	ifstream in(path);
	size_t n;
	int func;
	dimension=192;
	n=53387;
//	in >> dimension >> n >> func;
	for (int i = 0; i < n; ++i) {
		objs.emplace_back();
		//copy_n(istream_iterator<double>(in), dimension, back_inserter(objs.back()));
	}
}

void double_db_t::read(vector<Node> &dataset)
{
    for(int i=0;i<dataset.size();i++)
    {
        vector<float> a;
        for(int j=0;j<dataset[i].dim;j++)
            a.push_back(dataset[i].data[j]);
        objs.push_back(a);
    }
    dimension=dataset[0].dim;
}


double L1_db_t::dist(int x, int y) const
{
    #ifdef CALC_DIST_COUNT
    dcmp++;
    #endif
	double dist = 0.0;
	for (int i = 0; i < dimension; i++) {
		dist += fabs(objs[x][i] - objs[y][i]);
	}

	return dist;
}

double L2_db_t::dist(int x, int y) const
{
    #ifdef CALC_DIST_COUNT
    dcmp++;
    #endif
	double dist = 0.0;
	double dx;
	for (int i = 0; i < dimension; i++) {
        dx=(objs[x][i] - objs[y][i]);
		dist+=dx*dx;
	}
	dist = sqrt(dist);
	return dist;
}

double L5_db_t::dist(int x, int y) const
{
    #ifdef CALC_DIST_COUNT
    dcmp++;
    #endif
	double dist = 0.0;
	for (int i = 0; i < dimension; i++) {
		dist += pow(fabs(objs[x][i] - objs[y][i]), 5);
	}
	dist = pow(dist, 0.2);
	return dist;
}

double Linf_db_t::dist(int x, int y) const
{
    #ifdef CALC_DIST_COUNT
    dcmp++;
    #endif
	double dist = 0.0;
	for (int i = 0; i < dimension; i++) {
		dist = max(dist,1.0 * fabs(objs[x][i] - objs[y][i]));
	}
	return dist;
}

void str_db_t::read(string path)
{
	ifstream in(path);
	size_t n;
	in >> n;
	for (int i = 0; i < n; ++i) {
		objs.emplace_back();
		in >> objs.back();
	}
}

double str_db_t::dist(int x, int y) const
{
    #ifdef CALC_DIST_COUNT
    dcmp++;
    #endif
	const string& s1 = objs[x];
	const string& s2 = objs[y];
	auto n1 = s1.size();
	auto n2 = s2.size();
	vector<vector<int>> dist(n1 + 1, vector<int>(n2 + 1));
	for (size_t i = 0; i <= n1; i++)
		dist[i][0] = (int)i;
	for (size_t j = 0; j <= n2; j++)
		dist[0][j] = (int)j;
	for (size_t i = 1; i <= n1; i++) {
		for (size_t j = 1; j <= n2; j++) {
			if (s1[i - 1] == s2[j - 1])
				dist[i][j] = dist[i - 1][j - 1];
			else
				dist[i][j] = min({ dist[i - 1][j], dist[i][j - 1], dist[i - 1][j - 1] }) + 1;
		}
	}
	return dist[n1][n2];
}
