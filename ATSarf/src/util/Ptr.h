/**
  * @file   Ptr.h
  * @brief  this header file contains a user defined pointer class
  * @author Jad Makhlouta
  */
#ifndef PTR_H
#define PTR_H

/**
  * @class  Ptr
  * @author Jad Makhlouta
  * @brief  This class is a user defined pointer with special functionalities implemented for it
  */
template<class T>
class Ptr{
	public:
                T * p;  //!< initializing a regular pointer with template type
                /**
                  * This is the default constructor of the Ptr class
                  */
		inline Ptr(): p(NULL) {}
                /**
                  * This is a constructor where a pointer to an element is passed
                  */
		inline Ptr(T* t): p(t) {}
                /**
                  * This is the definition of an overloaded operator '<' where it
                  * compares both pointers' variable values
                  * @return true if first less than second else false
                  */
		inline bool operator< (const Ptr<T> & rhs) const {
			return (*p) < (*(rhs.p));
		}
                /**
                  * This is the definition of an overloaded operator '!=' where it
                  * compares both pointer's variable values for equality
                  * @return true if first is equal to second
                  */
		inline bool operator!= (const Ptr<T> & rhs) const {
			return (*p) != (*(rhs.p));
		}
                /**
                  * This is the definition of an overloaded operator '->' where it
                  * returns the pointer
                  * @return pointer present in class
                  */
		inline T* operator -> () { return p;}
                /**
                  * This is the definition of an overloaded operator '*' where it
                  * returns content of pointer's variable
                  * @return content of variable pointed to
                  */
		inline T& operator * () { return *p;}
};

#endif // PTR_H
