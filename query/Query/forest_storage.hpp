#include"../Node.hpp"
#include"./Query_storage.hpp"
#include<vector>
#include<queue>
#include<thread>
#include<algorithm>
typedef pair<double,int> PDI;
using namespace std;
struct Forest_storage
{
    int n,dim;
    vector<Query_storage*> qs;
    vector<int> lselect,rselect;
    vector<int> dimselect;
    Forest_storage(vector<Node> dataset,double e,string index,int page_size,int blocknum,int delta);
    vector<int>* rangequery(float *q,int dim,double r,vector<double> &qloss,double);
    vector<int>* thread_rangequery(float *q,int dim,double r,vector<double> &qloss);
    priority_queue<PDI> rangequery_knn(float *q,int dim,int k,double);
    bool insert(Node p);
    bool del(int idx);
    void setdelta(int delta);
private:
    int delta;
};

Forest_storage::Forest_storage(vector<Node> dataset,double e,string index,int page_size=4096,int blocknum=2000,int delta=-1)
{
    dim=dataset[0].data.size();
    for(int i=0;i<dataset.size();i++) dataset[i].get_dim(e);
    sort(dataset.begin(),dataset.end(),[](Node &a,Node &b){return a.dim<b.dim;});
    set<int> dimset;
    vector<Node> tmp;
    dimselect.resize(dim+1);
    int dimcnt=0;
    for(int i=0;i<dataset.size();)
    {
        int j=i;
        dimcnt++;
        while(j<dataset.size() and dataset[j].dim==dataset[i].dim)
        {
            tmp.push_back(dataset[j]);
            dimselect[dataset[j].dim]=qs.size();
            j++;
        }
        if((tmp.size()>=blocknum ) or j==dataset.size())
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
            qs.push_back(new Query_storage(tmp, e,index+"_"+to_string(qs.size()),page_size));
            tmp.resize(0);
        }
        i=j;
    }
   /* for(int i=0;i<dataset.size();i++)
    {
        dimset.insert(dataset[i].dim);
        if(dimset.size()>block_dim)
        {
            for(int i=0;i<tmp.size();i++) tmp[i].dim=tmp.back().dim;
            qs.push_back(new Query_storage(tmp,index+"_"+to_string(qs.size()),page_size));
            tmp.resize(0);
            dimset.clear();
            dimset.insert(dataset[i].dim);
        }
        dimselect[dataset[i].dim]=qs.size();
        tmp.push_back(dataset[i]);
    }
    if(tmp.size()) qs.push_back(new Query_storage(tmp,index+"_"+to_string(qs.size())));*/
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

vector<int>* Forest_storage::rangequery(float *q,int dim,double r,vector<double> &qloss,double rou=0)
{
    int st=0,ed=qs.size()-1;
    if(rou>1e-5)
    {
        int dt=min((int)r/rou,1e5);
        int d=dim+dt;
        int l=st,r=ed;
        while(l<r)
        {
            int mid=l+r>>1;
            if(qs[mid]->dim1>=d) r=mid;
            else l=mid+1;
        }
        ed=r;
        l=st;
        d=dim-dt;
        while(l<r)
        {
            int mid=l+r+1>>1;
            if(qs[mid]->dim1<=d) l=mid;
            else r=mid-1;
        }
        st=l;
    }
    vector<int>* res=new vector<int>();
    for(int i=st;i<=ed;i++)
    {
        vector<int>* tmp=qs[i]->rangequery(q,r,qloss[qs[i]->dim1]);
        for(int id:(*tmp))
            res->push_back(id);
        delete tmp;
    }
    return res;
}

vector<int>* Forest_storage::thread_rangequery(float *q,int dim,double r,vector<double> &qloss)
{
    int st=0,ed=qs.size()-1;
    vector<int>* res=new vector<int>();
    vector<thread*> tds(ed-st+1,nullptr);
    vector<vector<int>*> vcs(ed-st+1,nullptr);
    for(int i=st;i<=ed;i++)
        tds[i-st]=new thread(thread_storage_query_function,qs[i],q,r,qloss[qs[i]->dim1],&vcs[i-st]);
    for(int i=ed;i>=st;i--)
    {
        tds[i-st]->join();
        for(int id:(*vcs[i-st])) res->push_back(id);
        delete vcs[i-st];
        delete tds[i-st];
    }
    return res;
}

priority_queue<PDI> Forest_storage::rangequery_knn(float *q,int dim,int k,double rou=0)
{
    priority_queue<PDI> res;
    double radii=1e20;
    int delta=1e9,tmpdelta;
    int l=dimselect[dim],r=dimselect[dim]+1;
    vector<PDI> *cand;
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
            delete cand;
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
            delete cand;
            r++;
        }
    }
    return res;
}

bool Forest_storage::insert(Node p)
{
    return qs[dimselect[p.dim]]->insert(p);
}

bool Forest_storage::del(int idx)
{
    bool suc=false;
    for(int i=0;i<qs.size();i++) suc|=qs[i]->del(idx);
    return suc;
}

void Forest_storage::setdelta(int delta)
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
