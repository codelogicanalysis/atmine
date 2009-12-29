typedef struct word_info {
//add data
} word_info_t;


class Dictionary{


	public :

		virtual word_info_t * search(char *)=0 //search function
		
		virtual int * size()=0 //size function

		virtual bool insert (word_info_t *)=0 //insert function ...consider the cases of breakdown

		virtual bool delete_word (char *)=0 //delete function


		friend class dicIter;		

		dicIter *createIterator()const;  // Add a createIterator() member


};


class dicIter{ //Design an "iterator" class

	   
	    const Dictionary *dict;	 
		
	public:

	    dicIter(const Dictionary *s)
	    {

	    }
	    void first()
	    {

	    }
	    void next()
	    {

	    }
	    void last()
	    {

	    }
	    void previous()
	    {

	    }

	    bool isDone()
	    {

	    }
	    int currentItem()
	    {

	    }


};

dicIter *Dictionary::createIterator()const
{
  return new dicIter(this);
}


class Analyzer {
	public:
		virtual char * stemIt(char *,bool resume)=0  //stemmer function, resumes from previous point if resume=1, otherwise starts from beginning
		virtual char * buildIt(char *, bool resume )=0 //build function

};

