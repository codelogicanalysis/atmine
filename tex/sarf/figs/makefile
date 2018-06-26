# Small makefile for tex files

SRCS_PDF_TEX = ${patsubst %.fig, ../out/%.pdftex_t, ${wildcard *.fig}}
SRCS_PDF = ${patsubst %.fig, ../out/%.pdf, ${wildcard *.fig}}

all: $(SRCS_PDF_TEX)

echo:
	@echo $(SRCS_PDF_TEX)

../out/%.pdftex_t ../out/%.pdf: %.fig
	fig2dev -L pdftex -p dummy_arg $*.fig ../out/$*.pdf
	fig2dev -L pdftex_t -p out/$*.pdf $*.fig ../out/$*.pdftex_t

clean:
	rm -f $(SRCS_PDF_TEX)
	rm -f $(SRCS_PDF)

