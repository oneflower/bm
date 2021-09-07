#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

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
  ERR_ILLEGAL_OPERAND,
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
  case ERR_ILLEGAL_OPERAND:
    return "ERR_ILLEGAL_OPERAND";
  case ERR_DIV_BY_ZERO:
    return "ERR_DIV_BY_ZERO";
  default:
    assert(0 && "trap_as_cstr: Unreachable");
  }
}

typedef int64_t Word;

typedef enum {
	      INST_NOP = 0,
	      INST_PUSH,
	      INST_DUP,
	      INST_PLUS,
	      INST_MINUS,
	      INST_MULT,
	      INST_DIV,
	      INST_JMP,
	      INST_EQ,
	      INST_JMP_IF,
	      INST_HALT,
	      INST_PRINT_DEBUG,
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
  case INST_NOP:		return "INST_NOP";
  case INST_PUSH:		return "INST_PUSH";
  case INST_DUP:                return "INST_DUP";
  case INST_PLUS:		return "INST_PLUS";
  case INST_MINUS:		return "INST_MINUS";
  case INST_MULT:		return "INST_MULT";
  case INST_DIV:		return "INST_DIV";
  case INST_JMP:		return "INST_JMP";
  case INST_EQ:			return "INST_EQ";
  case INST_JMP_IF:		return "INST_JMP_IF";
  case INST_HALT:		return "INST_HALT";
  case INST_PRINT_DEBUG:	return "INST_PRINT_DEBUG";
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
#define MAKE_INST_DUP(addr)	{.type = INST_DUP, .operand = (addr)}
#define MAKE_INST_JMP_IF(addr)  {.type = INST_JMP_IF, .operand = (addr)}
#define MAKE_INST_EQ		{.type = INST_EQ}
#define MAKE_INST_HALT		{.type = INST_HALT}
#define MAKE_INST_PRINT_DEBUG	{.type = INST_PRINT_DEBUG}

Err bm_execute_inst(Bm *bm)
{
  if (bm->ip < 0 || bm->ip >= bm->program_size) {
    return ERR_ILLEGAL_INST_ACCESS;
  }

  Inst inst = bm->program[bm->ip];
  
  switch (inst.type) {
  case INST_NOP:
    bm->ip += 1;
    break;
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

  case INST_EQ:
    if (bm->stack_size < 2) {
      return ERR_STACK_UNDERFLOW;
    }

    bm->stack[bm->stack_size - 2] = bm->stack[bm->stack_size - 1] == bm->stack[bm->stack_size - 2];
    bm->stack_size -= 1;
    bm->ip += 1;
    break;

  case INST_JMP_IF:
    if (bm->stack_size < 2) {
      return ERR_STACK_UNDERFLOW;
    }

    if (bm->stack[bm->stack_size - 1]) {
      bm->stack_size -= 1;
      bm->ip = inst.operand;
    } else {
      bm->ip += 1;
    }
    break;
  
  case INST_HALT:
    bm->halt = 1;
    break;

  case INST_PRINT_DEBUG:
    if (bm->stack_size < 1) {
      return ERR_STACK_UNDERFLOW;
    }
    printf("%ld", bm->stack[bm->stack_size - 1]);
    bm->stack_size -= 1;
    bm->ip += 1;
    break;

  case INST_DUP:
    if (bm->stack_size >= BM_STACK_CAPACITY) {
      return ERR_STACK_OVERFLOW;
    }
    if (bm->stack_size - inst.operand <= 0) {
      return ERR_STACK_UNDERFLOW;
    }
    
    if (inst.operand < 0) {
      return ERR_ILLEGAL_OPERAND;
    }

    bm->stack[bm->stack_size] = bm->stack[bm->stack_size - 1 - inst.operand];
    bm->stack_size += 1;
    bm->ip += 1;
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

void bm_load_program_from_file(Bm *bm, const char *file_path)
{
  FILE *f = fopen(file_path, "rb");
  if (f == NULL) {
    fprintf(stderr, "ERROR: Could not open file `%s`: %s\n",
	    file_path, strerror(errno));
    exit(1);
  }

  if (fseek(f, 0, SEEK_END) < 0) {
    fprintf(stderr, "ERROR: Could not read file `%s`: %s\n",
	    file_path, strerror(errno));
    exit(1);
  } 

  long m = ftell(f);
  if (m < 0) {
    fprintf(stderr, "ERROR: Could not read file `%s`: %s\n",
	    file_path, strerror(errno));
    exit(1);
  }

  assert(m % sizeof(bm->program[0]) == 0);
  assert((size_t) m <= BM_PROGRAM_CAPACITY * sizeof(bm->program[0]));	 

  if (fseek(f, 0, SEEK_SET) < 0) {
    fprintf(stderr, "ERROR: Could not open file `%s`: %s\n",
	    file_path, strerror(errno));
    exit(1);
  }

  bm->program_size = fread(bm->program,
			   sizeof(bm->program[0]),
			   m / sizeof(bm->program[0]),
			   f);

  if (ferror(f)) {
    fprintf(stderr, "ERROR: Could not write to file `%s`: %s\n",
	    file_path, strerror(errno));
    exit(1);
  }
  fclose(f);
}

void bm_save_program_to_file(Inst *program,
			     size_t program_size,
			     const char *file_path)
{
  FILE *f = fopen(file_path, "wb");
  if (f == NULL) {
    fprintf(stderr, "ERROR: Could not open file `%s`: %s\n",
	    file_path, strerror(errno));
    exit(1);
  }

  
  fwrite(program, sizeof(program[0]), program_size, f);

  if (ferror(f)) {
    fprintf(stderr, "ERROR: Could not write to file `%s`: %s\n",
	    file_path, strerror(errno));
    exit(1);
  }

  fclose(f);
}

  
Bm bm = {0};
Inst program[] = {
		  MAKE_INST_PUSH(0), // 0
		  MAKE_INST_PUSH(1), // 1
		  MAKE_INST_DUP(1),  // 2
		  MAKE_INST_DUP(1),  // 3
		  MAKE_INST_PLUS,    // 4
		  MAKE_INST_JMP(2),  // 5
};

char *source_code =
  "push 0\n"
  "push 1\n"
  "dup 1\n"
  "dup 1\n"
  "plus\n"
  "jmp 2\n";

Inst bm_translate_line(char *line, size_t line_size)
{
  return (Inst) {0};
}

size_t bm_translate_source(char *source, size_t source_size,
			Inst *program, size_t program_capacity)
{
  while (source_size > 0) {
    char *end = memchr(source, '\n', source_size);
    size_t n = end != NULL ? (size_t) (end - source) : 0;

    printf("#%.*s#\n", (int) n, source);
 
    source = end;
    source_size -= n;

    if (source != NULL) {
      source += 1;
      source_size -= 1;
     }
  }
  return 0;
}

int main(void)
{
  bm.program_size = bm_translate_source(source_code,
				     strlen(source_code),
				     bm.program,
				     BM_PROGRAM_CAPACITY);

  return 0;
}

int main2(void)
{
  //bm_load_program_from_memory(&bm, program, ARRAY_SIZE(program));
  //bm_save_program_to_file(program, ARRAY_SIZE(program), "./fib.bm");
  bm_load_program_from_file(&bm, "./fib.bm");

  for (int i = 0; i < 69 && !bm.halt; ++i) {
    Err err = bm_execute_inst(&bm);
    bm_dump_stack(stdout, &bm);
    if (err != ERR_OK) {
      fprintf(stderr, "ERROR: %s\n", err_as_cstr(err));
      exit(1);
    }
  }
  bm_dump_stack(stdout, &bm);
   
  return 0;
}
