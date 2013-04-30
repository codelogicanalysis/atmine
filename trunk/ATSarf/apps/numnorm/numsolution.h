#ifndef NUMSOLUTION_H
#define NUMSOLUTION_H

class NumSolution
{
private:
    int start;
    int end;
    int number;
public:
    NumSolution();
    NumSolution(int, int, int);
    void setStart(int);
    void setEnd(int);
    void setNumber(int);
    int getStart();
    int getEnd();
    int getNumber();
};

#endif // NUMSOLUTION_H
