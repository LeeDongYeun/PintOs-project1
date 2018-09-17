#include <stdio.h>
#include "threads/thread.h"
#include "devices/timer.c"
#include "tests/threads/tests.h"

void test_list_func(void){
	struct thread *a;
	struct thread *b;

	a ->wake_ticks = 2;
	b ->wake_ticks = 3;

	if((bool)wake_ticks_compare(&a->elem, &b->elem, UNUSED)) printf("True");
	else printf("False");
}