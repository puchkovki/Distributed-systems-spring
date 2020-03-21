# **Parprog**
Семестровый курс по параллельному программированию для студентов 3 курса ФУПМ, МФТИ.
---
## **How to build**

1. Run ```build.sh``` file.
2. Or you may do it in any UNIX shell:

```
mpicc -Wall -Wextra -pedantic -O3 -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wcast-qual \
-Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all \
-fstack-protector -o $FULLDIR/test $FULLDIR/solution.cpp -lm
./test
```
