# Makefile for generating fmcad_coopt.pdf 

SRCS_PDF_TEX_T = ${patsubst figures/%.fig, figures/%.pdftex_t, ${wildcard figures/*.fig}}
SRCS_PDF_TEX = ${patsubst figures/%.fig, figures/%.pdftex_t, ${wildcard figures/*.fig}}

.PHONY:clean cleanall cleanaux 
.PRECIOUS: %.bbl %.aux %.ps

TODAY=$(shell date +%m-%d)
TODAY1=$(shell date +"%B %d, %Y")
STAMP=$(shell cat thesis.stamp)

all: merf.pdf

slides: slidesMerf.pdf

echo:
	@echo $(TODAY)
	@echo $(SRCS_PDFTEX_T)
	@echo $(SRCS_PS_TEX)

_mmpfig:
	#cd mmpfigs; make

figures: $(SRCS_PS_TEX_T) $(SRCS_PDF_TEX_T) _mmpfig

slides%.pdf: slides%.tex figures refstrings.sty slidesMacros.sty
	pdflatex slides$*.tex

%.aux: %.tex
	pdflatex $*.tex

%.bbl: %.bib %.aux
	-bibtex $*

%.ps.pdf: %.ps %.bbl
	ps2pdf $*.ps $*.ps.pdf

%.pdf: %.tex figures %.bbl 
	pdflatex $*.tex

%.ps: %.tex figures %.bbl
	latex $*.tex
#	dvips -o $*.ps $*.dvi
	dvips -P cmz -t letter -o $*.ps $*.dvi

figures/%.pdftex_t: figures/%.fig
#	fig2dev -L pdftex -l dummy_arg $*.fig $*.pdf : -l dumy for landscape
	fig2dev -L pdftex -p dummy_arg figures/$*.fig figures/$*.pdf # -p dummy for portrait
	fig2dev -L pdftex_t -p figures/$*.pdf figures/$*.fig figures/$*.pdftex_t

cleanaux: 
	-rm -rf *.aux *.log *.bbl *.blg *.dvi *.nav *.out *.snm *.toc *.lof *.lot

cleanfigs:
	-rm -rf $(SRCS_PDF_TEX_T) $(SRCS_PDF_TEX) *.sw 
	-rm $(SRCS_PS_TEX_T) $(SRCS_PS_TEX)

clean: cleanaux
	-rm -rf *.ps *.pdf

cleanall: clean cleanfigs
