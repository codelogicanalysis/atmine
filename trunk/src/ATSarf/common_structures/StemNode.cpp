#include "StemNode.h"
#include "../utilities/text_handling.h"
#include <QtAlgorithms>
#include <QDebug>

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
	void StemNode::add_info(long cat_id,RawDatasEntry raw_data_entry)
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

//Search_StemNode
	Search_StemNode::Search_StemNode()
	{
		//implement later
	}
	Search_StemNode::Search_StemNode(StemNode * node)
	{
		setNode(node);
	}
	void Search_StemNode::setNode(StemNode * node)
	{
		this->node=node;
		if (node==NULL)
			throw "cannot search a null pointer";
		cat_index=0;
#ifdef REDUCE_THRU_DIACRITICS
		rawdata_index=0;
#endif
	}
	bool Search_StemNode::retrieve(long &category_id)
	{
		if (cat_index<node->category_ids.size())
		{
			category_id=node->category_ids[cat_index];
			cat_index++;
			return true;
		}
		else
			return false;
	}
#ifdef REDUCE_THRU_DIACRITICS
	bool Search_StemNode::retrieve(minimal_item_info & info)
	{
		if (cat_index<node->category_ids.size())
		{
			if (rawdata_index<node->raw_datas[cat_index].size())
			{
				info.category_id=node->category_ids[cat_index];
				info.raw_data=node->raw_datas[cat_index][rawdata_index];
				info.type=STEM;
				info.POS=QString::null;
				info.abstract_categories=INVALID_BITSET;
				info.description=QString::null;
				rawdata_index++;
				return true;
			}
			else
			{
				cat_index++;
				rawdata_index=0;
				return retrieve(info);
			}
		}
		else
			return false;
	}
#endif

#endif
