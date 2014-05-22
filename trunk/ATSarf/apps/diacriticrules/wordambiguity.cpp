#include "wordambiguity.h"

WordAmbiguity::WordAmbiguity(QString *text, int *count) : Stemmer(text,0)
{
    this->text = text;
    this->count = count;
};

bool WordAmbiguity::on_match()
{
    *count = *count + 1;
    return true;
};

int WordAmbiguity::getAmbiguity() {
    return (*count);
}
