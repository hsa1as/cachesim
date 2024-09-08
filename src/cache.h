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

#ifndef CACHE_SIM
#define CACHE_SIM
#include<cstdint>
#include<vector>

#define ERROR(x) std::cerr<<"\033[1;31m"<<x<<"\033[0m"<<std::endl

enum RESULT{
  CACHE_HIT,
  CACHE_MISS,
  CACHE_ERR
};

class Statistics{
public:
  int rmisses;
  int wmisses;
  int rhits;
  int whits;
  int reads;
  int writes;
  int swap;
  int actual_swap;
  int writebacks;
  Statistics();
};

class Line{
  public:

  std::vector<uint32_t>   tags;
  std::vector<uint32_t>   counters;
  std::vector<bool>       valid;
  std::vector<bool>       dirty;
  int                     size;
  
  Line(int bpl, int blocksz);
  // Place block in the line. Why separate function ? TODO
  uint64_t    replaceBlock(uint32_t tag, uint8_t lru_inc);
  // Read a block in line
  RESULT      readBlock(uint32_t tag);  
  // Write to block in line
  RESULT      writeBlock(uint32_t tag);
  // Set dirty bit for tag
  RESULT      setDirty(uint32_t tag);
};

class Cache{
public:

  int                 size;
  int                 assoc;
  int                 blocksz;
  int                 numlines;
  int                 BITS_boff;
  int                 BITS_idx;
  int                 BITS_tag;
  bool                isVictim; 
  std::vector<Line>   lines;
  Statistics          stat;
  Cache *             parent;
  Cache *             vc;

  
            Cache(int size, int assoc, int blocksz);
  RESULT    read(uint32_t addr);
  RESULT    write(uint32_t addr); 
  uint64_t  placeVictim(uint32_t addr, bool dirty);
  void      createVC(int size, int assoc, int blocksz);
  void      makeVictim();
  void      setParent(Cache *parent);
  void      dumpCache();
  uint64_t  swap(uint32_t addr_in_vc, uint32_t addr_in_L1, bool dirty);

};

#endif
