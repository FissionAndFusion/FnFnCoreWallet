# Copyright (c) 2009-2010 Satoshi Nakamoto
# Copyright (c) 2011-2012 The LoMoCoin developers
# Distributed under the MIT/X11 software license, see the accompanying
# file license.txt or http://www.opensource.org/licenses/mit-license.php.

DEFS=-DBOOST_SPIRIT_THREADSAFE

DEFS += $(addprefix -I,$(CURDIR) $(CURDIR)/obj $(BOOST_INCLUDE_PATH) $(OPENSSL_INCLUDE_PATH))
LIBS = $(addprefix -L,$(BOOST_LIB_PATH) $(OPENSSL_LIB_PATH))

LMODE = dynamic
LMODE2 = dynamic
ifdef STATIC
	LMODE = static
	ifeq (${STATIC}, all)
		LMODE2 = static
	endif
endif

# for boost 1.37, add -mt to the boost libraries
LIBS += \
 -Wl,-B$(LMODE) \
   -l boost_system$(BOOST_LIB_SUFFIX) \
   -l boost_filesystem$(BOOST_LIB_SUFFIX) \
   -l boost_program_options$(BOOST_LIB_SUFFIX) \
   -l boost_thread$(BOOST_LIB_SUFFIX) \
   -l boost_date_time$(BOOST_LIB_SUFFIX) \
   -l boost_regex$(BOOST_LIB_SUFFIX) \
   -l ssl \
   -l crypto

LIBS+= \
 -Wl,-B$(LMODE2) \
   -l z \
   -l dl \
   -l readline \
   -l pthread 

# Hardening
# Make some classes of vulnerabilities unexploitable in case one is discovered.
#
    # This is a workaround for Ubuntu bug #691722, the default -fstack-protector causes
    # -fstack-protector-all to be ignored unless -fno-stack-protector is used first.
    # see: https://bugs.launchpad.net/ubuntu/+source/gcc-4.5/+bug/691722
    HARDENING=-fno-stack-protector

    # Stack Canaries
    # Put numbers at the beginning of each stack frame and check that they are the same.
    # If a stack buffer if overflowed, it writes over the canary number and then on return
    # when that number is checked, it won't be the same and the program will exit with
    # a "Stack smashing detected" error instead of being exploited.
    HARDENING+=-fstack-protector-all -Wstack-protector

    # Make some important things such as the global offset table read only as soon as
    # the dynamic linker is finished building it. This will prevent overwriting of addresses
    # which would later be jumped to.
    HARDENING+=-Wl,-z,relro -Wl,-z,now

    # Build position independent code to take advantage of Address Space Layout Randomization
    # offered by some kernels.
    # see doc/build-unix.txt for more information.
    ifdef PIE
        HARDENING+=-fPIE -pie
    endif

    # -D_FORTIFY_SOURCE=2 does some checking for potentially exploitable code patterns in
    # the source such overflowing a statically defined buffer.
    HARDENING+=-D_FORTIFY_SOURCE=2
#


DEBUGFLAGS=-g
CXXFLAGS=-O2
xCXXFLAGS=-pthread -Wall -Wextra -Wno-sign-compare -Wno-invalid-offsetof -Wno-unused-parameter -Wformat -Wformat-security \
    $(DEBUGFLAGS) $(DEFS) $(HARDENING) $(CXXFLAGS)

SOURCES= walleve/test.cpp \
	 walleve/util.cpp \
	 walleve/entry/entry.cpp \
	 walleve/event/event.cpp \
	 walleve/event/eventproc.cpp \
    	 walleve/stream/circular.cpp \
 	 walleve/stream/stream.cpp \
         walleve/base/base.cpp \
  	 walleve/docker/config.cpp \
   	 walleve/docker/docker.cpp \
   	 walleve/netio/nethost.cpp \
   	 walleve/netio/ioclient.cpp \
   	 walleve/netio/iocontainer.cpp \
   	 walleve/netio/ioproc.cpp \
   	 walleve/console/console.cpp \
   	 walleve/peernet/nodemngr.cpp \
   	 walleve/peernet/epmngr.cpp \
   	 walleve/peernet/peer.cpp \
   	 walleve/peernet/peernet.cpp \
   	 walleve/http/httpcookie.cpp \
   	 walleve/http/httpsse.cpp \
	 walleve/http/httputil.cpp \
  	 walleve/http/httpserver.cpp \
  	 walleve/http/httpget.cpp

#OBJECTS=$(SOURCES:.cpp=.o)
#DEPENDS=$(addprefix deps/, $(notdir $(SOURCES:.cpp=.P)))
OBJECTS=$(addprefix objs/, $(subst /,.,$(SOURCES:%.cpp=%.o)))

LIBRARY=libwalleve.a
TARGET=test

#Walleve
WL_CPPFLAGS += -Iwalleve/include -Iwalleve

all: $(TARGET) $(LIBRARY)

# auto-generated dependencies:
-include objs/*.P

.SECONDEXPANSION:
$(OBJECTS):objs/%.o: $$(subst .,/,%).cpp
	$(CXX) -c $(xCXXFLAGS) -MMD -o $@ $<
	@cp $(@:%.o=%.d) $(@:%.o=%.P); \
	  sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
	      -e '/^$$/ d' -e 's/$$/ :/' < $(@:%.o=%.d) >> $(@:%.o=%.P); \
	  rm -f $(@:%.o=%.d)

$(TARGET): $(OBJECTS)  
	$(CXX) $(xCXXFLAGS) -rdynamic -o $@ $^ $(LDFLAGS) $(LIBS) 

$(LIBRARY): $(OBJECTS)
	rm -f $@
	$(AR) -rs $@ $^ 
clean:
	-rm -f $(TARGET)
	-rm -f $(LIBRARY)
	-rm -f $(OBJECTS)
	-rm -f objs/*.P

FORCE:
