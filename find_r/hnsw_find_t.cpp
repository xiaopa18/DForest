#include<iostream>
#include<cstring>
#include<algorithm>
#include<cmath>
#include<vector>
#include"../query/Node.hpp"
#include"../query/myfunctions.hpp"
#include"./hnswlib/hnswlib.h"
#include"./hnswlib/hnswalg.h"
#define fi first
#define se second
using namespace std;
typedef long long LL;
int index_compdists;
string dir = "..\\data_set\\notre\\";
string dataset_file_name = "notre.csv";
string queryset_file_name = "notre_uniform10000_origin.csv";
string outfile=splitby(dataset_file_name,'.')[0]+"_r.txt";
vector<Node> dataset;
vector<Node> queryset;
vector<double> pro({0.01,0.03,0.05,0.07,0.1,2,4,8,16,32});
vector<int> knn({1});
vector<vector<double>> dists;

hnswlib::AlgorithmInterface<float>* create_hnsw(vector<Node> &dataset)
{
    hnswlib::L2Space * tmp = new hnswlib::L2Space(dataset[0].dim);
    hnswlib::AlgorithmInterface<float>* alg_hnsw = new hnswlib::HierarchicalNSW<float>(tmp,2*dataset.size());
    int base=dataset.size()/10,i=0;
    float tp[dataset[0].dim];
    for(Node &tmp:dataset)
    {
        i++;
        if(i%base==0) cout<<"build: "<<100.00*i/dataset.size()<<"%\n";
        for(int i=0;i<tmp.dim;i++) tp[i]=tmp.data[i];
        alg_hnsw->addPoint(tp,tmp.idx);
    }
    return alg_hnsw;
}

bool check(double t,double p)
{
    LL sz=0;
    for(int i=0;i<dists.size();i++)
    {
        int l=0,r=dists[i].size()-1;
        while(l<r)
        {
            int mid=l+r+1>>1;
            if(dists[i][mid]<=t) l=mid;
            else r=mid-1;
        }
        sz+=l+1;
    }
    return p*dataset.size()*queryset.size()<=100*sz;
}

int main()
{
    ofstream ouf(outfile,ios::out);
    if(!ouf.is_open())
    {
        cout<<"open "<<outfile<<" failed";
        exit(-1);
    }
    dataset=read_dataset(dir+dataset_file_name);
    cout<<"n:"<<dataset.size()<<"\t"<<"dim:"<<dataset[0].size()<<"\n";
    queryset=read_dataset(dir+queryset_file_name);
    float tp[dataset[0].dim];
    printf("read dataset queryset over\n");
    hnswlib::AlgorithmInterface<float>* hnsw=create_hnsw(dataset);
    printf("build over\n");
    clock_t qtime=0;
    double mx=0;
    qtime-=clock();
    for(int i=0;i<queryset.size();i++)
    {
        for(int j=0;j<queryset[i].data.size();j++) tp[j]=queryset[i].data[j];
        vector<double> now;
        auto res=hnsw->searchKnnCloserFirst(tp,max((int)((dataset.size()*pro.back()+99)/100),knn.back()));
        for(auto tp:res) now.push_back(tp.fi);
        sort(now.begin(),now.end());
        mx=max(mx,now.back());
        dists.push_back(now);
    }
    qtime+=clock();
    cout<<"query time sum:"<<qtime<<"ms\n";
    for(double p:pro)
    {
        double l=0,r=mx;
        while(r-l>1e-5)
        {
            double mid=(l+r)/2;
            if(check(mid,p)) r=mid;
            else l=mid;
        }
        ouf<<p<<"%:\t"<<r<<"\n";
    }
    for(int k:knn)
    {
        double r=0;
        for(int i=0;i<dists.size();i++)
            r=max(r,dists[i][k-1]);
        ouf<<k<<"nn:\t"<<r<<"\n";
    }
    system("pause");
    return 0;
}
