#include<iostream>
#include<algorithm>
#include<cstring>
#include"./myfunctions.hpp"

using namespace std;
string dataid="deep1M";
string queryid="uniform1000";
string filename="../data_set/"+dataid+"/"+dataid+"_"+queryid+"_afterpca.csv";
string es="0.294801";
double e;

int main(int argc,char **argv)
{
    if(argc!=1)
    {
        dataid=argv[1],queryid=argv[2];
        filename="../data_set/"+dataid+"/"+dataid+"_"+queryid+"_afterpca.csv";
        es=argv[3];
    }
    vector<Node> queryset=read_dataset(filename);
    e=stod(es);
    get_dim(queryset,e);
    double averou=0,avecnt=0;
    for(Node &q:queryset)
    {
        vector<double> s(q.data.size()+1,0);
        for(int i=q.data.size()-1;~i;i--) s[i]=sqrt(q.data[i]*q.data[i]+s[i+1]*s[i+1]);
        double aver=0;
        for(int i=0;i<=q.dim-1;i++)
            aver+=(s[i]-s[q.dim-1])/(q.dim-1);
        aver/=q.dim;
        if(aver>0)
        {
            averou+=aver;
            avecnt++;
        }
    }
    cout<<averou/avecnt<<endl;
    return 0;
}
