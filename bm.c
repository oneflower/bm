#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>

#define ARRAY_SIZE(xs) (sizeof(xs) / sizeof((xs)[0]))
#define BM_STACK_CAPACITY 1024
#define BM_PROGRAM_CAPACITY 1024
#define BM_EXECUTION_LIMIT 69

typedef enum {
  ERR_OK = 0,
  ERR_STACK_OVERFLOW,
  ERR_STACK_UNDERFLOW,
  ERR_ILLEGAL_INST,
  ERR_ILLEGAL_INST_ACCESS,
  ERR_DIV_BY_ZERO,
} Err;

const char *err_as_cstr(Err err)
{
  switch (err) {
  case ERR_OK:
    return "ERR_OK";
  case ERR_STACK_OVERFLOW:
    return "ERR_STACK_OVERFLOW";
  case ERR_STACK_UNDERFLOW:
    return "ERR_STACK_UNDERFLOW";
  case ERR_ILLEGAL_INST:
    return "ERR_ILLEGAL_INST";
  case ERR_ILLEGAL_INST_ACCESS:
    return "ERR_ILLEGAL_INST_ACCESS";
  case ERR_DIV_BY_ZERO:
    return "ERR_DIV_BY_ZERO";
  default:
    assert(0 && "trap_as_cstr: Unreachable");
  }
}

typedef int64_t Word;

typedef enum {
  INST_PUSH,
  INST_PLUS,
  INST_MINUS,
  INST_MULT,
  INST_DIV,
  INST_JMP,
  INST_HALT,
} Inst_Type;

typedef struct {
  Inst_Type type;
  Word operand;
} Inst;


typedef struct {
  Word stack[BM_STACK_CAPACITY];
  Word stack_size;

  Inst program[BM_PROGRAM_CAPACITY];
  Word program_size;
  Word ip;
  
  int halt;
} Bm;

const char *inst_type_as_cstr(Inst_Type type)
{
  switch (type) {
  case INST_PUSH:	return "INST_PUSH";
  case INST_PLUS:	return "INST_PLUS";
  case INST_MINUS:	return "INST_MINUS";
  case INST_MULT:	return "INST_MULT";
  case INST_DIV:	return "INST_DIV";
  case INST_JMP:	return "INST_JMP";
  case INST_HALT:	return "INST_HALT";
  default:
    assert(0 && "inst_type_as_cstr: unreachable");
  }
}

#define MAKE_INST_PUSH(value)	{.type = INST_PUSH, .operand = (value)}
#define MAKE_INST_PLUS		{.type = INST_PLUS}
#define MAKE_INST_MINUS		{.type = INST_MINUS}
#define MAKE_INST_MULT		{.type = INST_MULT}
#define MAKE_INST_DIV		{.type = INST_DIV}
#define MAKE_INST_JMP(addr)     {.type = INST_JMP, .operand = (addr)}
#define MAKE_INST_HALT		{.type = INST_HALT}

Err bm_execute_inst(Bm *bm)
{
  if (bm->ip < 0 || bm->ip >= bm->program_size) {
    return ERR_ILLEGAL_INST_ACCESS;
  }

  Inst inst = bm->program[bm->ip];
  
  switch (inst.type) {
  case INST_PUSH:
    if (bm->stack_size >= BM_STACK_CAPACITY) {
      return ERR_STACK_OVERFLOW;
    }
    bm->stack[bm->stack_size++] = inst.operand;
    bm->ip += 1;
    break;
    
  case INST_PLUS:
    if (bm->stack_size < 2) {
      return ERR_STACK_UNDERFLOW;
    }
    bm->stack[bm->stack_size - 2] += bm->stack[bm->stack_size - 1];
    bm->stack_size -= 1;
    bm->ip += 1;
    break;
    
  case INST_MINUS:
    if (bm->stack_size < 2) {
      return ERR_STACK_UNDERFLOW;
    }
    bm->stack[bm->stack_size - 2] -= bm->stack[bm->stack_size - 1];
    bm->stack_size -= 1;
    bm->ip += 1;
    break;
    
  case INST_MULT:
    if (bm->stack_size < 2) {
      return ERR_STACK_UNDERFLOW;
    }
    bm->stack[bm->stack_size - 2] *= bm->stack[bm->stack_size - 1];
    bm->stack_size -= 1;
    bm->ip += 1;
    break;

  case INST_DIV:
    if (bm->stack_size < 2) {
      return ERR_STACK_UNDERFLOW;
    }
    
    if (bm->stack[bm->stack_size - 1] == 0) {
      return ERR_DIV_BY_ZERO;
    }
    
    bm->stack[bm->stack_size - 2] /= bm->stack[bm->stack_size - 1];
    bm->stack_size -= 1;
    bm->ip += 1;
    break;

  case INST_JMP:
    bm->ip = inst.operand;
    break;

  case INST_HALT:
    bm->halt = 1;
    break;
    
  default:
    return ERR_ILLEGAL_INST;
  }
  
  return ERR_OK;
}

void bm_dump_stack(FILE *stream, const Bm *bm)
{
  fprintf(stream, "Stack:\n");
  if (bm->stack_size > 0) {
    for (Word i = 0; i < bm->stack_size; ++i) {
      fprintf(stream, "  %ld\n", bm->stack[i]);
    } 
  } else {
    fprintf(stream, "  [empty]\n");
  }
}

void bm_push_inst(Bm *bm, Inst inst)
{
  assert(bm->program_size < BM_PROGRAM_CAPACITY);
  bm->program[bm->program_size++] = inst;
}

void bm_load_program_from_memory(Bm *bm, Inst *program, size_t program_size)
{
  assert(program_size < BM_PROGRAM_CAPACITY);
  memcpy(bm->program, program, sizeof(program[0]) * program_size);
  bm->program_size = program_size;
}

Bm bm = {0};
Inst program[] = {
		  MAKE_INST_PUSH(0), // 0
		  MAKE_INST_PUSH(1), // 1
		  MAKE_INST_PLUS,
		  MAKE_INST_JMP(1),
};
 
int main()
{
   bm_load_program_from_memory(&bm, program, ARRAY_SIZE(program));
   bm_dump_stack(stdout, &bm);
   for (int i = 0; i < BM_EXECUTION_LIMIT && !bm.halt; ++i) {
     Err err = bm_execute_inst(&bm);
     bm_dump_stack(stdout, &bm);
     if (err != ERR_OK) {
       fprintf(stderr, "ERROR: %s\n", err_as_cstr(err));
       exit(1);
     }
   }
    
   return 0;
}
