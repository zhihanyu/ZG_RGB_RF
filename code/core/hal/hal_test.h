#ifndef HAL_TEST_H
#define HAL_TEST_H

typedef struct hal_test_
{
  int   no;
  int   arg[AT_SET_MAX_ARGC - 1];
  int   arg_cnt;
  int   result;
}hal_test_t;


typedef struct hal_test_name_map_
{
  int   type;
  char  name[48];
}hal_test_name_map_t;














#endif