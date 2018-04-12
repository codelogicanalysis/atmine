#include "editDistance.h"
#include <QString>
#include <QList>


int EditDistance::Compute(QString a, QString b, bool) {
    a = removeDiacritics(a);
    b = removeDiacritics(b);
    // Allocate distance matrix
    QList<QList<int> > d;
    QList<int> temp;

    for (int i = 0; i < b.size() + 1; i++) {
        temp.append(0);
    }

    for (int i = 0; i < a.size() + 1; i++) {
        d.append(temp);
    }

    // Compute distance
    for (int i = 0; i <= a.size(); i++) {
        d[i][0] = i;
    }

    for (int j = 0; j <= b.size(); j++) {
        d[0][j] = j;
    }

    for (int i = 1; i <= a.size(); i++) {
        for (int j = 1; j <= b.size(); j++) {
            if (CharComparer(a.at(i - 1), b.at(j - 1))) {
                // No change required
                d[i][j] = d[i - 1][j - 1];
            } else {
                d[i][ j] =
                    min(d[i - 1][ j] + 1,    // Deletion
                        min(d[i][ j - 1] + 1,    // Insertion
                            d[i - 1][ j - 1] + 1));       // Substitution
            }
        }
    }

    // Return final value
    return d[a.size()][ b.size()];
}
