TOP=$(PWD)
LIB_DIR=${TOP}/lib

DIRECTORIES=$(filter-out %cscope.out %makefile %.h, ${wildcard src/*})
APPS_DIRECTORIES=$(filter-out %cscope.out %makefile %.h, ${wildcard apps/*})

MKDIR_P = mkdir -p

.PHONY: directories

#DIRECTORIES+=bin

ifeq ($(findstring opt,$(MAKECMDGOALS)),opt)
    MAKEARGS+=opt
endif

ifeq ($(findstring 64,$(MAKECMDGOALS)),64)
    MAKEARGS+=64
endif

all: directories src apps
	@echo "building binaries..."
	@echo $(MAKECMDGOALS)
	cd bin; $(MAKE) $(MAKECMDGOALS); cd $(TOP);

directories: ${LIB_DIR}

${LIB_DIR}:
	${MKDIR_P} ${LIB_DIR}

%stamp:
	@touch $@

src: srcstamp
	@echo "building src directories..." 
	@echo $(DIRECTORIES)
	@for i in $(DIRECTORIES); do \
		cd $$i; $(MAKE) $(MAKECMDGOALS); cd $(TOP); \
	done
	@rm -rf srcstamp


apps: appsstamp
	@echo "building apps directories..." 
	@echo $(APPS_DIRECTORIES)
	@for i in $(APPS_DIRECTORIES); do \
		cd $$i; $(MAKE) $(MAKECMDGOALS); cd $(TOP); \
	done
	@rm -rf appsstamp

bin: binstamp
	@echo "building binaries..."
	@echo $(MAKECMDGOALS)
	cd bin; $(MAKE) $(MAKECMDGOALS); cd $(TOP);
	@rm -rf binstamp
	
docs:
	mkdir -p doc/html
	#cp doc/icon.jpg doc/html/
	doxygen

clean:
#	@echo $(DIRECTORIES)
	@for i in $(DIRECTORIES); do \
		cd $$i; $(MAKE) clean; cd $(TOP); \
	done
	@for i in $(APPS_DIRECTORIES); do \
		cd $$i; $(MAKE) clean; cd $(TOP); \
	done
	cd bin; $(MAKE) clean; cd $(TOP);

SRC_CPPSOURCES=${wildcard src/*/*.cpp}
SRC_HSOURCES=${wildcard src/*/*.h}

APPS_CPPSOURCES=${wildcard apps/*/*.cpp}
APPS_HSOURCES=${wildcard apps/*/*.h}

cscope.files: $(SRC_CPPSOURCES) $(SRC_HSOURCES) $(APPS_CPPSOURCES) $(APPS_HSOURCES)
	@find src -name "*.cpp" > cscope.files
	@find src -name "*.h" >> cscope.files
	@find apps -name "*.cpp" >> cscope.files
	@find apps -name "*.h" >> cscope.files

cscope: cscope.files 
	@cscope -C  -b


opt 32 64 cyg mtest ex0 ex1 ex2 ATSarf tagger numnorm: all


