# Makefile for generating fmcad_coopt.pdf 

SRCS_PDF_TEX_T = ${patsubst figs/%.fig, figs/%.pdftex_t, ${wildcard figs/*.fig}}
SRCS_PDF_TEX = ${patsubst figs/%.fig, figs/%.pdftex_t, ${wildcard figs/*.fig}}

SRCS_PS_TEX_T = ${patsubst psfigs/%.fig, psfigs/%.pstex_t, ${wildcard psfigs/*.fig}}
SRCS_PS_TEX = ${patsubst psfigs/%.fig, psfigs/%.pstex, ${wildcard psfigs/*.fig}}

.PHONY:clean cleanall cleanaux 
.PRECIOUS: %.bbl %.ps

TODAY=$(shell date +%m-%d)
TODAY1=$(shell date +"%B %d, %Y")
STAMP=$(shell cat thesis.stamp)

all: rp.pdf

slides: slidesthesis.pdf

thesis.html: thesis.pdf header.html footer.html
	sh mkthesis.html.sh

copy: thesis.html
	scp thesis.pdf zaraket@sunfire1.ece.utexas.edu:~/public_html/thesis-$(TODAY).pdf
	scp thesis.html slidesthesis.pdf zaraket@sunfire1.ece.utexas.edu:~/public_html/

cvs: thesis.pdf
	@echo "checking in from Makefile --$(TODAY1)" > message.cvs
	utcvs ci -F message.cvs thesis.tex my.bib refstrings.sty slidesthesis.tex

echo:
	@echo $(TODAY)
	@echo $(SRCS_PDFTEX_T)
	@echo $(SRCS_PS_TEX)

_mmpfig:
	cd mmpfigs; make

figures: $(SRCS_PS_TEX_T) $(SRCS_PDF_TEX_T) _mmpfig

slides%.pdf: slides%.tex figures refstrings.sty slidesMacros.sty
	pdflatex slides$*.tex

%.bbl: my.bib
	latex $*.tex
	-bibtex $*

%.pdf: %.ps
#	pdflatex $*.tex
	ps2pdf $*.ps 

%.ps: %.tex figures %.bbl
	latex $*.tex
#	dvips -o $*.ps $*.dvi
	dvips -P cmz -t letter -o $*.ps $*.dvi

figs/%.pdftex_t: figs/%.fig
#	fig2dev -L pdftex -l dummy_arg $*.fig $*.pdf : -l dumy for landscape
	fig2dev -L pdftex -p dummy_arg figs/$*.fig figs/$*.pdf # -p dummy for portrait
	fig2dev -L pdftex_t -p figs/$*.pdf figs/$*.fig figs/$*.pdftex_t

psfigs/%.pstex_t: psfigs/%.fig
	fig2dev -L pstex psfigs/$*.fig psfigs/$*.pstex
	fig2dev -L pstex_t -p psfigs/$*.pstex psfigs/$*.fig psfigs/$*.pstex_t

cleanaux: 
	-rm -rf *.aux *.log *.bbl *.blg *.dvi *.nav *.out *.snm *.toc *.lof *.lot

cleanfigs:
	-rm -rf $(SRCS_PDF_TEX_T) $(SRCS_PDF_TEX) *.sw 
	-rm $(SRCS_PS_TEX_T) $(SRCS_PS_TEX)

clean: cleanaux
	-rm -rf *.ps *.pdf

cleanall: clean cleanfigs
