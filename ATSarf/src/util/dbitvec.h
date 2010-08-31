#ifndef _ATMINE_DBITVEC_H_
#define _ATMINE_DBITVEC_H_
/*!
file: dbitvec.h
date: mar 3, 2010
author: fadi 

brief: provides a bitvector with [] access and dynamic size

  */

#include <string.h> // for memset

class dbitvec {
	unsigned int size;
	unsigned int bytes;
	unsigned char * data;
	public:
	typedef enum {
		NONE, MEM_EXCPT,BIT_BOUND_EXCPT
	} exception_em;

	dbitvec(): size(0), bytes(0), data(0) { }
	void resize(unsigned int length);
	dbitvec(unsigned int length): size(length) {
		bytes = (size+7) >> 3; // (ceiling(size/8))
		data = new unsigned char [bytes];
		if (data == NULL)
			throw MEM_EXCPT;
		reset();
	}
	dbitvec(const dbitvec & v1 ): size(v1.length()) {
		bytes = v1.getNumBytes();
		data = new unsigned char [bytes];
		if (data == NULL)
			throw MEM_EXCPT;
		memcpy(data, v1._data(), bytes);
	}
	~dbitvec()	{
		delete [] data;
	}
	unsigned char * _data() const {
		return data;
	}
	unsigned int getNumBytes() const {
		return bytes;
	}
	unsigned int length() const {
		return size;
	}
	void reset() {
		memset(data, 0, bytes);
	}
	//TODO: resize, copy constructor
	bool getBit(unsigned int i) const {
		if (i >= size)
			throw BIT_BOUND_EXCPT;
		unsigned int byte = i >> 3; // i / 8
		unsigned int bit = i & 0x7; // i % 8
		unsigned char mask = 1<< bit;
		return (data[byte] & mask ) != 0;
	}
	void setBit(unsigned int i, bool b = true) const {
		if (i >= size)
			throw BIT_BOUND_EXCPT;
		unsigned int byte = i >> 3; // i / 8
		unsigned int bit = i & 0x7; // i % 8
		unsigned char mask = 1<< bit;
		data [byte] = (b) ?
			(data[byte] | mask )  :
			(data[byte] & (~mask));
	}
	bool NothingSet();

	class Byte
	{ public:
		unsigned int bit;//offset within the byte
		dbitvec * vec;// pointer to vector
		Byte(unsigned int i, dbitvec * v) :bit(i),  vec(v) {}
		operator bool () {
			return vec->getBit(bit);
		}
		Byte & operator = (bool b) {
			vec->setBit(bit, b);
			return *this;
		}
	};
	Byte operator [] (unsigned int bit) {
		return Byte(bit, this);
	}
	dbitvec& operator=(const dbitvec& v1)
	{
	  bytes = v1.getNumBytes();
	  data = new unsigned char [bytes];
	  if (data == NULL)
		  throw MEM_EXCPT;
	  memcpy(data, v1._data(), bytes);
	  return *this;
	}
	void show();
};
inline bool operator == (const dbitvec & d1, const dbitvec & d2) //by Jad, TODO: check if could be made faster
{
#if 0
	return strcmp((const char*)d1._data(),(const char *)d2._data())==0;
#else
	unsigned int bytes1=d1.getNumBytes();
	if (bytes1!=d2.getNumBytes())
		return false;
#if 1
	return (memcmp(d1._data(),d2._data(),bytes1)==0);
#else
	for (unsigned int i=0;i<bytes1;i++)
		if (d1._data()[i]!=d2._data()[i])
			return false;
	return true;
#endif
#endif
}
inline bool operator != (const dbitvec & d1, const dbitvec &d2)
{
	return !(d1==d2);
}
#endif
