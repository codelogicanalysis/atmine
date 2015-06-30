# Introduction #

The narrator order graph contains a merger of all chains of narrators.
Two narrators are merged if they have a low distance (high identity) score.
The graph should allow and maintain an entry exit correspondence, this can be done by keeping a pointer to the narrators in their chain lists and providing parent/parents, child/children as interface functions.


# Details #

The graph is multi-rooted. We should keep a list of all rank 1 narrators. The leaves are the hadiths themselves.
We will consider a hash table record of all narrators where a merged narrator with X identified chain narrators will have X entries in the table pointing to it.

We will consider later different lookup and search mechanisms in the graph.

We will also consider a mechanism to visualize the graph.