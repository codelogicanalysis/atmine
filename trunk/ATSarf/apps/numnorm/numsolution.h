#ifndef NUMSOLUTION_H
#define NUMSOLUTION_H

class NumSolution
{
private:
    int start;
    int end;
    long number;
public:
    NumSolution();
    NumSolution(int, int, long);
    void setStart(int);
    void setEnd(int);
    void setNumber(long);
    int getStart();
    int getEnd();
    long getNumber();
};

#endif // NUMSOLUTION_H
