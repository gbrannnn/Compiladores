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
static void define_data_symbols(node_t *root, int *count, symbols_list_t **data_symbols_list);
static void data_symbols(tree_t *tree, parser_t *parser);


static void data_symbols(tree_t *tree, parser_t *parser) { int count = 1; 
                          define_data_symbols(tree->root, &count, &parser->data_symbols_list); }


void add_data_symbol(symbols_list_t **data_symbols_list, char *name, long int value , int address) {
  // Cria e preenche o novo nó
  symbols_list_t *new_node = malloc(sizeof(symbols_list_t));
  new_node->symbol = malloc(sizeof(symbol_t));
  new_node->symbol->value = value;
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
}

void init_parser(parser_t *parser) {
  parser->parse = parse;
  parser->validate = validate;
  parser->valid = 1;
  parser->data_symbols = data_symbols;
  symbols_list_t *data_symbols_list = NULL;
  add_data_symbol(&data_symbols_list, "@DBRESULT", 0, 255);
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

static void define_data_symbols(node_t *root, int *count, symbols_list_t **data_symbols_list) {
  if (root == NULL)
    return;
  
  if (root->type == NUMBER) {
    add_data_symbol(data_symbols_list, "@DB", root->value, 255 - *count);
    (*count)++;
  }

  define_data_symbols(root->left, count, data_symbols_list);
  define_data_symbols(root->right, count, data_symbols_list);
}

void print_data_symbols(symbols_list_t *data_symbols_list) {
  symbols_list_t *current = data_symbols_list;
  if (current == NULL) {
    printf("No data symbols found.\n");
    return;
  }
  while (current != NULL) {
    printf("%s %d %ld\n", current->symbol->name, current->symbol->address, current->symbol->value);
    current = current->next;
  }
}