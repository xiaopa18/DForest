#include<bits/stdc++.h>
#include"./query/Node.hpp"
#include"./query/myfunctions.hpp"
//#define origin
using namespace std;
mt19937 gen((unsigned int) time(nullptr));
string dataid="normal_n1000000_dim256",distibut="uniform";
int q_num=1000;
string filename="../data_set/"+dataid+"/"+dataid+".csv";
vector<vector<double>> dataset;
vector<double> mx,mn;
double pro=0.05;

void read()
{
    ifstream inf(filename,ios::in);
    string tmp;
    while(getline(inf,tmp))
    {
        vector<double> vc;
        string tp="";
        for(char ch:tmp)
        {
            if(ch==',')
            {
                if(tp!="") vc.push_back(stod(tp));
                tp="";
            }else tp.push_back(ch);
        }
        if(tp!="") vc.push_back(stod(tp));
        if(dataset.size() and vc.size()!=dataset.back().size()) exit(-2);
        dataset.push_back(vc);
    }
}

void create_uniform()
{
    uniform_int_distribution<int> intdis(0,dataset.size()-1);
    uniform_real_distribution<double> moddis(0,1.0);
    ofstream ouf("../data_set/"+dataid+"/"+dataid+"_"+distibut+to_string(q_num)+".csv",ios::out);
    #ifdef origin
        ofstream of("./data_set/"+dataid+"/"+dataid+"_"+distibut+to_string(q_num)+"_origin.csv",ios::out);
    #endif // origin
    while(q_num--)
    {
        int idx=intdis(gen);
        bool modify=moddis(gen)<pro;
        for(int i=0;i<dataset[idx].size();i++)
        {
            if(i)
            {
                ouf<<",";
                #ifdef origin
                of<<",";
                #endif // origin
            }
            if(moddis(gen)<pro)
                ouf<<normal_distribution<double>(dataset[idx][i],
                                                 min(mx[i]-dataset[idx][i],dataset[idx][i]-mn[i])/6)(gen);
            else ouf<<dataset[idx][i];
            #ifdef origin
            of<<dataset[idx][i];
            #endif // origin
        }
        ouf<<"\n";
        #ifdef origin
        of<<"\n";
        #endif // origin
    }
}

void create_zipf()
{
    auto zipf=[](int x,double C=1,double alp=0.5)->bool{return C/pow(x,alp);};
    vector<double> s(dataset.size(),0);
    for(int i=0;i<dataset.size();i++)
    {
        if(i) s[i]+=s[i-1];
        s[i]+=zipf(i+1);
    }
    uniform_real_distribution<double> idxdis(0,s.back());
    uniform_real_distribution<double> moddis(0,1.0);
    ofstream ouf("./data_set/"+dataid+"/"+dataid+"_"+distibut+to_string(q_num)+".csv",ios::out);
    #ifdef origin
        ofstream of("./data_set/"+dataid+"/"+dataid+"_"+distibut+to_string(q_num)+"_origin.csv",ios::out);
    #endif // origin
    while(q_num--)
    {
        double tmp=idxdis(gen);
        int l=0,r=dataset.size()-1;
        while(l<r)
        {
            int mid=l+r>>1;
            if(s[mid]>tmp) r=mid;
            else l=mid+1;
        }
        int idx=r;
        bool modify=moddis(gen)<pro;
        for(int i=0;i<dataset[idx].size();i++)
        {
            if(i)
            {
                ouf<<",";
                #ifdef origin
                of<<",";
                #endif // origin
            }
            if(moddis(gen)<pro)
                ouf<<normal_distribution<double>(dataset[idx][i],
                                                 min(mx[i]-dataset[idx][i],dataset[idx][i]-mn[i])/6)(gen);
            else ouf<<dataset[idx][i];
            #ifdef origin
            of<<dataset[idx][i];
            #endif // origin
        }
        ouf<<"\n";
        #ifdef origin
        of<<"\n";
        #endif // origin
    }
}

int main()
{
    read();
    mx=vector<double>(dataset[0].size(),-1e20),mn=vector<double>(dataset[0].size(),1e20);
    for(int i=0;i<dataset.size();i++)
        for(int j=0;j<dataset[i].size();j++)
            mx[j]=max(mx[j],dataset[i][j]),
            mn[j]=min(mn[j],dataset[i][j]);
    if(distibut=="uniform") create_uniform();
    else if(distibut=="zipf") create_zipf();
    return 0;
}
