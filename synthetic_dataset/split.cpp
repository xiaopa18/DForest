#include<bits/stdc++.h>
#include"../query/myfunctions.hpp"

using namespace std;
string dataid="";
string file_name="../data_set/"+dataid+"/"+dataid+".csv";
vector<int> dims({256,512,1024,2048,4096});
vector<int> ns({1e5,3e5,5e5,7e5,1e6});

int main()
{
    vector<vector<string>> dataset=read_csv(file_name);
    vector<string> sps=splitby(dataid,'_');
    for(int dim:dims)
    {
        ofstream ouf("./"+sps[0]+"_n"+to_string(ns[0])+"_dim"+to_string(dim),ios::out);
        for(int i=0;i<ns[0].size();i++)
        {
            for(int j=0;j<dim;j++)
            {
                if(j) ouf<<",";
                ouf<<dataset[i][j];
            }
            ouf<<"\n";
        }
        ouf.close();
    }
    for(int n:ns)
    {
        if(n==1e5) continue;
        ofstream ouf("./"+sps[0]+"_n"+to_string(n)+"_dim"+to_string(256),ios::out);
        for(int i=0;i<ns;i++)
        {
            for(int j=0;j<256;j++)
            {
                if(j) ouf<<",";
                ouf<<dataset[i][j];
            }
            ouf<<"\n";
        }
        ouf.close();
    }
    return 0;
}
