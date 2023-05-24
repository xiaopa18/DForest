#include<iostream>
#include<cstring>
#include<algorithm>
#include<fstream>
#include<ctime>
#include<cstdlib>
#include<random>
#define rd (dis(gen))
using namespace std;
mt19937 gen((unsigned int) time(nullptr));
uniform_real_distribution<double> dis(0, 1.0);

double gen_f(int mn,int mx)
{
    return mn+rd*(mx-mn);
}

int RandomUniform(int n)
{
    int top = ((((RAND_MAX - n) + 1) / n) * n - 1) + n;
    int r;
    do{
        r = rand();
    }while (r > top);
    return (r % n);
}

int gen_i(int mn,int mx)
{
    if(mx-mn<RAND_MAX)
    {
        return mn+RandomUniform(mx-mn+1);
    }
    return (int)(mn+rd*(mx-mn));
}

int main()
{
    srand((unsigned)time(NULL));
    int n,dim,mn,mx,isf;
    cout<<"plz input n dim mn mx isfloat(0,1):";
    cin>>n>>dim>>mn>>mx>>isf;
    string fname=string("Uniform")+"_n"+to_string(n)+"_dim"+to_string(dim)+"_mn"+to_string(mn)
                +"_mx"+to_string(mx);
    if(isf) fname+="_float.csv";
    else fname+="_int.csv";
    ofstream ouf(fname,ios::out);
    while(n--)
    {
        for(int i=0;i<dim;i++)
        {
            if(i) ouf<<",";
            if(isf) ouf<<gen_f(mn,mx);
            else ouf<<gen_i(mn,mx);
        }
        ouf<<endl;
    }
    return 0;
}
