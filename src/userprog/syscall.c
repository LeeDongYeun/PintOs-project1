#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "userprog/pagedir.h"
#include "threads/sysch.h"

static void syscall_handler (struct intr_frame *);
void *check_pointer(void *ptr);
void halt(void);
void exit(int status);
pid_t exec(const char *cmd_line);
int wait(pid_t pid);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open(const char *file);
int filesize(int fd);
int read(int fd, void *buffer, unsigned size);
int write(int fd, const void *buffer, unsigned size);
void seek(int fd, unsigned position);
unsigned tell(int fd);
void close(int fd);

struct lock lock_filesys;

void
syscall_init (void) 
{
	lock_init(&lock_filesys);
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
	  		f -> eax = (uint32_t) wait((int)check_pointer(esp_val+4));
	  		break;

	  	case SYS_CREATE:
	  		f -> eax = create((char *)check_pointer(esp_val+4), (unsigned)check_pointer(esp_val+8));
	  		break;

	  	case SYS_REMOVE:
	  		f -> eax = remove((char *)check_pointer(esp_val+4));
	  		break;

	  	case SYS_OPEN:
	  		f -> eax = open((char *)check_pointer(esp_val+4));
	  		break;

	  	case SYS_FILESIZE:
	  		f -> eax = filesize((int)check_pointer(esp_val+4));
	  		break;

	  	case SYS_READ:
	  		f -> eax = read((int)check_pointer(esp_val+4), (void *)check_pointer(esp_val+8), (unsigned)check_pointer(esp_val+12));
	  		break;

	  	case SYS_WRITE:
	  		f -> eax = write((int)check_pointer(esp_val+4), (void *)check_pointer(esp_val+8), (unsigned)check_pointer(esp_val+12));
	  		break;

	  	case SYS_SEEK:
	  		seek((int)check_pointer(esp_val+4), (unsigned)check_pointer(esp_val+8));
	  		break;

	  	case SYS_TELL:
	  		f -> eax = tell((int)check_pointer(esp_val+4));
	  		break;

	  	case SYS_CLOSE:
	  		close((int)check_pointer(esp_val+4));
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

int
wait(pid_t pid){

}

bool
create(const char *file, unsigned initial_size){
	bool result;

	lock_acquire(&lock_filesys);
	result = filesys_create(file, initial_size);
	lock_release(&lock_filesys);

	return result;
}

bool
remove(const char *file){
	bool result;
	
	lock_acquire(&lock_filesys);
	result = filesys_remove(file);
	lock_release(&lock_filesys);

	return result;
}



