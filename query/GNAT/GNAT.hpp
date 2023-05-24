#pragma once
#include <unordered_map>
#include <queue>
#include "index.hpp"
#include <algorithm>
#include <fstream>
#include<float.h>
using namespace std;
#include <iostream>
#include <chrono>
#include <set>
#include <cassert>
#include<fstream>

#define fi first
#define se second
using namespace chrono;
typedef pair<double,int> PDI;
struct GNAT_node_t
{
	vector<int> pivot;
	vector<GNAT_node_t> children; //孩子节点
	vector<vector<double>> min_dist, max_dist;


	vector<int> bucket;
	int num;

	double memory_used()
	{
	    double res=0;
        queue<GNAT_node_t*> q;
        q.push(this);
        while(q.size())
        {
            GNAT_node_t *tmp=q.front();
            q.pop();
            res+=sizeof(int)*(1.0*tmp->pivot.size()+tmp->bucket.size()+1);
            res+=sizeof(double)*(1.0*tmp->min_dist.size()+tmp->max_dist.size());
            for(int i=0;i<tmp->children.size();i++)
                q.push(&tmp->children[i]);
        }
        return res;
	}
};

class GNAT_t : public index_t
{
public:
	GNAT_node_t root;
	int MaxHeight,dim1;
	size_t max_pivot_cnt, min_pivot_cnt, avg_pivot_cnt;
	void select(size_t& pivot_cnt, vector<int>& objects, GNAT_node_t* root);
	void _build(GNAT_node_t* root, vector<int> objects, size_t pivot_size,int h);
	void _rangeSearch(const GNAT_node_t * root,float* query, double range,vector<PDI>* ret);

//	void _rangeSearch(const GNAT_node_t* root, int query, double range, int& res_size);
public:
    vector<PDI>* rangequery(float* q,double r);

    double memory_used()
    {
        return root.memory_used()+
                8+db->memory_used();
    }

	GNAT_t(vector<Node> &dataset, size_t avg_pivot_cnt,int Maxheight);
	void build();
	~GNAT_t(){
        if(db!=nullptr) delete db;
	}

	double dist(int x,int y)
	{
	    return db->dist(x,y);
	}
    double dist(int id,float *q)
    {
        return db->dist(id,q);
    }
};

GNAT_t::GNAT_t(vector<Node> &dataset,size_t avg_pivot_cnt,int Maxheight=100000000) :
	max_pivot_cnt(min(4 * avg_pivot_cnt, (size_t)256)),
	min_pivot_cnt(2),
	avg_pivot_cnt(avg_pivot_cnt),
	MaxHeight(Maxheight)
{
    dim1=dataset[0].dim;
    L2_db_t *db=new L2_db_t();
    db->read(dataset);
    this->db=db;
    build();
}

void GNAT_t::build() //建立树形索引
{
	vector<int> objects;
	for (int i = 0; i < (int)db->size(); ++i) {
		objects.push_back(i);
	}
	random_shuffle(objects.begin(), objects.end());
	//cout << "database size: " << objects.size() << endl;
	_build(&root, objects, avg_pivot_cnt,1); //从根开始: 根 --- 平均核心数 第一层
}

void GNAT_t::select(size_t& pivot_cnt, vector<int>& objects, GNAT_node_t* root)
{
	//sampling 抽样
	size_t sample_cnt = min(pivot_cnt * 3, objects.size());
	vector<int> sample(sample_cnt);
	copy(objects.end() - sample_cnt, objects.end(), sample.begin()); //所有数据点中后sample_cnt个复制到sample[]中
	objects.resize(objects.size() - sample_cnt);

	//calc dist between samples
	vector<vector<double>> d(sample_cnt, vector<double>(sample_cnt));
	for (size_t i = 0; i < sample_cnt; ++i) {
		for (size_t j = i + 1; j < sample_cnt; ++j) {
			d[i][j] = d[j][i] = dist(sample[i], sample[j]);
		}
	}

	vector<size_t> pivot_pos(pivot_cnt); //核心的坐标
	vector<bool> is_pivot(sample_cnt, false); //是否为核心
	//select first pivot
	for (size_t i = 0; i < sample_cnt; ++i) { //点到自身距离为无穷
		d[i][i] = DBL_MAX;
	}
	vector<double> dist_other(sample_cnt);
	for (size_t i = 0; i < sample_cnt; ++i) { //与点i距离最近的点
		dist_other[i] = *min_element(d[i].begin(), d[i].end());
	}
	auto p = max_element(dist_other.begin(), dist_other.end()) - dist_other.begin(); //与个点距离最近的点中，距离最远的点
	pivot_pos[0] = p;
	is_pivot[p] = true;

	//select pivots
	vector<double> dist_pivot(sample_cnt, DBL_MAX);
	for (size_t i = 0; i < sample_cnt; ++i) {
		d[i][i] = 0.0;
	}
	for (size_t i = 1; i < pivot_cnt; ++i) {
		for (size_t j = 0; j < sample_cnt; ++j) {
			dist_pivot[j] = min(dist_pivot[j], d[j][pivot_pos[i-1]]);
		}
		for (p = 0; is_pivot[p]; ++p)
			continue;
		for (auto j = p + 1; j < (int)sample_cnt; ++j) {
			if (dist_pivot[j] > dist_pivot[p] && !is_pivot[j]) {
				p = j;
			}
		}
		pivot_pos[i] = p;
		is_pivot[p] = true;
	}

	//save pivots
	for (auto i : pivot_pos) {
		root->pivot.push_back(sample[i]);
	}
	//put other samples back
	for (size_t i = 0; i < sample_cnt; ++i) {
		if (!is_pivot[i]) {
			objects.push_back(sample[i]);
		}
	}
}

void GNAT_t::_build(GNAT_node_t * root, vector<int> objects, size_t pivot_cnt,int h)
{
	if (objects.empty()) {
		return;
	}

	if (h < MaxHeight) {
		root->num = -1;
		auto& pivot = root->pivot;
		auto& children = root->children;
		auto& max_dist = root->max_dist;
		auto& min_dist = root->min_dist;
		min_dist.resize(pivot_cnt, vector<double>(pivot_cnt, DBL_MAX));
		max_dist.resize(pivot_cnt, vector<double>(pivot_cnt, 0.0));
		select(pivot_cnt, objects, root);

		static size_t completed = 0;
		completed += pivot_cnt;

		if (objects.empty()) {
			return;
		}

		vector<vector<int>> objs_children(pivot_cnt);
		for (int obj : objects) {
			vector<double> dist_pivot(pivot_cnt);
			for (size_t i = 0; i < pivot_cnt; ++i) {
				dist_pivot[i] = dist(obj, pivot[i]);
			}
			size_t closest_pivot = min_element(dist_pivot.begin(), dist_pivot.end()) - dist_pivot.begin();
			objs_children[closest_pivot].push_back(obj);
			for (size_t i = 0; i < pivot_cnt; ++i) {
				max_dist[i][closest_pivot] = max(max_dist[i][closest_pivot], dist_pivot[i]);
				min_dist[i][closest_pivot] = min(min_dist[i][closest_pivot], dist_pivot[i]);
			}
		}
		children.resize(pivot_cnt);
		for (size_t i = 0; i < pivot_cnt; ++i) {
			size_t next_pivot_cnt = objs_children[i].size() * avg_pivot_cnt * pivot_cnt / objects.size();
			next_pivot_cnt = max(min_pivot_cnt, next_pivot_cnt);
			next_pivot_cnt = min(max_pivot_cnt, next_pivot_cnt);
			_build(&children[i], objs_children[i], min(next_pivot_cnt, objs_children[i].size()), h + 1);
		}
	}
	else {
		root->num = objects.size();
		//cout<<"asd:"<<root->num<<"\n";
		for (int i = 0; i < root->num; i++) root->bucket.push_back(objects[i]);
	}
}

void GNAT_t::_rangeSearch(const GNAT_node_t * root,float* query, double range,vector<PDI>* res)
{
	if (root->num < 0) {
		auto& pivot = root->pivot;
		auto& children = root->children;
		auto& max_dist = root->max_dist;
		auto& min_dist = root->min_dist;
		size_t n = pivot.size();
		vector<double> d(n);
		for (size_t i = 0; i < n; ++i) {
			d[i] = dist(pivot[i], query);
			if (d[i] <= range) {
                res->push_back({d[i]*d[i],pivot[i]});
			}
		}
		for (size_t i = 0; i < n; ++i) {
			bool ok = true;
			for (int j = 0; ok && j < n; ++j) {
				ok &= (max_dist[j][i] >= d[j] - range);
				ok &= (min_dist[j][i] <= d[j] + range);
			}
			if (ok) {
				_rangeSearch(&children[i], query, range, res);
			}
		}
	}
	else {
		double d;
		for (int i = 0; i < root->num; i++) {
			d = dist(root->bucket[i], query);
			//cout<<d<<"\n";
			if (d <= range)
            {
                res->push_back({d*d,root->bucket[i]});
            }
		}
	}
}

vector<PDI>* GNAT_t::rangequery(float* q,double r)
{
    vector<PDI> *res=new vector<PDI>();
    _rangeSearch(&root,q, r,res);
    return res;
}

static void addResult(int k, double d, int idx, priority_queue<PDI>& result) {
	if (result.size() < k || d < result.top().fi) {
		result.push({d,idx});
	}
	if (result.size() > k) {
		result.pop();
	}
}

