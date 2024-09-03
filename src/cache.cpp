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

#include  <cmath>
#include <cstdlib>
#include "cache.h"

Line::Line(int bpl, int blocksz): size(bpl), tags(bpl, 0), counters(bpl,0){}
/*  this->size      =     bpl;
  this->tags      =     vector<int> (bpl, 0);
  this->counters  =     vector<int> ()
  memset(this->tags, 0, sizeof(uint32_t) * bpl);
  memset(this->counters, 0, sizeof(uint32_t) * bpl);
} */

// address: tag of block being replaced in line 
Line::replaceBlock(uint32_t newaddress){

  int ret = -1;
  int maxcounter = this->counters[0];
  int maxidx = 0;
  // Implement LRU
  for(int i = 1; i < this->size; i++){
    maxcounter = (maxcounter >= this->counters[i]) ? maxcounter : this->counters[i];
    maxidx = (maxcounter >= this->counters[i]) ? maxidx : i;
  }

  // i holds idx of block to be evicted.
  ret = this-> tags[i]
  this->tags[i]= = newaddress;

  // Return tag of evicted block
  return ret;

}

// addr : tag of the requested block
Line::getBlock(uint32_t addr){

  for(int i = 0; i < this->size; i++){
    if(this->tags[i] == addr){
      return CACHE_HIT;
    }
  }
  return CACHE_MISS;

}

Cache::Cache(int size, int assoc, int blocksz):size(size), 
                                               assoc(assoc), blocksz(blocksz), 
                                               numlines(size/(assoc*blocksz)){

  for(int i = 0; i < this->numlines; i++){
    lines.push_back(Line(assoc, blocksz));
  }

  this->stat = new Statistics();
  this->BITS_boff = log2(blocksz);
  this->BITS_idx = log2(numlines);
  this->BITS_tag = 32 - BITS_boff - BITS_idx;
  this->vc = NULL;
}

// addr: addr of the read. 
Cache::read(uint32_t addr){
  uint32_t addr_bak = addr;
  uint32_t boff = addr & (0xFFFFFFFF ^ (0xFFFFFFFF << this->BITS_boff));
  addr = addr >> this->BITS_boff;
  uint32_t idx  = addr & (0xFFFFFFFF ^ (0xFFFFFFFF << this->BITS_idx));
  addr = addr >> this->BITS_idx;
  uint32_t tag  = addr;
  line = this->lines[idx];
  RESULT result = line.getBlock(tag);
  if(result == CACHE_MISS){
    // oldblock holds tag of evicted block
    uint32_t oldblock = line.replaceBlock(addr);
    if(oldblock == -1){
      cerr<<"Replace Block failed with return -1 in "<<__FILE__<<" at lineno "<<__LINE__<<endl;
    }
    if(this->vc != NULL){
      vc.read(addr_bak);
      // Victim cache is fully associative and has different tag/idx/ bit numbers
      // best to keep the api simple and all of Cache's member functions take the entire
      // address as the parameter. oldblock has to be modified to inlcude all the bits now;
      uint32_t vc_evict_addr = (oldblock << (BITS_idx + BITS_boff)) + idx << BITS_boff ;
      vc.evict(vc_evict_addr);
    }else{
      if(this->parent==NULL) return CACHE_MISS;
      this->parent.read(addr);
    }
    return CACHE_MISS
  }else{
    return CACHE_HIT;
  }

}


// Basically same logic as read. Not sure if write buffers are needed as of right now
Cache::write(uint32_t addr){
  cerr<<"Not implemented yet. Please talk to me later\n";
  return CACHE_MISS;
}

// Ideally should not 
Cache::evict(uint32_t addr){
  cerr<<"Not implemented yet\n";
  return CACHE_MISS;
}

Cache::createVC(int size, int assoc, int blocksz){
  this->vc = new Cache(size, assoc, blocksz);
}
