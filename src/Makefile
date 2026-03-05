#Architecture
ARCH        := $(shell uname -m)

#Compiler
CC          := g++ -std=c++17 -Wno-psabi

# Directories
ENVIRODIR   := /usr/local/src/enviro/server/include
CHIPDIR     := /usr/local/src/Chipmunk2D

#Flags and Includes
CFLAGS      := -ggdb -shared -fPIC
INCLUDE     := -I $(ENVIRODIR) -I $(CHIPDIR)/include/chipmunk

#Target
TARGETDIR   := ../lib
SOURCES     := $(wildcard *.cc)
HEADERS     := $(wildcard *.h)
TARGETS     := $(patsubst %.cc,%.so,$(wildcard *.cc))
FULL_TARGETS := $(addprefix $(TARGETDIR)/, $(TARGETS))

#Default Make
all: $(FULL_TARGETS)

#Clean
clean:
	@$(RM) -rf $(TARGETDIR)/*.so

#Compile
$(TARGETDIR)/%.so: %.cc %.h
	$(CC) $(CFLAGS) $(INCLUDE) $< -o $@
