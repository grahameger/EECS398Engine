CC := g++ # This is the main compiler

LINTER := astyle
# CC := clang --analyze # and comment out the linker last line for sanity
SRCDIR := src
BUILDDIR := build
TESTDIR := test
TARGET := bin/engine

SRCEXT := cpp
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CPPFLAGS := -O2 -Wall -std=c++17 -march=native 
LIB := -pthread -lssl -lcrypto 
INC := -I include

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/Test%.o: $(TESTDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " Making test "
	@echo " $(CC) $(CPPFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CPPFLAGS) $(INC) -c -o $@ $<

$(BUILDDIR)/Debug%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " Making debug "
	@echo " $(CC) $(CPPFLAGS) $(INC) -c -D TEST -o $@ $<"; $(CC) $(CPPFLAGS) $(INC) -c -D TEST -o $@ $<

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CPPFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CPPFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) bin/*"; $(RM) -r $(BUILDDIR) bin/*; $(RM) -r wd; mkdir wd;

# Tests
tester:
	$(CC) $(CPPFLAGS) test/tester.cpp $(INC) $(LIB) -o bin/tester

robots:
	$(CC) $(CFLAGS) test/RobotsTxtTest.cpp $(INC) $(LIB) -o bin/robots-test

UTFOBJS := TestUtf8Numbers.o Utf8Numbers.o ByteStream.o String.o StringView.o
utfnum: $(patsubst %,$(BUILDDIR)/%,$(UTFOBJS))
	@echo " $(CC) $(CPPFLAGS) $^ $(INC) $(LIB) -o bin/utfnum"
	@$(CC) $(CPPFLAGS) $^ $(INC) $(LIB) -o bin/utfnum

POSTINGOBJS := TestPostingList.o Utf8Numbers.o ByteStream.o StringView.o String.o PostingList.o
posting: $(patsubst %,$(BUILDDIR)/%,$(POSTINGOBJS))
	@echo " $(CC) $(CPPFLAGS) $^ $(INC) $(LIB) -o bin/posting"
	@$(CC) $(CPPFLAGS) $^ $(INC) $(LIB) -o bin/posting

POSTINGSPLITOBJS := TestPostingListSplit.o Utf8Numbers.o ByteStream.o StringView.o String.o PostingList.o
split: $(patsubst %,$(BUILDDIR)/%,$(POSTINGSPLITOBJS))
	@echo " $(CC) $(CPPFLAGS) $^ $(INC) $(LIB) -o bin/split"
	@$(CC) $(CPPFLAGS) $^ $(INC) $(LIB) -o bin/split

INDEXOBJS := TestIndex.o DebugPostings.o StringView.o String.o PersistentBitVector.o mmap.o threading.o PostingList.o ByteStream.o Utf8Numbers.o ISR.o
index: $(patsubst %,$(BUILDDIR)/%,$(INDEXOBJS))
	@echo "rm -f testIndex*"
	@rm -f testIndex*
	@echo " $(CC) $(CPPFLAGS) $^ $(INC) $(LIB) -o bin/index"
	@$(CC) $(CPPFLAGS) $^ $(INC) $(LIB) -o bin/index

# Spikes
ticket:
	$(CC) $(CPPFLAGS) spikes/ticket.cpp $(INC) $(LIB) -o bin/ticket

.PHONY: clean
