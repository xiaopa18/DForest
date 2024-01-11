#define CALC_DIST_COUNT
//#define USE_GNAT
//#define USE_SAT
//#define USE_ENUM
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
string dataset_file = "../../data_set/"+dataid+"/"+dataid+"_afterpca.csv";
string queryset_file = "../../data_set/"+dataid+"/"+dataid+"_"+queryid+"_afterpca.csv";
string es="42516.318408";
double e,buildtime,querytranstime;
vector<string> rs({"46798"});
vector<int> ks({10});
vector<double> rss({});
vector<Node> dataset;
vector<Node> queryset;
ofstream ouf;

void once_calc(double e,int argc,char **argv)
{
    ifstream inf(string("../")+"rangequery.csv",ios::in);
    if(!inf.is_open())
    {
        string tmp;
        int cnt=0;
        while(getline(inf,tmp))
        {
            cnt++;
        }
        if(!cnt) ouf<<"dataid,queryid,r,e,build memory,build time,avg find points,avg dcmp,query time(ms)"<<endl;
    }
    else ouf<<"dataid,queryid,r,e,build memory,build time,avg find points,avg dcmp,query time(ms)"<<endl;
    inf.close();
    ouf.open(string("../")+"rangequery.csv",ios::app);
    if(!ouf.is_open())
    {
        cout<<string("../")+"rangequery.csv"<<" open failed"<<endl;
        exit(-1);
    }
    ouf.setf(ios::fixed);
    auto tist=steady_clock::now(),tied=steady_clock::now();
    tist=steady_clock::now();
    get_dim(queryset,e);
    tied=steady_clock::now();
    double red_dim_time=1.0*duration_cast<microseconds>(tied - tist).count()/1000/queryset.size();
    double time=0,usemem,sum=0,radii=0;
    tist=steady_clock::now();
    Forest fs(dataset,e,10);
    tied=steady_clock::now();
    usemem=fs.memory_used();
    cout<<"build over"<<endl;
    if(argc!=1) buildtime=stod(argv[5]);
    else buildtime=0;
    buildtime+=duration_cast<microseconds>(tied - tist).count()/1000;
    for(int i=0;i<rss.size();i++)
    {
        double r=rss[i];
        int cnt=0;
        time=0;
        dcmp=0;
        sum=0;
        for(Node &q:queryset)
        {
            vector<int> *res;
            tist=steady_clock::now();
            res=fs.rangequery(q.data.data(),q.dim,r,q.loss,q.rou);
            tied=steady_clock::now();
            time+=duration_cast<microseconds>(tied - tist).count();
            sum+=res->size();
            delete res;
        }
        map<string,int> idxmp;
        for(Query *qs:fs.qs) idxmp[qs->type]++;
        ouf<<dataid<<","<<queryid<<","<<r<<","<<e<<",";
        for(auto &[u,v]:idxmp) ouf<<u<<"("<<v<<") ";
        ouf<<","<<usemem<<"MB,"<<buildtime/1000.0<<"s,"<<sum/queryset.size()<<",";
        ouf<<dcmp/queryset.size()<<","<<time/1000/queryset.size()+red_dim_time+querytranstime<<endl;
        cout<<dataid<<" "<<"rangequery "<<r<<" over"<<endl;
    }
    ouf.close();

    inf.open(string("../")+"knn.csv",ios::in);
    if(!inf.is_open())
    {
        string tmp;
        int cnt=0;
        while(getline(inf,tmp))
        {
            cnt++;
        }
        if(!cnt) ouf<<"dataid,queryid,r,e,build memory,build time,avg find points,avg dcmp,query time(ms)"<<endl;
    }
    else ouf<<"dataid,queryid,r,e,build memory,build time,avg find points,avg dcmp,query time(ms)"<<endl;
    inf.close();
    ouf.open("../knn.csv",ios::app);
    if(!ouf.is_open())
    {
        cout<<"open "<<"../knn.csv"<<" is failed"<<endl;
        exit(-1);
    }
    ouf.setf(ios::fixed);
    for(int k:ks)
    {
        double all=0,acc=0,ol=0;
        time=0;
        dcmp=0;
        radii=0;
        double real_dcmp=0;
        for(int i=0;i<queryset.size();i++)
        {
            Node q=queryset[i];
            priority_queue<PDI> res,real;
            real_dcmp-=dcmp;
            tist=steady_clock::now();
            res=fs.rangequery_knn(q.data.data(),q.dim,k,q.rou);
            tied=steady_clock::now();
            real_dcmp+=dcmp;
            time+=duration_cast<microseconds>(tied - tist).count();
            radii+=sqrt(res.top().fi);
            all+=res.size();
        }
        map<string,int> idxmp;
        for(Query *qs:fs.qs) idxmp[qs->type]++;
        ouf<<dataid<<","<<queryid<<","<<k<<","<<e<<",";
        for(auto &[u,v]:idxmp) ouf<<u<<"("<<v<<") ";
        ouf<<","<<usemem<<"MB,"<<buildtime/1000.0<<"s,"<<radii/queryset.size()<<",";
        ouf<<real_dcmp/queryset.size()<<","<<time/1000/queryset.size()+red_dim_time+querytranstime<<endl;
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
        es=argv[3];
        buildtime=stod(argv[4]),querytranstime=stod(argv[5]);
        int m=stoi(argv[6]);
        for(int i=0;i<m;i++)
        {
            rs.push_back(string(argv[7+i]));
        }
    }
    e=stod(es);
    dataset_file = "../../data_set/"+dataid+"/"+dataid+"_afterpca.csv";
    queryset_file = "../../data_set/"+dataid+"/"+dataid+"_"+queryid+"_afterpca.csv";
    dataset=read_dataset(dataset_file);
    queryset=read_dataset(queryset_file);
    for(string rads:rs)
        rss.push_back(stod(rads));
    if(rss.size())sort(rss.begin(),rss.end());
    once_calc(e,argc,argv);
    return 0;
}
