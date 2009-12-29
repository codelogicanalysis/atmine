These files are taken from Buckwaler Morphological analyzer.

I modified aramorph.pl so that if it is given an empty input file it will return the subset of the dictionary that we need.

it works as follows:

perl aramorph.pl < input_empty > output_file

the output_file can be given any name as needed to generate ADJ, ADV, Numb, Nprop,...

To make the code working, edit the comments on line 354-340 in a similar matter....