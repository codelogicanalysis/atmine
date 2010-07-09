#ifndef _ATMINE_DBITVEC_H_
#define _ATMINE_DBITVEC_H_
/*!
file: dbitvec.h
date: mar 3, 2010
author: fadi 

brief: provides a bitvector with [] access and dynamic size

  */

class dbitvec {
	unsigned int size;
	unsigned int bytes;
	unsigned char * data;
	public:
	typedef enum {
		NONE, MEM_EXCPT,BIT_BOUND_EXCPT
	} exception_em;

	dbitvec();
	void resize(unsigned int length);
	dbitvec(unsigned int length);
	dbitvec(const dbitvec & v1 );
	~dbitvec();
	unsigned char * _data() const;
	unsigned int getNumBytes() const;
	unsigned int length() const;
	void reset() ;
	//TODO: resize, copy constructor
	bool getBit(unsigned int i) const;
	void setBit(unsigned int i, bool b = true) const;
	class Byte
	{ public:
		unsigned int bit;//offset within the byte
		dbitvec * vec;// pointer to vector
		Byte(unsigned int i, dbitvec * v);
		operator bool () ;
		Byte & operator = (bool b) ;
	};
	Byte operator [] (unsigned int bit);
};
#endif
