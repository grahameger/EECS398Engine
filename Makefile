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
CPPFLAGS := -g3 -Wall -std=c++17
LIB := -pthread -lssl -lcrypto
INC := -I include

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/Test%.o: $(TESTDIR)/Test%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " Making test "
	@echo " $(CC) $(CPPFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CPPFLAGS) $(INC) -c -o $@ $<

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(BUILDDIR)
	@echo " $(CC) $(CPPFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CPPFLAGS) $(INC) -c -o $@ $<

clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET); $(RM) -r wd; mkdir wd;

# Tests
tester:
	$(CC) $(CPPFLAGS) test/tester.cpp $(INC) $(LIB) -o bin/tester

robots:
	$(CC) $(CFLAGS) test/RobotsTxtTest.cpp $(INC) $(LIB) -o bin/robots-test

UTFOBJS := TestUtf8Numbers.o Utf8Numbers.o ByteStream.o String.o
utfnum: $(patsubst %,$(BUILDDIR)/%,$(UTFOBJS))
	@echo " $(CC) $(CPPFLAGS) $^ $(INC) $(LIB) -o bin/utfnum"
	@$(CC) $(CPPFLAGS) $^ $(INC) $(LIB) -o bin/utfnum

# Spikes
ticket:
	$(CC) $(CPPFLAGS) spikes/ticket.cpp $(INC) $(LIB) -o bin/ticket


.PHONY: clean
