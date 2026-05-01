#include "../include/parser.h"

static node_t *parse(parser_t *parser, tokenizer_t *tokenizer, char *str);
static void validate(parser_t *parser);

static node_t *create_node(token_t *token);
static node_t *match(parser_t *parser, tokenizer_t *tokenizer, type_t type);
static node_t *numeric_literal(parser_t *parser, tokenizer_t *tokenizer);
static node_t *binary_expression(parser_t *parser, tokenizer_t *tokenizer,
                                 func_ptr f, type_t type);
static node_t *parethesis_expression(parser_t *parser, tokenizer_t *tokenizer);
static node_t *primary_expression(parser_t *parser, tokenizer_t *tokenizer);
static node_t *multiplicative_expression(parser_t *parser,
                                         tokenizer_t *tokenizer);
static node_t *additive_expression(parser_t *parser, tokenizer_t *tokenizer);
static node_t *expression(parser_t *parser, tokenizer_t *tokenizer);
static node_t *program(parser_t *parser, tokenizer_t *tokenizer);
static void define_data_symbols(node_t *root, symbols_list_t **data_symbols_list);
static void data_symbols(tree_t *tree, parser_t *parser);
static void define_body_symbols(node_t *root, symbols_list_t **data_symbols_list);
static void body_symbols(tree_t *tree, parser_t *parser);
static void free_data_symbols(symbols_list_t *data_symbols_list);

int count = 1; // Contador para endereços de símbolos, começa em 1 porque @DBRESULT já ocupa o endereço 255
symbol_t last_symbol; // Variável global para armazenar o último símbolo adicionado, usada para resolver endereços de JMP e JZ

static void data_symbols(tree_t *tree, parser_t *parser) { define_data_symbols(tree->root, &parser->data_symbols_list); }

void add_data_symbol(symbols_list_t **data_symbols_list, char *name, long int value , int address, int line_count) {
  // Cria e preenche o novo nó
  symbols_list_t *new_node = malloc(sizeof(symbols_list_t));
  new_node->symbol = malloc(sizeof(symbol_t));
  new_node->symbol->value = value;
  new_node->symbol->line_count = line_count;
  new_node->symbol->used = 0;
  int str_l = strlen(name);
  new_node->symbol->name = malloc((str_l + 1) * sizeof(char));
  strcpy(new_node->symbol->name, name);

  new_node->symbol->address = address;
  
  new_node->next = NULL;

  // Encadeia no fim da lista
  if (*data_symbols_list == NULL) {
    *data_symbols_list = new_node;
  } else {
    symbols_list_t *current = *data_symbols_list;
    while (current->next != NULL)
      current = current->next;
    current->next = new_node;
  }
  count++;
}

static void body_symbols(tree_t *tree, parser_t *parser) {
  count = 0;
  define_body_symbols(tree->root, &parser->data_symbols_list);

  // Linha do HLT = count atual * 2 (cada instrução ocupa 2 bytes)
  int hlt_line = count * 2;

  add_data_symbol(&parser->data_symbols_list, "HLT", 0, 0, hlt_line);

  // Resolve todos os JZ pendentes (address == -1) para apontar para o HLT
  symbols_list_t *current = parser->data_symbols_list;
  while (current != NULL) {
    if (strcmp(current->symbol->name, "JZ") == 0 && current->symbol->line_count == -1) {
      current->symbol->address  = hlt_line;
      current->symbol->line_count = hlt_line;
    }
    current = current->next;
  }
}                                     

void init_parser(parser_t *parser) {
  parser->parse = parse;
  parser->validate = validate;
  parser->valid = 1;
  parser->data_symbols = data_symbols;
  parser->body_symbols = body_symbols;
  parser->free_data_symbols = free_data_symbols;
  symbols_list_t *data_symbols_list = NULL;
  add_data_symbol(&data_symbols_list, "@DBRESULT", 0, 255, -1);
  parser->data_symbols_list = data_symbols_list;
}

static node_t *parse(parser_t *parser, tokenizer_t *tokenizer, char *str) {

  tokenizer->load(tokenizer, str);

  if (tokenizer->str == NULL) {
    printf("Erro de sintaxe: Token inválido\n");
    parser->valid = 0;
    parser->lookahead = NULL;
    return NULL;
  }

  parser->lookahead = tokenizer->get_next_token(tokenizer);

  return program(parser, tokenizer);
}

static void validate(parser_t *parser) {
  if (parser->lookahead != NULL) {
    printf("Erro de sintaxe: Token inesperado\n");
    parser->valid = 0;
    free(parser->lookahead);
  }
}

static node_t *create_node(token_t *token) {
  node_t *node = (node_t *)malloc(sizeof(node_t));
  node->value = token->value;
  node->left = NULL;
  node->right = NULL;
  node->type = token->type;

  free(token);
  return node;
}

static node_t *match(parser_t *parser, tokenizer_t *tokenizer, type_t type) {
  token_t *token = parser->lookahead;

  if (token == NULL) {
    printf("Erro de sintaxe: Final inesperado\n");
    parser->valid = 0;
    return NULL;
  }

  if (token->type != type) {
    printf("Erro de sintaxe: Token inesperado\n");
    parser->valid = 0;
    return NULL;
  }

  parser->lookahead = tokenizer->get_next_token(tokenizer);

  if (type == PARENTHESIS) {
    free(token);
    return NULL;
  }

  return create_node(token);
}

static node_t *numeric_literal(parser_t *parser, tokenizer_t *tokenizer) {
  return match(parser, tokenizer, NUMBER);
}

static node_t *parethesis_expression(parser_t *parser, tokenizer_t *tokenizer) {
  match(parser, tokenizer, PARENTHESIS);
  node_t *exp = NULL;

  if (parser->lookahead != NULL && parser->lookahead->value != ')')
    exp = expression(parser, tokenizer);
  else
    parser->valid = 0;

  match(parser, tokenizer, PARENTHESIS);

  return exp;
}

static node_t *primary_expression(parser_t *parser, tokenizer_t *tokenizer) {
  if (parser->lookahead->type == PARENTHESIS)
    return parethesis_expression(parser, tokenizer);

  else
    return numeric_literal(parser, tokenizer);
}

static node_t *unary_expression(parser_t *parser, tokenizer_t *tokenizer) {
  node_t *operator = NULL;

  if (parser->lookahead == NULL) {
    printf("Erro de sintaxe: Final inesperado\n");
    parser->valid = 0;
    return NULL;
  }

  if (parser->lookahead->type == ADDITIVE_OPERATOR)
    operator = match(parser, tokenizer, ADDITIVE_OPERATOR);

  if (operator != NULL) {
    operator->type = UNARY_OPERATOR;
    operator->left = unary_expression(parser, tokenizer);
    return operator;
  }

  return primary_expression(parser, tokenizer);
}

static node_t *binary_expression(parser_t *parser, tokenizer_t *tokenizer,
                                 func_ptr f, type_t type) {
  node_t *left = f(parser, tokenizer);
  node_t *operator = NULL;
  node_t *right = NULL;

  while (parser->lookahead != NULL && parser->lookahead->type == type) {
    operator = match(parser, tokenizer, type);
    right = f(parser, tokenizer);
    operator->left = left;
    operator->right = right;
    left = operator;
  }
  return left;
}

static node_t *multiplicative_expression(parser_t *parser,
                                         tokenizer_t *tokenizer) {
  return binary_expression(parser, tokenizer, unary_expression,
                           MULTIPLICATION_OPERATOR);
}

static node_t *additive_expression(parser_t *parser, tokenizer_t *tokenizer) {
  return binary_expression(parser, tokenizer, multiplicative_expression,
                           ADDITIVE_OPERATOR);
}

static node_t *expression(parser_t *parser, tokenizer_t *tokenizer) {
  return additive_expression(parser, tokenizer);
}

static node_t *program(parser_t *parser, tokenizer_t *tokenizer) {
  return expression(parser, tokenizer);
}

static void define_data_symbols(node_t *root, symbols_list_t **data_symbols_list) {
  if (root == NULL)
    return;
  
  if (root->type == NUMBER) {
    add_data_symbol(data_symbols_list, "@DB", root->value, 255 - count, -1);
  }

    // Se há multiplicação, reserva os símbolos auxiliares uma única vez
  if (root->type == MULTIPLICATION_OPERATOR) {
    add_data_symbol(data_symbols_list, "@MUL_RESULT", 0, 255 - count, -1);
    add_data_symbol(data_symbols_list, "@MUL_COUNTER", 0, 255 - count, -1);
    add_data_symbol(data_symbols_list, "@NEG1", 255, 255 - count, -1); // 255 = -1 em complemento de 2
  }

  define_data_symbols(root->left, data_symbols_list);
  define_data_symbols(root->right, data_symbols_list);
}

void print_data_symbols(symbols_list_t *data_symbols_list) {
  symbols_list_t *current = data_symbols_list;
  
  if (current == NULL) {
    printf("No data symbols found.\n");
    return;
  }
  while (current != NULL) {
    if(strcmp(current->symbol->name, "@DBRESULT") == 0 || strcmp(current->symbol->name, "@DB") == 0){
      printf("%s %d %ld\n", current->symbol->name, current->symbol->address, current->symbol->value);
    } else if (strcmp(current->symbol->name, "JMP") == 0 || strcmp(current->symbol->name, "JZ") == 0) {
      printf("%s %d\n", current->symbol->name, current->symbol->line_count);
    } else {
      printf("%s %d\n", current->symbol->name, current->symbol->address);
    }
    current = current->next;
  }
}

int find_symbol_address(symbols_list_t *list, long int value, char *name) {
  symbols_list_t *current = list;
  while (current != NULL) {
    if (current->symbol->value == value && 
        strncmp(current->symbol->name, name, strlen(name)) == 0 &&
        current->symbol->used == 0) {
      current->symbol->used = 1;  // marca como consumido
      return current->symbol->address;
    }
    current = current->next;
  }
  return -1;
}

int find_symbol_address_peek(symbols_list_t *list, long int value, char *name) {
  symbols_list_t *current = list;
  while (current != NULL) {
    if (current->symbol->value == value && 
        strncmp(current->symbol->name, name, strlen(name)) == 0)
      return current->symbol->address;
    current = current->next;
  }
  return -1;
}

void generate_code_multiplication(node_t *left, node_t *right, symbols_list_t **data_symbols_list) {
  int left_addr     = find_symbol_address(*data_symbols_list, left->value, "@DB");
  int right_addr    = find_symbol_address(*data_symbols_list, right->value, "@DB");
  int dbresult_addr = find_symbol_address_peek(*data_symbols_list, 0,   "@DBRESULT");
  int result_addr   = find_symbol_address_peek(*data_symbols_list, 0,   "@MUL_RESULT");
  int counter_addr  = find_symbol_address_peek(*data_symbols_list, 0,   "@MUL_COUNTER");
  int neg1_addr     = find_symbol_address_peek(*data_symbols_list, 255, "@NEG1");

  // Zera @MUL_RESULT: LDA @DBRESULT (vale 0) → STA @MUL_RESULT
  add_data_symbol(data_symbols_list, "LDA", 0,            dbresult_addr, count * 2);
  add_data_symbol(data_symbols_list, "STA", result_addr,  result_addr,   count * 2);

  // Inicializa @MUL_COUNTER com B
  add_data_symbol(data_symbols_list, "LDA", right_addr,   right_addr,    count * 2);
  add_data_symbol(data_symbols_list, "STA", counter_addr, counter_addr,  count * 2);

  // Captura a linha do LOOP antes de emitir as instruções dele
  int loop_line = count * 2;

  // LOOP: @MUL_RESULT += A
  add_data_symbol(data_symbols_list, "LDA", result_addr,  result_addr,  count * 2);
  add_data_symbol(data_symbols_list, "ADD", left_addr,    left_addr,    count * 2);
  add_data_symbol(data_symbols_list, "STA", result_addr,  result_addr,  count * 2);

  // Decrementa @MUL_COUNTER
  add_data_symbol(data_symbols_list, "LDA", counter_addr, counter_addr, count * 2);
  add_data_symbol(data_symbols_list, "ADD", neg1_addr,    neg1_addr,    count * 2);
  add_data_symbol(data_symbols_list, "STA", counter_addr, counter_addr, count * 2);

  // JZ → resolvido depois em body_symbols (sentinela address == -1)
  add_data_symbol(data_symbols_list, "JZ",  0,        0,        count * 2);
  // JMP → já sabemos o destino
  add_data_symbol(data_symbols_list, "JMP", loop_line, loop_line, count * 2);
}

static void define_body_symbols(node_t *root, symbols_list_t **data_symbols_list) {
  if (root == NULL)
    return;
  
  if (root->type == NUMBER) {
    if (count == 0) {
      int addr = find_symbol_address(*data_symbols_list, root->value, "@DB");
      add_data_symbol(data_symbols_list, "LDA", root->value, addr, count);
      
    }
    return;
  }

  define_body_symbols(root->left, data_symbols_list);

  if (root->right != NULL) {
    char *op_name = NULL;

    switch (root->type) {
      case ADDITIVE_OPERATOR:
        op_name = "ADD";
        break;
      case MULTIPLICATION_OPERATOR:
        generate_code_multiplication( root->left, root->right, data_symbols_list);
        break;
      default:
        break;
    }

    if (op_name != NULL) {
      node_t *right_leaf = root->right;
      while (right_leaf->left != NULL) right_leaf = right_leaf->left;

      int addr = find_symbol_address(*data_symbols_list, right_leaf->value, "@DB");
      add_data_symbol(data_symbols_list, op_name, right_leaf->value, addr, count);
    }
  }

  if (root->right != NULL && root->right->type != NUMBER)
    define_body_symbols(root->right, data_symbols_list);
}

static void free_data_symbols(symbols_list_t *data_symbols_list) {
  symbols_list_t *current = data_symbols_list;
  while (current != NULL) {
    symbols_list_t *next = current->next;
    free(current->symbol);
    free(current);
    current = next;
  }
}