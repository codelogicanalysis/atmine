#ifndef _COMPATIBILITY_RULES_H
#define	_COMPATIBILITY_RULES_H

#include <QMap>
#include <QPair>
#include <QVector>
#include <QDebug>
#include <QDataStream>
#include "dbitvec.h"
#include "common.h"


class compatibility_rules
{
	private:
		typedef struct {
			int rc:16;		//currently <500 => more than enough
			unsigned int rule_t:3;// if valid : redundant with typecat1, typecat2
			unsigned int valid:1;
			unsigned int abstract1:1;
			unsigned int abstract2:1;
			unsigned int typecat1:2;
			unsigned int typecat2:2;
			unsigned int unused:6;
		} comp_rule_t;

		class NameIsAbstract {
			private:
				QString name;
				bool abstract:1;
				bool valid:1;
				friend QDataStream &operator<<(QDataStream &out, const NameIsAbstract &s);
				friend QDataStream &operator>>(QDataStream &in, NameIsAbstract &s);
			public:
				void set(const QString n, bool abstract){name=n;this->abstract=abstract;valid=true;
					//qDebug()<<n<<abstract;
				}
				QString & getName(){assert(valid);return name;}
				bool isAbstract(){assert(valid);return abstract;}
				bool isValid(){return valid;}
				NameIsAbstract(){valid=false;}
		};
		friend QDataStream &operator<<(QDataStream &out, const compatibility_rules::comp_rule_t &s);
		friend QDataStream &operator>>(QDataStream &in, compatibility_rules::comp_rule_t &s);
		friend QDataStream &operator<<(QDataStream &out, const compatibility_rules::NameIsAbstract &s);
		friend QDataStream &operator>>(QDataStream &in, compatibility_rules::NameIsAbstract &s);
	public:
		typedef QVector <QVector <comp_rule_t> > CompRuleLookupTable;
		typedef QVector<NameIsAbstract> CategoryNames;
		typedef QVector<int> CatIDList;
		typedef QMap<QString, int> CatName2CatIDMap;
		typedef QPair<QString, int> CatName2CatIDPair;
		typedef QMap<int, int> CatID2CatBitMap;
		typedef QPair<int, int> CatID2CatBitPair;
	private:
		CompRuleLookupTable crlTable;
		CategoryNames cat_names;
		CatName2CatIDMap catNamesMap;
		CatIDList absCatIDForBits;
		CatID2CatBitMap absCatBitMap;

		void fill();
		void readFromDatabaseAndBuildFile();
		void fillMap();
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
				return cat_names[id].getName();
			}catch(int i) {
				return "";
			}
		}
		int getCategoryID(const QString & name) //-1 is invalid
		{
			CatName2CatIDMap::iterator i=catNamesMap.find(name);
			if (i==catNamesMap.end())
				return -1;
			else
				return (*i);
		}
		int getAbstractCategoryID(const QString & name) //-1 is invalid, -2 not abstract
		{
		#if 1
			CatName2CatIDMap::iterator i=catNamesMap.find(name);
			if (i==catNamesMap.end())
				return -1;
			else
			{
				int id=*i;
				NameIsAbstract & n=cat_names[id];
				if (n.isAbstract())
					return id;
				else
					return -2;
			}
		#else
			return getCategoryID(name);
		#endif
		}
		int getAbstractCategoryBitIndex(int cat_id)
		{
			CatID2CatBitMap::iterator i=absCatBitMap.find(cat_id);
			if (i==absCatBitMap.end())
				return -1;
			else
				return *i;
		}
};


inline QDataStream &operator<<(QDataStream &out, const compatibility_rules::comp_rule_t &s)
{
	const void * temp=&s;
	out<<*((const quint32 *)temp);
	return out;
}

inline QDataStream &operator>>(QDataStream &in, compatibility_rules::comp_rule_t &s)
{
	quint32 r;
	in>>r;
	s=*((compatibility_rules::comp_rule_t *)((void *)&r));
	return in;
}

inline QDataStream &operator<<(QDataStream &out, const compatibility_rules::NameIsAbstract &s)
{
	out<<s.name<<s.abstract<<s.valid;
	return out;
}

inline QDataStream &operator>>(QDataStream &in, compatibility_rules::NameIsAbstract &s)
{
	bool abstract,valid;
	in>>s.name>>abstract>>valid;
	s.abstract=abstract;
	s.valid=valid;
	return in;
}


#endif	/* _COMPATIBILITY_RULES_H */

