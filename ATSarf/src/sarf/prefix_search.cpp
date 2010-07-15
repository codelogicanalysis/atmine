#include "logger.h"
#include "prefix_search.h"
#include "stemmer.h"

PrefixSearch::PrefixSearch(QString * text,int start):TreeSearch(PREFIX,text,start) { }
PrefixSearch::~PrefixSearch(){ }
