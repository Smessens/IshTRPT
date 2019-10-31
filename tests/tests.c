#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <CUnit/Basic.h>
#include <CUnit/CUnit.h>
#include <CUnit/Console.h>

void test_basic(void)
{
  printf("FIRST TEST : test basique\n");
  system("./test_basique.sh");
  //system("../src/receiver -o test_basique_out.dat :: 64342");
  //system("../senderprof localhost 64342 < test_in.dat");

  CU_ASSERT_EQUAL(system("sha512sum test_basique_out.dat"),system("sha512sum test_in.txt"));
}

void test_erreur(void)
{
  printf("SECOND TEST : test avec erreur\n");
  system("./test_erreur.sh");
  //system("../src/receiver -o test_erreur_out.dat :: 64341");
  //system(".././link_sim -p 64342 -P 64341 -e 30");
  //system("../senderprof localhost 64342 < test_in.dat");

  CU_ASSERT_EQUAL(system("sha512sum test_erreur_out.dat"),system("sha512sum test_in.txt"));
}

void test_delay(void)
{
  printf("THIRD TEST : test avec delay\n");
  system("./test_delay.sh");
  // system("../src/receiver -o test_delay_out.dat :: 64341");
  // system(".././link_sim -p 64342 -P 64341 -d 50");
  // system("../senderprof localhost 64342 < test_in.dat");

  CU_ASSERT_EQUAL(system("sha512sum test_delay_out.dat"),system("sha512sum test_in.txt"));
}

void test_loss(void)
{
  printf("FOURTH TEST : test avec des pertes\n");
  system("./test_lost.sh");
  // system("../src/receiver -o test_loss_out.dat :: 64341");
  // system(".././link_sim -p 64342 -P 64341 -l 30");
  // system("../senderprof localhost 64342 < test_in.dat");

  CU_ASSERT_EQUAL(system("sha512sum test_loss_out.dat"),system("sha512sum test_in.txt"));
}

void test_all(void)
{
  printf("FIFTH TEST : test avec un peu de tout\n");
  system("./test_all.sh");
  // system("../src/receiver -o test_all_out.dat :: 64341");
  // system(".././link_sim -p 64342 -P 64341 -e 15 -l 15  -d 25");
  // system("../senderprof localhost 64342 < test_in.dat");

  CU_ASSERT_EQUAL(system("sha512sum test_all_out.dat"),system("sha512sum test_in.txt"));
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
  if ((NULL == CU_add_test(pSuite, "Test basique", test_basic)) ||
      (NULL == CU_add_test(pSuite, "Test avec des erreurs", test_erreur)) ||
      (NULL == CU_add_test(pSuite, "Test avec du delay", test_delay)) ||
      (NULL == CU_add_test(pSuite, "Test avec des pertes", test_loss)) ||
      (NULL == CU_add_test(pSuite, "Test avec un peu de tout", test_all))) {
    CU_cleanup_registry();
    return CU_get_error();
  }
  CU_basic_set_mode(CU_BRM_SILENT);
  CU_basic_run_tests();
  CU_basic_show_failures(CU_get_failure_list());
  CU_cleanup_registry();
  return CU_get_error();
}
