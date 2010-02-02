extern "C" {
#include <stdio.h>
#include <err.h>
  // pgm.h is not very friendly with system headers... need to undef max() and min() afterwards
#include <pgm.h>
#undef max
#undef min
}

int main(int argc, char ** argv)
{
  pgm_init(&argc, argv);
  
  FILE * pgmfile(stdin);
  if (1 < argc) {
    pgmfile = fopen(argv[1], "rb");
    if ( ! pgmfile) {
      err(EXIT_FAILURE, "fopen(%s)", argv[1]);
    }
  }
  
  FILE * txtfile(stdout);
  if (2 < argc) {
    txtfile = fopen(argv[2], "w");
    if ( ! txtfile) {
      err(EXIT_FAILURE, "fopen(%s)", argv[2]);
    }
  }
  
  int ncols, nrows;
  gray maxval;
  int format;
  pgm_readpgminit(pgmfile, &ncols, &nrows, &maxval, &format);
  
  long int robot_x(0);
  long int robot_y(0);
  if (4 < argc) {
    robot_x = strtol(argv[3], NULL, 10);
    if (0 != errno) {
      err(EXIT_FAILURE, "strtol(%s)", argv[3]);
    }
    if (0 > robot_x) {
      robot_x = 0;
    }
    if (ncols <= robot_x) {
      robot_x = ncols - 1;
    }
    robot_y = strtol(argv[4], NULL, 10);
    if (0 != errno) {
      err(EXIT_FAILURE, "strtol(%s)", argv[4]);
    }
    if (0 > robot_y) {
      robot_y = 0;
    }
    if (nrows <= robot_y) {
      robot_y = nrows - 1;
    }
  }
  
  long int goal_x(ncols - 1);
  long int goal_y(nrows - 1);
  if (6 < argc) {
    goal_x = strtol(argv[5], NULL, 10);
    if (0 != errno) {
      err(EXIT_FAILURE, "strtol(%s)", argv[5]);
    }
    if (0 > goal_x) {
      goal_x = 0;
    }
    if (ncols <= goal_x) {
      goal_x = ncols - 1;
    }
    goal_y = strtol(argv[6], NULL, 10);
    if (0 != errno) {
      err(EXIT_FAILURE, "strtol(%s)", argv[6]);
    }
    if (0 > goal_y) {
      goal_y = 0;
    }
    if (nrows <= goal_y) {
      goal_y = nrows - 1;
    }
  }
  
  gray * row(pgm_allocrow(ncols));
  fprintf(txtfile,
	  "#PGM %s\n"
	  "#size %d %d\n"
	  "#maxval %d mapped to -1\n"
	  "#robot %ld %ld\n"
	  "#goal %ld %ld\n",
	  (1 < argc) ? argv[1] : "(stdin)",
	  ncols, nrows,
	  maxval,
	  robot_x, nrows - robot_y,
	  goal_x, nrows - goal_y);
  for (int ii(0); ii < nrows - 1; ++ii) {
    pgm_readpgmrow(pgmfile, row, ncols, maxval, format);
    for (int jj(0); jj < ncols; ++jj) {
      fprintf(txtfile, "%d ", (maxval > row[jj]) ? row[jj] : -1);
    }
    fprintf(txtfile, "\n");
  }
  pgm_freerow(row);
}
