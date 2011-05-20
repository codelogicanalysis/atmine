#include <QFile>
#include "stemmer.h"
#include "textParsing.h"
#include "timeRecognizer.h"

TimeParameters timeParameters;
unsigned int bit_ABSOLUTE_TIME, bit_RELATIVE_TIME;

enum wordType { ABS_T, REL_T, OTHER};
enum stateType { NOTHING_S , MAYBE_TIME_S, TIME_S};

class TimeEntity;

class TimeEntity {
private:
	typedef QVector<TimeEntity> TimeEntityVector;
	QString * text;
	long start, end;

	TimeEntityVector absoluteParts;
	TimeEntityVector relativeParts;
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
	void addRelativePart(long start, long end) {
		if (start>=this->start && end<=this->end) //later add check that there is no overlap
			relativeParts.append(TimeEntity(text,start,end));
	}
	const TimeEntityVector & getRelativeParts() const { return relativeParts;}
	const TimeEntityVector & getAbsoluteParts() const { return absoluteParts;}
	TimeEntity * getLastRelativePart() {
		if (relativeParts.size()>0)
			return &relativeParts[relativeParts.size()-1];
		else
			return NULL;
	}
	TimeEntity * getLastAbsolutePart() {
		if (absoluteParts.size()>0)
			return &absoluteParts[absoluteParts.size()-1];
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
	wordType currentType:2;
	stateType currentState:2;
	stateType nextState:2;
	bool followedByPunctuation:1;

	//info about time entity recognized
	int IWn;
	int IWt;
	int unused:25;
	TimeEntity * entityProcessed;

	TimeStateInfo (){
		entityProcessed=NULL;
		resetCounters();
		startPos=0;
		endPos=0;
		nextPos=0;
		lastEndPos=0;
		currentType=OTHER;
		currentState=NOTHING_S;
		nextState=NOTHING_S;
		followedByPunctuation=true;
	}
	void resetCurrentWordInfo()	{followedByPunctuation=false;}
	void resetCounters() {IWn=0;IWt=0;}
	void discardEntity() {
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
		case REL_T:
			return "REL_T";
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
	long abstract_ABSOLUTE_TIME=database_info.comp_rules->getAbstractCategoryID("Absolute Time");
	bit_ABSOLUTE_TIME=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_ABSOLUTE_TIME);
	long abstract_RELATIVE_TIME=database_info.comp_rules->getAbstractCategoryID("Relative Time");
	bit_RELATIVE_TIME=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_RELATIVE_TIME);
	if (abstract_RELATIVE_TIME<0)
	{
		if (abstract_ABSOLUTE_TIME<0) {
			bit_ABSOLUTE_TIME=0;
			bit_RELATIVE_TIME=0;
		} else {
			bit_RELATIVE_TIME=bit_ABSOLUTE_TIME;
		}
	}
}

class time_stemmer: public Stemmer
{
public:
	bool absoluteTime,relativeTime;
	long finish_pos;

	time_stemmer(QString * word, int start):Stemmer(word,start,false) {
		setSolutionSettings(M_ALL);
		init(start);
	}
	void init(int start) {
		this->info.start=start;
		this->info.finish=start;
		absoluteTime=false;
		relativeTime=false;
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
				prefix_infos=Prefix->affix_info;
				solution_position * s_inf=Suffix->computeFirstSolution();
				do
				{
					suffix_infos=Suffix->affix_info;
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
		if (stem_info->abstract_categories.getBit(bit_ABSOLUTE_TIME)){
			absoluteTime=true;
			finish_pos=info.finish;
			return false;
		}
		if (stem_info->abstract_categories.getBit(bit_RELATIVE_TIME)){
			relativeTime=true;
			finish_pos=info.finish;
			return true;
		}
		return true;
	}
};

bool getNextState(TimeStateInfo &  stateInfo)
{
	display(stateInfo.currentState);
	display("\n");
	if (stateInfo.followedByPunctuation) {
		display("<has punctuation>");
	}
	bool return_value=true;
	switch(stateInfo.currentState) {
	case NOTHING_S:
		if (stateInfo.currentType==REL_T) {
			stateInfo.nextState=MAYBE_TIME_S;
			stateInfo.resetCounters();
			TimeEntity * t=stateInfo.createEntity(time_text);
			t->setStart(stateInfo.startPos);
			t->setEnd(stateInfo.endPos);
			t->addRelativePart(stateInfo.startPos,stateInfo.endPos);
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
		if (stateInfo.currentType==REL_T) {
			stateInfo.nextState=MAYBE_TIME_S;
			stateInfo.resetCounters();
			TimeEntity * t=stateInfo.entityProcessed;
			t->setEnd(stateInfo.endPos);
			t->addRelativePart(stateInfo.startPos,stateInfo.endPos);
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
		if (stateInfo.currentType==REL_T) {
			stateInfo.nextState=TIME_S;
			stateInfo.resetCounters();
			TimeEntity * t=stateInfo.entityProcessed;
			t->setEnd(stateInfo.endPos);
			t->addRelativePart(stateInfo.startPos,stateInfo.endPos);
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

	return return_value;
}

inline wordType result(wordType t){display(t); return t;}
wordType getWordType(TimeStateInfo &  stateInfo)
{
	long  finish;
	bool has_punctuation;
	stateInfo.resetCurrentWordInfo();
	if (isNumber(time_text,currentPos,finish)) {
		stateInfo.endPos=finish;
		stateInfo.nextPos=next_positon(time_text,finish,has_punctuation);
		stateInfo.followedByPunctuation=has_punctuation;
		display(time_text->mid(stateInfo.startPos,finish-stateInfo.startPos+1)+":");
		return result(REL_T);
	}
	time_stemmer s(time_text,currentPos);
	s();
	finish=max(s.info.finish,s.finish_pos);
	if (finish==currentPos)
		finish=getLastLetter_IN_currentWord(time_text,currentPos);
	stateInfo.endPos=finish;
	stateInfo.nextPos=next_positon(time_text,finish,has_punctuation);
	stateInfo.followedByPunctuation=has_punctuation;
	display(time_text->mid(stateInfo.startPos,finish-stateInfo.startPos+1)+":");
	if (s.relativeTime )
		return result(REL_T);
	else if (s.absoluteTime)
		return result(ABS_T);
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
			prg->tag(t.getStart(),t.getLength(),Qt::darkYellow,false);//Qt::gray
			const TimeEntityVector & absolute=t.getAbsoluteParts();
			for (int j=0;j<absolute.size();j++)
				prg->tag(absolute[j].getStart(),absolute[j].getLength(),Qt::white,true);
			const TimeEntityVector & relative=t.getRelativeParts();
			for (int j=0;j<relative.size();j++)
				prg->tag(relative[j].getStart(),relative[j].getLength(),Qt::darkRed,true);
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
