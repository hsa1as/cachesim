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

#include <algorithm>
#include <stdio.h>
#include <cmath>
#include <cstdlib>
#include "cache.h"
#include <iostream>
#include <iomanip>
#include <utility>
#include <algorithm>


Statistics::Statistics(){
  this->rmisses = 0;
  this->wmisses = 0;
  this->rhits = 0;
  this->whits = 0;
  this->reads = 0;
  this->writes = 0;
  this->swap = 0;
  this->actual_swap = 0;
  this->writebacks = 0;
}

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
uint64_t Line::replaceBlock(uint32_t tag, uint8_t lru_inc = 1){

  uint64_t ret = 1LL << 33;

  // Check if any blocks are free (invalid)
  for(int i = 0; i < this->size; i++){
    if(this->valid[i] == false){
      this->tags[i] = tag;
      this->valid[i] = true;
      this->counters[i] = 0;
      for(int j = 0; j < this->size; j++){
        if(i == j) continue;
        this->counters[j]+=lru_inc;
      }
      return 1LL << 34;
    }
  }

  // No blocks are free, find block to replace
  int maxcounter = this->counters[0];
  int maxidx = 0;
  // Implement LRU
  int i = 1;
  for(; i < this->size; i++){
    if(maxcounter < this->counters[i]){
      maxcounter = this->counters[i];
      maxidx = i;
    }
  }

  // maxidx holds idx of block to be evicted.
  // Check if evicted block is dirty
  ret = this->tags[maxidx];
  if(this->dirty[maxidx]){
    ret = ret | 1LL << 35;
  }
  this->tags[maxidx] = tag;
  this->counters[maxidx] = 0;
  this->dirty[maxidx] = false;
  for(int j = 0; j < this->size; j++){
    if(j == maxidx) continue;
    this->counters[j]+=lru_inc;
  }

  // Return tag of evicted block
  return ret;
}

// addr : tag of the requested block
RESULT Line::readBlock(uint32_t tag){

  for(int i = 0; i < this->size; i++){
    if(this->tags[i] == tag && this->valid[i] == true){
      for(int j = 0; j < this->size; j++){
        if(i == j){
          counters[i] = 0;
        }else{
          this->counters[j]++;
        }
      }
      return CACHE_HIT;
    }
  }
  return CACHE_MISS;
}

// tag: tag of block to be written to
RESULT Line::writeBlock(uint32_t tag){
  for(int i = 0; i < this-> size; i++){
    if(this->tags[i] == tag){
      for(int j = 0; j<this->size; j++){
        if(i==j){
          this->counters[i] = 0;
        }else{
          this->counters[j]++;
        }
      }
      this->dirty[i] = true;
      return CACHE_HIT;
    }
  }
  return CACHE_MISS;
}

// Set dirty bit for a block in the line
RESULT Line::setDirty(uint32_t tag){
  for(int i = 0; i < this->size; i++){
    if(this->tags[i] == tag){
      this->dirty[i] = true;
      return CACHE_HIT;
    }
  }
  return CACHE_MISS;
}

Cache::Cache(int _size, int _assoc, int _blocksz):size(_size), 
                                               assoc(_assoc), blocksz(_blocksz), 
                                               numlines(_size/(_assoc*_blocksz)){

  for(int i = 0; i < this->numlines; i++){
    lines.push_back(Line(_assoc, _blocksz));
  }

  this->stat = Statistics();
  this->BITS_boff = log2(_blocksz);
  this->BITS_idx = log2(numlines);
  this->BITS_tag = 32 - BITS_boff - BITS_idx;
  this->vc = NULL;
  this->parent = NULL;
  this->isVictim = false;
}

// addr: addr of the read. 
RESULT Cache::read(uint32_t addr){
  this->stat.reads++;
  uint32_t addr_bak = addr;
  uint32_t boff = addr & (0xFFFFFFFF ^ (0xFFFFFFFF << this->BITS_boff));
  addr = addr >> this->BITS_boff;
  uint32_t idx  = addr & (0xFFFFFFFF ^ (0xFFFFFFFF << this->BITS_idx));
  addr = addr >> this->BITS_idx;
  uint32_t tag  = addr;
  RESULT result = this->lines[idx].readBlock(tag);
  // If we are a victim cache, we only want to return whether or not a block is in the cache without
  // doing any allocation on our own
  if(this->isVictim) return result;
  if(result == CACHE_MISS){
    this->stat.rmisses++;
    uint64_t oldblock = this->lines[idx].replaceBlock(tag);
    if((oldblock >> 33 ) & 1 ){
      ERROR("Replace Block failed with return -1");
    }
    // You just evicted a block. what if: it is valid, and dirty?
    // Need to writeback, but only if vc does NOT exist 
    // if vc exists, this evicted block is guaranteed to end up in vc. 
    // writebacks will be taken care of when the thing is evicted from vc
    // TODO: figure out if VC swap happens with a dirty block what happens
    if(((oldblock >> 34) & 1)!=1 && ((oldblock >> 35) & 1) == 1 && this->vc == NULL){
      // We evicted a valid, dirty block. Issue a writeback
      uint32_t parent_write_addr = oldblock & 0xFFFFFFFF;
      parent_write_addr = (parent_write_addr << (BITS_idx + BITS_boff)) + (idx << BITS_boff);
      if(this->parent != NULL) this->parent->write(parent_write_addr);
      this->stat.writebacks++;
    }

    // Victim cache interactions
    // Victim cache is fully associative and has different tag/idx/ bit numbers
    // best to keep the api simple and all of Cache's member functions take the entire
    // address as the parameter. oldblock has to be modified to inlcude all the bits now;
    // Check bit 34 to see if a block was evicted from the line
    if(this->vc != NULL && !((oldblock >> 34) & 1)){
      this->stat.swap++;
      RESULT vc_res = this->vc->read(addr_bak);
      uint32_t vc_place_addr = ((oldblock & 0xFFFFFFFF) << (BITS_idx + BITS_boff)) + (idx << BITS_boff) ;
      if(vc_res == CACHE_MISS){
        // Requested block is NOT in victim cache
        // Get block from parent memory
        if(this->parent != NULL) result = this->parent->read(addr_bak);
        // Put evicted oldblock in victim cache
        bool isdirty = (oldblock >> 35) & 1;
        uint64_t vc_evicted_block = this->vc->placeVictim(vc_place_addr, isdirty);
        // Maybe i shouldn't do this?
        if((vc_evicted_block >> 35) & 1 && !((vc_evicted_block >> 34) & 1)){
          // The VC Evicted a block that was dirty. We need to issue a VC writeback to parent.
          if(this->parent != NULL) this->parent->write(vc_evicted_block & 0xFFFFFFFF);
          this->stat.writebacks++;
        }
      }else{
        this->stat.actual_swap++;
        // Requested block IS in victim cache.
        // Swap oldblock in this cache with requested block in victim cache
        bool isdirty = (oldblock >> 35) & 1;
        uint64_t vc_swapped_block = this->vc->swap(addr_bak, vc_place_addr, isdirty);
        if(!(vc_swapped_block >> 34 & 1) && (vc_swapped_block >> 35 & 1)){
          // Need to set dirty bit for the block that we just brought 
          this->lines[idx].setDirty(addr);  
        }
      }
     
    }else{
      if(this->parent==NULL) return CACHE_MISS;
      this->parent->read(addr_bak);
    }
    return CACHE_MISS;
  }else{
    this->stat.rhits++;
    return CACHE_HIT;
  }

}

// Basically same logic as read. Not sure if write buffers are needed as of right now
RESULT Cache::write(uint32_t addr){
  this->stat.writes++;
  uint32_t addr_bak = addr;
  uint32_t boff = addr & (0xFFFFFFFF ^ (0xFFFFFFFF << this->BITS_boff));
  addr = addr >> this->BITS_boff;
  uint32_t idx  = addr & (0xFFFFFFFF ^ (0xFFFFFFFF << this->BITS_idx));
  addr = addr >> this->BITS_idx;
  uint32_t tag  = addr;
  // Check if block in line
  RESULT result = this->lines[idx].writeBlock(tag);
  // If we are victim, simply return the result
  if(this->isVictim) return result;
  if(result == CACHE_MISS){
    this->stat.wmisses++;
    // If block not in line, evict and place after read from parent and vc
    // oldblock holds tag of evicted block
    // TODO: instructing replaceBlock to not increment LRU counters to prevent 
    // double incrementing here and in subsequent call to writeBlock again 
    // ( needed to update dirty bits after block allocation through replaceBlock )
    // suggested soln: let replaceBlock NEVER update LRU counters
    // Do a second call to readBlock for Cache::read methods to increment LRU counters for read
    uint64_t oldblock = this->lines[idx].replaceBlock(tag, 0);
    if((oldblock >> 33 ) & 1 ){
      ERROR("Replace Block failed with return -1");
    }
    // You just evicted a block. what if: it is valid, and dirty?
    // Need to writeback
    // TODO: figure out if VC swap happens with a dirty block what happens
    if(((oldblock >> 34) & 1) != 1 && ((oldblock >> 35) & 1) == 1 && this->vc == NULL){
      // We evicted a valid, dirty block. Issue a writeback
      uint32_t parent_write_addr = oldblock & 0xFFFFFFFF;
      parent_write_addr = (parent_write_addr << (BITS_idx + BITS_boff)) + (idx << BITS_boff);
      if(this->parent != NULL) this->parent->write(parent_write_addr);
      this->stat.writebacks++;
    }

    // Since previous call to line.writeBlock missed, we need to call it again
    // after allocating the block in the line.
    RESULT intermediate = this->lines[idx].writeBlock(tag);
    if(intermediate != CACHE_HIT){
     ERROR("Cache MISS after placing block in line. Should not have happened\n");
    }
    // Victim cache interactions
    // Victim cache is fully associative and has different tag/idx/ bit numbers
    // best to keep the api simple and all of Cache's member functions take the entire
    // address as the parameter. oldblock has to be modified to inlcude all the bits now;
    // Check bit 34 to see if a block was evicted from the line
    if(this->vc != NULL && !((oldblock >> 34) & 1)){
      this->stat.swap++;
      // Do not need to call write on victim cache
      RESULT vc_res = this->vc->read(addr_bak);
      uint32_t vc_place_addr = (oldblock << (BITS_idx + BITS_boff)) + (idx << BITS_boff) ;
      if(vc_res == CACHE_MISS){
        // Requested block is NOT in victim cache
        // Get block from parent memory
        // TODO: Figure out:
        // Write allocate: If L1 misses on write, shoult L2 stats increase L2 read or L2 write?
        // Currently doing read 
        if(this->parent != NULL) result = this->parent->read(addr_bak);
        // Put evicted oldblock in victim cache
        // Check if oldblock is a dirty block
        bool isdirty = (oldblock >> 35) & 1;
        uint64_t vc_evicted_block = this->vc->placeVictim(vc_place_addr, isdirty);
        if( !( ( vc_evicted_block >> 34 ) & 1 ) && ( ( vc_evicted_block >> 35 ) & 1 ) ){
          // The VC Evicted a block that was valid and dirty. We need to issue a VC writeback to parent.
          if(this->parent != NULL) this->parent->write(vc_evicted_block & 0xFFFFFFFF);
          this->stat.writebacks++;
        }
      }else{
        this->stat.actual_swap++;
        // Requested block IS in victim cache.
        // Swap oldblock in this cache with requested block in victim cache
        bool isdirty = (oldblock >> 35) & 1;
        uint64_t vc_swapped_block = this->vc->swap(addr_bak, vc_place_addr, isdirty);
        if(!(vc_swapped_block >> 34 & 1) && ((vc_swapped_block >> 35) & 1)){
          // Need to set dirty bit for the block that we just brought 
          this->lines[idx].setDirty(addr);  
        }
        
      }
     }else{
      if(this->parent==NULL) return CACHE_MISS;
      this->parent->read(addr_bak);
    }
    return CACHE_MISS;
  }else{
    this->stat.whits++;
    return CACHE_HIT;
  }
}

void Cache::setParent(Cache *parent){
  this->parent = parent;
}

// Victim cache related functions
void Cache::createVC(int size, int assoc, int blocksz){
  this->vc = new Cache(size, assoc, blocksz);
  if(this->vc == 0){
    ERROR("new Cache returned 0 while making a victim cache");
  }
  this->vc->makeVictim();
}

void Cache::makeVictim(){
  this->isVictim = true;
}

uint64_t Cache::swap(uint32_t addr_in_vc, uint32_t addr_in_L1, bool dirty){
  if(this->isVictim == false){
    ERROR("Swap called on cache that is not a victim cache\n");
    return 1LL<<33; 
  }
  uint64_t retval = 1LL<<33;
  uint32_t tag_in_vc = addr_in_vc >> (this->BITS_boff + this->BITS_idx);
  uint32_t tag_in_L1 = addr_in_L1 >> (this->BITS_boff + this->BITS_idx);
  // just do this manually atp cause the object model ive made is so bad
  // only one line
  for(int i = 0; i < this->lines[0].size; i++){
    if(this->lines[0].tags[i] == tag_in_vc){
      retval = addr_in_vc;
      if(this->lines[0].dirty[i]) retval = retval | (1LL << 35);
      if(this->lines[0].valid[i] == false) retval = retval | (1LL << 34); // shouldn't happen
      this->lines[0].tags[i] = tag_in_L1;
      for(int j = 0; j < this->lines[0].size; j++){
        if(i == j) continue;
        this->lines[0].counters[j]++;
      }
      this->lines[0].dirty[i] = dirty;
      this->lines[0].valid[i] = true;
      this->lines[0].counters[i] = 0;
    }
  }
  return retval;
}
// Method used only for victim caches
uint64_t Cache::placeVictim(uint32_t addr, bool dirty){
  if(this->isVictim == false){
    ERROR("PlaceVictim called on cache that is not a victim cache\n");
    return 1LL<<33;
  }
  // place addr into this->lines[0] and set valid, dirty if(dirty), and update counters
  uint32_t addr_bak = addr;
  uint32_t boff = addr & (0xFFFFFFFF ^ (0xFFFFFFFF << this->BITS_boff));
  addr = addr >> this->BITS_boff;
  uint32_t idx  = addr & (0xFFFFFFFF ^ (0xFFFFFFFF << this->BITS_idx));
  addr = addr >> this->BITS_idx;
  uint32_t tag  = addr;
  RESULT result = this->lines[idx].readBlock(tag);
  if(result != CACHE_MISS){
    ERROR("This shouldn't have happened. placeVictim was called when the block exists in VC");
  }
  // replace a block in the line of VC with new block
  // do NOT update LRU counters if dirty because we will use writeBlock on the line
  // to set the dirty bit
  uint64_t evicted_block;
  if(dirty){
    evicted_block = this->lines[idx].replaceBlock(tag, 0);
    RESULT write_result = this->lines[idx].writeBlock(tag);
    if(write_result != CACHE_HIT){
      ERROR("writeBlock missed in placeVictim even after replaceBlock call");
      return 1LL<<33;
    }
  }else{
    evicted_block = this->lines[idx].replaceBlock(tag, 1);
  }
  // I think we can directly return evicted block?
  // I could implement writebacks here instead of doing it in L1
  // ,but that requires planning that i didn't do. 
  // bitmagic to reconstruct address while retaining status bits in upper 32 bits 
  return ((evicted_block & 0xFFFFFFFFLL<<32) | ((evicted_block & 0xFFFFFFFF)<<(this->BITS_boff + this->BITS_idx)));

}

// Output functions
void Cache::dumpCache(){
  int lineidx = 0;
  for(auto line: this->lines){
    std::cout<<std::left<<std::setw(8)<<"  set "<<std::setw(0)<<lineidx<<":\t";
    // Implement outputting LRU first
    // Make vector of pair -> tag and counters
    std::vector<std::pair<uint32_t, std::pair<uint32_t, bool>>> sorted_tag;
    for(int i = 0; i < line.size; i++){
      sorted_tag.push_back(std::make_pair(line.counters[i], std::make_pair(line.tags[i], line.dirty[i])));
    }
    std::sort(sorted_tag.begin(), sorted_tag.end());
    for(auto x: sorted_tag){
      std::cout<<std::hex<<x.second.first<<" "<<std::dec;
      if(x.second.second) std::cout<<"D";
      else std::cout<<" ";
      std::cout<<"\t";
    }
    std::cout<<std::endl;
    lineidx++;
  } 
}
