#ifndef CACHE_SIM
#define CACHE_SIM

enum RESULT{
  CACHE_HIT,
  CACHE_MISS,
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

  private:
  void        replace();
  void *      findBlock();
  
  public:
  
  Cache(int size, int assoc, int blocksz);
  RESULT read(uint32_t addr);
  void write(uint32_t addr);

}
