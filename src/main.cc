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
       ___)( )(___  
      (((__) (__)))
*/
#include "cache.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include "parse.h"
using namespace std;
void perfStat(Cache L1Cache){
  // Get L1 values
  float L1HT = 0, L1E = 0, L1A = 0;
  auto result = get_cacti_results(L1Cache.size, L1Cache.blocksz, L1Cache.assoc, &L1HT, &L1E, &L1A);
  if(result != 0){
    ERROR("Cacti failed getting L1 results");
  }
  float L2HT = 0, L2E = 0, L2A = 0;
  if(L1Cache.parent != NULL){
    result = get_cacti_results(L1Cache.parent->size, L1Cache.parent->blocksz, L1Cache.parent->assoc, &L2HT, &L2E, &L2A);
    if(result != 0){
      ERROR("Cacti failed getting L2 results");
    }
  }
  float VCHT = 0, VCE = 0, VCA = 0;
  if(L1Cache.vc != NULL){
    result = get_cacti_results(L1Cache.vc->size, L1Cache.vc->blocksz, L1Cache.vc->assoc, &VCHT, &VCE, &VCA);
    if(result != 0){
      VCHT = 0.2;
      VCE = 0;
      VCA = 0;
    }
  }
  // Calculate average access time :
  // Miss rate for ONLY L1+VC
  double L1MR = ((double)L1Cache.stat.rmisses + L1Cache.stat.wmisses - L1Cache.stat.actual_swap);
  L1MR /= (L1Cache.stat.reads + L1Cache.stat.writes);
  // VC Swap rate
  double VCSR = 0; 
  if(L1Cache.vc != NULL)VCSR = ((double)L1Cache.stat.swap)/((double)L1Cache.stat.reads + L1Cache.stat.writes);
  double AAT = 0, L1MP;
  if(L1Cache.parent != NULL){
    // Why does L2MR not have write misses ? 
    double L2MR = ((double)L1Cache.parent->stat.rmisses);
    L2MR /= ((double)L1Cache.parent->stat.reads);
    double L2MP = 20 + (double)L1Cache.blocksz/(16);
    L1MP = L2HT + L2MR * L2MP;
  }
  else{
    L1MP = 20 + (double)L1Cache.blocksz/(16);
  }
  // AAT is l1 hit time + Rate of misses routed to parent * miss penalty + Vc_access_rate*swap time
  AAT = L1HT + L1MR*(L1MP) + VCSR*(VCHT);
  long long scratch = (L1Cache.stat.reads + L1Cache.stat.writes);
  double e1 = scratch*(L1E);
  scratch = (L1Cache.stat.rmisses + L1Cache.stat.wmisses);
  e1 += scratch*(L1E);
  double e2 = 0, e3 = 0;
  scratch = 2*L1Cache.stat.swap;
  if(L1Cache.vc != NULL) e3 = scratch*((VCE));
  if(L1Cache.parent != NULL){
    scratch = (L1Cache.parent->stat.reads + L1Cache.parent->stat.writes);
    scratch += (L1Cache.parent->stat.rmisses + L1Cache.parent->stat.wmisses);
    e2 +=  scratch * (L2E);
    scratch = (L1Cache.parent->stat.rmisses + L1Cache.parent->stat.wmisses + L1Cache.parent->stat.writebacks);
    e2 += scratch*(0.0500000); // 0.05 nJ is Emem
  }else{
    if(L1Cache.vc == NULL) L1Cache.stat.actual_swap = 0;
    scratch = (L1Cache.stat.rmisses + L1Cache.stat.wmisses + L1Cache.stat.writebacks - L1Cache.stat.actual_swap);
    e2 = scratch * (0.0500000);
  }
  double final_edp;
  if(L1Cache.vc != NULL) final_edp = (e1 + e2 + e3)*(AAT);
  else final_edp = (e1 + e2)*(AAT);
  scratch = (L1Cache.stat.reads + L1Cache.stat.writes);
  final_edp *= scratch;
  double totarea = 0;
  totarea = L1A + VCA + L2A;
  cout<<setprecision(4);
  cout<<"1. average access time: "<<AAT<<endl;
  cout<<"2. energy-delay product: "<<final_edp<<endl;
  cout<<"3. total area: "<<totarea<<endl;
}

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
    if(L1Cache.vc == 0){
      ERROR("??");
    }
  }
  char op;
  string addr_s;
   
  ifstream infile(filename);
  while(infile >> op >> addr_s){
    char *p;
    uint32_t addr = strtol(addr_s.c_str(), &p, 16);
    switch(op){
      case 'r':
        L1Cache.read(addr);
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
    int traffic = L1Cache.stat.rmisses + L1Cache.stat.wmisses + L1Cache.stat.writebacks - L1Cache.stat.actual_swap;
    cout<<traffic<<endl;
  }

  cout<<endl;  

  cout<<"===== Simulation results (performance) ====="<<endl;
  perfStat(L1Cache);
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

