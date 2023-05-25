#define CALC_DIST_COUNT
#define CALC_DELTA
#define CALC_PAGE
#include<iostream>
#include<cstring>
#include<algorithm>
#include<fstream>
#include<string>
#include<time.h>
#include<thread>
#include<map>
#include"./Query/forest.hpp"
#include"./Query/forest_storage.hpp"
#include"./myfunctions.hpp"
#include<unordered_map>
#include<chrono>
#include<random>
#include<queue>
#include<set>
#define fi first
#define se second
using namespace std;
using namespace std::chrono;
const double eps=1e-6;
typedef pair<int,int> PII;
typedef pair<double,int> PDI;
#ifdef CALC_DIST_COUNT
double dcmp;
#endif
string dataid="audio";
string queryid="uniform1000";
string dataset_file = "../data_set/"+dataid+"/"+dataid+"_afterpca.csv";
string queryset_file = "../data_set/"+dataid+"/"+dataid+"_"+queryid+"_afterpca.csv";
string es="42516.318408",block_dims="1";
double e,buildtime,querytranstime;int blockdim;
vector<string> rs({"46798"});
//vector<string> rs;
vector<int> ks({10,20,30,40,50});
double rou=728.143704;
vector<double> rss;
vector<Node> dataset;
vector<Node> queryset;
int deltamax,page_size=32768;
ofstream ouf;
double IOtime,page;

void once_calc(double e,int blockdim)
{
    auto tist=steady_clock::now(),tied=steady_clock::now();
    tist=steady_clock::now();
    get_dim(queryset,e);
    tied=steady_clock::now();
    double red_dim_time=1.0*duration_cast<microseconds>(tied - tist).count()/1000/queryset.size();
    double time=0,usemem,sum=0;
    Forest fs(dataset,blockdim,e);
    ouf.open(string("../")+"rangequery_storage.csv",ios::app);
    if(!ouf.is_open())
    {
        cout<<string("../")+"rangequery_storage.csv"<<" open failed"<<endl;
        exit(-1);
    }
    ouf.setf(ios::fixed);
    tist=steady_clock::now();
    Forest_storage fss(dataset,blockdim,e,"./index/index",page_size);
    tied=steady_clock::now();
    cout<<"build over"<<endl;
    buildtime+=duration_cast<microseconds>(tied - tist).count()/1000;
    deltamax=-1;
    for(int i=0;i<rss.size();i++)
    {
        double r=rss[i];
        double all=0;
        fs.setdelta(-1);
        for(Node &q:queryset)
        {
            vector<int>* res=fs.rangequery(q.data.data(),q.dim,r,q.loss);
            all+=res->size();
            delete res;
        }
        //cout<<all<<"\n";
        IOtime=0;
        page=0;
        time=0;
        dcmp=0;
        sum=0;
        int dt=(int)(min(r/rou,1.0*fs.qs.back()->dim1));
        if(rou==-1) dt=-1;
        fss.setdelta(dt);
        for(Node &q:queryset)
        {
            vector<int> *res;
            tist=steady_clock::now();
            res=fss.rangequery(q.data.data(),q.dim,r,q.loss);
            tied=steady_clock::now();
            time+=duration_cast<microseconds>(tied - tist).count();
            sum+=res->size();
            delete res;
        }
        map<string,int> idxmp;
        for(Query *qs:fs.qs) idxmp[qs->type]++;
        //cout<<sum<<" "<<all<<endl;
        ouf<<dataid<<","<<queryid<<","<<r<<","<<e<<","<<rou<<","<<dt<<","<<blockdim<<","<<page_size<<",";
        for(auto &[u,v]:idxmp) ouf<<u<<"("<<v<<") ";
        ouf<<","<<"MB,"<<buildtime/1000.0<<"s,"<<sum/queryset.size()<<","<<100.0*sum/all<<"%,";
        ouf<<dcmp/queryset.size()<<","<<page/queryset.size()<<","<<time/1000/queryset.size()+querytranstime+red_dim_time<<","
        <<IOtime/1000/queryset.size()<<","<<time/1000/queryset.size()-IOtime/1000/queryset.size()+querytranstime+red_dim_time<<endl;
        cout<<dataid<<" "<<"rangequery "<<r<<" over"<<endl;
    }
    ouf.close();

    if(rou==-1) rou=0;
    ouf.open(string("../")+"knn_storage.csv",ios::app);
    if(!ouf.is_open())
    {
        cout<<string("../")+"knn_storage.csv"<<" open failed"<<endl;
        exit(-1);
    }
    ouf.setf(ios::fixed);
    for(int k:ks)
    {
        double all=0,acc=0,ol=0;
        IOtime=0;
        page=0;
        time=0;
        dcmp=0;
        sum=0;
        for(Node &q:queryset)
        {
            priority_queue<PDI> res,real;
            real=fs.rangequery_knn(q.data.data(),q.dim,k);
            tist=steady_clock::now();
            res=fss.rangequery_knn(q.data.data(),q.dim,k,rou);
            tied=steady_clock::now();
            time+=duration_cast<microseconds>(tied - tist).count();
            sum+=sqrt(res.top().fi);
            all+=res.size();
            acc+=accurate(res,real);
            ol+=overall(res,real,k);
        }
        map<string,int> idxmp;
        for(Query *qs:fs.qs) idxmp[qs->type]++;
        ouf<<dataid<<","<<queryid<<","<<k<<","<<e<<","<<blockdim<<","<<page_size<<",";
        for(auto &[u,v]:idxmp) ouf<<u<<"("<<v<<") ";
        ouf<<","<<"MB,"<<buildtime/1000.0<<"s,"<<sum/queryset.size()<<","<<acc/all*100<<"%,"<<ol/queryset.size()<<",";
        ouf<<dcmp/queryset.size()<<","<<page/queryset.size()<<","<<time/1000.0/queryset.size()+querytranstime+red_dim_time<<","
        <<IOtime/1000.0/queryset.size()<<","<<time/1000.0/queryset.size()-IOtime/1000.0/queryset.size()+querytranstime+red_dim_time<<endl;
        cout<<dataid<<" "<<"knn "<<k<<" over"<<endl;
    }
    ouf.close();
}

int main(int argc,char **argv)
{
    cout.setf(ios::fixed);
    if(argc!=1)
    {
        rs=vector<string>();
        dataid=argv[1],queryid=argv[2];
        es=argv[3],block_dims=argv[4],page_size=stoi(argv[5]);
        buildtime=stod(argv[6]),querytranstime=stod(argv[7]);
        int m=stoi(argv[8]);
        for(int i=0;i<m;i++)
        {
            rs.push_back(string(argv[9+i]));
        }
        rou=stod(argv[9+m]);
    }
    //rs=vector<string>();
    e=stod(es);blockdim=stoi(block_dims);
    dataset_file = "../data_set/"+dataid+"/"+dataid+"_afterpca.csv";
    queryset_file = "../data_set/"+dataid+"/"+dataid+"_"+queryid+"_afterpca.csv";
    dataset=read_dataset(dataset_file);
    queryset=read_dataset(queryset_file);
    get_dim(queryset,e);
    for(string rads:rs)
        rss.push_back(stod(rads));
    sort(rss.begin(),rss.end());
    once_calc(e,blockdim);
    ouf.close();
    return 0;
}
