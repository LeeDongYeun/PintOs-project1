#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/init.h"
#include "userprog/pagedir.h"
#include "threads/synch.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "lib/kernel/list.h"
#include "devices/input.h"

typedef int pid_t;

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

struct file *get_file(int fd);

struct lock lock_filesys;
struct file_descriptor{
	struct file *file;
	int fd;
	struct list_elem elem;

};

/*
file descriptor로 syscall_init에서 초기화하고
open함수에서 file을 오픈할 때마다 1씩 증가한다  
*/
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

/*
현재 thread에 있는 file_list에서 fd가 같은 값을 찾은 후 file에 대한
포인터를 리턴한다. 만약 없다면 -1을 리턴한다.
*/
struct file*
get_file(int fd){

	struct thread *curr = thread_current();
	struct list_elem *e;
	struct file_descriptor *file_descriptor;

	ASSERT (&curr->file_list != NULL);

	for(e = list_begin(&curr->file_list); e != list_end(&curr->file_list); e = list_next(e)){
		file_descriptor = list_entry(e, struct file_descriptor, elem);
		if(fd == file_descriptor->fd){
			return file_descriptor->file;
		}
	}

	return NULL;
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

/*
int
wait(pid_t pid){

}*/

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

/*
파일을 연 후 각 파일마다 fd를 배정하여 파일을 연 thread의 file_list에
file_descriptor라는 구조체를 저장한다. 그 후 fd를 증가시켜 준다.
*/
int
open(const char *file){
	bool result;

	lock_acquire(&lock_filesys);
	struct file *f = filesys_open(file);
	if(!file){
		result = -1;
	}
	else{
		struct thread *curr = thread_current();
		struct file_descriptor *file_descriptor;
		
		file_descriptor->file = f;
		file_descriptor->fd = curr->fd;
		list_push_back(&(curr->file_list), &(file_descriptor->elem));
		result = curr->fd;
		curr->fd++;
	}
	lock_release(&lock_filesys);

	return result;
}

/*
파일이 존재한다면 길이를 리턴하고
존재하지 않는다면 -1을 리턴한다.
*/
int
filesize(int fd){
	int result;

	lock_acquire(&lock_filesys);
	struct file * file = get_file(fd);

	if(!file){
		result = -1;
	}
	else{
		result = file_length(file);
	}
	lock_release(&lock_filesys);

	return result;
}


/*
fd 의 값이 0 이면 키보드로부터 버퍼에 값을 읽어오고,
아니면 fd에 맞는 file로부터 size만큼 값을 읽어온다.
*/
int
read(int fd, void *buffer, unsigned size){
	int result;

	if(fd == 0){
		unsigned i = 0;

		for(;i<size;i++)
			*((uint8_t *)buffer++) = input_getc();

		result = size;

	}
	else{
		lock_acquire(&lock_filesys);
		struct file *file = get_file(fd);

		if(!file){
			result = -1;
		}
		else{
			result = file_read(file, buffer, size);
		}
		lock_release(&lock_filesys);
	}

	return result;
}


int
write(int fd, const void *buffer, unsigned size){
	int result;

	if(fd == 1){
		putbuf(buffer, size);
		result = size;
	}
	else{
		lock_acquire(&lock_filesys);
		struct file *file = get_file(fd);

		if(!file){
			result = 0;
		}
		else{
			result = file_write(file, buffer, size);
		}
		lock_release(&lock_filesys);
	}

	return result;
}

void
seek(int fd, unsigned position){

	lock_acquire(&lock_filesys);
	struct file * file = get_file(fd);

	if(!file){
		file_seek(file, position);
	}

	lock_release(&lock_filesys);
}

unsigned
tell(int fd){
	off_t result;

	lock_acquire(&lock_filesys);
	struct file * file = get_file(fd);

	if(!file){
		result = -1;
	}
	else{
		result = file_tell(file);
	}
	lock_release(&lock_filesys);

	return result;
}

/*
현재 thread의 file_list에서 fd값이 일치하는 file을 찾은 후 
thread의 file_list에서 제거해주고, file 또한 닫는다.
*/
void
close(int fd){
	
	struct thread *curr = thread_current();
	struct list_elem *e;
	struct file_descriptor *file_descriptor;

	ASSERT (&curr->file_list != NULL);

	for(e = list_begin(&curr->file_list); e != list_end(&curr->file_list); e = list_next(e)){
		file_descriptor = list_entry(e, struct file_descriptor, elem);
		
		if(fd == file_descriptor->fd){
			list_remove(&file_descriptor->elem);

			lock_acquire(&lock_filesys);
			file_close(file_descriptor->file);
			lock_release(&lock_filesys);
			break;
		}
	}
}



