#include "biographyGraphUtilities.h"
#include "graph.h"


#ifdef SEGMENT_BIOGRAPHY_USING_POR
bool isRealNarrator(NarratorGraph * graph, Narrator * n,Biography::NarratorNodeList & list) {
	RealNarratorAction v(list);
	v.resetFound();
	graph->performActionToAllCorrespondingNodes(n,v);
	return v.isFound();
}
#endif
