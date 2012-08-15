#
# file:      regressionMakefile
# author:    F. Zaraket
# created:    feb 12, 2010
#-------------------------------------
# description: 
#    In this file we have macros that set necessary directives needed
#    to build code in a directory.
#-------------------------------------
# Kindly add below your modification stamps in the following format.
# Month Day,Year    Name    comment for modification
#
# Feb 12,2010    fadi   initial creation

APP_VERSION=0.1

BASE:=$(shell pwd | xargs basename)
#echobase:
#	@echo $(BASE)

REGRESS_DIRS+=mtest

MTESTFILES1=${wildcard mtest/*}
MTESTFILES2=${filter-out mtest/gold,$(MTESTFILES1)}
TST_MTEST_FILES=$(patsubst mtest/%,%, $(MTESTFILES2))
TST_MTEST_FILES_GOLD=$(patsubst %,%.gold, $(TST_MTEST_FILES))
TST_MTEST_FILES_GOLD_ERR=$(patsubst %,%.gold.err, $(TST_MTEST_FILES))

Rmtest: ../bin/mtest $(MTESTFILES2)
	@echo $(TST_MTEST_FILES)
	@for i in $(TST_MTEST_FILES); do \
	        echo "$$i  $$i.out $$i.err"; \
	        mkdir -p mtest/out; \
	        mkdir -p mtest/err; \
		../bin/mtest mtest/$$i > mtest/out/$$i.out 2> mtest/err/$$i.err; \
	        diff mtest/out/$$i.out mtest/gold/$$i.gold; \
	        if [ "$$?" -eq "0" ]; then \
	          echo "test $$i.out passed."; \
	        else  \
	          echo "test $$i.err failed."; \
                fi; \
	        diff mtest/err/$$i.err mtest/gold/$$i.gold.err; \
	        if [ "$$?" -eq "0" ]; then \
	          echo "test $$i.out passed."; \
	        else  \
	          echo "test $$i.err failed."; \
                fi; \
	        rm -rf mtest/out mtest/err; \
	done
