/*!
file: dbitvec.h
date: mar 3, 2010
author: fadi

brief: provides a bitvector with [] access and dynamic size

  */
#include "dbitvec.h"
#include <string.h> // for memset


	dbitvec::dbitvec() : size(0), bytes(0), data(0) { }

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

	class dbitvec::Byte
        { public:
		unsigned int bit;//offset within the byte
		dbitvec * vec;// pointer to vector
		Byte(unsigned int i, dbitvec * v) :
			bit(i),  vec(v) {}
		operator bool () {
			return vec->getBit(bit);
		}

		Byte & operator = (bool b) {
			vec->setBit(bit, b);
			return *this;
		}
	};

	dbitvec::Byte dbitvec::operator [] (unsigned int bit) {
		return Byte(bit, this);
	}



