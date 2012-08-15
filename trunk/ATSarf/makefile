TOP=$(PWD)

DIRECTORIES=$(filter-out %makefile %.h, ${wildcard src/*})
APPS_DIRECTORIES=$(filter-out %makefile %.h, ${wildcard apps/*})

#DIRECTORIES+=bin

ifeq ($(findstring opt,$(MAKECMDGOALS)),opt)
    MAKEARGS+=opt
endif

ifeq ($(findstring 64,$(MAKECMDGOALS)),64)
    MAKEARGS+=64
endif


all:
	@echo "building src directories..." 
	@echo $(DIRECTORIES)
	@for i in $(DIRECTORIES); do \
		cd $$i; $(MAKE) $(MAKECMDGOALS); cd $(TOP); \
	done
	@echo "building apps directories..." 
	@echo $(APPS_DIRECTORIES)
	@for i in $(APPS_DIRECTORIES); do \
		cd $$i; $(MAKE) $(MAKECMDGOALS); cd $(TOP); \
	done
	@echo "building binaries..."
	@echo $(MAKECMDGOALS)
	cd bin; $(MAKE) $(MAKECMDGOALS); cd $(TOP);
	
docs:
	mkdir -p doc/html
	#cp doc/icon.jpg doc/html/
	doxygen

clean:
#	@echo $(DIRECTORIES)
	@for i in $(DIRECTORIES); do \
		cd $$i; $(MAKE) clean; cd $(TOP); \
	done

opt 32 64 cyg ex0 ex1 ex2 ATSarf: all
