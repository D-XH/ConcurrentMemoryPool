#include<iostream>

using namespace std;

#include"ObjectPool.hh"

int main(){
    void* p = malloc(16);
    cout<< sizeof(p)<< endl;
    cout<< sizeof(int*)<< endl;
    cout<< sizeof(long long*)<< endl;
    cout<< p << endl;

    int* pp = nullptr;
    cout<< pp << endl;
    // pp = p;
    // cout<< pp << endl;
    return 0;
}