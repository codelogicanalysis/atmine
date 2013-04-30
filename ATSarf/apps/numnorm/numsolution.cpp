#include "numsolution.h"

NumSolution::NumSolution () {
    start = 0;
    end = 0;
    number = 0;
}

NumSolution::NumSolution(int _start, int _end, int _number)
{
    start = _start;
    end = _end;
    number = _number;
}

int NumSolution::getStart() {
    return start;
}

int NumSolution::getEnd() {
    return end;
}

int NumSolution::getNumber() {
    return number;
}

void NumSolution::setStart(int _start) {
    start = _start;
}

void NumSolution::setEnd(int _end) {
    end = _end;
}

void NumSolution::setNumber(int _number) {
    number = _number;
}
