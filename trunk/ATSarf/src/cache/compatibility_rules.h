#ifndef _COMPATIBILITY_RULES_H
#define	_COMPATIBILITY_RULES_H

#include <QVector>
#include <QDataStream>
#include "dbitvec.h"
#include "common.h"

typedef struct
  {
	int rc:16;		//currently <500 => more than enough
	unsigned int rule_t:3;// if valid : redundant with typecat1, typecat2
	unsigned int valid:1;
	unsigned int abstract1:1;
	unsigned int abstract2:1;
	unsigned int typecat1:2;
	unsigned int typecat2:2;
	unsigned int unused:6;
  } comp_rule_t;

class compatibility_rules
{
	public:
		typedef QVector <QVector <comp_rule_t> > CompRuleLookupTable;
		typedef QVector<QString> CategoryNames;
		//TODO: add support for getting id of a CategoryName
	private:
		CompRuleLookupTable crlTable;
		CategoryNames cat_names;

		void fill();
		void readFromDatabaseAndBuildFile();
    public:
		void buildFromFile();
		bool operator()(int id1,int id2)
		{
			try{
				return crlTable[id1][id2].valid;
			}catch(int i) {
				return false;
			}
		}
		bool operator()(int id1,int id2, long & id_r)//-1 is invalid
		{
			try{
				comp_rule_t & crule=crlTable[id1][id2];
				if (crule.rule_t==AA || crule.rule_t==CC)
				{
					//more than one category!!!!!!!!!
					id_r=crule.rc;
				}
				return crule.valid;
			}catch(int i) {
				return false;
			}
		}
		QString getCategoryName(int id)
		{
			try{
				return cat_names[id];
			}catch(int i) {
				return false;
			}
		}
};


inline QDataStream &operator<<(QDataStream &out, const comp_rule_t &s)
{
	const void * temp=&s;
	out<<*((const quint32 *)temp);
	return out;
}

inline QDataStream &operator>>(QDataStream &in, comp_rule_t &s)
{
	quint32 r;
	in>>r;
	s=*((comp_rule_t *)((void *)&r));
	return in;
}
#endif	/* _COMPATIBILITY_RULES_H */

