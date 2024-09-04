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

#include <cmath>
#include <cstdlib>
#include "cache.h"
#include <iostream>
Line::Line(int bpl, int blocksz): size(bpl), tags(bpl, 0), counters(bpl,0),
                                  dirty(bpl,0) ,valid(bpl, false){}
// address: tag of block being replaced in line 
// Retval: tag of the evicted block, if any
// if there are no evicted blocks ( i.e some blocks are invalid )
// we can use the upper 32 bits of the return value to signal this
// ( ret >> 33 ) & 1 represents an error 
// ( ret >> 34 ) & 1 represents usage of invalid block in the line
// ( ret >> 35 ) & 1 represents evicting of a dirty block. The owner
// of the line needs to perform a writeback to the parent
uint64_t Line::replaceBlock(uint32_t newaddress){

  uint64_t ret = 1 << 33;

  // Check if any blocks are free (invalid)
  for(int i = 0; i < this->size; i++){
    if(this->valid[i] == false){
      this->tags[i] = newaddress;
      this->valid[i] = true;
      this->counters[i] = 0;
      for(int j = 0; j < this->size; j++){
        if(i == j) continue;
        this->counters[j]++;
      }
      return 1 << 34;
    }
  }

  // No blocks are free, find block to replace
  int maxcounter = this->counters[0];
  int maxidx = 0;
  // Implement LRU
  int i = 1;
  for(; i < this->size; i++){
    maxcounter = (maxcounter >= this->counters[i]) ? maxcounter : this->counters[i];
    maxidx = (maxcounter >= this->counters[i]) ? maxidx : i;
  }

  // maxidx holds idx of block to be evicted.
  // Check if evicted block is dirty
  ret = this->tags[maxidx];
  if(this->dirty[maxidx]){
    ret = ret | 1 << 35;
  }
  this->tags[maxidx] = newaddress;
  this->counters[maxidx] = 0;
  this->dirty[maxidx] = 0;
  for(int j = 0; j < this->size; j++){
    if(j == maxidx) continue;
    this->counters[j]++;
  }

  // Return tag of evicted block

  return ret;

}

// addr : tag of the requested block
RESULT Line::readBlock(uint32_t addr){

  for(int i = 0; i < this->size; i++){
    if(this->tags[i] == addr){
      for(int j = 0; j < this->size; j++){
        if(i == j){
          counters[i] = 0;
        }else{
          this->counters[i]++;
        }
      }
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

  this->stat = Statistics();
  this->BITS_boff = log2(blocksz);
  this->BITS_idx = log2(numlines);
  this->BITS_tag = 32 - BITS_boff - BITS_idx;
  this->vc = NULL;
  this->parent = NULL;
  this->isVictim = false;
}

// addr: addr of the read. 
RESULT Cache::read(uint32_t addr){
  uint32_t addr_bak = addr;
  uint32_t boff = addr & (0xFFFFFFFF ^ (0xFFFFFFFF << this->BITS_boff));
  addr = addr >> this->BITS_boff;
  uint32_t idx  = addr & (0xFFFFFFFF ^ (0xFFFFFFFF << this->BITS_idx));
  addr = addr >> this->BITS_idx;
  uint32_t tag  = addr;
  Line line = this->lines[idx];
  RESULT result = line.readBlock(tag);
  if(result == CACHE_MISS){
    // oldblock holds tag of evicted block
    uint64_t oldblock = line.replaceBlock(addr);
    if((oldblock >> 33 ) & 1 ){
      std::cerr<<"Replace Block failed with return -1 in "
        <<__FILE__<<" at lineno "<<__LINE__<<std::endl;
    }
    
    // Victim cache interactions
    // Victim cache is fully associative and has different tag/idx/ bit numbers
    // best to keep the api simple and all of Cache's member functions take the entire
    // address as the parameter. oldblock has to be modified to inlcude all the bits now;
    // Check bit 34 to see if a block was evicted from the line
    if(this->vc != NULL && !((oldblock >> 34) & 1)){
      RESULT vc_res = vc->read(addr_bak);
      uint32_t vc_place_addr = (oldblock << (BITS_idx + BITS_boff)) + idx << BITS_boff ;
      if(vc_res == CACHE_MISS){
        // Requested block is NOT in victim cache
        // Get block from parent memory
        result = parent.read(addr_bak);
        // Put evicted oldblock in victim cache
        vc->placeVictim(vc_place_addr);
      }else{
        // Requested block IS in victim cache.
        // Swap oldblock in this cache with requested block in victim cache
        vc->swap(addr_bak, vc_place_addr);
      }
     
    }else{
      if(this->parent==NULL) return CACHE_MISS;
      this->parent->read(addr);
    }
    return CACHE_MISS;
  }else{
    return CACHE_HIT;
  }

}


// Basically same logic as read. Not sure if write buffers are needed as of right now
RESULT Cache::write(uint32_t addr){
  std::cerr<<"Not implemented yet. Please talk to me later\n";
  return CACHE_MISS;
}

// Method used only for victim caches
RESULT Cache::placeVictim(uint32_t addr){
  if(this->isVictim == false){
    cerr<<"PlaceVictim called on cache that is not a victim cache\n";
    return CACHE_ERR;
  }

  return CACHE_MISS;
}

void Cache::createVC(int size, int assoc, int blocksz){
  this->vc = new Cache(size, assoc, blocksz);
}

void Cache::makeVictim(){
  this->isVicitm = true;
}

void Cache::setParent(Cache *parent){
  this->parent = parent;
}

RESULT Cache::swap(uint32_t addr_in_vc, uint32_t addr_in_L1){
  if(this->isVictim == false){
    cerr<<"Swap called on cache that is not a victim cache\n";
    return CACHE_ERR
  }
}

