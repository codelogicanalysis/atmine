#include <iostream>
using namespace std;
#include <Qstring>
#include <QDebug>
#include <QFile>
#include <qtextcodec.h>
int main(){


//qDebug()<<name;

QFile file("in.txt");
if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    return 0;

QTextCodec *utf8_codec  = QTextCodec::codecForName("utf-8");


QTextStream in(&file);

//in.setEncoding(QTextStream::Unicode);
in.setCodec(utf8_codec);
QString line;

while (!in.atEnd()) {
    line = in.readLine();
}

QFile file2("out.txt");
if (!file2.open(QIODevice::WriteOnly | QIODevice::Text))
         return 0;

QTextStream out(&file2);
//out.setEncoding(QTextStream::Unicode);
out.setCodec(utf8_codec);

out << "The magic name is: " << line << "\n";
out<<line.length();

return 0;
}

