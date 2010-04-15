#include "StemNode.h"
#include "../utilities/text_handling.h"
#include <QtAlgorithms>

void StemNode::add_info(const long cat_id)
{
	#if !defined(REDUCE_THRU_DIACRITICS) && !defined(MEMORY_EXHAUSTIVE)
		QVector<long>::iterator i =   qLowerBound(category_ids.begin(),category_ids.end(), cat_id);
		if (*i!=cat_id)
			category_ids.insert(i, cat_id);
	#elif defined(REDUCE_THRU_DIACRITICS)
		add_info(cat_id,key);
	#endif
}
bool StemNode::exists(const long cat_id)
{
	QVector<long>::iterator i = qBinaryFind(category_ids.begin(),category_ids.end(), cat_id);
	return (i!=category_ids.end());
}
#ifdef REDUCE_THRU_DIACRITICS
	void StemNode::add_info(long cat_id,QString raw_data)
	{
		QVector<long>::iterator i = qLowerBound(category_ids.begin(),category_ids.end(), cat_id);
		int index=i-category_ids.begin();//maybe does not work for 64-bit machine
		if (*i!=cat_id)
		{
			category_ids.insert(index, cat_id);
			RawDatasEntry ent;
			ent.append(raw_data);
			raw_datas.insert(index,ent);
		}
		else
		{
			if (!raw_datas.at(index).contains(raw_data))
				raw_datas[index].append(raw_data);
		}
	}
	void StemNode::add_info(long cat_id,RawDatasEntry raw_data_entry)
	{
		QVector<long>::iterator i = qLowerBound(category_ids.begin(),category_ids.end(), cat_id);
		int index=i-category_ids.begin();//maybe does not work for 64-bit machine
		if (*i!=cat_id)
		{
			category_ids.insert(index, cat_id);
			RawDatasEntry ent;
			raw_datas.insert(index,ent);
			for (int j=0;j<raw_data_entry.size();j++)
				raw_datas[index].append(raw_data_entry[j]);
		}
		else
		{
			for (int j=0;j<raw_data_entry.size();j++)
				raw_datas[index].append(raw_data_entry[j]);
		}
	}
	bool StemNode::exists(long cat_id, QString raw_data)
	{
		QVector<long>::iterator i = qBinaryFind(category_ids.begin(),category_ids.end(), cat_id);
		if (i!=category_ids.end())
		{
			int index=i-category_ids.begin();//maybe does not work for 64-bit machine
			for (int j=0;j<raw_datas[index].size();j++)
				if (equal(raw_datas[index][j],raw_data))
					return true;
			return false;
		}
		else
			return false;
	}
	bool StemNode::get(long cat_id, RawDatasEntry &raw_data_entry)
	{
		QVector<long>::iterator i = qBinaryFind(category_ids.begin(),category_ids.end(), cat_id);
		if (i!=category_ids.end())
		{
			int index=i-category_ids.begin();//maybe does not work for 64-bit machine
			raw_data_entry=this->raw_datas[index];
			return true;
		}
		else
			return false;
	}
#endif
