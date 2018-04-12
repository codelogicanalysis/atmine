#ifndef SPLIT_AFFIXES_H
#define SPLIT_AFFIXES_H

#include <QDialog>
#include <QString>

class QTextEdit;
class QComboBox;
class QTextBrowser;
class QTableWidget;
class QGridLayout;

class SplitDialog:public QDialog{
	Q_OBJECT
public:
	SplitDialog();

public slots:
	void reverse_clicked();
	void split_clicked();
	void specialize_clicked();
	void export_clicked();
	void affixType_currentIndexChanged(int);
	void specializeAllDuplicateEntries();
	void removeStaleCategoriesAndAffixes();

private:
	void loadAffixList();
	void loadCompatibilityList();
	void renameCategory(int row,QString category_original, QString category_specialized);
	void specializeHelper(QString category_original, QString category_specialized, QString category_left, bool modifyLeftEntries=true);
	void findDuplicates();
	void findSimilarCategories(long category_id1_1,long category_id2, long resulting_category, QList<long> & list);
	void mergeSimilarCategories(bool first=true);
	void makeNonAcceptStatesAccept();
	void removeDummyRulesForConsistencyIfNotNeeded();
	QString getAffix(long id);
	bool hasSubjectObjectInconsistency(QString descriptionDesired,QString descriptionFound); //works for prefixes
	int getPartialConnectedRow(QString affix,QString raw_data, QString pos, QString description, QString catgeory_1, QString resultCat, bool partial=true);
	int getRow(const QString & affix,const QString & raw_data, const QString & pos, const QString & description);

private:
	int source_id;
	long cat_empty;
	QTextEdit * affix;
	QPushButton * split, *exportLists, *reverse, *specialize, *renameCat, *removeStale;
	QComboBox * affixType;
	QTextBrowser * errors;
	QString * errors_text;
	QTableWidget * originalAffixList, * compatRulesList;
	QGridLayout * grid;

	~SplitDialog();
};


#endif // SPLIT_AFFIXES_H
