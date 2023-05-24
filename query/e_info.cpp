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
vector<string> dataids({"Uniform_n100000_dim256","Uniform_n100000_dim512","Uniform_n100000_dim1024","Uniform_n100000_dim2048","Uniform_n100000_dim4096","Uniform_n300000_dim256"
,"Uniform_n500000_dim256","Uniform_n700000_dim256","Uniform_n1000000_dim256"});
string dataid="Uniform_n100000_dim256";
string dataset_file = "../data_set/"+dataid+"/"+dataid+"_afterpca.csv";
typedef pair<string,double> PSD;
map<int,vector<PSD>> mp;
vector<Node> dataset;

int main()
{
    for(string dataid:dataids)
    {
        dataset_file = "../data_set/"+dataid+"/"+dataid+"_afterpca.csv";
        dataset=read_dataset(dataset_file);
        get_dim(dataset,0);
        double dis=0;
        for(int i=0;i<dataset.size();i++)
        {
            double res=0;
            for(int j=0;j<dataset[i].data.size();j++)
                res+=dataset[i].data[j]*dataset[i].data[j];
            dis+=res/dataset.size();
        }
        dis=sqrt(dis);
        for(int i=1;i<=100;i++)
        {
            mp[i].push_back({dataid,dis*i/100});
        }
        cout<<dataid<<" over"<<endl;
    }
    ofstream ouf("../e_info.txt",ios::out);
    ouf.setf(ios::fixed);
    for(int i=1;i<=100;i++)
    {
        ouf<<i<<"%:\n";
        for(auto &tmp:mp[i])
            ouf<<tmp.fi<<"\t\t"<<tmp.se<<"\n";
    }
    return 0;
}
