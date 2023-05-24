#ifndef MYFUNCTIONS_H_
#define MYFUNCTIONS_H_

#include"Node.hpp"
#include<vector>
#include<fstream>
#include<cmath>
#include<algorithm>
#include<fstream>
#include<queue>
#include<set>
#define fi first
#define se second
using namespace std;
typedef pair<double,int> PDI;

int get_delta(vector<Node> &queryset,vector<vector<int>> result,double recall)
{
    int dim=queryset[0].data.size();
    double sum=0;
    for(int i=0;i<queryset.size();i++)
    {
        for(int j=1;j<result[i].size();j++)
            result[i][j]+=result[i][j-1];
        sum+=result[i].back();
    }
    int delta=-1;
    double rel=0;
    while(rel+1e-9<recall)
    {
        delta++;
        double fd=0;
        for(int i=0;i<queryset.size();i++)
            fd+=result[i][min(dim,queryset[i].dim+delta)]-result[i][max(0,queryset[i].dim-delta-1)];
        rel=fd/sum;
    }
    return delta;
}

bool file_is_exist(string file_name)
{
    ifstream inf(file_name,ios::in);
    if(inf.is_open())
    {
        inf.close();
        return true;
    }
    return false;
}

vector<vector<string>> read_csv(string file_name)
{
    vector<vector<string>> res;
    ifstream inf(file_name,ios::in);
    if(!inf.is_open())
    {
        cout<<file_name<<"文件不存在\n";
        exit(-1);
    }
    string line;
    while(getline(inf,line))
    {
       string tmp="";
       vector<string> now;
       for(char ch:line)
       {
           if(ch==',')
           {
               now.push_back(tmp);
               tmp="";
           }else tmp.push_back(ch);
       }
       if(tmp!="") now.push_back(tmp);
       res.push_back(now);
    }
    return res;
}

void read_csv(string file_name,vector<vector<string>> &res)
{
    ifstream inf(file_name,ios::in);
    if(!inf.is_open())
    {
        cout<<file_name<<"文件不存在\n";
        exit(-1);
    }
    string line;
    while(getline(inf,line))
    {
       string tmp="";
       res.push_back(vector<string>());
       for(char ch:line)
       {
           if(ch==',')
           {
               res.back().push_back(tmp);
               tmp="";
           }else tmp.push_back(ch);
       }
       if(tmp!="") res.back().push_back(tmp);
    }
}

vector<Node> read_dataset(string file_name)
{
    ifstream inf(file_name, ios::in);
    if(!inf.is_open())
    {
        cout<<file_name<<"文件不存在\n";
        exit(-1);
    }
    string tmp;
    vector<Node> res;
    int line = 0;
    while (getline(inf, tmp))
    {
        line++;
        //cout<<line<<endl;
        vector<float> data;
        string tp = "";
        for (size_t i = 0; i < tmp.size(); i++)
        {
            if (tmp[i] == ',')
            {
                if (tp != "") data.push_back(stod(tp));
                tp = "";
            }
            else tp += tmp[i];
        }
        if (tp != "") data.push_back(stod(tp));
        if (res.size() && data.size() != res.back().size())
        {
            cout << file_name << "文件第" << line << "行与上一行列数不一样" << "\n";
            exit(-1);
        }
        res.push_back(Node(data,line-1));
    }
    inf.close();
    return res;
}

vector<vector<int>> read_tag(string file_name)
{
    ifstream inf(file_name, ios::in);
    if(!inf.is_open())
    {
        cout<<file_name<<"文件不存在\n";
        exit(-1);
    }
    vector<vector<int>> res;
    string line;
    while(getline(inf,line))
    {
        int T=0,i=0;
        while(i<line.size() and line[i]!=',') T=T*10+line[i++]-'0';
        vector<int> now;
        while(T--)
        {
            i++;
            int tmp=0;
            while(i<line.size() and line[i]!=',') tmp=tmp*10+line[i++]-'0';
            now.push_back(tmp);
        }
        res.push_back(now);
    }
    inf.close();
    return res;
}

double dist(Node a,Node b)
{
    double res=0;
    for(auto it1=a.begin(),it2=b.begin();it1!=a.end() and it2!=b.end();it1++,it2++)
    {
        res += (*it1-*it2)*(*it1-*it2);
    }
    return sqrt(res);
}

vector<vector<Node>> split_by_dim(vector<Node> dataset)
{
    sort(dataset.begin(),dataset.end(),[&](Node &a,Node &b)->bool{return a.dim<b.dim;});
    vector<vector<Node>> res;
    vector<Node> tmp;
    for(size_t i=0;i<dataset.size();i++)
    {
        if(tmp.size()==0) tmp.push_back(dataset[i]);
        else if(tmp.back().dim==dataset[i].dim) tmp.push_back(dataset[i]);
        else
        {
            res.push_back(tmp);
            tmp=vector<Node>();
            tmp.push_back(dataset[i]);
        }
    }
    if(tmp.size()) res.push_back(tmp);
    return res;
}

vector<vector<Node>> split_by_scale(vector<Node> dataset,int scale)
{
    vector<vector<Node>> res;
    return res;
}

vector<vector<Node>> split_by_delta(vector<vector<Node>> dataset,int delta)
{
    vector<vector<Node>> res;
    int n=dataset.size();
    for(int i=0;i<(n+delta-1)/delta;i++)
    {
        vector<Node> tmp;
        for(int j=i*delta;j<min(n,(i+1)*delta);j++)
            for(Node &tp:dataset[j])
                tmp.push_back(tp);
        for(Node &tp:tmp)
            tp.dim=tmp.back().dim;
        res.push_back(tmp);
    }
    return res;
}

vector<string> splitby(string s,char ch)
{
    string tmp="";
    vector<string> res;
    for(char &c:s)
    {
        if(c==ch)
        {
            if(tmp!="") res.push_back(tmp);
            tmp="";
        }else tmp.push_back(c);
    }
    if(tmp!="") res.push_back(tmp);
    return res;
}

int select_query_r(int dim,vector<vector<Node>> &dataset)
{
    int l=0,r=dataset.size()-1;
    while(l<r)
    {
        int mid=l+r>>1;
        if(dataset[mid][0].dim>=dim) r=mid;
        else l=mid+1;
    }
    return r;
}

int select_query_l(int dim,vector<vector<Node>> &dataset)
{
    int l=0,r=dataset.size()-1;
    while(l<r)
    {
        int mid=l+r+1>>1;
        if(dataset[mid][0].dim<=dim) l=mid;
        else r=mid-1;
    }
    return l;
}

pair<vector<int>,vector<int>> select_arr(int delta,vector<vector<Node>> &dataset)
{
    vector<int> l(dataset[0][0].data.size()+1,0),r(dataset[0][0].data.size()+1,0);
    for(int i=0;i<=dataset[0][0].data.size();i++)
    {
        l[i]=select_query_l(i-delta,dataset);
        r[i]=select_query_r(i+delta,dataset);
    }
    return {l,r};
}

int accurate(priority_queue<PDI> a,priority_queue<PDI> b)
{
    set<int> s;
    while(a.size())
    {
        s.insert(a.top().se);
        a.pop();
    }
    int res=0;
    while(b.size())
    {
        if(s.count(b.top().se)) res++;
        b.pop();
    }
    return res;
}

double recall(priority_queue<PDI> real,priority_queue<PDI> fd)
{
    set<int> rmp;
    while(real.size())
    {
        rmp.insert(real.top().se);
        real.pop();
    }
    int fz=0,fm=fd.size();
    while(fd.size())
    {
        if(rmp.count(fd.top().se)) fz++;
        fd.pop();
    }
    return 100.0*fz/fm;
}

double overall(priority_queue<PDI> fd,priority_queue<PDI> real,int k)
{
    //cout<<real.size()<<" "<<fd.size()<<"\n";
    double res=0;
    while(real.size())
    {
        //cout<<sqrt(fd.top().fi)<<" "<<sqrt(real.top().fi)<<"\n";
        if(fabs(real.top().fi)>1e-5 and real.top().fi>1e-5 and fd.top().fi>1e-5) res+=sqrt(fd.top().fi)/sqrt(real.top().fi);
        else res+=1;
        fd.pop();
        real.pop();
    }
    return res/k;
}

#endif
