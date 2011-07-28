#include "narratordetector.h"
#include "graph.h"
#include "narratorHash.h"


class NarratorDetector
{
private:

	stateData currentData;
	QString * text;
	long current_pos;

	NarratorGraph * graph;

public:
	BiographyList * biographies;

private:

#ifdef SEGMENT_BIOGRAPHY_USING_POR
	class ReachableVisitor:public NodeVisitor {
		NarratorNodeIfc * target;
		ColorIndices & colorGuard;
		bool found;
	public:
		ReachableVisitor(NarratorNodeIfc * aTarget,ColorIndices & guard):colorGuard(guard) {target=aTarget;}
		void initialize(){
			found=false;
		}
		virtual void visit(NarratorNodeIfc & ,NarratorNodeIfc & , int) {	}
		virtual void visit(NarratorNodeIfc & n) {
			if (&n==target) {
				found=true;
			#if 0 //find a way to stop early even in a traversal where we are using a map
				colorGuard.setAllNodesVisited(controller->getVisitColorIndex()); //to stop further traversal
				colorGuard.setAllNodesVisited(controller->getFinishColorIndex());
			#endif
				qDebug()<<"<found>";
			}
		}
		virtual void finishVisit(NarratorNodeIfc & ){ }
		virtual void detectedCycle(NarratorNodeIfc & ){ }
		virtual void finish(){	}
		bool isFound() {return found;}
	};
	class Cluster;
	class NodeItem {
	public:
		NarratorNodeIfc * node;
		Cluster* cluster;
		int inDegree;
		int outDegree;
		NodeItem(NarratorNodeIfc * n) {
			node=n;
			cluster=NULL;
			inDegree=0;
			outDegree=0;
		}
	};
	typedef QList<NodeItem *> NodeItemList;
	typedef QList<NodeItemList> NodeItemGroup;
	class Cluster {
	public:
		int id;
		NodeItemList list;
		Cluster(NodeItem * n1,NodeItem * n2,int cluster_id){
			list.append(n1);
			list.append(n2);
			n1->cluster=this;
			n2->cluster=this;
			id=cluster_id;
		}
		void addNodeItem(NodeItem * n) {
			list.append(n);
			n->cluster=this;
		}
		int size() {
			return list.size();
		}
		NodeItem * operator[](int i){
			return list[i];
		}
		~Cluster() {
			for (int i=0;i<list.size();i++)
				delete list[i];
		}
	};
	class ClusterList {
	private:
		QList<Cluster *> list;
	public:
		void addCluster(Cluster * c,int id=-1){
			if (id<0 && c->id>=0)
				id=c->id;
			if (id>=0 && c->id<0)
				c->id=id;
			if (id<0 && c->id<0) {
				id=list.size();
				c->id=id;
			}
			assert(c->id==id);
			assert(list.size()<=id || list[id]==NULL);
			for (int i=list.size();i<=id;i++)
				list.append(NULL);
			list[id]=c;
		}
		int size() {
			return list.size();
		}
		Cluster * operator[](int i){
			return list[i];
		}
		~ClusterList() {
			for (int i=0;i<list.size();i++)
				if (list[i]!=NULL)
					delete list[i];
		}
		void mergeNodeItems(NodeItem * n1,NodeItem * n2) {
			if (n1->cluster==NULL && n2->cluster==NULL){
				Cluster * c=new Cluster(n1,n2,size());
				addCluster(c);
			} else if (n1->cluster!=NULL && n2->cluster==NULL) {
				n1->cluster->addNodeItem(n2);
				assert(n2->cluster==n1->cluster);
			} else if (n2->cluster!=NULL && n1->cluster==NULL) {
				n2->cluster->addNodeItem(n1);
				assert(n2->cluster==n1->cluster);
			} else {//in both cases:n1!=n2 or n1==n2
				return;
			}
		}
	};

	bool near(NarratorNodeIfc *n1, NarratorNodeIfc *n2) {
	#if 0
		ChainNodeIterator itr=n1.begin();
		for (;!itr.isFinished();++itr) {
			if (&itr.getChild()==&n2)
				return true;
			if (&itr.getParent()==&n2)
				return true;
		}
		return false;
	#else
		qDebug()<<"--test if near--("<<n1->CanonicalName()<<","<<n2->CanonicalName()<<")";
		if (n1==n2)
			return false;
		ReachableVisitor v(n2,graph->colorGuard);
		GraphVisitorController controller(&v,NULL);
		graph->BFS_traverse(controller,hadithParameters.bio_max_reachability,n1,1);
		bool found=v.isFound();
		if (!found) {
			graph->BFS_traverse(controller,hadithParameters.bio_max_reachability,n1,-1);
			found=v.isFound();
		}
		if (found) {
			qDebug()<<"found";
		}
		return found;
	#endif
	}
	bool near(NarratorNodeIfc * n, const Biography::NarratorNodeList & list) {
		for (int i=0;i<list.size();i++) {
			if (near(n,list[i]))
				return true;
		}
		return false;
	}
	int getLargestClusterSize(Biography::NarratorNodeGroups & list) {
		NodeItemGroup nodeItemgroups;
		//1-transform to nodeItems:
		for (int i=0;i<list.size();i++) {
			int size=list[i].size();
			if (size>0) {
				nodeItemgroups.append(NodeItemList());
				for (int j=0;j<size;j++) {
					NodeItem * node=new NodeItem(list[i][j]);
					nodeItemgroups[i].append(node);
				}
			}
		}
		ClusterList clusters;
		//1-each node put it in its own list or merge it with a group of already found if it is near them
		int size=nodeItemgroups.size();
		for (int i=0;i<size;i++) {
			for (int j=i+1;j<size;j++) {
				int size1=nodeItemgroups[i].size();
				int size2=nodeItemgroups[j].size();
				for (int i2=0;i2<size1;i2++) {
					for (int j2=0;j2<size2;j2++) {
						NodeItem * item1=nodeItemgroups[i][i2],
								 * item2=nodeItemgroups[j][j2];
						if (near(item1->node,item2->node)) {
							clusters.mergeNodeItems(item1,item2);
						}
					}
				}
			}
		}
		//3-return largest list size
		int largest=1;
		int index=-1;
		for (int i=0;i<clusters.size();i++) {
			if (clusters[i]->size()>largest) {
				largest=clusters[i]->size();
				index=i;
			}
		}
	#if 1
		if (index>=0) {
			qDebug()<<largest<<"\n";
			for (int i=0;i<clusters[index]->size();i++){
				qDebug()<<(*clusters[index])[i]->node->CanonicalName()<<"\n";
			}
			qDebug()<<"\n";
		}
	#endif
		return largest;
	}

#endif

public:
#ifdef SEGMENT_BIOGRAPHY_USING_POR
	NarratorDetector(NarratorGraph * graph) {this->graph=graph; }
	NarratorDetector() {}
#else
	NarratorDetector() {graph=NULL;}
#endif
	int segment(QString input_str,ATMProgressIFC *prg)  {
		QFile chainOutput(chainDataStreamFileName);

		chainOutput.remove();
		if (!chainOutput.open(QIODevice::ReadWrite))
			return 1;
		QDataStream chainOut(&chainOutput);
		QFile input(input_str);
		if (!input.open(QIODevice::ReadOnly)) {
			out << "File not found\n";
			return 1;
		}
		QTextStream file(&input);
		file.setCodec("utf-8");
		text=new QString(file.readAll());
		if (text==NULL)	{
			out<<"file error:"<<input.errorString()<<"\n";
			return 1;
		}
		if (text->isEmpty()) { //ignore empty files
			out<<"empty file\n";
			return 0;
		}
		long text_size=text->size();
		long  biographyStart=-1;
		currentData.initialize();

	#ifdef CHAIN_BUILDING
		HadithData *currentBiography=new HadithData(text,false,graph);
		currentBiography->initialize(text);
		//display(QString("\ninit%1\n").arg(currentBiography->narrator->m_narrator.size()));
	#else
		chainData *currentBiography=NULL;
	#endif
		long  biographyEnd;
		int biography_Counter=1;

		StateInfo stateInfo;
		stateInfo.resetCurrentWordInfo();
		stateInfo.currentState=TEXT_S;
		stateInfo.nextState=TEXT_S;
		stateInfo.lastEndPos=0;
		stateInfo.startPos=0;
	#ifdef PUNCTUATION
		stateInfo.previousPunctuationInfo.fullstop=true;
	#endif
		while(stateInfo.startPos<text->length() && isDelimiter(text->at(stateInfo.startPos)))
			stateInfo.startPos++;
	#ifdef PROGRESSBAR
		prg->setCurrentAction("Parsing Biography");
	#endif
		for (;stateInfo.startPos<text_size;) {
			if((proceedInStateMachine(stateInfo,currentBiography,currentData)==false)) {
			#ifdef SEGMENT_BIOGRAPHY_USING_POR
				Biography::NarratorNodeGroups & realNarrators=currentBiography->biography->nodeGroups;
				int num=getLargestClusterSize(realNarrators);
				if (num>=hadithParameters.bio_narr_min) {
			#else
				if (currentData.narratorCount>=hadithParameters.bio_narr_min) {
			#endif
					//biographyEnd=currentData.narratorEndIndex;
					biographyEnd=stateInfo.endPos;
					currentBiography->biography->setEnd(biographyEnd);
				#ifdef DISPLAY_HADITH_OVERVIEW
					//biographyStart=currentData.biographyStartIndex;
					biographyStart=currentBiography->biography->getStart();
					//long end=text->indexOf(QRegExp(delimiters),sanadEnd);//sanadEnd is first letter of last word in sanad
					//long end=stateInfo.endPos;
					out<<"\n"<<biography_Counter<<" new biography start: "<<text->mid(biographyStart,display_letters)<<endl;
					out<<"sanad end: "<<text->mid(biographyEnd-display_letters+1,display_letters)<<endl<<endl;
				#ifdef CHAIN_BUILDING
					currentBiography->biography->serialize(chainOut);
					currentBiography->biography->setStart(stateInfo.nextPos);
					//currentChain->chain->serialize(displayed_error);
				#endif
				#endif
					biography_Counter++;
				}
			}
			stateInfo.currentState=stateInfo.nextState;
			stateInfo.startPos=stateInfo.nextPos;
			stateInfo.lastEndPos=stateInfo.endPos;
		#ifdef PUNCTUATION
			stateInfo.previousPunctuationInfo=stateInfo.currentPunctuationInfo;
			if (stateInfo.number) {
				stateInfo.previousPunctuationInfo.fullstop=true;
				stateInfo.previousPunctuationInfo.has_punctuation=true;
			}
			/*if (stateInfo.previousPunctuationInfo.has_punctuation)
				stateInfo.previousPunctuationInfo.fullstop=true;*/
			if (stateInfo.previousPunctuationInfo.fullstop) {
				if (currentBiography->biography!=NULL)
					delete currentBiography->biography;
				currentBiography->biography=new Biography(graph,text,stateInfo.startPos);
			}


		#endif
	#ifdef PROGRESSBAR
			prg->report((double)stateInfo.startPos/text_size*100+0.5);
			if (stateInfo.startPos==text_size-1)
				break;
	#endif
		}
	#if defined(DISPLAY_HADITH_OVERVIEW)
		if (biographyStart<0)
		{
			out<<"no biography found\n";
			chainOutput.close();
		#if defined(TAG_HADITH)
			prg->startTaggingText(*text);
			prg->finishTaggingText();
		#endif
			return 2;
		}
		chainOutput.close();
	#endif
	#ifdef CHAIN_BUILDING //just for testing deserialize
		QFile f("biography_chains.txt");
		if (!f.open(QIODevice::WriteOnly))
			return 1;
		QTextStream file_biography(&f);
			file_biography.setCodec("utf-8");

		if (!chainOutput.open(QIODevice::ReadWrite))
			return 1;
		QDataStream tester(&chainOutput);
		int tester_Counter=1;
	#ifdef TEST_BIOGRAPHIES
		biographies=new BiographyList;
		biographies->clear();
	#endif
	#if defined(TAG_HADITH)
		prg->startTaggingText(*text);
	#endif
		while (!tester.atEnd())
		{
			Biography * s=new Biography(graph,text);
			s->deserialize(tester);
		#ifdef TEST_BIOGRAPHIES
			biographies->append(s);
		#endif
		#if defined(TAG_HADITH)
			for (int j=0;j<s->size();j++)
			{
				const Narrator * n=(*s)[j];
				if (n->m_narrator.size()==0) {
					out<<"found a problem an empty narrator in ("<<tester_Counter<<","<<j<<")\n";
					continue;
				}
				if (s->isReal(j))
					prg->tag(n->getStart(),n->getLength(),Qt::darkYellow,false);
				else
					prg->tag(n->getStart(),n->getLength(),Qt::darkGray,false);
				for (int i=0;i<n->m_narrator.size();i++)
				{
					NarratorPrim * nar_struct=n->m_narrator[i];
					if (nar_struct->isNamePrim()) {
						if (((NamePrim*)nar_struct)->learnedName) {
							prg->tag(nar_struct->getStart(),nar_struct->getLength(),Qt::blue,true);
							//error<<nar_struct->getString()<<"\n";
						}
						else
							prg->tag(nar_struct->getStart(),nar_struct->getLength(),Qt::white,true);
					}
					else if (((NameConnectorPrim *)nar_struct)->isFamilyConnector())
						prg->tag(nar_struct->getStart(),nar_struct->getLength(),Qt::darkRed,true);
					else if (((NameConnectorPrim *)nar_struct)->isPossessive())
						prg->tag(nar_struct->getStart(),nar_struct->getLength(),Qt::darkMagenta,true);
				}
			}
		#else
			hadith_out<<tester_Counter<<" ";
			s->serialize(hadith_out);
		#endif
			tester_Counter++;
			s->serialize(file_biography);
		}
		chainOutput.close();
		f.close();
	#endif
	#ifndef TAG_HADITH
		prg->startTaggingText(*hadith_out.string()); //we will not tag but this will force a text to be written there
	#else
		prg->finishTaggingText();
	#endif
		if (currentBiography!=NULL)
			delete currentBiography;
		return 0;
	}

	void freeMemory() { //called if we no longer need stuctures of this class
		for (int i=0;i<biographies->size();i++)
			delete (*biographies)[i];
		delete text;
	}
};

int biographyHelper(QString input_str,ATMProgressIFC *prg) {
	input_str=input_str.split("\n")[0];
	NarratorDetector s;
	s.segment(input_str,prg);
#ifdef TEST_BIOGRAPHIES
	s.freeMemory();
#endif
	return 0;
}

#ifdef TEST_BIOGRAPHIES
BiographyList * getBiographies(QString input_str,NarratorGraph* graph,ATMProgressIFC *prg) {
	input_str=input_str.split("\n")[0];
#ifdef SEGMENT_BIOGRAPHY_USING_POR
	NarratorDetector s(graph);
#else
	NarratorDetector s;
#endif
	s.segment(input_str,prg);
	return s.biographies;
}
#endif
