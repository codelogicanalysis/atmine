#ifndef PTR_H
#define PTR_H

template<class T>
class Ptr{
	public:
		T * p;
		inline Ptr(): p(NULL) {}
		inline Ptr(T* t): p(t) {}
		inline bool operator< (const Ptr<T> & rhs) const {
			return (*p) < (*(rhs.p));
		}
		inline bool operator!= (const Ptr<T> & rhs) const {
			return (*p) != (*(rhs.p));
		}
		inline T* operator -> () { return p;}
		inline T& operator * () { return *p;}
};

#endif // PTR_H
