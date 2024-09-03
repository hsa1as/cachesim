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

#ifndef CACHE_SIM
#define CACHE_SIM

#include<vector>

enum RESULT{
  CACHE_HIT,
  CACHE_MISS,
  CACHE_ERR
}

class Statistics(){
  int misses;
  int hits;
  int reads;
  int writes;
  int swap;

  public:
  Statistics();
}

class Line{
  vector<int>           tags;
  vector<int>           counters;
  int                   size;
  public:
  
  Line(int bpl, int blocksz);
  // Replace block with oldaddress, with block having newaddress. Return tag of evicted block
  int         replaceBlock(uint32_t newaddress);
  // Get address of block having address = address
  RESULT      getBlock(uint32_t address);  

}

class Cache{
  int           size;
  int           assoc;
  int           blocksz;
  int           numlines;
  int           BITS_boff;
  int           BITS_idx;
  int           BITS_tag;
  vector<Line>  lines;
  Statistics    stat;
  Cache *       parent;
  Cache *       vc;

  public:
  
  Cache(int size, int assoc, int blocksz);
  RESULT read(uint32_t addr);
  RESULT write(uint32_t addr); 
  RESULT evict(uint32_t addr);
  void   createVC(int size, int assoc, int blocksz);
}

#endif
