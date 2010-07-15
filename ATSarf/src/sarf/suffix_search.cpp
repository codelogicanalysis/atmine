#include "logger.h"
#include "common.h"
#include "suffix_search.h"
#include "stemmer.h"

SuffixSearch::SuffixSearch(QString * text,int start):TreeSearch(SUFFIX,text,start) { }
SuffixSearch::~SuffixSearch() { }

