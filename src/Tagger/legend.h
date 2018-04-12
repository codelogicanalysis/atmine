#ifndef LEGEND_H
#define LEGEND_H

class Legend
{private:
    int ID;
    QColor textColor;
    QColor bckgColor;
    bool italic;
    bool bold;
    bool underlined;
    QString source;


public:
    Legend();
    Legend(int);
    Legend(int,QColor,QColor);
    Legend(int,QColor,QColor,bool,bool,bool);
    void setID(int);
    void setTextColor(QColor);
    void setBckgColor(QColor);
    void setItalic(bool);
    void setBold(bool);
    void setUnderlined(bool);
    void setSource(QString);
    QString getSource();
    int getID();
    QColor getTextColor();
    QColor getBckgColor();
    bool isItalic();
    bool isBold();
    bool isUnderlined();
     bool operator==(const Legend &);

};

#endif
