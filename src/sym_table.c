#include "sym_table.h"
#include "tree.h"

static size_t size_chart[TOKEN_TYPE_COUNT] = {
	#define SIZE_MAP(kwd, size) [kwd] = size,
	#include "semantic_mappings.def"
};

static struct TableRecord* record_create(struct AST* node, TOKEN_TYPE type){
	if(node->type != IDENT) return NULL; // For now processing only identifiers, but keeping options open
	struct TableRecord* new_record = malloc(sizeof(struct TableRecord));

	new_record->name = node->value.s;
	new_record->type = type;
	new_record->is_func = false; // Currently not supporting function definitions. (Might Change later)
	new_record->data.v.stack_offset = 0;
	new_record->data.v.assigned = false;

	return new_record;
}

struct SymbolTable* symtable_init(struct SymbolTable* parent){
	struct SymbolTable* new_table = malloc(sizeof(struct SymbolTable));

	new_table->hmap = hmap_init();
	new_table->parent = parent;
	new_table->keys = stream_init();

	if(parent==NULL) new_table->total_bytes = 0;
	else new_table->total_bytes = parent->total_bytes;

	new_table->max_bytes = new_table->total_bytes;

	return new_table;
}

struct TableRecord* symtable_get(struct SymbolTable* table, struct AST* node){
	
	if(node->type != IDENT) return NULL; // For now processing only identifiers, but keeping options open
	
	const char* name = node->value.s;
	struct TableRecord* result = NULL;

	while(table!=NULL){
		result = (struct TableRecord*) hmap_get(table->hmap, name);
		if(result!=NULL) break;
		table = table->parent;
	};

	return result;
}

struct TableRecord* symtable_get_local(struct SymbolTable* table, struct AST* node){
	if(node->type != IDENT) return NULL;
	const char* name = node->value.s;
	return (struct TableRecord*) hmap_get(table->hmap, name);
}

struct TableRecord* symtable_add(struct SymbolTable* table, struct AST* node, TOKEN_TYPE type){
	if(node->type != IDENT) return NULL;
	struct TableRecord* new = record_create(node, type);

	char* name = node->value.s;
	table->total_bytes += size_chart[type];
	new->data.v.stack_offset = -(table->total_bytes);	

	struct TableRecord* old = hmap_add(table->hmap, name, new);
	if(old == NULL) stream_append(table->keys, name);

	return new;
}

void symtable_print(struct SymbolTable* table){
	printf("TOTAL BYTES: %lu\n", table->total_bytes);
	printf("HAS PARENT: %s\n", (table->parent == NULL) ? "NO" : "YES" );

	struct StreamIterator it = stream_getIterator(table->keys);
	for(size_t i = 0; i<it.n; i++){
		char* name = it.arr[i];
		struct TableRecord* record = (struct TableRecord*) hmap_get(table->hmap, name);

		if(record!=NULL) printf("%s : %s : %d : %d : %d\n", name, record->name, record->type, record->data.v.stack_offset, record->data.v.assigned);
	}	
}
