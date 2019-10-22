#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include "CUnit/Basic.h"
#include "CUnit/CUnit.h"
#include "CUnit/Console.h"

void test_basic(void)
{
  printf("FIRST TEST : test basique\n");
  system("../src/receiver -f test_basique_out.dat :: 2456");
  system("../src/senderprof localhost 1348 < test_basique_in.dat");
  int diff = system("diff test_basique_in.dat test_basique_out.dat");
  CU_ASSERT_EQUAL(WEXITSTATUS(diff),0);
}

int main(int argc, char const *argv[]) {
  CU_pSuite pSuite = NULL;

  if(CUE_SUCCESS != CU_initialize_registry()) {
     return CU_get_error();
   }
   pSuite = CU_add_suite("Ma suite de tests", NULL, NULL);
   if(NULL == pSuite) {
      CU_cleanup_registry();
      return CU_get_error();
    }
    if ((NULL == CU_add_test(pSuite, "Test basique", test_basic)))
{
  CU_cleanup_registry();
  return CU_get_error();
}
CU_basic_set_mode(CU_BRM_SILENT);
CU_basic_run_tests();
CU_basic_show_failures(CU_get_failure_list());
CU_cleanup_registry();
return CU_get_error();
}
