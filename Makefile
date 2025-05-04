CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

INCLUDEDIR = include
SRCDIR = src
BUILDDIR = build

# # SOURCES := $(wildcard $(SRCDIR)/*.cpp)
# SOURCES := $(shell find src -name '*.cpp')
# OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(SOURCES))
# TARGET = balancer


# all: $(BUILDDIR)/$(TARGET)

# $(BUILDDIR)/$(TARGET): $(OBJECTS)
# 	$(CXX) -o $@ $^

# $(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
# 	$(CXX) $(CXXFLAGS) -I $(INCLUDEDIR) -c -o $@ $^

# clean:
# 	rm $(BUILDDIR)/*
# 	rm $(OBJECTS) $(BUILDDIR)/$(TARGET)

# .PHONY: all clean

TESTDIR = test

MAINSOURCE = $(SRCDIR)/balancer.cpp
MAINOBJECT = $(MAINSOURCE:$(SRCDIR)/%.cpp=$(BUILDDIR)/%.o)
SOURCES = $(filter-out $(MAINSOURCE), $(shell find src -name '*.cpp'))
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(SOURCES))
TARGET = balancer

TESTSOURCES = $(shell find test -name '*.cpp')
TESTTARGETS = $(patsubst $(TESTDIR)/%.cpp, $(BUILDDIR)/%, $(TESTSOURCES))


all: $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET): $(MAINOBJECT) $(OBJECTS)
	$(CXX) -o $@ $^

test: $(TESTTARGETS)

$(BUILDDIR)/%: $(TESTDIR)/%.cpp $(OBJECTS)
	$(CXX) $(CXXFLAGS) -I $(INCLUDEDIR) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I $(INCLUDEDIR) -c -o $@ $^

$(MAINOBJECT): $(MAINSOURCE)
	$(CXX) $(CXXFLAGS) -I $(INCLUDEDIR) -c -o $@ $^

clean:
	rm $(BUILDDIR)/*

.PHONY: all clean
