#ifndef INFLECTIONS_H
#define INFLECTIONS_H

#include "common.h"
#include <QString>
#include <QStringList>

namespace InflectionsDelimitors{
	const static QString start="//";
	const static QString middle="||";
	const static QString end="\\";
};


class ApplyInflections {
protected:
	QChar startLetter;
	bool applyPlusRules:1;
protected:
	virtual bool applyStripDiacriticChange()=0;
	virtual QString & getNonDiacriticField()=0;
public:
	ApplyInflections(bool applyPlusRules) { this->applyPlusRules=applyPlusRules;}
	void operator()(QString & inflection,QString & field) {
		//assume inflections in form (r/d)//{[reg exp]/(+1)/(+2)}||replacement\\ ...
		if (inflection.isEmpty() || inflection.count(' ')==inflection.size())
			return;
		QStringList l=inflection.split(InflectionsDelimitors::end);
		QString s;
		foreach(s,l) {
			while (s.startsWith(' '))
				s.remove(0,1);
			if (s.isEmpty())
				continue;
			QString start_prefix=QString()+startLetter+InflectionsDelimitors::start;
			if (s.startsWith(start_prefix)) {
				//inflection.remove(s+InflectionsDelimitors::end);
				s.remove(start_prefix);
				QStringList changeList=s.split(InflectionsDelimitors::middle);
				assert(changeList.size()==2);
				QString rule=changeList[0];
				QString replacement=changeList[1];
				QString replacement_withoutDiacritics=removeDiacritics(replacement);
				QString & nonDiacriticField=getNonDiacriticField();
				bool non_diacritic=applyStripDiacriticChange();
				if (applyPlusRules && rule=="(+1)") { //after the first (i.e. prepend before second)
					field=replacement+field;
					if (non_diacritic)
						nonDiacriticField=replacement_withoutDiacritics+nonDiacriticField;
				} else if (applyPlusRules && rule=="(+2)") { //after the second (i.e. append after second)
					field=field+replacement;
					if (non_diacritic)
						nonDiacriticField=nonDiacriticField+replacement_withoutDiacritics;
				} else {
					field.replace(rule,replacement);
					if (non_diacritic) {
						QString rule_withoutDiacritics=removeDiacritics(rule);
						nonDiacriticField.replace(rule_withoutDiacritics,replacement_withoutDiacritics);
					}
				}
			}
		}
	}
};


class RawDataInflections:protected ApplyInflections {
private:
	QString affix;
public:
	RawDataInflections(): ApplyInflections(true) {
		startLetter='r';
	}
protected:
	virtual bool applyStripDiacriticChange() { return true;}
	virtual QString & getNonDiacriticField() { return affix;}
public:
	void operator()(QString & inflection,QString & affix,QString & raw_data) {
		this->affix=affix;
		ApplyInflections::operator ()(inflection,raw_data);
		affix=this->affix;
	}
};

class DescriptionInflections:public ApplyInflections {
private:
	QString dummy;
public:
	DescriptionInflections(bool applyPlusRules):ApplyInflections(applyPlusRules) {
		startLetter='d';
	}
protected:
	virtual bool applyStripDiacriticChange() { return false;}
	virtual QString & getNonDiacriticField() { return dummy;}
};

class POSInflections :public DescriptionInflections {
public:
	POSInflections(bool applyPlusRules):DescriptionInflections(applyPlusRules) {
		startLetter='p';
	}
};


inline void applyRawDataInflections(QString & inflection, QString & affix,QString & raw_data) {
	RawDataInflections r;
	r(inflection,affix,raw_data);
}


inline void applyDescriptionInflections(QString & inflection, QString & description, bool first=false) {
	DescriptionInflections d(!first);
	d(inflection,description);
}

inline void applyPOSInflections(QString & inflection, QString & pos, bool first=false) {
	POSInflections d(!first);
	d(inflection,pos);
}

inline bool hasDescriptionRule(QString inflection) { //is conservative function, may return true when it is not, otherwise has to do split operation to be 100% accurate
	QString start_prefix='d'+InflectionsDelimitors::start;
	return (inflection.contains(start_prefix));
}

inline void applyInflections(QString inflectionRule, minimal_item_info & previous, minimal_item_info & current) {
	applyPOSInflections(inflectionRule,current.POS);
	applyPOSInflections(inflectionRule,previous.POS,true);

	if (hasDescriptionRule(inflectionRule)) {
		QString desc=current.description();
		applyDescriptionInflections(inflectionRule,desc);
		current.setDescription(desc);

		desc=previous.description();
		applyDescriptionInflections(inflectionRule,desc,true);
		previous.setDescription(desc);
	}
}

inline void applyInflections(QString inflectionRule, minimal_item_info & current) {
	minimal_item_info previous;
	applyInflections(inflectionRule,previous,current);
}


#endif // INFLECTIONS_H
