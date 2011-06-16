#include <QFile>
#include "stemmer.h"
#include "textParsing.h"
#include "timeRecognizer.h"


TimeParameters timeParameters;
QList<unsigned int> bits_ABSOLUTE_TIME;
unsigned int bit_TIME_PREPOSITION, bit_NUMBER, bit_DAY_NAME,bit_TIME_UNIT;

enum wordType { ABS_T, PREP_T, NUM, OTHER};
enum stateType { NOTHING_S , MAYBE_TIME_S, TIME_S};

class TimeEntity;

class TimeEntity {
private:
	typedef QVector<TimeEntity> TimeEntityVector;
	QString * text;
	long start, end;

	TimeEntityVector absoluteParts;
	TimeEntityVector prepositionParts;
	TimeEntityVector numberParts;
public:
	TimeEntity() { this->text=NULL; start=0;end=0;}
	TimeEntity(QString * text) { this->text=text; start=0;end=0;}
	TimeEntity(QString * text,long start) { this->text=text; this->start=start;end=start;}
	TimeEntity(QString * text,long start, long end) { this->text=text; this->start=start;this->end=end;}
	void setStart(long s) { start=s;}
	void setEnd(long e) { end=e; }
	int getStart() const {return start;}
	int getLength() const {	return end - start + 1;}
	virtual int getEnd() const { return end;}
	QString getString() const {
		int length=getLength();
		if (length<0)
			return "";
		return text->mid(getStart(), length);
	}
	void addAbsolutePart(long start, long end) {
		if (start>=this->start && end<=this->end) //later add check that there is no overlap
			absoluteParts.append(TimeEntity(text,start,end));
	}
	void addPrepositionPart(long start, long end) {
		if (start>=this->start && end<=this->end) //later add check that there is no overlap
			prepositionParts.append(TimeEntity(text,start,end));
	}
	void addNumberPart(long start, long end) {
		if (start>=this->start && end<=this->end) //later add check that there is no overlap
			numberParts.append(TimeEntity(text,start,end));
	}
	const TimeEntityVector & getPrepositionParts() const { return prepositionParts;}
	const TimeEntityVector & getAbsoluteParts() const { return absoluteParts;}
	const TimeEntityVector & getNumberParts() const { return numberParts;}
	TimeEntity * getLastPrepositionPart() {
		if (prepositionParts.size()>0)
			return &prepositionParts[prepositionParts.size()-1];
		else
			return NULL;
	}
	TimeEntity * getLastAbsolutePart() {
		if (absoluteParts.size()>0)
			return &absoluteParts[absoluteParts.size()-1];
		else
			return NULL;
	}
	TimeEntity * getLastNumberPart() {
		if (numberParts.size()>0)
			return &numberParts[numberParts.size()-1];
		else
			return NULL;
	}
};

typedef QVector<TimeEntity> TimeEntityVector;

class TimeStateInfo {
public:
	long lastEndPos;
	long startPos;
	long endPos;
	long nextPos;
	int numWords;
	wordType currentType:2;
	stateType currentState:2;
	stateType nextState:2;
	bool isTimeUnit:1;
	bool hasTimeUnit:1;
	int unused:24;
	PunctuationInfo punctuationInfo;

	//info about time entity recognized
	int IWn;
	int IWt;
	TimeEntity * entityProcessed;

	TimeStateInfo (){
		entityProcessed=NULL;
		resetCounters();
		startPos=0;
		endPos=0;
		nextPos=0;
		lastEndPos=0;
		numWords=0;
		isTimeUnit=false;
		hasTimeUnit=false;
		currentType=OTHER;
		currentState=NOTHING_S;
		nextState=NOTHING_S;
		punctuationInfo.reset();
	}
	void resetCurrentWordInfo()	{punctuationInfo.reset();isTimeUnit=false;}
	void resetCounters() {IWn=0;IWt=0;}
	void discardEntity() {
		numWords=0;
		hasTimeUnit=false;
		if (entityProcessed!=NULL) {
			delete entityProcessed;
			entityProcessed=NULL;
		}
	}
	TimeEntity * createEntity(QString * text) {
		discardEntity();
		entityProcessed=new TimeEntity(text);
		return entityProcessed;
	}
	long getTimeEntityStartPosition() {
		if (entityProcessed!=NULL)
			return entityProcessed->getStart();
		else
			return -1;
	}
};

QString * time_text=NULL;
long currentPos;
TimeEntityVector * timeVector=NULL;

#ifdef TIMEDEBUG
inline QString type_to_text(wordType t)
{
	switch(t)
	{
		case ABS_T:
			return "ABS_T";
		case PREP_T:
			return "PREP_T";
		case NUM:
			return "NUM";
		case OTHER:
			return "OTHER";
		default:
			return "UNDEFINED-TYPE";
	}
}
inline QString type_to_text(stateType t)
{
	switch(t)
	{
		case NOTHING_S:
			return "NOTHING_S";
		case MAYBE_TIME_S:
			return "MAYBE_TIME_S";
		case TIME_S:
			return "TIME_S";
		default:
			return "UNDEFINED-TYPE";
	}
}
inline void display(wordType t)
{
	out<<type_to_text(t)<<" ";
	//qDebug() <<type_to_text(t)<<" ";
}
inline void display(stateType t)
{
	out<<type_to_text(t)<<" ";
	//qDebug() <<type_to_text(t);
}
inline void display(QString t)
{
	out<<t;
	//qDebug() <<t;
}
#else
#define display(c)
#endif

void time_initialize(){
	QList<QString> abs,rel;
	abs.append("Month Name");
	abs.append("Day Name");
	long abstract_DAY_NAME=database_info.comp_rules->getAbstractCategoryID("Day Name");
	bit_DAY_NAME=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_DAY_NAME);
	abs.append("Relative Time");
	abs.append("Holiday");
	abs.append("Season");
	abs.append("Time Unit");
	abs.append("Time Reference");
	for (int i=0;i<abs.count();i++) {
		long abstract_ABSOLUTE_TIME=database_info.comp_rules->getAbstractCategoryID(abs[i]);
		bits_ABSOLUTE_TIME.append(database_info.comp_rules->getAbstractCategoryBitIndex(abstract_ABSOLUTE_TIME));
	}
	long abstract_TIME_PREPOSITION=database_info.comp_rules->getAbstractCategoryID("Time Preposition");
	long abstract_NUMBER=database_info.comp_rules->getAbstractCategoryID("Number");
	long abstract_TIME_UNIT=database_info.comp_rules->getAbstractCategoryID("Time Unit");
	bit_TIME_PREPOSITION=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_TIME_PREPOSITION);
	bit_NUMBER=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_NUMBER);
	bit_TIME_UNIT=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_TIME_UNIT);
}

class time_stemmer: public Stemmer
{
public:
	bool absoluteTime,timePreposition, number,isTimeUnit;
	long finish_pos;

	time_stemmer(QString * word, int start):Stemmer(word,start,false) {
		setSolutionSettings(M_ALL);
		init(start);
	}
	void init(int start) {
		this->info.start=start;
		this->info.finish=start;
		absoluteTime=false;
		timePreposition=false;
		number=false;
		isTimeUnit=false;
		finish_pos=start;
	}
	bool on_match() {
		solution_position * S_inf=Stem->computeFirstSolution();
		do
		{
			stem_info=Stem->solution;
		#ifdef GET_AFFIXES_ALSO
			solution_position * p_inf=Prefix->computeFirstSolution();
			do
			{
				prefix_infos=&Prefix->affix_info;
				solution_position * s_inf=Suffix->computeFirstSolution();
				do
				{
					suffix_infos=&Suffix->affix_info;
		#endif
					if (!analyze())
						return false;
		#ifdef GET_AFFIXES_ALSO
				}while (Suffix->computeNextSolution(s_inf));
				delete s_inf;
			}while (Prefix->computeNextSolution(p_inf));
			delete p_inf;
		#endif
		}while (Stem->computeNextSolution(S_inf));
		delete S_inf;
		return true;
	}

	bool analyze() {
	#if 0
		if (Suffix->info.finish>=Suffix->info.start)
			return true;//ignore solutions having suffixes
	#endif
		for (int i=0;i<bits_ABSOLUTE_TIME.count();i++) {
		#ifdef GET_AFFIXES_ALSO
			if (stem_info->abstract_categories.getBit(bits_ABSOLUTE_TIME[i])){
				if (bits_ABSOLUTE_TIME[i]==bit_DAY_NAME) {
					for (int j=0;j<prefix_infos->size();j++) {
						if (prefix_infos->at(j).POS=="Al/DET+") {
							absoluteTime=true;
							finish_pos=info.finish;
							return false;
						}
					}
					return true;
				}
		#else
			{
		#endif
				absoluteTime=true;
				finish_pos=info.finish;
				isTimeUnit=(equal(stem_info->raw_data,QString("")+tha2+alef+noon+ya2) ||
						   equal(stem_info->raw_data,QString("")+_3yn+alef+meem)) &stem_info->abstract_categories.getBit(bit_TIME_UNIT);
				return false;
			}
		}
		if (stem_info->abstract_categories.getBit(bit_TIME_PREPOSITION)){
			timePreposition=true;
			finish_pos=info.finish;
			return true;
		}
		if (stem_info->abstract_categories.getBit(bit_NUMBER)){
			number=true;
			finish_pos=info.finish;
			return true;
		}
		return true;
	}
};

bool getNextState(TimeStateInfo &  stateInfo)
{
	display(stateInfo.currentState);
	display(stateInfo.currentType);
	display(QString(" numWords=%1 ").arg(stateInfo.numWords));
	if (stateInfo.hasTimeUnit) {
		display(" {has time unit} ");}
	bool punc=false;
	if (stateInfo.punctuationInfo.hasEndingPunctuation()) {
		display("<has Ending punctuation>");
		punc=true;
	}
	display("\n");

	bool return_value=true;
	switch(stateInfo.currentState) {
	case NOTHING_S:
		if (stateInfo.currentType==PREP_T) {
			stateInfo.nextState=MAYBE_TIME_S;
			stateInfo.resetCounters();
			TimeEntity * t=stateInfo.createEntity(time_text);
			t->setStart(stateInfo.startPos);
			t->setEnd(stateInfo.endPos);
			t->addPrepositionPart(stateInfo.startPos,stateInfo.endPos);
		} else if (stateInfo.currentType==NUM) {
			stateInfo.nextState=MAYBE_TIME_S;
			stateInfo.resetCounters();
			TimeEntity * t=stateInfo.createEntity(time_text);
			t->setStart(stateInfo.startPos);
			t->setEnd(stateInfo.endPos);
			t->addNumberPart(stateInfo.startPos,stateInfo.endPos);
		} else if (stateInfo.currentType==ABS_T) {
			stateInfo.nextState=TIME_S;
			stateInfo.resetCounters();
			TimeEntity * t=stateInfo.createEntity(time_text);
			t->setStart(stateInfo.startPos);
			t->setEnd(stateInfo.endPos);
			t->addAbsolutePart(stateInfo.startPos,stateInfo.endPos);
		} else {
			stateInfo.nextState=NOTHING_S;
		}
		break;

	case MAYBE_TIME_S:
		if (stateInfo.currentType==NUM) {
			stateInfo.nextState=MAYBE_TIME_S;
			stateInfo.resetCounters();
			TimeEntity * t=stateInfo.entityProcessed;
			t->setEnd(stateInfo.endPos);
			t->addNumberPart(stateInfo.startPos,stateInfo.endPos);
		} else if (stateInfo.currentType==PREP_T) {
			stateInfo.nextState=MAYBE_TIME_S;
			stateInfo.resetCounters();
			TimeEntity * t=stateInfo.entityProcessed;
			t->setEnd(stateInfo.endPos);
			t->addPrepositionPart(stateInfo.startPos,stateInfo.endPos);
		} else if (stateInfo.currentType==ABS_T) {
			stateInfo.nextState=TIME_S;
			stateInfo.resetCounters();
			TimeEntity * t=stateInfo.entityProcessed;
			t->setEnd(stateInfo.endPos);
			t->addAbsolutePart(stateInfo.startPos,stateInfo.endPos);
		} else {
			if (stateInfo.IWn<timeParameters.num_before_absolute) {
				stateInfo.nextState=MAYBE_TIME_S;
				stateInfo.IWn++;
			} else {
				stateInfo.nextState=NOTHING_S;
				stateInfo.resetCounters();
				stateInfo.discardEntity();
			}
		}
		break;

	case TIME_S:
		if (stateInfo.currentType==NUM) {
			stateInfo.nextState=TIME_S;
			stateInfo.resetCounters();
			TimeEntity * t=stateInfo.entityProcessed;
			t->setEnd(stateInfo.endPos);
			t->addNumberPart(stateInfo.startPos,stateInfo.endPos);
		} else if (stateInfo.currentType==ABS_T) {
			stateInfo.nextState=TIME_S;
			stateInfo.resetCounters();
			TimeEntity * t=stateInfo.entityProcessed;
			t->setEnd(stateInfo.endPos);
			t->addAbsolutePart(stateInfo.startPos,stateInfo.endPos);
		} else {
			if (stateInfo.IWt<timeParameters.num_before_absolute) {
				stateInfo.nextState=TIME_S;
				stateInfo.IWt++;
			} else {
				stateInfo.nextState=NOTHING_S;
				stateInfo.resetCounters();
				timeVector->append(*stateInfo.entityProcessed);
				stateInfo.entityProcessed=NULL;
			}
		}
		break;

	default:
		break;
	}
	if (punc) {
		if (stateInfo.nextState==TIME_S) {
			stateInfo.nextState=NOTHING_S;
			stateInfo.resetCounters();
			timeVector->append(*stateInfo.entityProcessed);
			stateInfo.entityProcessed=NULL;
		} else if (stateInfo.nextState==MAYBE_TIME_S) {
			stateInfo.nextState=NOTHING_S;
			stateInfo.resetCounters();
			stateInfo.discardEntity();
		}
	}
	if (stateInfo.nextState!=NOTHING_S) {
		stateInfo.numWords++;
		if (stateInfo.isTimeUnit)
			stateInfo.hasTimeUnit=true;
	}
	return return_value;
}

inline wordType result(wordType t){display(t); return t;}
wordType getWordType(TimeStateInfo &  stateInfo)
{
	long  finish;
	stateInfo.resetCurrentWordInfo();
	if (isNumber(time_text,currentPos,finish)) {
		stateInfo.endPos=finish;
		stateInfo.nextPos=next_positon(time_text,finish,stateInfo.punctuationInfo);
		display(time_text->mid(stateInfo.startPos,finish-stateInfo.startPos+1)+":");

		bool ok;
		long num=time_text->mid(currentPos,finish-currentPos+1).toLong(&ok);
		if (ok && num > 1000 && num <2050) //is a year
			return(ABS_T);
		else
			return result(NUM);
	}
	time_stemmer s(time_text,currentPos);
	s();
	finish=max(s.info.finish,s.finish_pos);
	if (finish==currentPos)
		finish=getLastLetter_IN_currentWord(time_text,currentPos);
	stateInfo.endPos=finish;
	stateInfo.nextPos=next_positon(time_text,finish,stateInfo.punctuationInfo);
	display(time_text->mid(stateInfo.startPos,finish-stateInfo.startPos+1)+":");
	stateInfo.isTimeUnit=s.isTimeUnit;
	if (s.timePreposition )
		return result(PREP_T);
	else if (s.absoluteTime)
		return result(ABS_T);
	else if (s.number)
		return result(NUM);
	else
		return result(OTHER);
}


int timeRecognizeHelper(QString input_str,ATMProgressIFC *prg) {
	QFile input(input_str.split("\n")[0]);
	if (!input.open(QIODevice::ReadOnly))
	{
		out << "File not found\n";
		return 1;
	}
	QTextStream file(&input);
	file.setCodec("utf-8");
	assert (time_text==NULL);
	time_text=new QString(file.readAll());
	if (time_text->isNull()) {
		out<<"file error:"<<input.errorString()<<"\n";
		return 1;
	}
	if (time_text->isEmpty()) //ignore empty files
	{
		out<<"empty file\n";
		return 0;
	}
	long text_size=time_text->size();
	assert (timeVector==NULL);
	timeVector=new TimeEntityVector();

	TimeStateInfo stateInfo;
	stateInfo.resetCurrentWordInfo();
	stateInfo.currentState=NOTHING_S;
	stateInfo.nextState=NOTHING_S;
	stateInfo.lastEndPos=0;
	currentPos=0;
	while(currentPos<time_text->length() && isDelimiter(time_text->at(currentPos)))
		currentPos++;
	prg->setCurrentAction("Parsing Text File");
	prg->startTaggingText(*time_text);
	for (;currentPos<text_size;)
	{
		stateInfo.startPos=currentPos;
		stateInfo.resetCurrentWordInfo();
		stateInfo.currentType=getWordType(stateInfo);
		currentPos=stateInfo.nextPos;//here currentPos is changed
		int timeVectorSize=timeVector->size();
		getNextState(stateInfo);
		if (timeVector->size()>timeVectorSize) {
			TimeEntity & t=(*timeVector)[timeVectorSize];
			if (!(stateInfo.hasTimeUnit && !t.getString().contains(' '))) {
				out << t.getString()<<"\n";
				prg->tag(t.getStart(),t.getLength(),Qt::darkYellow,false);//Qt::gray
				const TimeEntityVector & absolute=t.getAbsoluteParts();
				for (int j=0;j<absolute.size();j++)
					prg->tag(absolute[j].getStart(),absolute[j].getLength(),Qt::white,true);
				const TimeEntityVector & preposition=t.getPrepositionParts();
				for (int j=0;j<preposition.size();j++)
					prg->tag(preposition[j].getStart(),preposition[j].getLength(),Qt::darkRed,true);
				const TimeEntityVector & numbers=t.getNumberParts();
				for (int j=0;j<numbers.size();j++)
					prg->tag(numbers[j].getStart(),numbers[j].getLength(),Qt::darkMagenta,true);
			}
		}
		stateInfo.currentState=stateInfo.nextState;
		stateInfo.lastEndPos=stateInfo.endPos;
		prg->report((double)currentPos/text_size*100+0.5);
		if (currentPos==text_size-1)
			break;
	}
	input.close();
	prg->finishTaggingText();
	delete time_text;
	time_text=NULL;
	delete timeVector;
	timeVector=NULL;
	return 0;
}
