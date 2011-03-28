#include "StemNode.h"
#include "text_handling.h"
#include <QtAlgorithms>
#include <QVector>
#include <QList>
#include <QDataStream>

void StemNode::add_info(const long cat_id)
{
	#if !defined(REDUCE_THRU_DIACRITICS) && !defined(MEMORY_EXHAUSTIVE)
		QVector<long>::iterator i =   qLowerBound(category_ids.begin(),category_ids.end(), cat_id);
		if (i==category_ids.end() || *i!=cat_id)
			category_ids.insert(i, cat_id);
	#elif defined(REDUCE_THRU_DIACRITICS)
		add_info(cat_id,key);
	#endif
}
#ifdef REDUCE_THRU_DIACRITICS
	void StemNode::add_info(long cat_id,QString & raw_data)
	{
		QVector<long>::iterator i = qLowerBound(category_ids.begin(),category_ids.end(), cat_id);
		int index=i-category_ids.begin();//maybe does not work for 64-bit machine
		if (i==category_ids.end() || *i!=cat_id)
		{
			category_ids.insert(index, cat_id);
			RawDatasEntry ent;
			ent.append(raw_data);
			raw_datas.insert(index,ent);
		}
		else
		{
			//qDebug() << raw_datas.size();
			if (!raw_datas[index].contains(raw_data))
				raw_datas[index].append(raw_data);
		}
	}
	void StemNode::add_info(long cat_id,RawDatasEntry & raw_data_entry)
	{
		QVector<long>::iterator i = qLowerBound(category_ids.begin(),category_ids.end(), cat_id);
		int index=i-category_ids.begin();//maybe does not work for 64-bit machine
		if (i==category_ids.end() || *i!=cat_id)
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

#endif
