#define CALC_DIST_COUNT
//#define USE_GNAT
//#define USE_SAT
#include<iostream>
#include<cstring>
#include<algorithm>
#include<fstream>
#include<string>
#include<time.h>
#include<thread>
#include<map>
#include"./Query/forest.hpp"
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
string es="42516.31841",block_dims="1";
double e,buildtime,querytranstime;int blockdim;
vector<string> rs({"46798"});
double rou=728.143704;
vector<int> ks({10,20,30,40,50});
vector<double> rss({});
vector<Node> dataset;
vector<Node> queryset;
int pid,deltamax;
ofstream ouf;

void once_calc(double e,int blockdim,int argc,char **argv)
{
    ouf.open(string("../")+"rangequery.csv",ios::app);
    if(!ouf.is_open())
    {
        cout<<string("../")+"rangequery.csv"<<" open failed"<<endl;

        exit(-1);
    }
    ouf.setf(ios::fixed);
    auto tist=steady_clock::now(),tied=steady_clock::now();
    //e=r/2;
    tist=steady_clock::now();
    get_dim(queryset,e);
    tied=steady_clock::now();
    double red_dim_time=1.0*duration_cast<microseconds>(tied - tist).count()/1000/queryset.size();
    double time=0,usemem,sum=0,radii=0;
    tist=steady_clock::now();
    Forest fs(dataset,blockdim,e);
    tied=steady_clock::now();
    usemem=fs.memory_used();
    cout<<"build over"<<endl;
    if(argc!=1) buildtime=stod(argv[5]);
    else buildtime=0;
    buildtime+=duration_cast<microseconds>(tied - tist).count()/1000;
    for(int i=0;i<rss.size();i++)
    {
        double r=rss[i];
        fs.setdelta(-1);
        double all=0;
        int cnt=0;
        for(Node &q:queryset)
        {
            vector<int>* res=fs.rangequery(q.data.data(),q.dim,r,q.loss);
            all+=res->size();
            delete res;
        }
        int dt=(int)(min(r/rou,1.0*fs.qs.back()->dim1));
        if(rou==-1) dt=-1;
        fs.setdelta(dt);
        time=0;
        dcmp=0;
        sum=0;
        for(Node &q:queryset)
        {
            vector<int> *res;
            tist=steady_clock::now();
            res=fs.rangequery(q.data.data(),q.dim,r,q.loss);
            tied=steady_clock::now();
            time+=duration_cast<microseconds>(tied - tist).count();
            sum+=res->size();
//            for(int t:(*res))
//                if(dist(q,dataset[t])>r)
//                    cout<<"NO"<<endl;
            delete res;
        }
        map<string,int> idxmp;
        for(Query *qs:fs.qs) idxmp[qs->type]++;
        ouf<<dataid<<","<<queryid<<","<<r<<","<<e<<","<<rou<<","<<dt<<","<<blockdim<<",";
        //ouf<<dataid<<","<<queryid<<","<<r<<","<<e<<","<<rou<<","<<-1<<","<<blockdim<<",";
        for(auto &[u,v]:idxmp) ouf<<u<<"("<<v<<") ";
        //cout<<sum<<" "<<all<<endl;
        ouf<<","<<usemem<<"MB,"<<buildtime/1000.0<<"s,"<<sum/queryset.size()<<","<<100.0*sum/all<<"%,";
        ouf<<dcmp/queryset.size()<<","<<time/1000/queryset.size()+red_dim_time+querytranstime<<endl;
        cout<<dataid<<" "<<"rangequery "<<r<<" over"<<endl;
    }
    ouf.close();

    ouf.open("../knn.csv",ios::app);
    if(!ouf.is_open())
    {
        cout<<"open "<<"../knn.csv"<<" is failed"<<endl;
        exit(-1);
    }
    ouf.setf(ios::fixed);
    if(rou==-1) rou=0;
    for(int k:ks)
    {
        double all=0,acc=0,ol=0;
        time=0;
        dcmp=0;
        radii=0;
        for(int i=0;i<queryset.size();i++)
        {
            Node q=queryset[i];
            priority_queue<PDI> res,real;
            real=fs.rangequery_knn(q.data.data(),q.dim,k);
            tist=steady_clock::now();
            res=fs.rangequery_knn(q.data.data(),q.dim,k,rou);
            tied=steady_clock::now();
            time+=duration_cast<microseconds>(tied - tist).count();
            radii+=sqrt(res.top().fi);
            all+=res.size();
            acc+=accurate(res,real);
            ol+=overall(res,real,k);
        }
        map<string,int> idxmp;
        for(Query *qs:fs.qs) idxmp[qs->type]++;
        ouf<<dataid<<","<<queryid<<","<<k<<","<<e<<","<<blockdim<<",";
        for(auto &[u,v]:idxmp) ouf<<u<<"("<<v<<") ";
        ouf<<","<<usemem<<"MB,"<<buildtime/1000.0<<"s,"<<radii/queryset.size()<<","<<acc/all*100<<"%,"<<ol/queryset.size()<<",";
        ouf<<dcmp/queryset.size()<<","<<time/1000/queryset.size()+red_dim_time+querytranstime<<endl;
        cout<<dataid<<" knn "<<k<<" over"<<endl;
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
        es=argv[3],block_dims=argv[4];
        buildtime=stod(argv[5]),querytranstime=stod(argv[6]);
        int m=stoi(argv[7]);
        for(int i=0;i<m;i++)
        {
            rs.push_back(string(argv[8+i]));
        }
        rou=stod(argv[8+m]);
    }
    //rs=vector<string>();
    pid=GetCurrentPid();
    e=stod(es);blockdim=stoi(block_dims);
    dataset_file = "../data_set/"+dataid+"/"+dataid+"_afterpca.csv";
    queryset_file = "../data_set/"+dataid+"/"+dataid+"_"+queryid+"_afterpca.csv";
    dataset=read_dataset(dataset_file);
    queryset=read_dataset(queryset_file);
    for(string rads:rs)
        rss.push_back(stod(rads));
    if(rss.size())sort(rss.begin(),rss.end());
    once_calc(e,blockdim,argc,argv);
    return 0;
}
