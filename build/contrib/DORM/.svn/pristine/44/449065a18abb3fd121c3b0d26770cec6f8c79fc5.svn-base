SRCS=${wildcard src/[[:upper:]]*.cpp}
OBJS=${patsubst src/%.cpp, obj/%.o, ${SRCS}}
TESTSRCS=${wildcard tests/*.cpp}
TESTBINS=${patsubst tests/%.cpp, tests/bin/%, ${TESTSRCS}}

TESTOBJECTS=${wildcard tests/objects/*.hpp tests/objects/*/*.hpp}
TESTGENS=${patsubst tests/objects/%.hpp, tests/obj/%_.cxx, ${TESTOBJECTS}}
TESTOBJS=${patsubst tests/obj/%.cxx, tests/obj/%.o, ${TESTGENS}}

ifdef COVERAGE
TESTCOVERAGE=test-coverage
LLVM_PROFDATA?=llvm-profdata
LLVM_COV?=llvm-cov
LLVM_MERGE_PROFDATA=if [ -f $${PROFBASE}.profraw ]; then ${LLVM_PROFDATA} merge $${PROFBASE}.prof* --output $${PROFBASE}.proftemp; mv $${PROFBASE}.proftemp $${PROFBASE}.profdata; fi

TESTFAILOBJECTS=${wildcard tests/fail-objects/*.hpp tests/fail-objects/*/*.hpp}
TESTFAILGENS=${patsubst tests/fail-objects/%.hpp, tests/fail-obj/%_.cxx, ${TESTFAILOBJECTS}}
TESTFAILOBJS=${patsubst tests/fail-obj/%.cxx, tests/fail-obj/%.o, ${TESTFAILGENS}}

CXXFLAGS+=-DTMP_DIR=\"test-tmp\"
endif

LIBDIRS=/usr/local/lib /usr/local/lib/mysql
LIBS=mysqlcppconn mysqlclient thr stdc++

CXXFLAGS+=-pipe -g -Wall -std=c++14 -pthread -fPIC ${INCLUDES}

ifdef DEBUG
ifdef COVERAGE
CXXFLAGS+=-O0 -ferror-limit=3 -fno-omit-frame-pointer -DDORM_DB_DEBUG -fprofile-instr-generate -fcoverage-mapping
else
CXXFLAGS+=-O0 -ferror-limit=3 -fno-omit-frame-pointer -fsanitize=address -DDORM_DB_DEBUG
endif
else
CXXFLAGS+=-O3
endif

INCDIRS=include
CXXFLAGS+=${addprefix -I, ${INCDIRS}} -isystem /usr/local/include

all: lib/libDORM.so bin/generate_object bin/emit_object_SQL

.PHONY: clean
clean:
	-rm -fr bin obj lib tests/obj tests/bin
	
.PHONY: clean-tests
clean-tests:
	-rm -fr tests/obj tests/bin *.profraw *.profdata test-tmp tests/fail-obj

obj/%.d: src/%.cpp include/%.hpp
	@echo 'Generating makefile $@ from $<'
	@mkdir -p `dirname $@`
	@echo -n 'obj/' > $@
	@${CXX} ${CXXFLAGS} -MM $< >> $@ || rm $@

ifneq ($(MAKECMDGOALS),clean)
DEPENDS=${patsubst src/%.cpp, obj/%.d, ${SRCS}}
-include ${DEPENDS}
endif

obj/%.o: src/%.cpp include/%.hpp
	@echo 'Compiling $@ from $<'
	@${CXX} ${CXXFLAGS} -c -o $@ $<

bin/%: src/%.cpp
	@echo 'Building $@ from $<'
	@mkdir -p `dirname $@`
	@${CXX} ${CXXFLAGS} -o $@ $<

lib/libDORM.so: ${OBJS}
	@echo 'Building shared library $@'
	@mkdir -p `dirname $@`
	@${CXX} ${CXXFLAGS} -shared -Wl,-soname,libDORM.so -o $@ $^


ifdef DEBUG
bin/generate_object: test-tmp/.ignore

test-tmp/.ignore:
	@mkdir -p test-tmp
	@touch test-tmp/.ignore
endif

.PHONY: tests
tests: ${OBJS} ${TESTOBJS} ${TESTBINS} ${TESTCOVERAGE}

TEST_DB=LLVM_PROFILE_FILE=emit_object_SQL.profraw bin/emit_object_SQL ${TESTOBJECTS} 2>/dev/null | mysql --database=test --user=test --password="" 2>/dev/null

test-all: bin/emit_object_SQL ${TESTBINS}
	@rm -f tests.profraw tests.profdata
	@PROFBASE="tests"; \
	for testbin in tests/bin/*; do \
		echo "Performing test: $${testbin}"; \
		${TEST_DB}; \
		LLVM_PROFILE_FILE="$${PROFBASE}.profraw" $${testbin} || exit 2; \
		${LLVM_MERGE_PROFDATA}; \
	done
	
test-all-coverage:
	@${LLVM_COV} report --instr-profile tests.profdata lib/libDORM.so

tests/bin/%: tests/%.cpp ${OBJS} ${TESTOBJS}
	@echo 'Building test $@ from $<'
	@mkdir -p `dirname $@`
	@${CXX} ${CXXFLAGS} -Itests -Itests/obj -Itests/objects ${LIBDIRS:%=-L%} -o $@ $< ${OBJS} ${TESTOBJS} ${LIBS:%=-l%}

tests/obj/%_.cxx: tests/objects/%.hpp bin/generate_object 
	@echo 'Generating object helper $@ from $<'
	@mkdir -p `dirname $@`
	@LLVM_PROFILE_FILE="generate_object.profraw" bin/generate_object -d tests/obj $< 1>tests/obj/$*.log 2>&1
	@PROFBASE="generate_object"; ${LLVM_MERGE_PROFDATA}

tests/obj/%.o: tests/obj/%.cxx
	@echo 'Compiling object helper $@ from $<'
	@mkdir -p `dirname $@`
	@${CXX} ${CXXFLAGS} -Itests -Itests/obj -Itests/objects -c -o $@ $<

tests/obj/Test_.o: tests/obj/Test/Frog_.cxx tests/obj/Test/Single_.cxx

.PHONY: test-coverage test-coverage-verbose test-coverage-no-such-file test-coverage-no-args test-coverage-cant-create-output
test-coverage: ${TESTOBJS} ${TESTFAILGENS} test-coverage-no-such-file test-coverage-no-args test-coverage-cant-create-output
	@${LLVM_COV} report bin/generate_object --instr-profile generate_object.profdata

test-coverage-verbose: test-coverage
	@${LLVM_COV} show bin/generate_object --instr-profile generate_object.profdata

test-coverage-no-such-file:
	@-LLVM_PROFILE_FILE="generate_object.profraw" bin/generate_object -d tests/fail-obj no-such-file 1>/dev/null 2>&1
	@PROFBASE="generate_object"; ${LLVM_MERGE_PROFDATA}

test-coverage-no-args:
	@-LLVM_PROFILE_FILE="generate_object.profraw" bin/generate_object 1>/dev/null 2>&1
	@PROFBASE="generate_object"; ${LLVM_MERGE_PROFDATA}

test-coverage-cant-create-output:
	@chmod 555 test-tmp 
	@-LLVM_PROFILE_FILE="generate_object.profraw" bin/generate_object -d /../no-such-directory tests/objects/Test.hpp 1>/dev/null 2>&1
	@chmod 755 test-tmp
	@PROFBASE="generate_object"; ${LLVM_MERGE_PROFDATA}

tests/fail-obj/%_.cxx: tests/fail-objects/%.hpp bin/generate_object 
	@echo 'Generating failing object helper $@ from $<'
	@mkdir -p `dirname $@`
	@-LLVM_PROFILE_FILE="generate_object.profraw" bin/generate_object -d tests/fail-obj $< 1>tests/obj/$*.log 2>&1
	@PROFBASE="generate_object"; ${LLVM_MERGE_PROFDATA}

tests/fail-obj/%.o: tests/fail-obj/%.cxx
	@echo 'Compiling failed object helper $@ from $<'
	@mkdir -p `dirname $@`
	@${CXX} ${CXXFLAGS} -Itests -Itests/fail-obj -Itests/fail-objects -c -o $@ $<
	