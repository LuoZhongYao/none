#include <string.h>
#include <limits.h>
#include <unistd.h>

static int offset=0;
static int nchar=0;
static int getlin(char *s);
static int nextc(void);

int main(int argc, char **argv)
{
	char line[_POSIX_MAX_INPUT+1];

	if (argc<2 || isatty(STDIN_FILENO)) {
		write(STDOUT_FILENO, "goto error\n", 11);
		lseek(STDIN_FILENO, 0, SEEK_END);
		return 1;
	}
	lseek(STDIN_FILENO, 0, SEEK_SET);
	do {
		if (getlin(line)) {
			write(STDOUT_FILENO, "label not found\n", 16);
			return 1;
		}
	} while (strcmp(line, argv[1])!=0);
	lseek(0, offset, SEEK_SET);
	return 0;
}

int getlin(char *s)
{
	int ch, i;
	i = 0;

	nchar=0;
	while ((ch=nextc())!='\0' && ch!=':') {
		while(ch!='\n' && ch!='\0')
			ch = nextc();
	}
	if (ch=='\0')
		return 1;
	while ((ch=nextc())==' ');
	while (ch!=' ' && ch!='\n' && ch!='\0') {
		s[i++] = ch;
		ch = nextc();
	}
	while(ch != '\n')
		ch = nextc();
	s[i] = '\0';
	return 0;
}

int nextc(void)
{
	char cc;
	offset++;
	
	if (nchar++ >= _POSIX_MAX_INPUT)
		write(STDOUT_FILENO, "line overflow\n", 14);

	if(read(STDIN_FILENO, &cc, 1))
		return cc;
	else
		return 0;
}
