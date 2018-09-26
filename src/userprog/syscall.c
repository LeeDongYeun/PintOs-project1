#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "userprog/pagedir.h"

static void syscall_handler (struct intr_frame *);
void *check_pointer(void *ptr);
void halt(void);
void exit(int status);
pid_t exec(const char *cmd_line);


void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
	int *esp_val = check_pointer(f->esp);

	switch(*esp_val){
	  	case SYS_HALT:
	  		halt();
	  		break;

	  	case SYS_EXIT:
	  		exit(*(int *)check_pointer(esp_val+4));
	  		break;

	  	case SYS_EXEC:
	  		f -> eax = (uint32_t) exec(*(char *)check_pointer(esp_val+4));
	  		break;

	  	case SYS_WAIT:
	  		break;

	  	case SYS_CREATE:
	  		break;

	  	case SYS_REMOVE:
	  		break;

	  	case SYS_OPEN:
	  		break;

	  	case SYS_FILESIZE:
	  		break;

	  	case SYS_READ:
	  		break;

	  	case SYS_WRITE:
	  		break;

	  	case SYS_SEEK:
	  		break;

	  	case SYS_TELL:
	  		break;

	  	case SYS_CLOSE:
	  		break;

	  	default :
	  		printf("default");
	  		break;

	}
	printf ("system call!\n");
	thread_exit ();
}

void*
check_pointer(void *ptr){
	void *result = 0;
	if(ptr >= PHYS_BASE){
		exit(-1);
	}

	else{

		void *pointer = pagedir_get_page(thread_current()->pagedir, ptr);
		if(!pointer){
			exit(-1);
		}
		else{
			result = ptr;
		}
	}

	return result;
}

void
halt(void){
	power_off();
}

void
exit(int status){
	struct thread *t = thread_current ();

	printf("%s: exit(%d)\n", t->name, status);
	thread_exit();
}

pid_t
exec(const char *cmd_line){
	return process_execute(cmd_line);
}