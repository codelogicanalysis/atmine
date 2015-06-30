# AUBSarf Installation #

## Prerequisites ##

  * Install the following packages using the "Synaptic Package Manager":
    * mysql-client
    * mysql-server
    * libmysqlcppconn
    * libqt4-sql-mysql
    * libmysqlcppconn-dev
    * subversion
    * qt-sdk
    * qtcreator
    * g++

The ATSarf Gui needs the following additional package for graph generation
  * graphviz

The "tagger" tools needs the following additional pacakge for json parsing
  * libqjson


## Details ##

  * Get our latest version of source code from the following page http://code.google.com/p/atmine/source/checkout
  * Download datrie from http://linux.thai.net/~thep/datrie/datrie.html#Download then open datrie/libdatrie-0.2.3 & "./configure" & "make"
  * Fix the links on the folder ATSarf/third/ to point to correctly:
> > - 'qt' to refer to the folder of your current version of QT
> > - 'datrie' to the folder of your datrie that you downloaded and installed
> > (use the following command to make symbolic links: 'ln -s')
  * Build the source code using QT for the file ATSarf/QT Make/ATSarfTrie.pro
  * Run the resulting Executable generated. **Enjoy!**