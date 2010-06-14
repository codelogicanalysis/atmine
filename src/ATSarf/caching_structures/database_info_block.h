#ifndef DATABASE_INFO_H
#define DATABASE_INFO_H

#include "compatibility_rules.h"
#include "../common_structures/tree.h"
#include "../common_structures/atmTrie.h"
#include "../common_structures/common.h"
#include <QString>
#include <QDateTime>

#ifdef GUI_SPECIFIC
#include "../gui/mainwindow.h"
#include "ui_mainwindow.h"
#endif

class database_info_block
{
    public:
        tree* Prefix_Tree;
        tree* Suffix_Tree;
#ifdef USE_TRIE
		ATTrie * Stem_Trie;
		QVector<StemNode> * trie_nodes;
#endif
        compatibility_rules * rules_AA;
        compatibility_rules * rules_AB;
        compatibility_rules * rules_AC;
        compatibility_rules * rules_BC;
        compatibility_rules * rules_CC;
        database_info_block();
#ifdef GUI_SPECIFIC
		void fill(Ui::MainWindow *m_ui);
#else
		void fill();
#endif
        ~database_info_block();
};

extern database_info_block database_info;
extern QString trie_path;
extern QString trie_list_path;
extern QDateTime executable_timestamp;

#endif // DATABASE_INFO_H
