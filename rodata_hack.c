#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <string.h>
#include <unistd.h>

int disable_rodata_protection(const char *text_data) {
	size_t text_addr = (size_t)text_data;
	size_t text_len = strlen(text_data);

	int pagesize = sysconf(_SC_PAGE_SIZE);
	if (pagesize == -1) {
		perror("sysconf");
		return -1;
	}

	printf("pagesize = <%d>\n", pagesize);
	printf("addr mod of pagesize = <%zu>\n", text_addr % pagesize);

	size_t aligned_text_addr = text_addr - text_addr % pagesize;
	size_t mprotect_size = text_addr % pagesize + text_len + 1;
	printf("mprotect_disable_len = <%zu>\n", mprotect_size);

	if (mprotect((void *)aligned_text_addr, 
		    mprotect_size,
		    PROT_READ | PROT_WRITE | PROT_EXEC)) {
		perror("mprotect");
		return -1;
	}
	
	return 0;
}

int main() {
	if (disable_rodata_protection("Denis")) {
		exit(EXIT_FAILURE);
	}

	strcpy("Denis", "Loh");

	printf("Denchik is %s\n", "Denis");

	return 0;
}
