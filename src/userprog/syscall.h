#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "lib/kernel/list.h"

void syscall_init (void);

struct file_descriptor{
	struct file *file;
	int fd;
	struct list_elem elem;
};
#endif /* userprog/syscall.h */
