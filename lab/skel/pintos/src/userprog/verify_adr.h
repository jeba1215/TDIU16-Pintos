#ifndef VERIFY_ADR_H
#define VERIFY_ADR_H
#include "lib/stdbool.h"
#include "lib/stdint.h"

bool verify_fix_length(void*, uint32_t);
bool verify_variable_length(char*);

#endif
