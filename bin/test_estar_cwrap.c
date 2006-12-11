/* 
 * Copyright (C) 2005 Roland Philippsen <roland dot philippsen at gmx net>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */


#include <estar/cwrap.h>


static int create(int argc, char ** argv);


int main(int argc, char ** argv)
{
  double obstacle_meta;
  int handle = create(argc, argv);
  if(handle < 0)
    return -1;
  estar_get_obstacle_meta(handle, & obstacle_meta);
  
  printf("estar_add_goal(handle, 0, 0, 0)\n");
  estar_add_goal(handle, 0, 0, 0);
  while(estar_have_work(handle)){
    char buf[128];
    estar_dump_queue(handle, stdout, 0);
    estar_dump_grid(handle, stdout);
    printf("[ENTER or 'q'] ");
    if((fgets(buf, 128, stdin) == buf) && ('q' == buf[0])){
      estar_destroy(handle);
      return 0;
    }
    estar_compute_one(handle, 0.5);
  }
  estar_dump_grid(handle, stdout);
  
  printf("estar_set_meta(1, 1, %e)\n", obstacle_meta);
  estar_set_meta(handle, 1, 1, obstacle_meta);
  while(estar_have_work(handle)){
    char buf[128];
    estar_dump_queue(handle, stdout, 0);
    estar_dump_grid(handle, stdout);
    printf("[ENTER or 'q'] ");
    if((fgets(buf, 128, stdin) == buf) && ('q' == buf[0])){
      estar_destroy(handle);
      return 0;
    }
    estar_compute_one(handle, 0.5);
  }
  estar_dump_grid(handle, stdout);
  
  estar_destroy(handle);
  return 0;
}


int create(int argc, char ** argv)
{
  const char * kernel_name = "lsm";
  unsigned xsize = 5;
  unsigned ysize = 3;
  
  if(argc > 1)
    kernel_name = argv[1];
  if((argc > 2) && (1 != sscanf(argv[2], "%ud", & xsize))){
    fprintf(stderr, "xsize: number expected, got \"%s\"\n", argv[2]);
    return -1;
  }
  if((argc > 3) && (1 != sscanf(argv[3], "%ud", & ysize))){
    fprintf(stderr, "ysize: number expected, got \"%s\"\n", argv[3]);
    return -1;
  }
  printf("kernel_name = \"%s\"\n"
	 "xsize = %u\n"
	 "ysize = %u\n",
	 kernel_name, xsize, ysize);
  
  return estar_create(kernel_name, xsize, ysize, 1, 0, stdout);
}
