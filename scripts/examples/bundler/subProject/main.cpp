#include "helpers.h"
#if RUNNING_OFFLINE
#include <offline_from_main>
#endif

int main(int argc, char *argv[]) {
  helpers::help();

  return 0;
}
