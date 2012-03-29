#include "morphemes.h"
#include "database_info_block.h"

int bit_PROC3, bit_PROC2, bit_PROC1, bit_PROC0, bit_DET, bit_ENC0;
int * morpheme_bits[morpheme_bits_size]= {&bit_PROC3, &bit_PROC2, &bit_PROC1, &bit_PROC0, &bit_DET, &bit_ENC0};

void morpheme_initialize() {
	for (int i=0;i<morpheme_bits_size;i++) {
		long abstract_id=database_info.comp_rules->getAbstractCategoryID(morpheme_abstract_desc[i]);
		(*morpheme_bits[i])=database_info.comp_rules->getAbstractCategoryBitIndex(abstract_id);
	}
}
