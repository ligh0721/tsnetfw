# TSNetFw Demo Makefile

CXX = g++
PROTOC = protoc
AR = ar
RANLIB = ranlib
NM = nm
CFLAGS = -g -Wall
CXXFLAGS = 
LDFLAGS =
LIBS =

BINDIR = ../../../bin
OBJDIR = ../../../obj

#LIBS = -L../../../bin

#LIB_ROOT = .
INCLUDE = -I../..
CXXFLAGS += $(INCLUDE)

ifeq ($(static), 1)
    #LDFLAGS += -static
endif

LIBS += -L/usr/local/lib
ifneq ($(cygwin), 1)
    LIBS += ../../../bin/libtsnetfw.a
    #LIBS += /usr/local/mysql/lib/mysql/libmysqlclient.a
    LIBS += -lmysqlclient
    #LIBS += /usr/local/lib/libreadline.a -lncurses
    LIBS += -lreadline -lncurses
    LIBS += /usr/local/lib/liblua.a
    #LIBS += /usr/lib/libprotobuf.a
    #LIBS += -lmysqlclient
    #LIBS += -llua
    #LIBS += -lreadline -lncurses
    ifeq ($(static), 1)
        LIBS += /usr/lib/libmysqlclient.a /usr/lib/libreadline.a /usr/local/lib/liblua.a
    else
	#LIBS += -lmysqlclient -lreadline -lncurses
        LIBS += -ldl
    endif
endif

LIBS += -lz -lpthread
LDFLAGS += $(LIBS)


TSNETFW_TEST = $(BINDIR)/tsnetfw_test
BIN = $(TSNETFW_TEST)

ifneq ($(cygwin), 1)
DEMO_PROXY = $(BINDIR)/DemoProxyService
DEMO_RECORD = $(BINDIR)/DemoRecordService
DEMO_REQUESTTEST = $(BINDIR)/DemoRequestTest
DEMO_MULTIPROCESS = $(BINDIR)/DemoMultiProcessStarter
DEMO_LABORATORY = $(BINDIR)/DemoLaboratory
DEMO_HASH = $(BINDIR)/DemoHashService
DEMO_TIMER = $(BINDIR)/DemoTimerService
DEMO_MONDRV = $(BINDIR)/DemoMonitorDriver
DEMO_UDPDSP = $(BINDIR)/DemoUdpDispenser
BIN += $(DEMO_PROXY)
BIN += $(DEMO_RECORD)
BIN += $(DEMO_REQUESTTEST)
BIN += $(DEMO_MULTIPROCESS)
BIN += $(DEMO_LABORATORY)
BIN += $(DEMO_HASH)
BIN += $(DEMO_TIMER)
BIN += $(DEMO_MONDRV)
BIN += $(DEMO_UDPDSP)
endif





OBJ = $(patsubst %.proto, $(OBJDIR)/%.pb.o, $(wildcard *.proto)) $(patsubst %.cpp, $(OBJDIR)/%.o, $(wildcard *.cpp)) $(patsubst %.c, $(OBJDIR)/%.o, $(wildcard *.c))
PROTO = $(patsubst %.proto, %.pb.cc, $(wildcard *.proto)) $(patsubst %.proto, %.pb.h, $(wildcard *.proto))

.PHONY: all clean mkdir install
all: $(BIN)

clean:
	-rm -rf $(PROTO) $(OBJ) $(BIN)
	
mkdir:
	-mkdir -p $(BINDIR)


$(TSNETFW_TEST): $(OBJDIR)/tsnetfw_test.o 
	$(CXX) $^ -o $@ $(CFLAGS) $(LDFLAGS)
	
$(DEMO_PROXY): $(OBJDIR)/DemoProxyService.o $(BINDIR)/libtsnetfw.a
	$(CXX) $^ -o $@ $(CFLAGS) $(LDFLAGS)
	
$(DEMO_RECORD): $(OBJDIR)/DemoRecordService.o $(BINDIR)/libtsnetfw.a
	$(CXX) $^ -o $@ $(CFLAGS) $(LDFLAGS)
	
$(DEMO_REQUESTTEST): $(OBJDIR)/DemoRequestTest.o $(BINDIR)/libtsnetfw.a
	$(CXX) $^ -o $@ $(CFLAGS) $(LDFLAGS)
	
$(DEMO_MULTIPROCESS): $(OBJDIR)/DemoMultiProcessStarter.o $(BINDIR)/libtsnetfw.a
	$(CXX) $^ -o $@ $(CFLAGS) $(LDFLAGS)

$(DEMO_LABORATORY): $(OBJDIR)/DemoLaboratory.o $(BINDIR)/libtsnetfw.a
	$(CXX) $^ -o $@ $(CFLAGS) $(LDFLAGS)

$(DEMO_HASH): $(OBJDIR)/DemoHashService.o $(BINDIR)/libtsnetfw.a
	$(CXX) $^ -o $@ $(CFLAGS) $(LDFLAGS)
	
$(DEMO_TIMER): $(OBJDIR)/DemoTimerService.o $(BINDIR)/libtsnetfw.a
	$(CXX) $^ -o $@ $(CFLAGS) $(LDFLAGS) -loi_lite
	
$(DEMO_MONDRV): $(OBJDIR)/DemoMonitorDriver.o $(BINDIR)/libtsnetfw.a
	$(CXX) $^ -o $@ $(CFLAGS) $(LDFLAGS) -loi_lite
	
$(DEMO_UDPDSP): $(OBJDIR)/DemoUdpDispenser.o $(BINDIR)/libtsnetfw.a
	$(CXX) $^ -o $@ $(CFLAGS) $(LDFLAGS)


%.pb.h %.pb.cc: %.proto
	$(PROTOC) $< --cpp_out=.
	
$(wildcard *.cpp): $(PROTO)
	
$(OBJDIR)/%.o: %.cc %.h
	$(CXX) $< -c -o $@ $(CFLAGS) $(CXXFLAGS)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $< -c -o $@ $(CFLAGS) $(CXXFLAGS)
	
	


PREFIX = /usr

install:
	
