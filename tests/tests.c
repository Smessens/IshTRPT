#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "CUnit/Basic.h"
#include "CUnit/CUnit.h"

int main(int argc, char const *argv[]) {
  CU_pSuite pSuite = NULL;

  if(CUE_SUCCESS != CU_initialize_registry()) {
     return CU_get_error();
   }
   pSuite = CU_add_suite("Suite de tests sur le protocole", NULL, NULL);
   if(NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
    }
  return 0;
}
