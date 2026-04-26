#include "../include/utils.h"

long int str_to_num(char *str) {
  char *end;
  return strtol(str, &end, DECIMAL);
}
