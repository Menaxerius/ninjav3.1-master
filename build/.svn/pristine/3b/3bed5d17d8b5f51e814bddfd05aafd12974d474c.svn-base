SRCS=${wildcard src/*.cpp}
OBJS=${patsubst src/%.cpp, obj/%.o, ${SRCS}}

DORMDIR=contrib/DORM
SRCDORM=${DORMDIR}/Makefile
LIBDORM=${DORMDIR}/lib/libDORM.so

# all these need to go into WWW library 
WWW=Handler RegistryHandler ReqResp Request Response StaticHandler StaticTemplateHandler config_loader server
WWW_PATTERNS=${patsubst %, obj/%.o, ${WWW}}

WWW_OBJS=${filter ${WWW_PATTERNS}, ${OBJS}}
UTIL_OBJS=${filter-out ${WWW_PATTERNS}, ${OBJS}}

DEBUG_FLAGS+= -DFOLLOW_SYMLINKS

LIBDIRS=
LDFLAGS=${LIBDIRS} ${LDRPATHS}  

CXXFLAGS+=-g -std=c++14 -pipe -fPIC -Wall

ifdef DEBUG
CXXFLAGS+=-O0 -ferror-limit=2 -fno-omit-frame-pointer -fsanitize=address
else
CXXFLAGS+=-O3
endif

.PATH: src include obj lib
.SUFFIXES: .o .cpp .hpp .so .a

all: lib/libDomWWW.so lib/libDomUtil.so

clean:
	@rm -fr lib obj

${SRCDORM}:
	@svn co svn://svn.code.sf.net/p/dom-s-orm/code/trunk ${DORMDIR}

${LIBDORM}: ${SRCDORM}
	@(cd ${DORMDIR}; gmake)

lib/libDomWWW.so: ${WWW_OBJS}
	@mkdir -p `dirname $@`
	${CXX} ${CXXFLAGS} -shared -Wl,-soname,libDomWWW.so -o $@ $^


lib/libDomUtil.so: ${UTIL_OBJS}
	@mkdir -p `dirname $@`
	${CXX} ${CXXFLAGS} -shared -Wl,-soname,libDomUtil.so -o $@ $^


obj/%.o: src/%.cpp include/%.hpp
	@mkdir -p `dirname $@` 
	${CXX} ${CXXFLAGS} -c ${DEBUG_FLAGS} -o $@ -Iinclude -Icontrib/DORM/include -isystem /usr/local/include -isystem /usr/local/include/mysql src/$*.cpp

obj/server.o: ${LIBDORM}
obj/WebSocketHandler.o: ${LIBDORM}
