

string** segment_word(string str){
string ** a=new strin 
string segmented;
prefix_len=0;
suffix_len=0;
str_len=str.length();

	while(prefix_len<=4){
		prefix=str.substr(0,prefix_len);
		stem_len=str_len-prefix_len;
		suffix_len=0
		while ((stem_len>=1) and (suffix_len<=6)){
			
			stem=str.substr (prefix_len. stem_len);
		    suffix = str.substr((prefix_len + stem_len), suffix_len);

			//push the segmented word parts
			if (prefix_exists (prefix)){
				if (suffix_exists(suffix){
					if (stem_exists(stem){
						# all 3 components exist in their respective lexicons, but are they compatible? (check the $cat pairs)
						char **prefix_array, **suffix_array;
						id_category * stem_array;//create a new struct
						prefix_count=get_prefix_cats(prefix, prefix_array);
						stem_count=get_stem_cats (stem, stem_array);
						suffix_count=get_suffix_cats (suffix, prefix_suffix);
						
						for (int i=0; i<prefix_count;i++){
							for (int j=0; j<stem_count;j++){
								for (int k=0; k<suffix_count;k++){
									cat_a=prefix_array[i];
									cat_b=stem_array[j];
									cat_c=suffix_array[k];
									if (A_B_C_compatible(cat_a,cat_b,cat_c))
										get_and_output_info_id (stem_id)


								}
							}	
						}	
					}
				}
			}
			
			stem_len--;
			suffix_len++;
		}
		prefix_len++
	}
		
}
