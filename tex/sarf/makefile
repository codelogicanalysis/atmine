# Small makefile for tex files
#
# input files are .tex and .bib files in the current directory
#  and .fig files in the figs directory
# the make command will first build the latex figures in pdftex/pdftex_t format in the out directory
#  then it will build the .pdf main file for bibtex to use the .aux file to build the .bbl file
#  then the .pdf file is built in the out directory

.PHONY:clean 

.PRECIOUS: out/%.bbl out/%.aux

all: out/aclsarf.pdf


echo:
	@echo "SRCS_PDF_TEX:" $(SRCS_PDF_TEX)

figures: 
	@cd figs;make 

out/%.aux: %.tex refs/aclsarf.bib
	@mkdir -p out
	pdflatex -output-directory=out $*.tex

out/%.bbl: out/%.aux
	@mkdir -p out
	bibtex out/$*.aux

out/%.pdf: out/%.bbl figures
	@mkdir -p out
	pdflatex -output-directory=out $*.tex
	pdflatex -output-directory=out $*.tex


clean: 
	@cd out;rm -rf *
