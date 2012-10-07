#ifndef SARFTAGTYPE_H
#define SARFTAGTYPE_H
#include <QString>
#include <QVector>
#include <QPair>
#include "tagtype.h"

class SarfTagType : public TagType {

public:
    SarfTagType();
    SarfTagType( QString, QVector< QPair< QString , QString > >, QString, int, QString, QString, int, bool, bool, bool, Source);
//private:
    //QString tag;
    QVector< QPair< QString , QString > > tags;
    /*
    QString description;
    int id;
    QString fgcolor;
    QString bgcolor;
    int font;
    bool underline;
    bool bold;
    bool italic;
    */
};

#endif // SARFTAGTYPE_H
