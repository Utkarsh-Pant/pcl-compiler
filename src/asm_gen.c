#include "asm_gen.h"

const char* ASM_Map[TAC_COUNT] = {
        #define ASM_MAP(tac, asm) [tac] = asm,
        #include "asm.def"
};

static void resolve_op(struct TACArg arg, char* dest){

        if(arg.is_literal){
        	if(arg.is_string){
			sprintf(dest, "%s", arg.data.value.s);
			return;
		}
		sprintf(dest, "%d", arg.data.value.i);
                return;
        }

        if(arg.data.arg == NULL){
                sprintf(dest, "");
                return;
        }

        if(arg.data.arg->data.v.is_global)
                sprintf(dest, "%s [rip + %s]",
                               arg.data.arg->type == KWD_INT? "DWORD PTR" : "BYTE PTR",
                                arg.data.arg->name);
        else{
                sprintf(dest, "%s [rbp%+d]",
                               arg.data.arg->type == KWD_INT? "DWORD PTR" : "BYTE PTR",
                                arg.data.arg->data.v.stack_offset);
        }

}

extern struct Stream* global_string_stream;
extern size_t global_string_stream_count;

static void gen_rodata(FILE* fptr){
	if(global_string_stream == NULL || global_string_stream_count == 0) return;
	fprintf(fptr, "         .section .rodata\n");
	
	struct StreamIterator it = stream_getIterator(global_string_stream);
	for (size_t i = 0; i < it.n; i++) {
        fprintf(fptr, ".LC%zu:\n", i);
        fprintf(fptr, "         .string \"%s\"\n", (char*)it.arr[i]);
    	}
    	fprintf(fptr, "\n");
}

static void gen_bss(struct SymbolTable* table, FILE* fptr){
        fprintf(fptr, ".bss\n");

        struct StreamIterator it = stream_getIterator(table->keys);
        for (size_t i = 0; i < it.n; i++) {
                struct TableRecord* rec = hmap_get(table->hmap, it.arr[i]);

                if (rec != NULL && !rec->is_func) {
                        fprintf(fptr, "%s:\n", rec->name);

                        if (rec->type == KWD_INT) fprintf(fptr, "    .zero 4\n");
                        else if (rec->type == KWD_CHR) fprintf(fptr, "    .zero 1\n");
                }
        }
        fprintf(fptr, "\n");

}

static void gen_load_op(FILE* fptr, struct TACArg arg, const char* op_str) {
    if (arg.is_literal) fprintf(fptr, "         mov eax, %s\n", op_str);
    else if (arg.data.arg->type == KWD_CHR) fprintf(fptr, "         movsx eax, %s\n", op_str);
    else fprintf(fptr, "         mov eax, %s\n", op_str);
}

static void gen_store_op(FILE* fptr, struct TableRecord* res, const char* res_str) {
    if (res == NULL) return;

    if (res->type == KWD_CHR)
        fprintf(fptr, "         mov %s, al\n", res_str);
    else
        fprintf(fptr, "         mov %s, eax\n", res_str);

}

static void gen_cmp(struct TAC tac, const char* op1, const char* op2, const char* res, const char* name, FILE* fptr) {
    gen_load_op(fptr, tac.arg1, op1);
    if (!tac.arg2.is_literal && tac.arg2.data.arg->type == KWD_CHR) {
        fprintf(fptr, "         movsx ecx, %s\n", op2);
        fprintf(fptr, "         cmp eax, ecx\n");
    } else {
        fprintf(fptr, "         cmp eax, %s\n", op2);
    }
    fprintf(fptr, "         %s al\n", name);
    fprintf(fptr, "         movzx eax, al\n");
    gen_store_op(fptr, tac.res, res);
}


static void gen_TAC_COPY(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr) {
    gen_load_op(fptr, tac.arg1, op1);
    gen_store_op(fptr, tac.res, res);
}

static void gen_TAC_CAST(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr) {
    gen_load_op(fptr, tac.arg1, op1);
    gen_store_op(fptr, tac.res, res);
}

static void gen_TAC_DIV(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr) {
    gen_load_op(fptr, tac.arg1, op1);
    fprintf(fptr, "         cdq\n");
    if (tac.arg2.is_literal) fprintf(fptr, "         mov ecx, %s\n", op2);
    else if (tac.arg2.data.arg->type == KWD_CHR) fprintf(fptr, "         movsx ecx, %s\n", op2);
    else fprintf(fptr, "         mov ecx, %s\n", op2);
    fprintf(fptr, "         idiv ecx\n");
    gen_store_op(fptr, tac.res, res);
}

static void gen_TAC_MOD(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr) {
    gen_load_op(fptr, tac.arg1, op1);
    fprintf(fptr, "         cdq\n");
    if (tac.arg2.is_literal) fprintf(fptr, "         mov ecx, %s\n", op2);
    else if (tac.arg2.data.arg->type == KWD_CHR) fprintf(fptr, "         movsx ecx, %s\n", op2);
    else fprintf(fptr, "         mov ecx, %s\n", op2);
    fprintf(fptr, "         idiv ecx\n");
    fprintf(fptr, "         mov eax, edx\n");
    gen_store_op(fptr, tac.res, res);
}

static void gen_TAC_LSHIFT(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr) {
    gen_load_op(fptr, tac.arg1, op1);
    if (tac.arg2.is_literal) fprintf(fptr, "         mov ecx, %s\n", op2);
    else if (tac.arg2.data.arg->type == KWD_CHR) fprintf(fptr, "         movsx ecx, %s\n", op2);
    else fprintf(fptr, "         mov ecx, %s\n", op2);
    fprintf(fptr, "         sal eax, cl\n");
    gen_store_op(fptr, tac.res, res);
}

static void gen_TAC_RSHIFT(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr) {
    gen_load_op(fptr, tac.arg1, op1);
    if (tac.arg2.is_literal) fprintf(fptr, "         mov ecx, %s\n", op2);
    else if (tac.arg2.data.arg->type == KWD_CHR) fprintf(fptr, "         movsx ecx, %s\n", op2);
    else fprintf(fptr, "         mov ecx, %s\n", op2);
    fprintf(fptr, "         sar eax, cl\n");
    gen_store_op(fptr, tac.res, res);
}


static void gen_TAC_CMP_EQ(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr)  { gen_cmp(tac, op1, op2, res, "sete", fptr); }
static void gen_TAC_CMP_NEQ(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr) { gen_cmp(tac, op1, op2, res, "setne", fptr); }
static void gen_TAC_CMP_LT(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr)  { gen_cmp(tac, op1, op2, res, "setl", fptr); }
static void gen_TAC_CMP_GT(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr)  { gen_cmp(tac, op1, op2, res, "setg", fptr); }
static void gen_TAC_CMP_LTE(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr) { gen_cmp(tac, op1, op2, res, "setle", fptr); }
static void gen_TAC_CMP_GTE(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr) { gen_cmp(tac, op1, op2, res, "setge", fptr); }

static void gen_TAC_LABEL(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr) {
    fprintf(fptr, ".L%d:\n", tac.tac_id);
}

static void gen_TAC_JMP(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr) {
    fprintf(fptr, "         jmp .L%d\n", tac.tac_id);
}

static void gen_TAC_JMF(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr) {
    gen_load_op(fptr, tac.arg1, op1);
    fprintf(fptr, "         cmp eax, 0\n");
    fprintf(fptr, "         je .L%d\n", tac.tac_id);
}

static void gen_TAC_NOT(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr) {
    gen_load_op(fptr, tac.arg1, op1);
    fprintf(fptr, "         cmp eax, 0\n");
    fprintf(fptr, "         sete al\n");
    fprintf(fptr, "         movzx eax, al\n");
    gen_store_op(fptr, tac.res, res);
}

static void gen_TAC_NEG(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr) {
    gen_load_op(fptr, tac.arg1, op1);
    fprintf(fptr, "         neg eax\n");
    gen_store_op(fptr, tac.res, res);
}

static void gen_TAC_BIT_NOT(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr) {
    gen_load_op(fptr, tac.arg1, op1);
    fprintf(fptr, "         not eax\n");
    gen_store_op(fptr, tac.res, res);
}

static void gen_TAC_ADDR(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr) {
    fprintf(fptr, "         lea rax, %s\n", op1);
    gen_store_op(fptr, tac.res, res);
}

static void gen_TAC_PARAM(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr) {
    const char* reg64[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    const char* reg32[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};

    int idx = tac.tac_id;
    
    if (idx > 5) {
        fprintf(fptr, "         # Error: Stack arguments (>6) not implemented yet\n");
        return;
    }
    
    if (tac.arg1.is_literal && tac.arg1.is_string) {
        fprintf(fptr, "         lea %s, [rip + %s]\n", reg64[idx], op1);
    } 
    else if (tac.arg1.is_addr_of) {
        if (tac.arg1.data.arg->data.v.is_global) {
            fprintf(fptr, "         lea %s, [rip + %s]\n", reg64[idx], tac.arg1.data.arg->name);
        } else {
            fprintf(fptr, "         lea %s, [rbp%+d]\n", reg64[idx], tac.arg1.data.arg->data.v.stack_offset);
        }
    } 
    else {
        gen_load_op(fptr, tac.arg1, op1);
        fprintf(fptr, "         mov %s, eax\n", reg32[idx]);
    }
    
}

static void gen_TAC_CALL(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr) {
    fprintf(fptr, "         mov eax, 0\n");

    fprintf(fptr, "         call %s\n", tac.arg1.data.arg->name);

    if (tac.res != NULL) {
        gen_store_op(fptr, tac.res, res);
    }
}
static void gen_default(struct TAC tac, const char* op1, const char* op2, const char* res, FILE* fptr){

        gen_load_op(fptr, tac.arg1, op1);
        if (!tac.arg2.is_literal && tac.arg2.data.arg->type == KWD_CHR){
                fprintf(fptr, "         movsx ecx, %s\n", op2);
                fprintf(fptr, "         %s eax, ecx\n", ASM_Map[tac.op]);
        }
        else{
                fprintf(fptr, "         %s eax, %s\n", ASM_Map[tac.op], op2);
        }
        gen_store_op(fptr, tac.res, res);
}

static void generate_asm_tac(struct TACList* list, struct SymbolTable* table, FILE* fptr){

        for(int i = 0; i<list->n; i++){
                struct TAC tac = list->arr[i];

                char op1[64];
                char op2[64];
                char res[64];

                resolve_op(tac.arg1, op1);
                resolve_op(tac.arg2, op2);

                struct TACArg res_op; // dummy
                res_op.is_literal = false;
                res_op.data.arg=tac.res;

                resolve_op(res_op, res);


                switch(tac.op){
                        #define CUSTOM_GEN(tac) case tac: gen_##tac(list->arr[i], op1, op2, res, fptr); break;
                        #include "asm.def"
                        default: gen_default(tac, op1, op2, res, fptr);
                                 break;
                }
        }
}

void gen_asm(struct TACList* list, struct SymbolTable* table, const char* filename){

        FILE* fptr = fopen(filename,"w");
	fprintf(fptr, "         .intel_syntax noprefix\n");
	
	gen_rodata(fptr);
        gen_bss(table, fptr);

	//Safety Check:
	if(table->total_bytes > table->max_bytes) table->max_bytes = table->total_bytes;

        int stack_size = (table->max_bytes +15)& ~15;
        fprintf(fptr, "         .text\n");
        fprintf(fptr, "         .globl main\n");
        fprintf(fptr, "main:\n");
        fprintf(fptr, "         push rbp\n");
        fprintf(fptr, "         mov rbp, rsp\n");

        if(stack_size>0) fprintf(fptr, "                sub rsp, %d\n", stack_size);
        fprintf(fptr, "\n");


        generate_asm_tac(list, table, fptr);


        fprintf(fptr, "         mov eax, 0\n");
        fprintf(fptr, "         mov rsp, rbp\n");
        fprintf(fptr, "         pop rbp\n");
        fprintf(fptr, "         ret\n");

	fprintf(fptr, ".section .note.GNU-stack,\"\",@progbits\n");
	fclose(fptr);
}
