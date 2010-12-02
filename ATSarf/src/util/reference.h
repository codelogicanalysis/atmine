#ifndef REFERENCE_H
#define REFERENCE_H
#include <QDebug>

template <class T>
class Reference
{
private:
	T * object;
	bool destroy;
public:
	Reference( T & object,bool destroy)
	{
		//qDebug()<< "c "<<(long)this<<"-->"<<(long)&object;
		this->object=&object;
		this->destroy=destroy;
	}
	Reference<T> & operator==(Reference<T> & c)
	{
		object=c.object;
		destroy=c.destroy;
		return *this;
	}
	T & Rfc()
	{
		return *object;
	}
	~Reference()
	{
		if (destroy)
		{
			//qDebug()<< "d "<<(long)this<<"-->"<<(long)&object;
			delete object;
		}
		else
		{
			//qDebug()<< "n "<<(long)this<<"-->"<<(long)&object;
		}
	}
};
#endif // REFERENCE_H
