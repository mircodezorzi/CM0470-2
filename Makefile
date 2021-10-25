BINDIR := bin
DUMPDIR := dump
OUTDIR := out

CC := g++
CXXFLAGS := -std=c++20 -O3 -DNDEBUG -lfmt -Wall -Wextra -Wpedantic

all: pipeline pipeline_unicode

pipeline: pipeline.cpp Makefile
	mkdir -p $(BINDIR) $(DUMPDIR) $(OUTDIR)
	@printf "\033[1mCompiling Assignment 1\033[0m\n"
	$(CC) -o $(BINDIR)/pipeline pipeline.cpp $(CXXFLAGS)
	@printf "\033[1mDumping Disassembly\033[0m\n"
	objdump --source -C -F -f -M intel $(BINDIR)/pipeline > $(DUMPDIR)/pipeline.asm
	@printf "\033[1mRunning Code\033[0m\n"
	$(BINDIR)/pipeline > $(OUTDIR)/pipeline.txt

pipeline_unicode: pipeline_unicode.cpp Makefile
	mkdir -p $(BINDIR) $(DUMPDIR) $(OUTDIR)
	@printf "\033[1mCompiling Assignment 1 (Spinoff)\033[0m\n"
	$(CC) -o $(BINDIR)/pipeline_unicode pipeline_unicode.cpp $(CXXFLAGS)
	@printf "\033[1mDumping Disassembly\033[0m\n"
	objdump --source -C -F -f -M intel $(BINDIR)/pipeline_unicode > $(DUMPDIR)/pipeline_unicode.asm
	@printf "\033[1mRunning Code\033[0m\n"
	$(BINDIR)/pipeline_unicode > $(OUTDIR)/pipeline_unicode.txt
