# TSNetFw Makefile
CXX = $(TOOLCHAIN)g++
PROTOC = protoc
AR = $(TOOLCHAIN)ar
RANLIB = $(TOOLCHAIN)ranlib
NM = $(TOOLCHAIN)nm
MAKE = make
CFLAGS = -g -Wall
CXXFLAGS = 
LDFLAGS =
LIBS =
DEMOMAKE = 

ifeq ($(cygwin), 1)
    DEMOMAKE += cygwin=1
else
    CFLAGS += -fPIC
endif

BINDIR = ../bin
OBJDIR = ../obj
DEMODIR = ./demo/src

#LIB_ROOT = .
#INCLUDE = $(LIB_ROOT)/include
#CXXFLAGS += -I$(INCLUDE)

ifeq ($(static), 1)
    #LDFLAGS += -static
endif


LIGS = -ldl -lpthread

#LIBS += /usr/local/lib/libprotobuf.a
#LIBS += lib64/libcomm_oi.a

LDFLAGS += $(LIBS)


TSNETFW_STATIC = libtsnetfw.a
TSNETFW_SHARE = $(patsubst %.a,%.so,$(TSNETFW_STATIC))

BIN = $(BINDIR)/$(TSNETFW_STATIC) $(BINDIR)/$(TSNETFW_SHARE)

OBJ = $(patsubst %.proto,$(OBJDIR)/%.pb.o,$(wildcard *.proto)) $(patsubst %.cpp,$(OBJDIR)/%.o,$(wildcard *.cpp)) $(patsubst %.c,$(OBJDIR)/%.o,$(wildcard *.c))
PROTO = $(patsubst %.proto,%.pb.cc,$(wildcard *.proto)) $(patsubst %.proto,%.pb.h,$(wildcard *.proto))


PREFIX = /usr/local


.PHONY: all clean mkdir pack install uninstall
all: $(BIN)
	$(MAKE) all -C $(DEMODIR)
	
clean:
	-rm -rf $(PROTO) $(OBJ) $(BIN) $(BINDIR)/tsnetfw.tar.bz2
	-$(MAKE) clean -C $(DEMODIR)
	
mkdir:
	-mkdir -p $(BINDIR) $(OBJDIR)

pack: all
	-mkdir -p $(BINDIR)/tmp/include/tsnetfw $(BINDIR)/tmp/lib
	cp $(wildcard *.h) $(BINDIR)/tmp/include/tsnetfw
	cp $(wildcard *.inl) $(BINDIR)/tmp/include/tsnetfw
	cp $(BINDIR)/$(TSNETFW_STATIC) $(BINDIR)/tmp/lib
	cp $(BINDIR)/$(TSNETFW_SHARE) $(BINDIR)/tmp/lib
	cd $(BINDIR)/tmp && tar jcf tsnetfw.tar.bz2 include lib && mv tsnetfw.tar.bz2 ../
	rm -rf $(BINDIR)/tmp

install:
	-mkdir -p $(PREFIX)/include/tsnetfw
	-mkdir -p $(PREFIX)/lib
	-mkdir -p $(PREFIX)/bin
	-cp $(BINDIR)/$(TSNETFW_STATIC) $(PREFIX)/lib/
	-cp $(BINDIR)/$(TSNETFW_SHARE) $(PREFIX)/lib/
	-cp $(wildcard *.h) $(PREFIX)/include/tsnetfw/
	-cp $(wildcard *.inl) $(PREFIX)/include/tsnetfw/
    
uninstall:
	-rm -rf $(PREFIX)/include/tsnetfw
	-rm $(PREFIX)/lib/$(TSNETFW_STATIC)
	-rm $(PREFIX)/lib/$(TSNETFW_SHARE)

$(BINDIR)/$(TSNETFW_STATIC): $(OBJ)
	$(AR) q $@ $^ $(LIBS)
	$(RANLIB) $@
#	$(NM) $@

$(BINDIR)/$(TSNETFW_SHARE): $(OBJ)
	$(CXX) $^ -o $@ -fPIC -shared $(CFLAGS) $(LDFLAGS)

%.pb.h %.pb.cc: %.proto
	$(PROTOC) $< --cpp_out=.
	
$(wildcard *.cpp): $(PROTO)
	
$(OBJDIR)/%.o: %.cc %.h
	$(CXX) $< -c -o $@ $(CFLAGS) $(CXXFLAGS)

$(OBJDIR)/%.o: %.cpp %.inl %.h
	$(CXX) $< -c -o $@ $(CFLAGS) $(CXXFLAGS)
	
	
