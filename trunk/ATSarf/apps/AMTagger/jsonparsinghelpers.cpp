#include "jsonparsinghelpers.h"

void processText(QString *text) {

    _atagger->wordIndexMap.clear();
    _atagger->isStatementEndFSSet.clear();
    _atagger->isStatementEndPSet.clear();
    int start = 0;
    int wordIndex = 1;
    while(start != text->count()) {
        Word word = nextWord(*text, start);
        if(word.isStatementStartFS && wordIndex != 1) {
            _atagger->isStatementEndFSSet.insert(wordIndex-1);
        }
        if(word.isStatementStartPunct && wordIndex != 1) {
            _atagger->isStatementEndPSet.insert(wordIndex-1);
        }
        if(word.word.isEmpty()) {
            break;
        }

        _atagger->wordIndexMap.insert(word.start,wordIndex);
        start = word.end + 1;
        wordIndex = wordIndex + 1;
    }
    _atagger->wordCount = wordIndex - 1;
}

bool process_TagTypes(QByteArray &tagtypedata) {
    QJson::Parser parser;
    bool ok;

    QVariantMap result = parser.parse (tagtypedata,&ok).toMap();

    if (!ok) {
        return false;
    }

    foreach(QVariant type, result["TagTypeSet"].toList()) {
        QString tag;
        QString desc;
        QString foreground_color;
        QString background_color;
        int font;
        bool underline = false;
        bool bold;
        bool italic;

        QVariantMap typeElements = type.toMap();

        tag = typeElements["Tag"].toString();
        desc = typeElements["Description"].toString();
        foreground_color = typeElements["foreground_color"].toString();
        background_color = typeElements["background_color"].toString();
        font = typeElements["font"].toInt();
        //underline = typeElements["underline"].toBool();
        bold = typeElements["bold"].toBool();
        italic = typeElements["italic"].toBool();

        if(!typeElements.value("Features").isNull()) {

            QVector < Quadruple< QString , QString , QString , QString > > tags;
            foreach(QVariant sarfTags, typeElements["Features"].toList()) {
                QVariantMap st = sarfTags.toMap();
                Quadruple< QString , QString , QString , QString > quad;
                if(!(st.value("Prefix").isNull())) {
                    quad.first = "Prefix";
                    quad.second = st.value("Prefix").toString();
                    if(!(st.value("Negation").isNull())) {
                        quad.third = st.value("Negation").toString();
                    }
                    if(!(st.value("Relation").isNull())) {
                        quad.fourth = st.value("Relation").toString();
                    }
                    tags.append(quad);
                }
                else if(!(st.value("Stem").isNull())) {
                    quad.first = "Stem";
                    quad.second = st.value("Stem").toString();
                    if(!(st.value("Negation").isNull())) {
                        quad.third = st.value("Negation").toString();
                    }
                    if(!(st.value("Relation").isNull())) {
                        quad.fourth = st.value("Relation").toString();
                    }
                    tags.append(quad);
                }
                else if(!(st.value("Suffix").isNull())) {
                    quad.first = "Suffix";
                    quad.second = st.value("Suffix").toString();
                    if(!(st.value("Negation").isNull())) {
                        quad.third = st.value("Negation").toString();
                    }
                    if(!(st.value("Relation").isNull())) {
                        quad.fourth = st.value("Relation").toString();
                    }
                    tags.append(quad);
                }
                else if(!(st.value("Prefix-POS").isNull())) {
                    quad.first = "Prefix-POS";
                    quad.second = st.value("Prefix-POS").toString();
                    if(!(st.value("Negation").isNull())) {
                        quad.third = st.value("Negation").toString();
                    }
                    if(!(st.value("Relation").isNull())) {
                        quad.fourth = st.value("Relation").toString();
                    }
                    tags.append(quad);
                }
                else if(!(st.value("Stem-POS").isNull())) {
                    quad.first = "Stem-POS";
                    quad.second = st.value("Stem-POS").toString();
                    if(!(st.value("Negation").isNull())) {
                        quad.third = st.value("Negation").toString();
                    }
                    if(!(st.value("Relation").isNull())) {
                        quad.fourth = st.value("Relation").toString();
                    }
                    tags.append(quad);
                }
                else if(!(st.value("Suffix-POS").isNull())) {
                    quad.first = "Suffix-POS";
                    quad.second = st.value("Suffix-POS").toString();
                    if(!(st.value("Negation").isNull())) {
                        quad.third = st.value("Negation").toString();
                    }
                    if(!(st.value("Relation").isNull())) {
                        quad.fourth = st.value("Relation").toString();
                    }
                    tags.append(quad);
                }
                else if(!(st.value("Prefix-Gloss").isNull())) {
                    quad.first = "Prefix-Gloss";
                    quad.second = st.value("Prefix-Gloss").toString();
                    if(!(st.value("Negation").isNull())) {
                        quad.third = st.value("Negation").toString();
                    }
                    if(!(st.value("Relation").isNull())) {
                        quad.fourth = st.value("Relation").toString();
                    }
                    tags.append(quad);
                }
                else if(!(st.value("Stem-Gloss").isNull())) {
                    quad.first = "Stem-Gloss";
                    quad.second = st.value("Stem-Gloss").toString();
                    if(!(st.value("Negation").isNull())) {
                        quad.third = st.value("Negation").toString();
                    }
                    if(!(st.value("Relation").isNull())) {
                        quad.fourth = st.value("Relation").toString();
                    }
                    tags.append(quad);
                }
                else if(!(st.value("Suffix-Gloss").isNull())) {
                    quad.first = "Suffix-Gloss";
                    quad.second = st.value("Suffix-Gloss").toString();
                    if(!(st.value("Negation").isNull())) {
                        quad.third = st.value("Negation").toString();
                    }
                    if(!(st.value("Relation").isNull())) {
                        quad.fourth = st.value("Relation").toString();
                    }
                    tags.append(quad);
                }
                else if(!(st.value("Category").isNull())) {
                    quad.first = "Category";
                    quad.second = st.value("Category").toString();
                    if(!(st.value("Negation").isNull())) {
                        quad.third = st.value("Negation").toString();
                    }
                    if(!(st.value("Relation").isNull())) {
                        quad.fourth = st.value("Relation").toString();
                    }
                    tags.append(quad);
                }
            }

            _atagger->isSarf = true;
            _atagger->insertSarfTagType(tag,tags,desc,foreground_color,background_color,font,underline,bold,italic,sarf,original);
        }
        else {
            _atagger->isSarf = false;
            _atagger->insertTagType(tag,desc,foreground_color,background_color,font,underline,bold,italic,user,original);
        }
    }

    if(!(result.value("MSFs").isNull())) {
        /** The tagtype file contains MSFs **/

        foreach(QVariant msfsData, result.value("MSFs").toList()) {

            /** List of variables for each MSFormula **/
            QString bgcolor;
            QString fgcolor;
            QString name;
            QString init;
            //QString after;
            QString actions;
            QString includes;
            QString members;
            QString description;
            int i;
            int usedCount;
            bool isFullStop;

            /** This is an MSFormula **/
            QVariantMap msformulaData = msfsData.toMap();

            name = msformulaData.value("name").toString();
            init = msformulaData.value("init").toString();
            //after = msformulaData.value("after").toString();
            actions = msformulaData.value("actions").toString();
            includes = msformulaData.value("includes").toString();
            members = msformulaData.value("members").toString();
            description = msformulaData.value("description").toString();
            fgcolor = msformulaData.value("fgcolor").toString();
            bgcolor = msformulaData.value("bgcolor").toString();
            i = msformulaData.value("i").toInt();
            usedCount = msformulaData.value("usedCount").toInt();
            if(!(msformulaData.value("delimiter").isNull())) {
                isFullStop = msformulaData.value("delimiter").toBool();
            }
            else {
                isFullStop = true;
            }

            MSFormula* msf = new MSFormula(name, NULL);
            msf->isFullStop = isFullStop;
            msf->includes = includes;
            msf->members = members;
            msf->fgcolor = fgcolor;
            msf->bgcolor = bgcolor;
            msf->description = description;
            msf->i = i;
            msf->usedCount = usedCount;
            _atagger->msfVector->append(msf);

            /** Get MSFormula MSFs **/
            foreach(QVariant msfData, msformulaData.value("MSFs").toList()) {
                readMSF(msf, msfData, msf);
            }

            /** Get relations **/
            if(!(msformulaData.value("Relations").isNull())) {
                foreach(QVariant relationsData, msformulaData.value("Relations").toList()) {
                    QVariantMap relationData = relationsData.toMap();
                    QString relationName = relationData.value("name").toString();
                    QString e1Label = relationData.value("e1Label").toString();
                    QString e2Label = relationData.value("e2Label").toString();
                    QString edgeLabel = relationData.value("edgeLabel").toString();

                    QString entity1_Name = relationData.value("entity1").toString();
                    MSF* entity1 = msf->map.value(entity1_Name);
                    QString entity2_Name = relationData.value("entity2").toString();
                    MSF* entity2 = msf->map.value(entity2_Name);
                    MSF* edge = NULL;
                    if(!(relationData.value("edge").isNull())) {
                        QString edge_Name = relationData.value("edge").toString();
                        edge = msf->map.value(edge_Name);
                    }
                    Relation* relation = new Relation(relationName,entity1,e1Label,entity2,e2Label,edge,edgeLabel);
                    msf->relationVector.append(relation);
                }
            }
        }
    }
    return true;
}

bool readMSF(MSFormula* formula, QVariant data, MSF *parent) {
    /** Common variables in MSFs **/
    QString name;
    QString init;
    //QString after;
    QString actions;
    QString returns;
    QString parentName;
    QString type;

    QVariantMap msfData = data.toMap();
    name = msfData.value("name").toString();
    init = msfData.value("init").toString();
    //after = msfData.value("after").toString();
    actions = msfData.value("actions").toString();
    returns = msfData.value("returns").toString();
    parentName = msfData.value("parent").toString();
    type = msfData.value("type").toString();

    if(type == "mbf") {
        /** This is MBF **/
        QString bf = msfData.value("MBF").toString();
        bool isF = msfData.value("isFormula").toBool();

        /** initialize MBF **/
        MBF* mbf = new MBF(name,parent,bf,isF);
        mbf->init = init;
        //mbf->after = after;
        mbf->actions = actions;
        mbf->returns = returns;
        formula->map.insert(name, mbf);

        /** Check parent type and add accordingly **/
        if(parent->isFormula()) {
            MSFormula* prnt = (MSFormula*)parent;
            prnt->addMSF(mbf);
        }
        else if(parent->isUnary()) {
            UNARYF* prnt = (UNARYF*)parent;
            prnt->setMSF(mbf);
        }
        else if(parent->isBinary()) {
            BINARYF* prnt = (BINARYF*)parent;
            if(prnt->leftMSF == NULL) {
                prnt->setLeftMSF(mbf);
            }
            else {
                prnt->setRightMSF(mbf);
            }
        }
        else if(parent->isSequential()) {
            SequentialF* prnt = (SequentialF*)parent;
            prnt->addMSF(mbf);
        }
        else {
            return false;
        }

        return true;
    }
    else if(type == "unary") {
        /** This is a UNARY formula **/
        QString operation = msfData.value("op").toString();
        Operation op;
        if(operation == "?") {
            op = KUESTION;
        }
        else if(operation == "*") {
            op = STAR;
        }
        else if(operation == "+") {
            op = PLUS;
        }
        else if(operation.contains('^')) {
            op = UPTO;
        }
        else {
            return false;
        }
        int limit = -1;
        if(operation.contains('^')) {
            bool ok;
            limit = operation.mid(1).toInt(&ok);
            if(!ok) {
                return false;
            }
        }

        /** Initialize a UNARYF **/
        UNARYF* uf = new UNARYF(name,parent,op,limit);
        uf->init = init;
        //uf->after = after;
        uf->actions = actions;
        uf->returns = returns;
        formula->map.insert(name,uf);

        /** Check parent type and add accordingly **/
        if(parent->isFormula()) {
            MSFormula* prnt = (MSFormula*)parent;
            prnt->addMSF(uf);
        }
        else if(parent->isUnary()) {
            UNARYF* prnt = (UNARYF*)parent;
            prnt->setMSF(uf);
        }
        else if(parent->isBinary()) {
            BINARYF* prnt = (BINARYF*)parent;
            if(prnt->leftMSF == NULL) {
                prnt->setLeftMSF(uf);
            }
            else {
                prnt->setRightMSF(uf);
            }
        }
        else if(parent->isSequential()) {
            SequentialF* prnt = (SequentialF*)parent;
            prnt->addMSF(uf);
        }
        else {
            return false;
        }

        /** Proceed with child MSF **/
        return readMSF(formula,msfData.value("MSF"),uf);
    }
    else if(type == "binary") {
        /** This is a BINARY formula **/
        QString operation = msfData.value("op").toString();
        Operation op;
        if(operation == "&") {
            op = AND;
        }
        else if(operation == "|") {
            op = OR;
        }
        else {
            return false;
        }

        /** Initialize BINARYF **/
        BINARYF* bif = new BINARYF(name,parent,op);
        bif->init = init;
        //bif->after = after;
        bif->actions = actions;
        bif->returns = returns;
        formula->map.insert(name, bif);

        /** Check parent type and add accordingly **/
        if(parent->isFormula()) {
            MSFormula* prnt = (MSFormula*)parent;
            prnt->addMSF(bif);
        }
        else if(parent->isUnary()) {
            UNARYF* prnt = (UNARYF*)parent;
            prnt->setMSF(bif);
        }
        else if(parent->isBinary()) {
            BINARYF* prnt = (BINARYF*)parent;
            if(prnt->leftMSF == NULL) {
                prnt->setLeftMSF(bif);
            }
            else {
                prnt->setRightMSF(bif);
            }
        }
        else if(parent->isSequential()) {
            SequentialF* prnt = (SequentialF*)parent;
            prnt->addMSF(bif);
        }
        else {
            return false;
        }

        /** Proceed with child MSFs **/
        bool first = readMSF(formula,msfData.value("leftMSF"),bif);
        bool second = readMSF(formula,msfData.value("rightMSF"),bif);
        if(first && second) {
            return true;
        }
        else {
            return false;
        }
    }
    else if(type == "sequential") {
        /** This is a sequential formula **/
        /** Initialize a SequentialF **/
        SequentialF* sf = new SequentialF(name,parent);
        sf->init = init;
        //sf->after = after;
        sf->actions = actions;
        sf->returns = returns;
        formula->map.insert(name, sf);

        /** Check parent type and add accordingly **/
        if(parent->isFormula()) {
            MSFormula* prnt = (MSFormula*)parent;
            prnt->addMSF(sf);
        }
        else if(parent->isUnary()) {
            UNARYF* prnt = (UNARYF*)parent;
            prnt->setMSF(sf);
        }
        else if(parent->isBinary()) {
            BINARYF* prnt = (BINARYF*)parent;
            if(prnt->leftMSF == NULL) {
                prnt->setLeftMSF(sf);
            }
            else {
                prnt->setRightMSF(sf);
            }
        }
        else if(parent->isSequential()) {
            SequentialF* prnt = (SequentialF*)parent;
            prnt->addMSF(sf);
        }
        else {
            return false;
        }

        /** Proceed with children **/
        foreach(QVariant seqMSFData, msfData.value("MSFs").toList()) {
            if(!(readMSF(formula,seqMSFData,sf))) {
                return false;
            }
        }

        return true;
    }
    else {
        return false;
    }
}

bool readMatch(MSFormula* formula, QVariant data, Match* parent) {
    /** Common variables in a match **/
    QString type;
    QString msfName;

    QVariantMap matchData = data.toMap();
    type = matchData.value("type").toString();
    msfName = matchData.value("msf").toString();

    if(type == "key") {
        int pos = matchData.value("pos").toInt();
        int length = matchData.value("length").toInt();
        QString key = matchData.value("key").toString();
        QString word = matchData.value("word").toString();
        MSF* msf = formula->map.value(msfName);
        KeyM* keym = new KeyM(parent,key,pos,length);
        keym->word = word;
        keym->msf = msf;
        parent->setMatch(keym);
        return true;
    }
    else if(type == "unary") {
        MSF* msf = formula->map.value(msfName);
        int limit = matchData.value("limit").toInt();
        Operation op = (Operation)(matchData.value("op").toInt());
        UnaryM* unary = new UnaryM(op,parent,limit);
        unary->msf = msf;
        foreach(QVariant unaryMatchData, matchData.value("matches").toList()) {
            if(!(readMatch(formula,unaryMatchData,unary))) {
                return false;
            }
        }
        parent->setMatch(unary);
        return true;
    }
    else if(type == "binary") {
        MSF* msf = formula->map.value(msfName);
        Operation op = (Operation)(matchData.value("op").toInt());
        BinaryM* binary = new BinaryM(op,parent);
        binary->msf = msf;
        QVariant leftMatchData = matchData.value("leftMatch");
        if(!(readMatch(formula,leftMatchData,binary))) {
            return false;
        }
        if(!(matchData.value("rightMatch").isNull())) {
            QVariant rightMatchData = matchData.value("rightMatch");
            if(!(readMatch(formula,rightMatchData,binary))) {
                return false;
            }
        }
        parent->setMatch(binary);
        return true;
    }
    else if(type == "sequential") {
        SequentialM* seq = new SequentialM(parent);
        if(msfName == "_NULL_") {
            seq->msf = NULL;
        }
        else {
            MSF* msf = formula->map.value(msfName);
            seq->msf = msf;
        }
        foreach(QVariant seqMatchData, matchData.value("matches").toList()) {
            if(!(readMatch(formula,seqMatchData,seq))) {
                return false;
            }
        }
        parent->setMatch(seq);
        return true;
    }
    else {
        /// MERFTag case
        MERFTag* merftag = new MERFTag();
        MSF* msf = formula->map.value(msfName);
        merftag->msf = msf;
        QString formulaName = matchData.value("formula").toString();
        for(int i=0; i<_atagger->msfVector->count(); i++) {
            if(_atagger->msfVector->at(i)->name == formulaName) {
                merftag->formula = _atagger->msfVector->at(i);
                break;
            }
        }
        QVariant merftagMatchData = matchData.value("match");
        if(!(readMatch(formula,merftagMatchData,merftag))) {
            return false;
        }
        parent->setMatch(merftag);
        return true;
    }
    return false;
}

bool simulateMBF() {

    _atagger->isSarf = true;

    QString text = _atagger->text;

    /** Process Tag Type and Create Hash function for Synonymity Sets **/

    QHash< QString, QSet<QString> > synSetHash;

    for( int i=0; i< (_atagger->tagTypeVector->count()); i++) {

        /** Check if tag source is sarf tag types **/
        if(_atagger->tagTypeVector->at(i)->source != sarf) {
            continue;
        }

        const SarfTagType * tagtype = (SarfTagType*)(_atagger->tagTypeVector->at(i));
        for(int j=0; j < (tagtype->tags.count()); j++) {
            const Quadruple< QString , QString , QString , QString > * tag = &(tagtype->tags.at(j));
            if(tag->fourth.contains("Syn")) {

                int order = tag->fourth.mid(3).toInt();
                GER ger(tag->second,1,order);
                ger();

                QString gloss_order = tag->second;
                gloss_order.append(QString::number(order));
                QSet<QString> glossSynSet;
                for(int i=0; i<ger.descT.count(); i++) {
                    const IGS & igs = ger.descT[i];
                    glossSynSet.insert(igs.getGloss());
                }
                synSetHash.insert(gloss_order,QSet<QString>(glossSynSet));
            }
        }
    }

    /** Synonymity Sets Created **/

    /** Process Text and analyse each work using sarf **/

    AutoTagger autotag(&text,&synSetHash);
    autotag();

    _atagger->isTagMBF = true;

    return true;
}
