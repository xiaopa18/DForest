#include<bits/stdc++.h>
#include"./query/Node.hpp"
#include"./query/myfunctions.hpp"
//#define origin
using namespace std;
mt19937 gen((unsigned int) time(nullptr));
string dataid="enron";
int q_num=40000;
string filename="./data_set/"+dataid+"/"+dataid+".csv";
vector<vector<double>> dataset;
vector<double> mx,mn;
double pro=0.5;
//vector<string> dataids({"audio","enron","deep1M","sift","sun","notre","nuswide","trevi"});
//vector<string> dataids({"enron","trevi"});
vector<string> dataids({"deep1M"});

void read()
{
    dataset.clear();
    ifstream inf(filename,ios::in);
    if(!inf.is_open())
    {
        cout<<"open" << " " << filename << " is failed\n";
        exit(-1);
    }
    //cout<<"asdasd"<<endl;
    string tmp;
    while(getline(inf,tmp))
    {
        vector<double> vc;
        string tp="";
        for(char ch:tmp)
        {
            if(ch==',')
            {
                while(tp.size() and (!(tp.back()>='0' and tp.back()<='9')) and tp.back()!='.') tp.pop_back();
                if(tp.size())
                {
                    vc.push_back(stod(tp));
                }
                tp="";
            }else tp.push_back(ch);
        }
        while(tp.size() and (!(tp.back()>='0' and tp.back()<='9')) and tp.back()!='.') tp.pop_back();
        if(tp.size())
        {
           // cout<<"asd:"<<tp<<" "<<tp.size()<<endl;
            vc.push_back(stod(tp));
        }
        if(dataset.size() and vc.size()!=dataset.back().size()) exit(-2);
        dataset.push_back(vc);
    }
}

int main()
{
    for(string dataid:dataids)
    {
        filename="./data_set/"+dataid+"/"+dataid+".csv";
        //cout<<"asd"<<endl;
        read();
        cout<<dataid<<" "<<dataset.size()<<" "<<dataset[0].size()<<endl;
        //cout<<dataid<<endl;
        mx=vector<double>(dataset[0].size(),-1e20),mn=vector<double>(dataset[0].size(),1e20);
        for(int i=0;i<dataset.size();i++)
            for(int j=0;j<dataset[i].size();j++)
                mx[j]=max(mx[j],dataset[i][j]),
                mn[j]=min(mn[j],dataset[i][j]);
        uniform_int_distribution<int> intdis(0,dataset.size()-1);
        uniform_real_distribution<double> moddis(0,1.0);
        ofstream ouf("./data_set/"+dataid+"_insert"+to_string(q_num)+".csv",ios::out);
        #ifdef origin
            ofstream of("./data_set/"+dataid+"/"+dataid+"_"+distibut+to_string(q_num)+"_origin.csv",ios::out);
        #endif // origin
        for(int i=0;i<q_num;i++)
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
                {
                    ouf<<normal_distribution<double>(dataset[idx][i],
                                                     min(mx[i]-dataset[idx][i],dataset[idx][i]-mn[i])/6)(gen);
//                    cout<<normal_distribution<double>(dataset[idx][i],
//                                                     min(mx[i]-dataset[idx][i],dataset[idx][i]-mn[i])/6)(gen);
                }
                else ouf<<dataset[idx][i];
                #ifdef origin
                of<<dataset[idx][i];
                #endif // origin
            }
            ouf<<endl;
            #ifdef origin
            of<<"\n";
            #endif // origin
        }
        cout<<dataid<<" build over"<<endl;
    }
    return 0;
}
