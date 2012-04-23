#ifndef INSTANCEITERATOR_H
#define INSTANCEITERATOR_H

class InstanceIterator {
public:
	virtual void start()=0;
	virtual bool getInstance(double & value, double & weight)const=0; //returns true if this value is valid, false if all instances read
	virtual void next()=0;
};

#endif // INSTANCEITERATOR_H
