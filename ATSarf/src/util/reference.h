#ifndef REFERENCE_H
#define REFERENCE_H
#include <QDebug>

template <class T>
class Reference
{
protected:
	class _data
	{
	public:
		T * object;
		bool destroy;
		int refcount;
		Reference<T> * head;

		_data(T * o, bool d,int r,Reference<T> * t):object(o),destroy(d),refcount(r), head(t) {}
		bool remove(Reference<T> * o)
		{
			refcount--;
			Reference<T> * curr=head;
			if (o==head)
			{
				head=head->next;
				return true;
			}
			while (curr->next!=NULL)
			{
				if (curr->next==o)
				{
					curr->next=o->next;
					return true;
				}
				curr=curr->next;
			}
			refcount++; //should never happen
			return false;
		}
	} * _d;
	Reference<T> * next;
	friend class _data;
public:
	Reference( T & object,bool destroy=false)
	{
		//qDebug()<< "c "<<(long)this<<"-->"<<(long)&object;
		_d=new _data(&object,destroy,1,this);
		next=NULL;
	}
	Reference<T> & operator=(const Reference<T> & c)
	{
		_d=c._d;
		next=_d->head;
		_d.head=this;
		_d->refcount++;
		return *this;
	}
	T * getObject()
	{
		return _d->object;
	}
	T & operator * ()
	{
		return Rfc();
	}
	T * operator -> ()
	{
		return getObject();
	}
	T & Rfc()
	{
		return *getObject();
	}
	~Reference()
	{
		_d->remove(this);
		if ( _d->refcount==0)
		{
			if (_d->destroy )
			{
				//qDebug()<< "d "<<(long)this<<"-->"<<(long)&object;
				delete getObject();
			}
			delete _d;
		}
	}
};
#endif // REFERENCE_H
