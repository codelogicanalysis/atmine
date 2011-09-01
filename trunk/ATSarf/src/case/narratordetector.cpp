#include <QPair>
#include <QInputDialog>
#include "narratordetector.h"
#include "graph.h"
#include "narratorHash.h"
#include "mergeLists.h"


class NarratorDetector
{
private:
	typedef Biography::MatchingNode MatchingNode;

	StateData currentData;
	QString * text;
	long current_pos;

	NarratorGraph * graph;
	ATMProgressIFC *prg;

public:
	BiographyList * biographies;
	QList<int> nodeIds;

private:

#ifdef SEGMENT_BIOGRAPHY_USING_POR
#ifndef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY
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
#else
	typedef QList<Narrator*> NarratorList;
	NarratorHash hash;
	NarratorList narratorList;

	class ScoredSet {
	public:
		QSet<int> setBioNarrIndicies;
		QSet<NarratorNodeIfc*> setGraphNodes;

		double score;

		ScoredSet(double score) {
			this->score=score;
		}
	};
	typedef QList<ScoredSet> NodeGroups;
	class NodePositionMap {
	private:
		typedef QMap<NarratorNodeIfc *,int> Map;
		int max;
		Map map;
	public:
		NodePositionMap() {
			max=0;
		}
		int getIndex(NarratorNodeIfc * node) {
			Map::const_iterator i = map.find(node);
			if (i != map.end()) {
				return i.value();
			} else {
				map[node]=max;
				max++;
				return max-1;
			}
		}
	};
	typedef QList<int> IndexList;
	typedef QList<IndexList *> IndexLists;

	void getSortedIndexList(NarratorNodeIfc * node,IndexList & list) {
		assert(!node->isGroupNode());
		IndexLists lists;
		FindIndiciesAction action(node,lists);
		if (node->isGraphNode()) {
			for (int i=0;i<node->size();i++) {
				hash.performActionToAllCorrespondingNodes(&(*node)[i],action);
			}
		} else {
			ChainNarratorNode & c_node=dynamic_cast<ChainNarratorNode &>(*node);
			hash.performActionToAllCorrespondingNodes(&c_node,action);
		}
	#ifndef ASSUMESORTED
		for (int i=0;i<lists.size();i++) {
			qSort(*lists[i]);
		}
	#endif
		MergeLists<int> merge(lists,list);
		merge.mergeLists();
		for (int i=1;i<list.size();i++) {
			assert(list[i-1]<=list[i]);
		}
	}

	class GetNeighborIndexListsVisitor: public NodeVisitor {
	private:
		NarratorDetector & detector;
		NarratorNodeIfc * center;
		IndexLists & indexlists;
	public:
		GetNeighborIndexListsVisitor(NarratorDetector & aDetector,NarratorNodeIfc * center, IndexLists & lists)
				:detector(aDetector),indexlists(lists) {
			this->center=center;
		}
		virtual void initialize(){}
		virtual bool visit(NarratorNodeIfc & n){
			if (&n!=center) {
				indexlists.append(new IndexList);
				detector.getSortedIndexList(&n,*indexlists.last());
			}
			return true;
		}
		virtual void visit(NarratorNodeIfc & ,NarratorNodeIfc & , int ){}
		virtual void finish(){}
		virtual void detectedCycle(NarratorNodeIfc & ){}
		virtual void finishVisit(NarratorNodeIfc & ){}
	};

	class FindIndiciesAction: public NarratorHash::BiographyAction {
	private:
		NarratorNodeIfc * node;
		NodePositionMap indexMap;
		IndexLists & lists;
	public:
		FindIndiciesAction(NarratorNodeIfc * node, IndexLists & indexLists): lists(indexLists){
			this->node=node;
		}
		void action(const QString &, GroupNode *gnode, int index, double similarity) {
			NarratorNodeIfc * n=&gnode->getCorrespondingNarratorNode();
			assert(n!=NULL);
			if (n==this->node) {
				int listIndex=indexMap.getIndex(gnode);
				if (listIndex>=lists.size()) {
					assert(listIndex==lists.size());
					IndexList * PositionList=new IndexList;
					lists.append(PositionList);
				}
			#ifdef ASSUMESORTED
				if (lists[listIndex]->size()>0)
					assert(index>lists[listIndex]->last());
			#endif
				lists[listIndex]->append(index);
			}
		}
	};

	bool updateGroups(ScoredSet & set,NodeGroups & groups) { //return true if changes occured
		#define MAX_SIZE 3
		if (set.score>=hadithParameters.bio_threshold) {
			bool add=false;
			for (int i=0;i<groups.size();i++) {
				if (groups[i].score<set.score) {
					add=true;
				} else if (add) {
					groups.insert(i,set);
					break;
				}
			}
			if (groups.size()>MAX_SIZE) {
				assert(add);
				assert(groups.size()==MAX_SIZE+1);
				groups.removeLast();
			} else if (!add && groups.size()<MAX_SIZE) {
				groups.append(set);
				return true;
			}
			return add;
		}
		return false;
	}

	void addNarrators(Biography* biography) {
		Biography::NarratorNodeGroups & realNodes=biography->nodeGroups;
		int listCount=realNodes.size();
		for (int i=0;i<listCount;i++) {
			int size=realNodes[i].size();
			if (size>0) {
				int index=narratorList.size();
				narratorList.append((*biography)[i]);
				qSort(realNodes[i]);
				for (int j=0;j<size;j++) {
					Biography::MatchingNode m_node=realNodes[i][j];
					assert(m_node.node->isGroupNode());
					GroupNode * node=dynamic_cast<GroupNode *>(m_node.node);
					hash.addNode(node,index);
				}
			}
		}
	}

	inline bool areNear(int index1,int index2) {
		int diff1=narratorList[index1]->getEnd()-narratorList[index2]->getStart(),
			diff2=narratorList[index1]->getStart()-narratorList[index2]->getEnd();
		return absVal(diff1)<hadithParameters.bio_nrc_max ||
			absVal(diff2)<hadithParameters.bio_nrc_max;
	}

	void segmentChosenBiography(NarratorNodeIfc * node,NodeGroups & topSets) {
		topSets.clear();
		IndexList centerIndicies;
		getSortedIndexList(node,centerIndicies);

		IndexLists indexLists;
		GetNeighborIndexListsVisitor visitor(*this,node,indexLists);
		GraphVisitorController cont(&visitor,NULL);
		node->BFS_traverse(cont,1,1);
		node->BFS_traverse(cont,1,-1);

		prg->setCurrentAction("Checking Neighborhood for '"+node->CanonicalName()+"'");
		prg->report(0);
		for (int i=0;i<centerIndicies.size();i++) {
			int centerIndex=centerIndicies[i];
			ScoredSet set(0);
			set.setBioNarrIndicies.insert(centerIndex);
			for (int j=0;j<indexLists.size();j++) {
				IndexList & neighboringIndicies=*indexLists[j];
				IndexList::const_iterator itr=qLowerBound(neighboringIndicies,centerIndex);
				for (int i=0;i<2;i++) {
					if (itr!=neighboringIndicies.end()) {
						int neighborIndex=*itr;
						if (areNear(centerIndex,neighborIndex)) {
							if (!set.setBioNarrIndicies.contains(neighborIndex)) {
								set.score++;
								set.setBioNarrIndicies.insert(neighborIndex);
								break;
							}
						}
					}
					if (itr!=neighboringIndicies.begin())
						itr--;
				}
			}
			updateGroups(set,topSets);
			prg->report((double)i/centerIndicies.size()*100+0.5);
		}
		for (int i=0; i<indexLists.size();i++) {
			delete indexLists[i];
		}
		prg->setCurrentAction("Complete");
		prg->report(100);
	}

	int getNextRealNodeId(int id) {
		assert (id>=0);
		while (id<graph->all_nodes.size()) {
			if (!graph->all_nodes[id]->isActualNode())
				id++;
			else
				return id;
		}
		return -1;
	}

#endif

#endif

public:
#ifdef SEGMENT_BIOGRAPHY_USING_POR
	NarratorDetector(NarratorGraph * graph)
	#ifdef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY
		:hash(graph)
	#endif
	{
		this->graph=graph; biographies=NULL;
	}
	NarratorDetector()
	#ifdef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY
		:hash(NULL)
	#endif
	{
		biographies=NULL;
	}
#else
	NarratorDetector() {graph=NULL; biographies=NULL;}
#endif
	int segment(QString input_str,ATMProgressIFC *prg)  {
		this->prg=prg;
		QFile chainOutput(chainDataStreamFileName);

		chainOutput.remove();
		if (!chainOutput.open(QIODevice::ReadWrite))
			return 1;
	#ifndef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY
		QDataStream chainOut(&chainOutput);
	#endif
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
		currentData.initialize();

	#ifdef CHAIN_BUILDING
		HadithData *currentBiography=new HadithData(text,false,graph);
		currentBiography->initialize(text);
		//display(QString("\ninit%1\n").arg(currentBiography->narrator->m_narrator.size()));
	#else
		chainData *currentBiography=NULL;
	#endif
	#if !defined(SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY)
		long  biographyStart=-1;
		long  biographyEnd;
		int biography_Counter=1;
	#endif

		StateInfo stateInfo;
		stateInfo.resetCurrentWordInfo();
		stateInfo.currentState=TEXT_S;
		stateInfo.nextState=TEXT_S;
		stateInfo.lastEndPos=0;
		stateInfo.startPos=0;
		stateInfo.processedStructure=INITIALIZE;
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
			#if defined(SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY) && defined(SEGMENT_BIOGRAPHY_USING_POR)
				addNarrators(currentBiography->biography);
			#else
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
				} else
					delete currentBiography->biography;
			#endif
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
		prg->report(100);
	#if defined(DISPLAY_HADITH_OVERVIEW) && !defined(SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY)
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
	#ifdef CHAIN_BUILDING
		int tester_Counter=1;
	#ifdef TEST_BIOGRAPHIES
		biographies=new BiographyList;
		biographies->clear();
	#endif
	#ifndef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY
		if (!chainOutput.open(QIODevice::ReadWrite))
			return 1;
		QDataStream tester(&chainOutput);
	#else
		if (nodeIds.size()==0) {
			graph->printAllNodesList();
			bool ok;
			do {
				int i = QInputDialog::getInt(NULL, QString("Find Biography for:"),
											QString("Id of Node "),0, 0, graph->all_nodes.size()-1,1, &ok);
				 if (ok)
					 nodeIds<<i;
			} while (ok);
			for (int i=0;i<nodeIds.size();i++) {
				int id=getNextRealNodeId(nodeIds[i]);
				if (id>=0) {
					nodeIds[i]=id;
					qDebug()<<graph->all_nodes[id]->toString();
				} else {
					nodeIds.removeAt(i);
					i--;
				}
			}
		}
		assert(nodeIds.size()>0);
	#endif
	#if defined(TAG_BIOGRAPHY)
		prg->startTaggingText(*text);
	#endif
	#ifndef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY
		while (!tester.atEnd()) {
			Biography * s=new Biography(graph,text);
			s->deserialize(tester);
		#ifdef TEST_BIOGRAPHIES
			biographies->append(s);
		#endif
			for (int j=0;j<s->size();j++) {
				const Narrator * n=(*s)[j];
	#else
		for (int i=0;i<nodeIds.size();i++) {
			NodeGroups topSets;
			segmentChosenBiography(graph->getNode(nodeIds[i]),topSets);
			for (int k=0;k<topSets.size();k++) {
				Biography * bio=new Biography(graph,text);
				biographies->append(bio);
				QSet<int>::iterator itr= topSets[k].setBioNarrIndicies.begin();
				int j=0;
				for (;itr!=topSets[k].setBioNarrIndicies.end();itr++) {
					Narrator * n=narratorList[*itr];
					bio->addRealNarrator(n);//addNarrator(n);
					j++;
	#endif
			#ifdef TAG_BIOGRAPHY
					if (n->m_narrator.size()==0) {
						out<<"found a problem an empty narrator in ("<<tester_Counter<<","<<j<<")\n";
						continue;
					}
				#ifndef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY
					if (!s->isReal(j))
						prg->tag(n->getStart(),n->getLength(),Qt::darkGray,false);
					else
				#endif
						prg->tag(n->getStart(),n->getLength(),Qt::darkYellow,false);
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
			#endif
				tester_Counter++;

			}
	#ifdef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY
		}
	#else
		chainOutput.close();
	#endif
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
	#ifdef SEGMENT_AFTER_PROCESSING_ALL_BIOGRAPHY
		for (int i=0;i<narratorList.size();i++)
			delete narratorList[i];
	#endif
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
BiographyList * getBiographies(QString input_str,NarratorGraph* graph,ATMProgressIFC *prg, int nodeID) {
	input_str=input_str.split("\n")[0];
#ifdef SEGMENT_BIOGRAPHY_USING_POR
	NarratorDetector s(graph);
#else
	NarratorDetector s;
#endif
	if (nodeID>=0)
		s.nodeIds.append(nodeID);
	s.segment(input_str,prg);
	return s.biographies;
}
#endif
