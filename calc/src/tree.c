#include "../include/tree.h"

static void print_tree(node_t *root);
static void free_all(tree_t *tree);
static void eval(tree_t *tree);
static void print(tree_t *tree);
static void free_tree(node_t *root);
static long evaluate(node_t *root, int *is_valid);
static void print_type(type_t type);
static long mult_div(node_t *root, int *is_valid);
static long add_sub(node_t *root);
static long unary(node_t *root);
int modulo(long left, long right);

static void free_all(tree_t *tree) { free_tree(tree->root); }

static void print(tree_t *tree) { print_tree(tree->root); }

static void eval(tree_t *tree) {
  int is_valid = 1;
  long result = evaluate(tree->root, &is_valid);

  if (is_valid)
    printf("%ld\n", result);
  else
    printf("Erro: divisor não pode ser zero.\n");
}

void init_tree(tree_t *tree) {
  tree->root = NULL;
  tree->print = print;
  tree->free_all = free_all;
  tree->eval = eval;
}

static long evaluate(node_t *root, int *is_valid) {
  if (root == NULL)
    return 0;

  evaluate(root->left, is_valid);
  evaluate(root->right, is_valid);

  if (root->type == MULTIPLICATION_OPERATOR)
    root->value = mult_div(root, is_valid);

  if (root->type == ADDITIVE_OPERATOR)
    root->value = add_sub(root);

  if (root->type == UNARY_OPERATOR)
    root->value = unary(root);

  return root->value;
}

static void print_tree(node_t *root) {
  if (root == NULL)
    return;

  print_type(root->type);

  if (root->type == NUMBER)
    printf("%ld\n", root->value);

  else
    printf("%c\n", (char)root->value);

  print_tree(root->left);
  print_tree(root->right);
}

static void free_tree(node_t *root) {
  if (root == NULL)
    return;

  free_tree(root->left);
  free_tree(root->right);

  free(root);
}

static void print_type(type_t type) {
  switch (type) {
  case NUMBER:
    printf("Tipo: Número\n");
    break;
  case ADDITIVE_OPERATOR:
    printf("Tipo: Operador aditivo\n");
    break;
  case MULTIPLICATION_OPERATOR:
    printf("Tipo: Operador multiplicativo\n");
    break;
  case UNARY_OPERATOR:
    printf("Tipo: Operador unário\n");
    break;
  default:
    printf("Tipo: Inválido\n");
  }
}

static long mult_div(node_t *root, int *is_valid) {
  if (root->value == '*')
    return root->left->value * root->right->value;
  if (root->value == '/') {
    if (root->right->value != 0)
      return root->left->value / root->right->value;

    *is_valid = 0;
    return 0;
  } else
    return modulo(root->left->value, root->right->value);
}

static long add_sub(node_t *root) {
  if (root->value == '+')
    return root->left->value + root->right->value;
  else
    return root->left->value - root->right->value;
}

static long unary(node_t *root) {
  if (root->value == '-')
    return root->left->value * -1;
  else
    return root->left->value;
}

int modulo(long left, long right) { return (left % right + right) % right; }
