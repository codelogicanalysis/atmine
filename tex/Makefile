# Makefile for generating fmcad_coopt.pdf 

SRCS_PDF_TEX_T = ${patsubst figs/%.fig, figs/%.pdftex_t, ${wildcard figs/*.fig}}
SRCS_PDF_TEX = ${patsubst figs/%.fig, figs/%.pdftex_t, ${wildcard figs/*.fig}}

SRCS_PS_TEX_T = ${patsubst psfigs/%.fig, psfigs/%.pstex_t, ${wildcard psfigs/*.fig}}
SRCS_PS_TEX = ${patsubst psfigs/%.fig, psfigs/%.pstex, ${wildcard psfigs/*.fig}}

.PHONY:clean cleanall cleanaux 
.PRECIOUS: %.bbl %.ps

all: sarf.pdf

slides: slidescvhdl.pdf


figures: $(SRCS_PS_TEX_T) $(SRCS_PDF_TEX_T)

slides%.pdf: slides%.tex figures refstrings.sty slidesMacros.sty
	pdflatex slides$*.tex

%.bbl: fzAr.bib
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
