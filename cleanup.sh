#!/bin/bash

for postfix in \~ .o .a .data .plot .plotfig .plotall .pdf  .pyc .bak
  do
  find . -name \*$postfix | xargs rm
done

for name in .sconsign .gdb_history .DS_Store
  do
  find . -name $name | xargs rm
done

rm -rf lib/ inc/ bin/
