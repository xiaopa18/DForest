#include<iostream>
#include<cstring>
#include<algorithm>
#include "./Loss.hpp"
using namespace std;

int main()
{
    Loss loss;
    for(int i=1;i<=16;i++) cout<<loss.insert(i)<<" ";
    cout<<endl;
    loss.show();
    for(int i=0;i<16;i++) cout<<loss.query(i)<<" ";
    cout<<endl;
    loss.show();
    cout<<endl<<endl;
    loss.del(4);
    loss.show();
    for(int i=0;i<15;i++) cout<<loss.query(i)<<" ";
    return 0;
}
