TARGET   := libvsos_e820.a
INCDIR   := include
SRCDIR   := src
OBJDIR   := build

CC       := i686-elf-gcc
CXX      := i686-elf-g++
AR       := i686-elf-ar
CXXFLAGS := -ffreestanding -fno-exceptions -fno-rtti -Wall -Wextra -std=c++17 -I$(INCDIR)
ARFLAGS  := rcs

SOURCES  := $(SRCDIR)/e820.cpp
OBJECTS  := $(OBJDIR)/e820.o

all: $(TARGET)

$(OBJDIR):
    mkdir -p $(OBJDIR)

$(OBJDIR)/e820.o: $(SRCDIR)/e820.cpp | $(OBJDIR)
    $(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
    $(AR) $(ARFLAGS) $@ $^

clean:
    rm -rf $(OBJDIR) $(TARGET)

.PHONY: all clean
