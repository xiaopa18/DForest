#include<iostream>
#include<cstring>
#include<algorithm>
#include<fstream>
#include<ctime>
#include<cstdlib>
#include<random>
#include<vector>

using namespace std;
mt19937 gen((unsigned int) time(nullptr));
uniform_real_distribution<double> dis(0, 1.0);
int n,basen,cyc,mx;

int rd(int n)
{
    if(n<RAND_MAX)
    {
        int top = ((((RAND_MAX - n) + 1) / n) * n - 1) + n;
        int r;
        do{
            r = rand();
        }while (r > top);
        return (r % n);
    }
    return (int)(n*dis(gen));
}

void dfs(int use,int num,int step,vector<int> &k,vector<int> &res)
{
    if(!use)
    {
        res.push_back(num%(mx+1));
        return ;
    }
    if(step>=basen or step+use-1>=basen) return ;
    dfs(use-1,num+k[use-1]*res[step],step+1,k,res);
    dfs(use,num,step+1,k,res);
}

vector<int> genvc(int n,vector<vector<int>> &ks)
{
    vector<int> res;
    for(int i=0;i<n;i++)
        res.push_back(rd(mx));
    for(int i=1;i<n;i++)
        dfs(i,0,0,ks[i-1],res);
    return res;
}

int main()
{
    cout<<"plz input n basen cyc maxnumber:";
    cin>>n>>basen>>cyc>>mx;
    int dim=((1<<basen)-2+basen)*cyc;
    string file_name=string("Syntheic")+"_n"+to_string(n)+"_dim"+to_string(dim)
                    +"_basen"+to_string(basen)+"_mx"+to_string(mx)+".csv";
    ofstream ouf(file_name,ios::out);
    vector<vector<int>> dataset(n,vector<int>());
    vector<vector<vector<int>>> ks;
    for(int i=0;i<cyc;i++)
    {
        vector<vector<int>> k;
        for(int i=1;i<basen;i++)
        {
            vector<int> kp;
            int t=i;
            while(t--) kp.push_back(rd(100));
            k.push_back(kp);
        }
        ks.push_back(k);
    }
    for(int i=0;i<n;i++)
    {
        for(int j=0;j<cyc;j++)
        {
            vector<int> tmp=genvc(basen,ks[j]);
            for(int tp:tmp) ouf<<tp<<",";
        }
        ouf<<"\n";
    }
    ouf.close();
    return 0;
}
