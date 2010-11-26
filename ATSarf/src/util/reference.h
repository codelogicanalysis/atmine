#ifndef REFERENCE_H
#define REFERENCE_H

template <class T>
class Reference
{
private:
	T * object;
	bool destroy;
public:
	Reference( T & object,bool destroy)
	{
		this->object=&object;
		this->destroy=destroy;
	}
	T & Rfc()
	{
		return *object;
	}
	~Reference()
	{
		if (destroy)
			delete object;
	}
};
#endif // REFERENCE_H
