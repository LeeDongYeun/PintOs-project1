#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"

static void syscall_handler (struct intr_frame *);
void check_pointer(void *ptr);

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
	  		exit(*(esp_val+4));
	  		break;

	  	case SYS_EXEC:
	  		exec(*(esp_val+4));
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

	else if(*ptr == NULL){
		exit(-1);
	}
	else{

		void *pointer = pagedir_get_page(thread_current()->pagedir, vaddr);
		if(!pointer){
			exit(-1);
		}
		else{
			result = ptr;
		}
	}

	return result;
}