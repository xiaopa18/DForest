#include<iostream>
#include<cstring>
#include<algorithm>
#include<cmath>
#include<vector>
#include"../query/Node.hpp"
#include"../query/myfunctions.hpp"
#include"../query/Query/forest.hpp"
#define fi first
#define se second
using namespace std;
typedef long long LL;
int index_compdists;
string dataid="deep1M";
string dir = "../data_set/"+dataid+"/";
string dataset_file_name = dataid+"_afterpca.csv";
string queryset_file_name = dataid+"_uniform1000_afterpca.csv";
string outfile=splitby(dataset_file_name,'.')[0]+"_r.txt";
vector<Node> dataset;
vector<Node> queryset;
vector<double> pro({0.1,0.5,1,2,4});
vector<vector<double>> dists;
double e=0.4;
int blockdim=4;

int main(int argc,char **argv)
{
    if(argc==4)
    {
        dataid=argv[1];e=stod(argv[2]);blockdim=stoi(argv[3]);
    }
    dir = "../data_set/"+dataid+"/";
    dataset_file_name = dataid+"_afterpca.csv";
    queryset_file_name = dataid+"_uniform1000_afterpca.csv";
    outfile=splitby(dataset_file_name,'.')[0]+"_r.txt";
    ofstream ouf(outfile,ios::out);
    if(!ouf.is_open())
    {
        cout<<"open "<<outfile<<" failed";
        exit(-1);
    }
    ouf.setf(ios::fixed);
    dataset=read_dataset(dir+dataset_file_name);
    queryset=read_dataset(dir+queryset_file_name);
    get_dim(queryset,e);
    Forest fs(dataset,blockdim,e);
    double rm=0;
    int k=(int)(dataset.size()*pro.back()/100);
    for(Node &q:queryset)
    {
        priority_queue<PDI> res=fs.knn(q.data.data(),q.dim,k);
        rm=max(rm,sqrt(res.top().fi));
    }
    for(double p:pro)
    {
        double l=0,r=rm;
        while(r-l>1e-5)
        {
            double mid=(r+l)/2;
            double sum=0;
            for(Node &q:queryset)
            {
                vector<int> *res=fs.rangequery(q.data.data(),q.dim,mid,q.loss);
                sum+=res->size();
                delete res;
            }
            if(sum/dataset.size()/queryset.size()>p/100) r=mid;
            else l=mid;
        }
        ouf<<p<<"%\t"<<r<<endl;
        cout<<p<<"%\t"<<r<<endl;
    }
    //system("pause");
    return 0;
}
