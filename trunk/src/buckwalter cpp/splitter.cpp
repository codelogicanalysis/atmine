/*
#include <iostream>
using namespace std;

int main ()
{

    int splitter[32][5]={{-1},{0,-1},{1,-1},{0,1,-1},{2,-1},
                         {0,2,-1},{1,2,-1},{0,1,2,-1},{3,-1},
                         {0,3,-1},{1,3,-1},{0,1,3,-1},{2,3,-1},
                         {0,2,3,-1},{1,2,3,-1},{0,1,2,3,-1},
                         {4,-1},{0,4,-1},{1,4,-1},{0,1,4,-1},
                         {2,4,-1},{0,2,4,-1},{1,2,4,-1},{0,1,2,4,-1},
                         {3,4,-1},{0,3,4,-1},{1,3,4,-1},{0,1,3,4,-1},
                         {2,3,4,-1},{0,2,3,4,-1},{1,2,3,4,-1},{0,1,2,3,4}};

  // printf ("%d,%d,%d",splitter[0][0],splitter[1][2],splitter[31][2]);
    return 0;


     void generate() {         for (int i = 0;i<32;i++        )
        {                 out<<"{";
             bitset<6> h(i);
             for (int j=0;j<5;j++)
   {
  if (h[j]==1)
                    out<<QString("%1,").arg(j);
       }                 out<<"-1},";         } }

}
*/