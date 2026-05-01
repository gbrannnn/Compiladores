#include "../include/parser.h"
#include "../include/tokenizer.h"
#include "../include/tree.h"
#include "../include/assembler.h" 

int main(int argc, char *argv[]) {
  if (argc < 2)
    return 0;

  tokenizer_t tokenizer;
  parser_t parser;
  tree_t ast;
  assembler_t assembler;

  init_tokenizer(&tokenizer);
  init_parser(&parser);
  init_tree(&ast);
  init_assembler(&assembler);

  ast.root = parser.parse(&parser, &tokenizer, argv[1]);

  parser.validate(&parser);

  if (parser.valid)
    ast.eval(&ast);

  // ast.print(&ast);
  parser.data_symbols(&ast, &parser);
  parser.body_symbols(&ast, &parser);

  print_data_symbols(parser.data_symbols_list);

  /* Gera código Neander */
  assemble(&assembler, ast.root, parser.data_symbols_list, "output.mem");

  parser.free_data_symbols(parser.data_symbols_list);
  ast.free_all(&ast);
  tokenizer.free_str(&tokenizer);
  assembler.free_assembler(&assembler);

  return 0;
}
