/*
 * =====================================================================================
 *
 *       Filename:  assembler.h
 *
 *    Description:  Assembler para gerar código Neander (.mem) a partir da AST
 *
 *        Version:  1.0
 *        Created:  2026/04/30
 *       Compiler:  gcc
 *
 * =====================================================================================
 */

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "../include/tree.h"
#include "../include/parser.h"

/* Instrações Neander */
typedef enum {
  NOP = 0,
  STA = 16,
  LDA = 32,
  ADD = 48,
  OR = 64,
  AND = 80,
  NOT = 96,
  JMP = 128,
  JN = 144,
  JZ = 160,
  HLT = 240
} neander_mnemo_t;

/* Estrutura para o arquivo .mem */
typedef struct {
  uint8_t header[4];    /* "NDR " */
  uint8_t data[512];    /* 512 bytes de dados/código */
} mem_file_t;

/* Estrutura para o assembler */
typedef struct assembler {
  mem_file_t mem;
  int pc;               /* Program counter - próxima posição de instrução */
  int data_ptr;         /* Ponteiro para área de dados */
  symbols_list_t *symbols;
  
  /* Métodos */
  void (*generate_code) (struct assembler*, node_t*, symbols_list_t*);
  void (*write_file) (struct assembler*, const char*);
  void (*free_assembler) (struct assembler*);
} assembler_t;

/* Funções públicas */
void init_assembler(assembler_t *assembler);
void assemble(assembler_t *assembler, node_t *ast_root, symbols_list_t *data_symbols_list, const char *output_file);

#endif
