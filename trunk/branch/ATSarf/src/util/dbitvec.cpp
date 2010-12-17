/*!
file: dbitvec.h
date: mar 3, 2010
author: fadi

brief: provides a bitvector with [] access and dynamic size

  */
#include "dbitvec.h"
#include <string.h> // for memset


/*
dbitvec::dbitvec() : size(0), bytes(0), data(0) { }
*/
void dbitvec::resize(unsigned int length)  {
	unsigned int newbytes = (length+7) >> 3;
	unsigned char * newdata =  new unsigned char [newbytes];

	if (newdata == NULL)
		throw MEM_EXCPT;

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
/*
dbitvec::dbitvec(unsigned int length) : size(length) {
	bytes = (size+7) >> 3; // (ceiling(size/8))
	data = new unsigned char [bytes];
	if (data == NULL)
		throw MEM_EXCPT;
	reset();
}

dbitvec::dbitvec(const dbitvec & v1 ) : size(v1.length()) {
	bytes = v1.getNumBytes();
	data = new unsigned char [bytes];
	if (data == NULL)
		throw MEM_EXCPT;
	memcpy(data, v1._data(), bytes);
}

dbitvec::~dbitvec() {
	delete [] data;
}
unsigned char * dbitvec::_data() const {
	return data;
}

unsigned int dbitvec::getNumBytes() const {
	return bytes;
}
unsigned int dbitvec::length() const {
	return size;
}

void dbitvec::reset() {
	memset(data, 0, bytes);
}

//TODO: resize, copy constructor

bool dbitvec::getBit(unsigned int i) const {
	if (i >= size)
		throw BIT_BOUND_EXCPT;
	unsigned int byte = i >> 3; // i / 8
	unsigned int bit = i & 0x7; // i % 8
	unsigned char mask = 1<< bit;
	return (data[byte] & mask ) != 0;
}

void dbitvec::setBit(unsigned int i, bool b) const {
	if (i >= size)
		throw BIT_BOUND_EXCPT;
	unsigned int byte = i >> 3; // i / 8
	unsigned int bit = i & 0x7; // i % 8
	unsigned char mask = 1<< bit;
	data [byte] = (b) ?
		(data[byte] | mask )  :
		(data[byte] & (~mask));
}

dbitvec::Byte::Byte(unsigned int i, dbitvec * v) :bit(i),  vec(v) {}

dbitvec::Byte::operator bool () {
	return vec->getBit(bit);
}

dbitvec::Byte & dbitvec::Byte::operator = (bool b) {
	vec->setBit(bit, b);
	return *this;
}

dbitvec::Byte dbitvec::operator [] (unsigned int bit) {
	return Byte(bit, this);
}

// by Jad:

bool operator == (const dbitvec & d1, const dbitvec & d2)
{
#if 0
	return strcmp((const char*)d1._data(),(const char *)d2._data())==0;
#else
	unsigned int bytes1=d1.getNumBytes();
	if (bytes1!=d2.getNumBytes())
		return false;
	for (unsigned int i=0;i<bytes1;i++)
		if (d1._data()[i]!=d2._data()[i])
			return false;
	return true;
#endif
}
bool operator != (const dbitvec & d1, const dbitvec & d2)
{
	return !(d1==d2);
}
*/
bool dbitvec::NothingSet()
{
	dbitvec temp(length());
	return *this==temp;
}
/*
dbitvec& dbitvec::operator=(const dbitvec& v1)
{
	bytes = v1.getNumBytes();
	data = new unsigned char [bytes];
	if (data == NULL)
		throw MEM_EXCPT;
	memcpy(data, v1._data(), bytes);
	return *this;
}
*/
#if 1
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
