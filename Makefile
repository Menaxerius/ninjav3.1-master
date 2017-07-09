BINARIES=server
BINARY_ENV=ASAN_SYMBOLIZER_PATH=${HOME}/llvm-symbolizer LD_LIBRARY_PATH=output:build/lib:build/contrib/DORM/lib

all:
	(cd objects; ${MAKE})
	(cd templates; ${MAKE})
	(cd handlers; ${MAKE})
	(cd src; ${MAKE})

clean:
	(cd objects; ${MAKE} clean)
	(cd templates; ${MAKE} clean)
	(cd handlers; ${MAKE} clean)
	(cd src; ${MAKE} clean)

restart: shutdown startup

shutdown:
	@for BINARY in ${BINARIES}; do \
		while [ -e $${BINARY}.pid ] && ps -p`cat $${BINARY}.pid` 1>/dev/null 2>&1; do \
			echo Killing PID `cat $${BINARY}.pid`: $${BINARY}...; \
			kill `cat $${BINARY}.pid`; \
			sleep 5; \
		done; \
		rm -f $${BINARY}.pid 1>/dev/null 2>&1; \
	done
			  

startup:
	@for BINARY in ${BINARIES}; do \
		if [ -f $${BINARY}.log ]; then mv $${BINARY}.log $${BINARY}.log.old; fi; \
		if [ -e $${BINARY}.pid ] && ps -p`cat $${BINARY}.pid` 1>/dev/null 2>&1; then echo $$BINARY already running; continue; fi; \
		echo Starting $${BINARY}...; \
		${BINARY_ENV} output/$${BINARY} 1>$${BINARY}.log 2>&1 & \
		if [ "$$!" = "" ]; then \
			echo $${BINARY} failed to start; \
			continue; \
		fi; \
		echo $$! > $${BINARY}.pid; \
		echo $${BINARY} running with PID `cat $${BINARY}.pid`; \
	done
