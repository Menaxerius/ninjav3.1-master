include ../build/gmake/src.gmk

OBJS+=${addprefix ${OUTPUT_DIR}/src/, shabal.o json_error.o AccountCache.o BlockCache.o SubmissionCache.o}
OBJS+=${addprefix ${OUTPUT_DIR}/src/, account_updater.o blockchain_monitor.o blockchain_refresh.o block_forger.o bonus_processor.o database_trimmer.o reward_payer.o}

${OUTPUT_DIR}/server: ${OBJS}

${OUTPUT_DIR}/nonce_stress: ${OUTPUT_DIR}/src/nonce_stress.o 
	${CXX} ${CXXFLAGS} ${LDFLAGS} ${addprefix -L,${LIBDIRS}} -o $@ $^ -lDomUtil -lcrypto -lcurl -lthr

${OUTPUT_DIR}/src/shabal.o: shabal.h shabal-stripped.s
	c++ -g -c -o $@ shabal-stripped.s

shabal-stripped.s: shabal.s
	grep -v '^\s*\#' shabal.s > shabal-stripped.s
