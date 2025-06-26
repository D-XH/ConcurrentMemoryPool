#include<iostream>

using namespace std;

#include"ConcurrentAlloc.hh"
#include<vector>

int main(){
    // void* arr[1010] = {0};
    // for(int i = 1; i < 1000; i++){
    //     arr[i] = ConcurrentAlloc(i*2+i%20);
    //     if(arr[i] == nullptr){
    //         cout<<i*2+i%20<< endl;
    //         return 0;
    //     }
    // }
    // for(int i = 1; i < 1000; i++){
    //     ConcurrentFree(arr[i]);
    // }


    vector<thread> v;
    v.push_back(thread([](){
        void* arr[1010] = {0};
        for(int i = 1; i < 1000; i++){
            arr[i] = ConcurrentAlloc(i*2+i%20);
            if(arr[i] == nullptr){
                cout<<i*2+i%20<< endl;
                return;
            }
        }
        for(int i = 1; i < 1000; i++){
            ConcurrentFree(arr[i]);
        }

    }));
    v.push_back(thread([](){
        void* arr[1010] = {0};
        for(int i = 1; i < 1000; i++){
            arr[i] = ConcurrentAlloc(i*4);
            if(arr[i] == nullptr){
                cout<<i*2+i%20<< endl;
                return;
            }
        }
        for(int i = 1; i < 1000; i++){
            ConcurrentFree(arr[i]);
        }

    }));

    for(auto& t: v){
        t.join();
    }

    return 0;
}