#ifndef SARFTAGTYPE_H
#define SARFTAGTYPE_H
#include <QString>
#include <QVector>
#include <QPair>
class SarfTagType
{
public:
    SarfTagType();
    SarfTagType( QString, QVector< QPair< QString , QString > >, QString, int, QString, QString, int, bool, bool, bool);
//private:
    QString tag;
    QVector< QPair< QString , QString > > tags;
    QString description;
    int id;
    QString fgcolor;
    QString bgcolor;
    int font;
    bool underline;
    bool bold;
    bool italic;
};

#endif // SARFTAGTYPE_H
