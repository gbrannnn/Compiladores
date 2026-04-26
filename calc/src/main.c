#include "../include/parser.h"
#include "../include/tokenizer.h"
#include "../include/tree.h"

int main(int argc, char *argv[]) {
  if (argc < 2)
    return 0;

  tokenizer_t tokenizer;
  parser_t parser;
  tree_t ast;

  init_tokenizer(&tokenizer);
  init_parser(&parser);
  init_tree(&ast);

  ast.root = parser.parse(&parser, &tokenizer, argv[1]);

  parser.validate(&parser);

  if (parser.valid)
    ast.eval(&ast);

  ast.print(&ast);

  ast.free_all(&ast);
  tokenizer.free_str(&tokenizer);

  return 0;
}
