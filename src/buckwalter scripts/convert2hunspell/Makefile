all: normalisation generation

generation:
	gawk -f convert2hunspell.awk -v name=arabic tableab.0 tablebc.0 tableac.0 prefixes.0 suffixes.0 stems.0
	./makealias arabic.dic arabic.aff

normalisation: prefixes.0 suffixes.0 stems.0 tableab.0

prefixes.0: ../data/dictprefixes
	sed 's/^		/0	0	/' ../data/dictprefixes | tr -d '\r' >prefixes.0


suffixes.0: ../data/dictsuffixes
	sed 's/^		/0	0	/;s/^	/0	/' ../data/dictsuffixes | tr -d '\r' >suffixes.0

stems.0: ../data/dictstems
	cat ../data/dictstems | tr -d '\r' >stems.0

tableab.0:
	tr -d '\r' <../data/tableab >tableab.0
	tr -d '\r' <../data/tablebc >tablebc.0
	tr -d '\r' <../data/tableac >tableac.0

clean:
	rm -f *.0 arabic*.dic arabic*.aff
