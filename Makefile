CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

INCLUDEDIR = include
SRCDIR = src
BUILDDIR = build

# SOURCES := $(wildcard $(SRCDIR)/*.cpp)
SOURCES := $(shell find src -name '*.cpp')
OBJECTS := $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(SOURCES))
TARGET = balancer

all: $(BUILDDIR)/$(TARGET)

$(BUILDDIR)/$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I $(INCLUDEDIR) -c -o $@ $^

clean:
	rm $(OBJECTS) $(BUILDDIR)/$(TARGET)

.PHONY: all clean
