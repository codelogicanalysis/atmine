#include <QString>
#include <cstdlib>
#include <time.h>
#include <QString>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QRegExp>
#include <QStringList>
#include <assert.h>
#include "logger.h"
#include "ATMProgressIFC.h"
#include "text_handling.h"
#include "diacritics.h"

int mada(QString folderName, ATMProgressIFC * /*prg*/) {
	QFile input_analysis(folderName+"/atb_analysis1");
	QFile input_tokenize(folderName+"/atb_tokenization");
	if (!input_analysis.open(QIODevice::ReadOnly) || !input_tokenize.open(QIODevice::ReadOnly)) {
		out << "File not found\n";
		return 1;
	}
	QTextStream file_analysis(&input_analysis);
	file_analysis.setCodec("utf-8");
	QTextStream file_tokenize(&input_tokenize);
	file_tokenize.setCodec("utf-8");
	QStringList entries;
	QString line;
	int correct=0, all_count=0, half_correct=0, normalization_correct=0,larger_incorrect=0,
		smaller_incorrect=0,diff_incorrect=0,not_found=0,non_diacritics_correct=0;
	while (!file_tokenize.atEnd()) {
		//read from tokenization
		line=file_tokenize.readLine(0);
		if (line.isEmpty())
			continue;
		all_count++;
		entries=line.split("\t");
		assert(entries.size()==3);
		QString word=entries[0];
		QString correct_analysis=entries[1];
		QString correct_tokenization=entries[2];
		correct_analysis=correct_analysis.remove(".VN");
		correct_analysis=correct_analysis.replace("ACCGEN","ACC");

		//read from analysis
		line=file_analysis.readLine(0);
		entries=line.split("\t");
		assert(entries.size()==2);
		int count=entries[1].toInt();
		QString word_analysis=entries[0];
		assert (equal(word,word_analysis));
		int i;
		for (i=0;i<=count;i++) {
			if (i!=count)
				line=file_analysis.readLine(0);
			entries=line.split("\t");
			assert(entries.size()==2);
			QString analysis=entries[0];
			QString tokenization=entries[1];
			if (analysis.startsWith('+'))
				analysis=analysis.remove(0,1);
			if (analysis.endsWith('+'))
				analysis=analysis.remove(analysis.size()-1,1);
			tokenization.remove(" ");

			if (analysis==correct_analysis || i==count) {
				if (tokenization==correct_tokenization)
					correct++;
				else {
					out<<word<<"\t"<<correct_tokenization<<"\t"<<tokenization;
					if (equal(tokenization,correct_tokenization)) {
						if (countDiacritics(correct_tokenization)>0) {
							non_diacritics_correct++;
							out<<"\t-";
						} else {
							half_correct++;
							out<<"\t~";
						}
					} else if (tokenization.count('+')==correct_tokenization.count('+')) {
						int diff=correct_tokenization.size()-tokenization.size();
						if (diff==0) {
							normalization_correct++;
							out<<"\t*";
						} else if  (diff>0) {
							larger_incorrect++;
							out<<"\t>";
						} else if (diff<0) {
							smaller_incorrect++;
							out<<"\t<";
						}
					} else {
						if (i==count) {
							not_found++;
							out<<"!!!";
							displayed_error<<word<<"\t"<<correct_analysis<<"\t"<<all_count<<"\n";
						} else {
							diff_incorrect++;
							out<<"!";
						}
					}
					out<<"\n";
				}
				for (int j=i+1;j<count;j++)
					file_analysis.readLine(0);
				break;
			}
		}
	}
	input_analysis.close();
	input_tokenize.close();

	displayed_error<<"Correct:\t\t"<<correct<<"/"<<all_count<<"=\t"<<((double)correct)/all_count<<"\n";
	displayed_error<<"-Correct:\t\t"<<non_diacritics_correct<<"/"<<all_count<<"=\t"<<((double)non_diacritics_correct)/all_count<<"\n";
	displayed_error<<"~Correct:\t\t"<<half_correct<<"/"<<all_count<<"=\t"<<((double)half_correct)/all_count<<"\n";
	displayed_error<<"*Correct:\t\t"<<normalization_correct<<"/"<<all_count<<"=\t"<<((double)normalization_correct)/all_count<<"\n";
	displayed_error<<">Incorrect:\t\t"<<larger_incorrect<<"/"<<all_count<<"=\t"<<((double)larger_incorrect)/all_count<<"\n";
	displayed_error<<"<Incorrect:\t\t"<<smaller_incorrect<<"/"<<all_count<<"=\t"<<((double)smaller_incorrect)/all_count<<"\n";
	displayed_error<<"!Incorrect:\t\t"<<diff_incorrect<<"/"<<all_count<<"=\t"<<((double)diff_incorrect)/all_count<<"\n";
	displayed_error<<"Not Found:\t\t"<<not_found<<"/"<<all_count<<"=\t"<<((double)not_found)/all_count<<"\n";
	return 0;
}

