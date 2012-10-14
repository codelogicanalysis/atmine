#ifndef GER_H
#define GER_H

#include <QString>
#include "alpha.h"
#include "gamma.h"

#define infinity -1

class GER
{
  public:
	GER(QString word, int order = infinity);
	bool operator()();

 private:
	QString word;
	int order;
	QStringList wGloss;
	QStringList wStem;
};
#endif // GER_H
