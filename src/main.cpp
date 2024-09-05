/*
     .--,       .--,
    ( (  \.---./  ) )
     '.__/o   o\__.'
        {=  ^  =}
         >  -  <
 ____.""`-------`"".____
/                       \
\ CS6600: Assignment 1  /
/ Cache Simulator       \
\ Author: R Sai Ashwin  /
/ Roll No: NS24Z344     \
\_______________________/
       ___)( )(___  jgs
      (((__) (__)))
*/
#include "cache.h"
#include<iostream>
#include<fstream>

using namespace std;

int main(int argc, char** argv){
  if(argc != 8){
    cerr<<"Need 8 arguments to run\n";
    return -1;
  }
  int l1s = stoi(argv[1]), l1a= stoi(argv[2]), bsz= stoi(argv[3]), vcs= stoi(argv[4])
    , l2s= stoi(argv[5]), l2a= stoi(argv[6]);
  string filename = argv[7];
  Cache L1Cache = Cache(l1s, l1a, bsz);
  if(l2s != 0){
    L1Cache.setParent(new Cache(l2s, l2a, bsz)); 
  }
  if(vcs != 0){
    L1Cache.createVC(vcs*bsz, vcs, bsz);
  }
  char op;
  string addr_s;
   
  ifstream infile(filename);
  while(infile >> op >> addr_s){
    char *p;
    uint32_t addr = strtol(addr_s.c_str(), &p, 16);
    RESULT res;
    switch(op){
      case 'r':
        res = L1Cache.read(addr);
        break;
      case 'w':
        L1Cache.write(addr);
        break;
    }
  }
  L1Cache.dumpCache();
  
  // DEBUG stdin
 /*
  int n;
  cin>>n;
  for(int i = 0; i < n; i++){
    char *p;
    cin>>op;
    cin>>addr_s;
    uint32_t addr = strtol(addr_s.c_str(), &p, 16);
    RESULT res;
    switch(op){
      case 'r':
        res = L1Cache.read(addr);
        break;
      case 'w':
        L1Cache.write(addr);
        break;
    }
    if(res == CACHE_HIT) cout<<"HIT!"<<endl;
    else cout<<"MISS!"<<endl;
    L1Cache.dumpCache();
  }*/
  cout<<"STATS\n";
  cout<<"L1 Reads: "<<L1Cache.stat.reads<<endl;
  cout<<"L1 Read Misses: "<<L1Cache.stat.rmisses<<endl;
  cout<<"L1 Read Hits: "<<L1Cache.stat.rhits<<endl;
  cout<<"L1 Writes: "<<L1Cache.stat.writes<<endl;
  cout<<"L1 Write misses: "<<L1Cache.stat.wmisses<<endl;
  cout<<"Writebacks from L1/VC: "<<L1Cache.stat.writebacks<<endl;
  if(l1s != 0){
    cout<<"L2 STATS\n";
    cout<<"L2 Reads: "<<L1Cache.parent->stat.reads<<endl;
    cout<<"L2 Read Misses: "<<L1Cache.parent->stat.rmisses<<endl;
    cout<<"L2 Read Hits: "<<L1Cache.parent->stat.rhits<<endl;
    cout<<"L2 Writes: "<<L1Cache.parent->stat.writes<<endl;
    cout<<"L2 Write misses: "<<L1Cache.parent->stat.wmisses<<endl;
    cout<<"Writebacks from L1/VC: "<<L1Cache.parent->stat.writebacks<<endl;
  }  
}
