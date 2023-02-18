/*
 * Loader Implementation
 *
 * 2022, Operating Systems
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "exec_parser.h"

static so_exec_t *exec;
int fd;

static void segv_handler(int signum, siginfo_t *info, void *context)
{
	// daca primesc alt semnal decat SEGFAULT
	if(signum != SIGSEGV) {
		signal(signum, SIG_DFL);
		return;
	}

	// nu am informatii despre SEGFAULT
	if (info == NULL) {
		signal(SIGSEGV, SIG_DFL);
		return;
	}

	// parcurg segmentele
	for (int i = 0; i < exec->segments_no; i++) {
		int addr = (int)info->si_addr;  // adresa unde ne-am luat SEGFAULT
		so_seg_t *segment = exec->segments + i;

		if (segment->data == NULL)
			segment->data = (void *)calloc(1, segment->mem_size / getpagesize() + 1);
		// verific daca SEGFAULTUL se afla in pagina actuala
		if (segment->vaddr <= addr && addr <= segment->vaddr + segment->mem_size) {
			int page_nr = (addr - segment->vaddr) / getpagesize();

			if (((char *)(segment->data))[page_nr] == 0) {
				// pagina n-a mai fost mapata
				((char *)(segment->data))[page_nr] = 1;
				mmap((void *)(segment->vaddr + getpagesize() * page_nr), getpagesize(),
				  PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0);

				// sunt pe ultima pagina
				int len = segment->file_size -
				  (segment->file_size / getpagesize()) * getpagesize();

				// nu sunt pe ultima pagina
				if (page_nr != segment->file_size / getpagesize())
					len = getpagesize();

				// intreaga pagina apartine lui memsize
				if (page_nr * getpagesize() > segment->file_size)
					len = 0;

				// scriu din file descriptor
				lseek(fd, segment->offset + page_nr * getpagesize(), SEEK_SET);
				read(fd, (void *)(segment->vaddr + getpagesize() * page_nr), len);
				mprotect((void *)(segment->vaddr + getpagesize() * page_nr),
				  getpagesize(), segment->perm);

				return;
			}
		}
	}

	// SEGFAULTUL nu e intre segmente
	signal(SIGSEGV, SIG_DFL);
}

int so_init_loader(void)
{
	int rc;
	struct sigaction sa;

	memset(&sa, 0, sizeof(sa));
	sa.sa_sigaction = segv_handler;
	sa.sa_flags = SA_SIGINFO;
	rc = sigaction(SIGSEGV, &sa, NULL);
	if (rc < 0) {
		perror("sigaction");
		return -1;
	}
	return 0;
}

int so_execute(char *path, char *argv[])
{
	fd = open(path, O_RDONLY);
	exec = so_parse_exec(path);
	if (!exec)
		return -1;

	so_start_exec(exec, argv);

	close(fd);

	return -1;
}
