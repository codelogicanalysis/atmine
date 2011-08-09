#include "narratordetector.h"
#include "graph.h"
#include "narratorHash.h"
#include <QPair>


class NarratorDetector
{
private:
	typedef Biography::MatchingNode MatchingNode;

	StateData currentData;
	QString * text;
	long current_pos;

	NarratorGraph * graph;

public:
	BiographyList * biographies;

private:

#ifdef SEGMENT_BIOGRAPHY_USING_POR
	class ClusterGroup;
	class NodeItem {
	public:
		NarratorNodeIfc * node;
		double similarity;
		int inDegree;
		int outDegree;
		int stepsToCenter;

		NodeItem(const MatchingNode & mNode) {
			node=mNode.node;
			inDegree=0;
			outDegree=0;
			stepsToCenter=0;
			similarity=mNode.similarity;
		}
		NodeItem(NarratorNodeIfc * node,int stepsToCenter,double similarity) {
			this->node=node;
			this->stepsToCenter=stepsToCenter;
			this->similarity=similarity;
			inDegree=0;
			outDegree=0;
		}
	};
	typedef QList<NodeItem *> NodeItemList;
	typedef QList<NodeItemList> NodeItemGroup;
	class ClusterGroup {
	private:
		int numNarrators;
	public:
		NarratorNodeIfc * center;
		NodeItemGroup groups;
	public:
		ClusterGroup(NarratorNodeIfc * center) {
			this->center=center;
			numNarrators=0;
		}
		void addNodeItem(NodeItem * n,int narId) {
			while (groups.size()<=narId) {
				groups.append(NodeItemList());
			}
			if (groups[narId].size()==0)
				numNarrators++;
			groups[narId].append(n);
		}
		int size() {
			return groups.size();
		}
		NodeItemList & operator[](int i){
			return groups[i];
		}
		int getNumNarrators() { //i.e. no empty size
			return numNarrators;
		}
		~ClusterGroup() {
			int size=groups.size();
			for (int i=0;i<size;i++) {
				int size=groups[i].size();
				for (int j=0;j<size;j++)
					delete groups[i][j];
			}
		}
	};
	class ClusterList {
	private:
		QList<ClusterGroup *> list;
	public:
		void addCluster(ClusterGroup * c,int id=-1){
			if (id<0) {
				id=list.size();
			}
			assert(list.size()<=id || list[id]==NULL);
			for (int i=list.size();i<=id;i++)
				list.append(NULL);
			list[id]=c;
		}
		int size() {
			return list.size();
		}
		ClusterGroup * operator[](int i){
			return list[i];
		}
		~ClusterList() {
			for (int i=0;i<list.size();i++)
				if (list[i]!=NULL)
					delete list[i];
		}
	};
	typedef QList<NodeItem *> Cluster;
	class ClusterScore {
	public:
		int narrCount, stepCount;
		double similarityProduct;
		ClusterScore (int narr,int steps, double similarity): narrCount(narr),stepCount(steps), similarityProduct(similarity) {}
	};
	class ScoredCluster {
	public:
		Cluster * cluster;
		ClusterScore score;
		NarratorNodeIfc * center;
		ScoredCluster(NarratorNodeIfc * center,int narr,int steps=0, double similarity=0):score(narr,steps,similarity) {
			cluster=new Cluster;
			this->center=center;
		}

		bool operator <(const ScoredCluster & rhs) const{
			return (score.narrCount<rhs.score.narrCount ||
					 (score.narrCount==rhs.score.narrCount && score.stepCount>rhs.score.stepCount) ||
					 (score.narrCount==rhs.score.narrCount && score.stepCount==rhs.score.stepCount
							&& score.similarityProduct<rhs.score.similarityProduct)
					);
		}
	};
	typedef QList<ScoredCluster> ScoredClusterList;
	class Trash { //all items saved in its structures are deleted when object goes out of scope
	public:
		QList<Cluster *> clusters;
		QList<ClusterGroup*> clusterGroups;
		~Trash() {
			int size=clusters.size();
			for (int i=0;i<size;i++) {
				delete clusters[i];
			}
			size=clusterGroups.size();
			for (int i=0;i<size;i++) {
				delete clusterGroups[i];
			}
		}
	};

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
		virtual bool visit(NarratorNodeIfc & n) {
			if (!found) {
				if (&n==target) {
					found=true;
				#if 0 //find a way to stop early even in a traversal where we are using a map
					colorGuard.setAllNodesVisited(controller->getVisitColorIndex()); //to stop further traversal
					colorGuard.setAllNodesVisited(controller->getFinishColorIndex());
				#endif
				#ifdef DEBUG_BIOGRAPHY_CLUSTERING
					qDebug()<<"<found>";
				#endif
				}
			}
			return !found;
		}
		virtual void finishVisit(NarratorNodeIfc & ){ }
		virtual void detectedCycle(NarratorNodeIfc & ){ }
		virtual void finish(){	}
		bool isFound() {return found;}
	};
	class ClusteringVisitor:public NodeVisitor {
		ClusterGroup & cluster;
		NodeItemGroup & groups;
	public:
		ClusteringVisitor(ClusterGroup & aCluster,NodeItemGroup & g):cluster(aCluster),groups(g) {}
		void initialize(){	}
		virtual void visit(NarratorNodeIfc & ,NarratorNodeIfc & , int) {	}
		virtual bool visit(NarratorNodeIfc & n) {
			if (!n.hasSomeBiographyIndex())
				return false;
			int size=n.getBiographIndexCount();
			for (int i=0;i<size;i++) {
				int index=n.getBiographyIndex(i);
			#if 0
				double similarity=-1;
				NodeItemList & l=groups[index];
				for (int j=0;j<l.size();j++) {
					if (l.at(j)->node==&n) {
						similarity=l.at(j)->similarity;
						break;
					}
				}
				assert(similarity>0);
			#else
				double similarity=1;
			#endif
				NodeItem * item=new NodeItem(&n,controller->getParentStack().size(),similarity);
				cluster.addNodeItem(item,index);
			}
			return true;
		}
		virtual void finishVisit(NarratorNodeIfc & ){ }
		virtual void detectedCycle(NarratorNodeIfc & ){ }
		virtual void finish(){	}
	};

	class ClusterIterator {
	private:
		ClusterGroup & clusterGroup;
		QList<int> indicies;
		int currentIndex;
	public:
		ClusterIterator(ClusterGroup & c): clusterGroup(c) {
			assert(c.size()>0);
			for (int i=0;i<clusterGroup.size();i++) {
				indicies<<0;
			}
			currentIndex=0;
		}
		ClusterIterator & operator++() {
			if	(clusterGroup[currentIndex].size()==0 || indicies[currentIndex]==clusterGroup[currentIndex].size()-1 ||
					(clusterGroup[currentIndex].size()>0 && clusterGroup[currentIndex][indicies[currentIndex]]->stepsToCenter==1)
				) {
				indicies[currentIndex]=0;
				while (++currentIndex<clusterGroup.size() && clusterGroup[currentIndex].size()==0)
					;
				if (!isFinished()) {
					operator ++();
				#if 0
					for (int i=0;i<currentIndex;i++) {
						assert (indicies[i]==0);
					}
				#endif
					if (!isFinished())
						currentIndex=0;
				}
			} else {
				indicies[currentIndex]++;
			}
			return *this;
		}
		bool isFinished() const {
			assert(currentIndex<=clusterGroup.size());
			return currentIndex==clusterGroup.size();
		}
		ScoredCluster operator*() {
			ScoredCluster sc(clusterGroup.center,clusterGroup.getNumNarrators(),0,1);
			Cluster &c=*sc.cluster;
			ClusterScore &s=sc.score;
			for (int i=0;i<indicies.size();i++) {
				if (clusterGroup[i].size()>0) {
					NodeItem * n=clusterGroup[i][indicies[i]];
					c<<n;
					s.stepCount+=n->stepsToCenter;
					s.similarityProduct*=n->similarity;
				}
			}
			return sc;
		}
	};


	int getLargestClusterSize(Biography::NarratorNodeGroups & list) {
		Trash trash; //for no leakage

		NodeItemGroup nodeItemgroups;
		//1-transform to nodeItems:
		int listCount=list.size();
		for (int i=0;i<listCount;i++) {
			int size=list[i].size();
			if (size>0) {
				nodeItemgroups.append(NodeItemList());
				for (int j=0;j<size;j++) {
					NodeItem * node=new NodeItem(list[i][j]);
					nodeItemgroups[i].append(node);
					node->node->addBiographyIndex(i);
				}
				qSort(nodeItemgroups[i]);
			}
		}

		//find all possible clusters (from all possible cluster centers)
		ScoredCluster bestCluster(NULL,0,(unsigned int)-1,0);
		int groupSize=nodeItemgroups.size();
		for (int i=0;i<groupSize;i++) {
			for (int j=0;j<nodeItemgroups[i].size();j++) {
				NarratorNodeIfc * n=nodeItemgroups[i][j]->node;
				ClusterGroup *c =new ClusterGroup(n);
				ClusteringVisitor v(*c,nodeItemgroups);
				GraphVisitorController cont(&v,graph);
				graph->DFS_traverse(*n,cont,1,1);
				graph->DFS_traverse(*n,cont,1,-1);
				bool chosen=false;
				if (c->getNumNarrators()>=hadithParameters.bio_narr_min) {
					ClusterIterator itr(*c);
					for (;!itr.isFinished();++itr) {
						ScoredCluster cluster=*itr;
						if (bestCluster<cluster) {
							if (bestCluster.center!=NULL) {
								assert(trash.clusters.contains(bestCluster.cluster));
								trash.clusters.removeOne(bestCluster.cluster);
								delete bestCluster.cluster;
							}
							bestCluster=cluster;
							trash.clusters.append(cluster.cluster);
							chosen=true;
						} else
							delete cluster.cluster;
					}
				}
				if (chosen) {
					if (trash.clusterGroups.size()>0) {
						assert (trash.clusterGroups.size()==1);
						delete trash.clusterGroups[0];
						trash.clusterGroups.clear();
						trash.clusterGroups.append(c);
					}
				} else
					delete c;
			}
		}
		listCount=nodeItemgroups.size();
		for (int i=0;i<listCount;i++) {
			NodeItemList & l=nodeItemgroups[i];
			int size=l.size();
			for (int j=0;j<size;j++) {
				l[j]->node->clearBiographyIndicies();
			}
		}

		if (bestCluster.center==NULL)
			return 0;
	#if 1
		ScoredCluster & c=bestCluster;
		qDebug()<<"["<<c.center->CanonicalName()<<"]\n("<<c.score.narrCount<<","<<c.score.stepCount<<","<<c.score.similarityProduct<<")\n";
		for (int i=0;i<c.cluster->size();i++){
			qDebug()<<(*c.cluster)[i]->node->CanonicalName()<<"\n";
		}
		qDebug()<<"\n";
	#endif
		return c.score.narrCount;
	}

#endif

public:
#ifdef SEGMENT_BIOGRAPHY_USING_POR
	NarratorDetector(NarratorGraph * graph) {this->graph=graph; biographies=NULL; }
	NarratorDetector() {biographies=NULL;}
#else
	NarratorDetector() {graph=NULL; biographies=NULL;}
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
			if((proceedInStateMachine(stateInfo,currentBiography,currentData)==false) ||(stateInfo.nextPos>=text_size-1)) {
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
			/*if (stateInfo.previousPunctuationInfo.fullstop) {
				if (currentBiography->biography!=NULL)
					delete currentBiography->biography;
				currentBiography->biography=new Biography(graph,text,stateInfo.startPos);
			}*/


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
		#if defined(TAG_BIOGRAPHY)
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
	#if defined(TAG_BIOGRAPHY)
		prg->startTaggingText(*text);
	#endif
		while (!tester.atEnd())
		{
			Biography * s=new Biography(graph,text);
			s->deserialize(tester);
		#ifdef TEST_BIOGRAPHIES
			biographies->append(s);
		#endif
		#if defined(TAG_BIOGRAPHY)
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
			//hadith_out<<tester_Counter<<" ";
			//s->serialize(hadith_out);
		#endif
			tester_Counter++;
			s->serialize(file_biography);
		}
		chainOutput.close();
		f.close();
	#endif
	#ifndef TAG_BIOGRAPHY
		//prg->startTaggingText(*hadith_out.string()); //we will not tag but this will force a text to be written there
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
