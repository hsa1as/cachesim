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
#include<iomanip>
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
  // Simulator config
  cout<<"===== Simulator configuration ====="<<endl;
  cout<<left<<setw(24)<<"  L1_SIZE:"<<setw(0)<<l1s<<endl;
  cout<<left<<setw(24)<<"  L1_ASSOC:"<<setw(0)<<l1a<<endl;
  cout<<left<<setw(24)<<"  L1_BLOCKSIZE:"<<setw(0)<<bsz<<endl;
  cout<<left<<setw(24)<<"  VC_NUM_BLOCKS:"<<setw(0)<<vcs<<endl;
  cout<<left<<setw(24)<<"  L2_SIZE:"<<setw(0)<<l2s<<endl;
  cout<<left<<setw(24)<<"  L2_ASSOC:"<<setw(0)<<l2a<<endl;
  cout<<left<<setw(24)<<"  trace_file:"<<setw(0)<<filename<<endl;
  cout<<endl;

  // Dump L1 Cache contens
  cout<<"===== L1 contents ====="<<endl;
  L1Cache.dumpCache();
  cout<<endl;

  if(vcs != 0){
    cout<<"===== VC contents ====="<<endl;
    L1Cache.vc->dumpCache();
    cout<<endl;
  }
  // Dump L2 Cache contents
  if(l2s != 0){
    // Dump L1 Cache contens
    cout<<"===== L2 contents ====="<<endl;
    L1Cache.parent->dumpCache();
    cout<<endl;
  }
  // Proper STATS
  cout<<fixed<<setprecision(4);
  cout<<"===== Simulation results (raw) ====="<<endl;
  cout<<left<<setw(51)<<"  a. number of L1 reads:"<<setw(0);
  cout<<L1Cache.stat.reads<<endl;
  cout<<left<<setw(51)<<"  b. number of L1 read misses:"<<setw(0);
  cout<<L1Cache.stat.rmisses<<endl;
  cout<<left<<setw(51)<<"  c. number of L1 writes:"<<setw(0);
  cout<<L1Cache.stat.writes<<endl;
  cout<<left<<setw(51)<<"  d. number of L1 write misses:"<<setw(0);
  cout<<L1Cache.stat.wmisses<<endl;
  cout<<left<<setw(51)<<"  e. number of swap requests:"<<setw(0);
  cout<<L1Cache.stat.swap<<endl;
  cout<<left<<setw(51)<<"  f. swap request rate:"<<setw(0);
  cout<<(double)((double)L1Cache.stat.swap/(L1Cache.stat.reads + L1Cache.stat.writes))<<endl;
  cout<<left<<setw(51)<<"  g. number of swaps:"<<setw(0);
  cout<<L1Cache.stat.actual_swap<<endl;
  cout<<left<<setw(51)<<"  h. combined L1+VC miss rate:"<<setw(0);
  cout<<(double)(((double)L1Cache.stat.rmisses + L1Cache.stat.wmisses - L1Cache.stat.actual_swap)/(L1Cache.stat.reads + L1Cache.stat.writes))<<endl;
  cout<<left<<setw(51)<<"  i. number writebacks from L1/VC:"<<setw(0);
  cout<<L1Cache.stat.writebacks<<endl;
  cout<<left<<setw(51)<<"  j. number of L2 reads:"<<setw(0);
  if(l2s != 0) cout<<L1Cache.parent->stat.reads<<endl;
  else cout<<0<<endl;
  cout<<left<<setw(51)<<"  k. number of L2 read misses:"<<setw(0);
  if(l2s != 0) cout<<L1Cache.parent->stat.rmisses<<endl;
  else cout<<0<<endl;
  cout<<left<<setw(51)<<"  l. number of L2 writes:"<<setw(0);
  if(l2s != 0)   cout<<L1Cache.parent->stat.writes<<endl;
  else cout<<0<<endl;
  cout<<left<<setw(51)<<"  m. number of L2 write misses:"<<setw(0);
  if(l2s != 0)  cout<<L1Cache.parent->stat.wmisses<<endl;
  else cout<<0<<endl;
  cout<<left<<setw(51)<<"  n. L2 miss rate:"<<setw(0);
  if(l2s != 0)   cout<<(double)(((double)L1Cache.parent->stat.rmisses)/(L1Cache.parent->stat.reads))<<endl;
  else cout<<0.0f<<endl;
  cout<<left<<setw(51)<<"  o. number of writebacks from L2:"<<setw(0);
  if(l2s != 0)   cout<<L1Cache.parent->stat.writebacks<<endl;
  else cout<<0<<endl;
  cout<<left<<setw(51)<<"  p. total memory traffic:"<<setw(0);
  if(l2s != 0){
    int traffic = L1Cache.parent->stat.rmisses + L1Cache.parent->stat.wmisses + L1Cache.parent->stat.writebacks;
    cout<<traffic<<endl;
  }else{
    int traffic = L1Cache.stat.rmisses + L1Cache.stat.wmisses + L1Cache.stat.writebacks - L1Cache.stat.swap;
    cout<<traffic<<endl;
  }

  cout<<endl;  

  cout<<"===== Simulation results (performance) ====="<<endl;

}
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

