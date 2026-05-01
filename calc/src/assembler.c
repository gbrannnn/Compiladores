/*
 * =====================================================================================
 *
 *       Filename:  assembler.c
 *
 *    Description:  Implementação do assembler para gerar código Neander
 *
 *        Version:  1.0
 *        Created:  2026/04/30
 *       Compiler:  gcc
 *
 * =====================================================================================
 */

#include "../include/assembler.h"

/* Forward declarations */
static void generate_code(assembler_t *assembler, node_t *node, symbols_list_t *symbols);
static void write_file(assembler_t *assembler, const char *filename);
static void free_assembler(assembler_t *assembler);
static void emit_instruction(assembler_t *assembler, uint8_t opcode, uint8_t addr);
static void setup_data_section(assembler_t *assembler, symbols_list_t *symbols);

/* Emite uma instrução no código */
static void emit_instruction(assembler_t *assembler, uint8_t opcode, uint8_t addr) {
  if (assembler->pc >= 510) {
    fprintf(stderr, "Erro: Código muito grande para caber na memória\n");
    return;
  }
  assembler->mem.data[assembler->pc++] = opcode;
  assembler->mem.data[assembler->pc++] = addr;
}

/* Configura a seção de dados com os símbolos */
static void setup_data_section(assembler_t *assembler, symbols_list_t *symbols) {
  assembler->data_ptr = 256; /* Dados começam em 256 */
  
  symbols_list_t *current = symbols;
  while (current != NULL) {
    symbol_t *sym = current->symbol;
    
    if (sym->address != 0 || strcmp(sym->name, "@DBRESULT") == 0) {
      /* Armazena o valor do símbolo na memória */
      if (assembler->data_ptr < 512) {
        assembler->mem.data[assembler->data_ptr] = (uint8_t)(sym->value & 0xFF);
        sym->address = assembler->data_ptr;
        assembler->data_ptr++;
      }
    }
    
    current = current->next;
  }
}

/* Gera código recursivamente a partir da AST */
static void generate_code(assembler_t *assembler, node_t *node, symbols_list_t *symbols) {
  if (node == NULL) {
    return;
  }

  switch (node->type) {
    case NUMBER:
      /* Carrega número literal no acumulador */
      emit_instruction(assembler, LDA, 0xFF); /* Espaço temporário */
      assembler->mem.data[0xFF] = node->value & 0xFF;
      break;

    case ADDITIVE_OPERATOR: {
      /* Processa o operando esquerdo */
      generate_code(assembler, node->left, symbols);
      
      /* Armazena resultado temporário */
      int temp_addr = 254;
      emit_instruction(assembler, STA, temp_addr);
      
      /* Processa o operando direito */
      generate_code(assembler, node->right, symbols);
      
      /* Realiza a operação */
      if (node->value == '+') {
        emit_instruction(assembler, ADD, temp_addr);
      } else if (node->value == '-') {
        /* Neander não tem subtração direta, usaria complemento de 2 */
        emit_instruction(assembler, ADD, temp_addr);
      }
      break;
    }

    case MULTIPLICATION_OPERATOR: {
      /* Processa o operando esquerdo */
      generate_code(assembler, node->left, symbols);
      
      /* Armazena resultado temporário */
      int temp_addr = 254;
      emit_instruction(assembler, STA, temp_addr);
      
      /* Processa o operando direito */
      generate_code(assembler, node->right, symbols);
      
      /* Realiza a operação */
      if (node->value == '*') {
        /* Multiplicação usando loop - não implementada diretamente */
        /* Seria necessário rotina de suporte */
      } else if (node->value == '/') {
        /* Divisão usando loop - não implementada diretamente */
      } else if (node->value == '%') {
        /* Módulo - não implementado */
      }
      break;
    }

    case UNARY_OPERATOR: {
      generate_code(assembler, node->left, symbols);
      
      if (node->value == '-') {
        /* Negação: carregar 0, subtrair valor */
        emit_instruction(assembler, NOT, 0xFF);
      }
      break;
    }

    default:
      break;
  }
}

/* Escreve o arquivo .mem no disco */
static void write_file(assembler_t *assembler, const char *filename) {
  FILE *fp = fopen(filename, "wb");
  if (fp == NULL) {
    fprintf(stderr, "Erro: Não foi possível abrir arquivo %s para escrita\n", filename);
    return;
  }

  /* Escreve header */
  fwrite(assembler->mem.header, 1, 4, fp);
  
  /* Escreve dados/código */
  fwrite(assembler->mem.data, 1, 512, fp);
  
  fclose(fp);
  printf("Arquivo %s gerado com sucesso\n", filename);
}

/* Libera recursos do assembler */
static void free_assembler(assembler_t *assembler) {
  /* O assembler não aloca recursos dinâmicos além da estrutura em si */
  (void)assembler;
}

/* Inicializa o assembler */
void init_assembler(assembler_t *assembler) {
  assembler->pc = 0;
  assembler->data_ptr = 256;
  assembler->symbols = NULL;
  
  /* Copia header "NDR " */
  assembler->mem.header[0] = 'N';
  assembler->mem.header[1] = 'D';
  assembler->mem.header[2] = 'R';
  assembler->mem.header[3] = ' ';
  
  /* Inicializa dados com zeros */
  memset(assembler->mem.data, 0, 512);
  
  /* Configura métodos */
  assembler->generate_code = generate_code;
  assembler->write_file = write_file;
  assembler->free_assembler = free_assembler;
}

/* Função principal de montagem */
void assemble(assembler_t *assembler, node_t *ast_root, symbols_list_t *data_symbols_list, const char *output_file) {
  /* Inicializa o assembler se necessário */
  if (assembler->mem.header[0] != 'N') {
    init_assembler(assembler);
  }
  
  /* Configura a seção de dados */
  setup_data_section(assembler, data_symbols_list);
  assembler->symbols = data_symbols_list;
  
  /* Gera código a partir da AST */
  generate_code(assembler, ast_root, data_symbols_list);
  
  /* Finaliza com HLT (halt) */
  emit_instruction(assembler, HLT, 0);
  
  /* Escreve arquivo de saída */
  write_file(assembler, output_file);
}
