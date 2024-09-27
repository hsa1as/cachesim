CC = g++
OPT = -O3
OP_DIR = outputs
REF_DIR = ref_outputs
#OPT = -g
WARN = -Wall -Wno-reorder
CFLAGS = $(OPT) $(WARN) $(INC) $(LIB)

# List all your .cpp files here (source files, excluding header files)
SIM_SRC = main.cpp cache.cpp

# List corresponding compiled object files here (.o files)
SIM_OBJ = main.o cache.o 
 
#################################

# default rule

all: cache_sim
	@echo "my work is done here..."


# rule for making cache_sim
cache_sim: $(SIM_OBJ)
	$(CC) -o cache_sim $(CFLAGS) $(SIM_OBJ) -lm

	@echo "-----------DONE WITH CACHE_SIM-----------"


# generic rule for converting any .cc file to any .o file
 
.cc.o:
	$(CC) $(CFLAGS)  -c $*.cc

# generic rule to make .o from .cc, edited the above 
#%.o: %.cc
#	$(CC) $(CFLAGS) -c -o $@ $<
# type "make clean" to remove all .o files plus the cache_sim binary

clean:
	rm -f *.o cache_sim
	rm -rf ./$(OP_DIR)

# type "make clobber" to remove all .o files (leaves cache_sim binary)

clobber:
	rm -f *.o
	rm -rf ./$(OP_DIR)

run: all
	-rm -rf ./$(OP_DIR)
	mkdir ./$(OP_DIR)
	./cache_sim 1024 2 16 0 0 0 gcc_trace.txt > ./$(OP_DIR)/output_0.txt
	./cache_sim 1024 2 16 16 0 0 gcc_trace.txt > ./$(OP_DIR)/output_1.txt
	./cache_sim 1024 2 16 0 8192 4 gcc_trace.txt > ./$(OP_DIR)/output_2.txt
	./cache_sim 1024 2 16 16 8192 4 gcc_trace.txt > ./$(OP_DIR)/output_3.txt
	./cache_sim 1024 1 16 0 0 0 gcc_trace.txt > ./$(OP_DIR)/output_4.txt
	./cache_sim 1024 1 16 16 0 0 gcc_trace.txt > ./$(OP_DIR)/output_5.txt
	./cache_sim 1024 1 16 0 8192 4 gcc_trace.txt > ./$(OP_DIR)/output_6.txt
	./cache_sim 1024 1 16 16 8192 4 gcc_trace.txt > ./$(OP_DIR)/output_7.txt

diff: run 
	@echo "Diff for output # 0"
	-diff -iw ./$(OP_DIR)/output_0.txt ./$(REF_DIR)/gcc.output0.txt
	@echo "Diff for output # 1"
	-diff -iw ./$(OP_DIR)/output_1.txt ./$(REF_DIR)/gcc.output1.txt
	@echo "Diff for output # 2"
	-diff -iw ./$(OP_DIR)/output_2.txt ./$(REF_DIR)/gcc.output2.txt
	@echo "Diff for output # 3"
	-diff -iw ./$(OP_DIR)/output_3.txt ./$(REF_DIR)/gcc.output3.txt
	@echo "Diff for output # 4"
	-diff -iw ./$(OP_DIR)/output_4.txt ./$(REF_DIR)/gcc.output4.txt
	@echo "Diff for output # 5"
	-diff -iw ./$(OP_DIR)/output_5.txt ./$(REF_DIR)/gcc.output5.txt
	@echo "Diff for output # 6"
	-diff -iw ./$(OP_DIR)/output_6.txt ./$(REF_DIR)/gcc.output6.txt
	@echo "Diff for output # 7"
	-diff -iw ./$(OP_DIR)/output_7.txt ./$(REF_DIR)/gcc.output7.txt






