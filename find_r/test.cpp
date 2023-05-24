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
string dataid="trevi";
string dir = "../data_set/"+dataid+"/";
string dataset_file_name = dataid+"_afterpca.csv";
string queryset_file_name = dataid+"_uniform1000_afterpca.csv";
vector<int> ks({10,50,100,500,1000,10000,30000});
vector<Node> dataset;
vector<Node> queryset;

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

int main()
{
    dataset=read_dataset(dir+dataset_file_name);
    cout<<"n:"<<dataset.size()<<"\t"<<"dim:"<<dataset[0].size()<<"\n";
    queryset=read_dataset(dir+queryset_file_name);
    float tp[dataset[0].dim];
    printf("read dataset queryset over\n");
    hnswlib::AlgorithmInterface<float>* hnsw=create_hnsw(dataset);
    printf("build over\n");

    double mx=0;
    for(int k:ks)
    {
        clock_t qtime=0;
        qtime-=clock();
        for(int i=0;i<queryset.size();i++)
        {
            for(int j=0;j<queryset[i].data.size();j++) tp[j]=queryset[i].data[j];
            auto res=hnsw->searchKnnCloserFirst(tp,100);
        }
        qtime+=clock();
        cout<<k<<" query time sum:"<<1.0*qtime/queryset.size()<<"ms\n";
    }
    system("pause");
    return 0;
}
