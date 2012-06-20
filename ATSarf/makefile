TOP=$(PWD)

DIRECTORIES=$(filter-out %makefile %.h, ${wildcard src/*})
DIRECTORIES+=bin

ifeq ($(findstring opt,$(MAKECMDGOALS)),opt)
    MAKEARGS+=opt
endif

ifeq ($(findstring 64,$(MAKECMDGOALS)),64)
    MAKEARGS+=64
endif


all:
	@echo $(DIRECTORIES)
	@for i in $(DIRECTORIES); do \
		cd $$i; $(MAKE) $(MAKECMDGOALS); cd $(TOP); \
	done
	
docs:
	mkdir -p doc/html
	#cp doc/icon.jpg doc/html/
	doxygen

clean:
#	@echo $(DIRECTORIES)
	@for i in $(DIRECTORIES); do \
		cd $$i; $(MAKE) clean; cd $(TOP); \
	done

opt 32 64 cyg: all
