BINDIR := bin
DUMPDIR := dump
OUTDIR := out

CC := g++
CXXFLAGS := -std=c++20 -O3 -DNDEBUG -lfmt

all: pipeline

pipeline: pipeline.cpp Makefile
	mkdir -p $(BINDIR) $(DUMPDIR) $(OUTDIR)
	@printf "\033[1mCompiling Assignment 1\033[0m\n"
	$(CC) -o $(BINDIR)/pipeline pipeline.cpp $(CXXFLAGS)
	@printf "\033[1mDumping Disassembly\033[0m\n"
	objdump --source -C -F -f -M intel $(BINDIR)/pipeline > $(DUMPDIR)/pipeline.asm
	@printf "\033[1mRunning Code\033[0m\n"
	$(BINDIR)/pipeline > $(OUTDIR)/pipeline.txt
