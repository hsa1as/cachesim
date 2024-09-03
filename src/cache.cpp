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

Line::replaceBlock(uint32_t oldaddress, uint32_t newaddress){

  int ret = -1;
  for(int i = 0; i < this->size; i++){
    int curtag = tags[i];
    if(curtag == oldaddress){
      this->tags[i] = newaddress;
      ret = oldaddress;
    }
  }
  return ret;

}

Line::getBlock(uint32_t address){

  for(int i = 0; i < this->size; i++){
    if(this->tags[i] == address){
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

Cache::read(uint32_t addr){

  uint32_t boff = addr & (0xFFFFFFFF ^ (0xFFFFFFFF << this->BITS_boff));
  addr = addr >> this->BITS_boff;
  uint32_t idx  = addr & (0xFFFFFFFF ^ (0xFFFFFFFF << this->BITS_idx));
  addr = addr >> this->BITS_idx;
  uint32_t tag  = addr;
  line = this->lines[idx];
  RESULT result = line.getBlock(tag);
  if(result == CACHE_MISS){
    uint32_t oldblock = line.replaceBlock(addr);
    if(this->vc != NULL){
      vc.read(addr);
      vc.evict(oldblock);
    }else{
      this->parent.read(addr);
    }
    return CACHE_MISS
  }else{
    return CACHE_HIT;
  }

}




