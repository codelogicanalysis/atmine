#include "numnorm.h"

NumNorm::NumNorm(QString *_text) {
    text = _text;
    previous = 0;
    current = 0;
    currentH = 0;
    isKey = false;
    isHundred = false;
    isNumberDone = false;
    numtype = None;

    numberStart = -1;
    numberEnd = -1;

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

    // fill the flexibility table
    hashFlexible.insert("kilogram", 1000);
    hashFlexible.insert("kilometer(s)", 1000);
    hashFlexible.insert("millimeter", 0.001);
    hashFlexible.insert("milligram", 0.001);
    hashFlexible.insert("meter", 1);
    hashFlexible.insert("gram", 1);
}

bool NumNorm::operator ()() {
    int start = 0;
    while(start != text->count()) {
        Word word = nextWord(*text, start);
        if(word.word.isEmpty()) {
            break;
        }
        int val = -1;
        Number number(this, &word, &val);
        number();

        if(numtype == TenDigit) {
            digitsTensActions(val);
        }
        else if(numtype == Key) {
            keyActions(val);
        }
        else if(numtype == Hundred) {
            hundredActions(val);
        }
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
    NumSolution numSolution(numberStart, numberEnd, number);
    isKey = false;
    isHundred = false;
    previous = 0;
    current = 0;
    currentH = 0;
    numberStart = -1;
    numberEnd = -1;
    extractedNumbers.append(numSolution);
    cout << number << '\n';
}

void NumNorm::digitsTensActions(int val) {
    if(isHundred) {
        currentH += val;
    }
    else {
        if(current == 0) {
            current = val;
        }
        else {
            if(isKey) {
                previous += current;
                current = val;
            }
            else {
                current += val;
            }
        }
    }
    isKey = false;
}

void NumNorm::keyActions(int val) {
    if(isHundred) {
        if(current != 0) {
            previous += current;
        }
        current = currentH * val;
        currentH = 0;
        isHundred = false;
        isKey = true;
    }
    else {
        if(current == 0) {
            current = val;
            isKey = true;
        }
        else {
            if(!(isKey)) {
                isKey = true;
                current = current * val;
            }
            else {
                previous += current;
                current = val;
            }
        }
    }
}

void NumNorm::hundredActions(int val) {
    isHundred = true;
    if(current == 0) {
        currentH = val;
    }
    else {
        if(!isKey) {
            currentH = current * val;
            current = 0;
        }
        else {
            currentH = val;
        }
    }
    isKey = false;
}
