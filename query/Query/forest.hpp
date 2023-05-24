#include"../Node.hpp"
#include"./Query.hpp"
#include<vector>
#include<queue>
#include<thread>
#include<algorithm>
typedef pair<double,int> PDI;
using namespace std;
#ifdef CALC_DELTA
extern int deltamax;
#endif // CALC_DELTA
#ifdef DELTA_AVG
extern double deltaavg;
#endif // DELTA_AVG
#ifdef everyblocknumequal
extern int bknum;
#endif // everyblocknumequal
struct Forest
{
    int n,dim;
    vector<Query*> qs;//每块
    vector<int> lselect,rselect;
    vector<int> dimselect;
    Forest(vector<Node> dataset,int block_dim,double e,int,int delta);
    ~Forest();
    vector<int>* rangequery(float *q,int dim,double r,vector<double> &loss);
    vector<int>* thread_rangequery(float *q,int dim,double r,vector<double> &loss);
    priority_queue<PDI> rangequery_knn(float *q,int dim,int k,double rou);
    double memory_used();
    bool insert(Node p);
    bool del(int idx);
    void setdelta(int delta);
    void view();
private:
    int delta;
};

double Forest::memory_used()
{
    double mem=8+4*(lselect.size()+rselect.size()+dimselect.size());
    mem+=qs.size()+sizeof(void*);
    for(int i=0;i<qs.size();i++)
        mem+=qs[i]->memory_used();
    return mem/1024/1024;
}

Forest::Forest(vector<Node> dataset,int block_dim,double e,int blocknum=2000,int delta=-1)
{
    dim=dataset[0].data.size();
    dimselect.resize(dim+1);
    for(int i=0;i<dataset.size();i++) dataset[i].get_dim(e);
    #ifndef everyblocknumequal
    sort(dataset.begin(),dataset.end(),[](Node &a,Node &b){return a.dim<b.dim;});
    set<int> dimset;//set不会重复的
    vector<Node> tmp;//每一块
    int dimcnt=0;
    for(int i=0;i<dataset.size();)
    {
        //cout<<i<<"\n";
        int j=i;
        dimcnt++;
        while(j<dataset.size() and dataset[j].dim==dataset[i].dim)
        {
            tmp.push_back(dataset[j]);
            dimselect[dataset[j].dim]=qs.size();
            j++;
        }
        if((tmp.size()>=blocknum and dimcnt>=block_dim) or j==dataset.size())
        {
            dimcnt=0;
            if(dataset.size()-j<blocknum)
            {
                while(j<dataset.size())
                {
                    dimselect[dataset[j].dim]=qs.size();
                    tmp.push_back(dataset[j++]);
                    tmp.back().dim=tmp[0].dim;
                }
            }
            for(int i=0;i<tmp.size();i++) tmp[i].dim=tmp.back().dim;
            qs.push_back(new Query(tmp,e));
            tmp.resize(0);
        }
        i=j;
    }
    /*for(int i=0;i<dataset.size();i++)
    {
        dimset.insert(dataset[i].dim);
        if(dimset.size()>block_dim)//当前的块中点的维度的数量已经到达blockdim了
        {
            for(int i=0;i<tmp.size();i++) tmp[i].dim=tmp.back().dim;
            qs.push_back(new Query(tmp));
            tmp.resize(0);
            dimset.clear();
            dimset.insert(dataset[i].dim);
        }
        tmp.push_back(dataset[i]);
        dimselect[dataset[i].dim]=qs.size();
    }
    if(tmp.size()) qs.push_back(new Query(tmp));*/
    #else
    sort(dataset.begin(),dataset.end(),[](Node &a,Node &b){if(a.dim!=b.dim) return a.dim<b.dim; return a.loss[a.dim]<b.loss[b.dim]});
    int number=dataset.size()/bknum,yu=dataset.size()%bknum;
    now=0;
    for(int i=0;i<bknum;i++)
    {
        vector<Node> tmp;
        int t=number;
        if(!i) t+=yu;
        while(t--)
            tmp.push_back(dataset[now++]);
        for(int j=0;j<tmp.size();j++)
        {
            dimselect[tmp[j].dim]=qs.size();
            tmp[j].dim=tmp.back().dim;
        }
        qs.push_back(new Query(tmp));
    }
    #endif
    int zero=0;
    dimselect[dim]=qs.size()-1;
    while(zero<=dim and dimselect[zero]==0) zero++;
    for(int i=dim-1;i>zero;i--)
        if(dimselect[i]==0)
            dimselect[i]=dimselect[i+1];
    lselect.resize(dim+1);
    rselect.resize(dim+1);
    setdelta(delta);
}

Forest::~Forest()
{
    for(int i=0;i<qs.size();i++) delete qs[i];
}

void Forest::view()
{
    for(int i=0;i<qs.size();i++)
    {
        printf("num:%5d\t\tdim:%3d\t\t",qs[i]->get_num(),qs[i]->dim1);
        cout<<qs[i]->type<<endl;
    }

}

vector<int>* Forest::rangequery(float *q,int dim,double r,vector<double> &loss)
{
    int st=lselect[dim],ed=rselect[dim];
    vector<int>* res=new vector<int>();
    for(int i=st;i<=ed;i++)
    {
        vector<int>* tmp=qs[i]->rangequery(q,r,loss[qs[i]->dim1]);
        for(int id:(*tmp))
            res->push_back(id);
        delete tmp;
    }
    return res;
}

vector<int>* Forest::thread_rangequery(float *q,int dim,double r,vector<double> &loss)
{
    int st=lselect[dim],ed=rselect[dim];
    vector<int>* res=new vector<int>();
    vector<thread*> tds(ed-st+1,nullptr);
    vector<vector<int>*> vcs(ed-st+1,nullptr);
    for(int i=st;i<=ed;i++)
    {
        tds[i-st]=new thread(thread_rangequery_function,qs[i],q,r,loss[qs[i]->dim1],&vcs[i-st]);
    }
    for(int i=ed;i>=st;i--)
    {
        tds[i-st]->join();
        for(int id:(*vcs[i-st])) res->push_back(id);
        #ifdef CALC_DELTA
        if(vcs[i-st]->size())
        {
            deltamax=max({deltamax,dim-qs[i]->dim1,qs[i]->dim1-dim});
        }
        #endif // CALC_DELTA
        #ifdef DELTA_AVG
        if(vcs[i-st]->size())
        {
            deltaavg+=1.0*max({dim-qs[i]->dim1,qs[i]->dim1-dim})*vcs[i-st]->size();
        }
        #endif // DELTA_AVG
        delete vcs[i-st];
        delete tds[i-st];
    }
    return res;
}

priority_queue<PDI> Forest::rangequery_knn(float *q,int dim,int k,double rou=0)
{
    priority_queue<PDI> res;
    double radii=1e20;
    int delta=1e9,tmpdelta;
    int l=dimselect[dim],r=dimselect[dim]+1;
    while(l>=max(dim-delta,0) or r<min(dim+delta,(int)qs.size()))
    {
        if(l>=max(dim-delta,0))
        {
            qs[l]->rangequery_knn(q,k,res,radii);
            if(res.size()==k)
            {
                radii=sqrt(res.top().first);
                if(rou)
                {
                    tmpdelta=(int)(radii/rou);
                    if(tmpdelta>=0 and tmpdelta<delta) delta=tmpdelta;
                }
            }
            l--;
        }
        if(r<min(dim+delta+1,(int)qs.size()))
        {
            qs[r]->rangequery_knn(q,k,res,radii);
            if(res.size()==k)
            {
                radii=sqrt(res.top().first);
                if(rou)
                {
                    tmpdelta=(int)(radii/rou);
                    if(tmpdelta>=0 and tmpdelta<delta) delta=tmpdelta;
                }
            }
            r++;
        }
    }
    return res;
}

bool Forest::insert(Node p)
{
    return qs[dimselect[p.dim]]->insert(p);
}

bool Forest::del(int idx)
{
    bool suc=false;
    for(int i=0;i<qs.size();i++) suc|=qs[i]->del(idx);
    return suc;
}

void Forest::setdelta(int delta)
{
    this->delta=delta;
    for(int i=0;i<=dim;i++)
    {
        if(delta==-1)
        {
            lselect[i]=0,rselect[i]=qs.size()-1;
            continue;
        }
        int d=i+delta;
        int l=0,r=qs.size()-1;
        while(l<r)
        {
            int mid=l+r>>1;
            if(qs[mid]->dim1>=d) r=mid;
            else l=mid+1;
        }
        rselect[i]=r;
        d=i-delta;
        l=0,r=qs.size()-1;
        while(l<r)
        {
            int mid=l+r+1>>1;
            if(qs[mid]->dim1<=d) l=mid;
            else r=mid-1;
        }
        lselect[i]=l;
    }
}
