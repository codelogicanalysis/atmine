#include "numnorm.h"
#include "word.h"

NumNorm::NumNorm(QString *_text) {
    text = _text;
    previous = 0;
    current = 0;
    currentH = 0;
    isKey = false;
    isHundred = false;
    isNumberDone = false;

    // fill the hash table
    hashGlossInt.insert("one", 1);
    hashGlossInt.insert("two", 2);
    hashGlossInt.insert("three", 3);
    hashGlossInt.insert("four", 4);
    hashGlossInt.insert("five", 5);
    hashGlossInt.insert("six", 6);
    hashGlossInt.insert("seven", 7);
    hashGlossInt.insert("eight", 8);
    hashGlossInt.insert("nine", 9);
    hashGlossInt.insert("ten", 10);
    hashGlossInt.insert("twenty", 20);
    hashGlossInt.insert("thirty", 30);
    hashGlossInt.insert("fourty", 40);
    hashGlossInt.insert("fifty", 50);
    hashGlossInt.insert("sixty", 60);
    hashGlossInt.insert("seventy", 70);
    hashGlossInt.insert("eighty", 80);
    hashGlossInt.insert("ninety", 90);
    hashGlossInt.insert("hundred", 100);
    hashGlossInt.insert("thousand", 1000);
    hashGlossInt.insert("thousands", 1000);
    hashGlossInt.insert("million", 1000000);
    hashGlossInt.insert("millions", 1000000);
    hashGlossInt.insert("billion", 1000000000);
}

bool NumNorm::operator ()() {
    int start = 0;
    // fix case when last words are a number
    // won't be added to numbers
    while(start != text->count()) {
        Word word = nextWord(*text, start);
        if(word.word.isEmpty()) {
            break;
        }
        Number number(this, word.word);
        number();

        int sum = previous + current + currentH;
        if(isNumberDone && sum != 0) {
            numberFound();
        }
        start = word.end + 1;
    }

    int sum = previous + current + currentH;
    if(sum != 0) {
        numberFound();
    }
    return true;
}

void NumNorm::numberFound() {
    int number = previous + current + currentH;
    //cout << previous << ' ' << current << ' ' << currentH << '\n';
    isKey = false;
    isHundred = false;
    previous = 0;
    current = 0;
    currentH = 0;
    extractedNumbers.append(number);
    cout << number << '\n';
}
