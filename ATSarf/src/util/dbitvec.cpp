/*!
file: dbitvec.h
date: mar 3, 2010
author: fadi

brief: provides a bitvector with [] access and dynamic size

  */
#include "dbitvec.h"
#include <string.h> // for memset


void dbitvec::resize(unsigned int length)  {
	unsigned int newbytes = (length+7) >> 3;
	unsigned char * newdata =  new unsigned char [newbytes];

        if (newdata == NULL) {
                qDebug()<<"line 17 cpp\n";
		throw MEM_EXCPT;
            }

	if (size == 0) {
		size = length;
		bytes = newbytes;
		data = newdata;
		reset();
		return;
	}
	memset(newdata, 0, newbytes);
	memcpy (newdata, data, ( bytes < newbytes)? bytes : newbytes);

	size = length;
	bytes = newbytes;
	delete [] data;

	data = newdata;
}

bool dbitvec::NothingSet()
{
	dbitvec temp(length());
	return *this==temp;
}

#if 0
#include <QString>
#include <QDebug>
void dbitvec::show()
{
	QString r;
	for (int i=size-1;i>=0;i--)
		r.append(getBit(i)?"1":"0");
	qDebug() <<r;
}
#else
void dbitvec::show(){}
#endif

#if 0
#include <QFile>
void testStreaming()
{
	dbitvec v(113),b;
	v.setBit(2);
	v.setBit(105);
	QFile file("temp");
	if (file.open(QIODevice::WriteOnly))
	{
		QDataStream out(&file);   // we will serialize the data into the file
		out<< v;
		file.close();
	}
	else
		error <<"Unexpected Error";
	QFile file1("temp");
	if (file1.open(QIODevice::ReadOnly))
	{
		QDataStream in(&file1);    // read the data serialized from the file
		in>> b;
		file1.close();
	}
	assert(b==v);
	return 0;
}
#endif
