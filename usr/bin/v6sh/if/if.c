/*
 * if exprression cmd [arg]....
 * if 是 test 命令的前身。在测试完表达式之后，如果为真则执行后面的命令。
 * 这里的 if 命令建立在 v7 test 命令基础上。
 */

#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

static int	ac;
static char	**av;
static int	ap;
static char	*tmp;
static jmp_buf	env;

#define EQ(a,b)	((tmp=a)==0?0:(strcmp(tmp,b)==0))
#define DIR 1
#define FIL 2

static char *nxtarg(int mt);
static int expr(void);
static int e1(void);
static int e2(void);
static int e3(void);
static int tio(char *a, int  f);
static int ftype(char *f);
static int fsizep(char *f);
static void synbad(char *s1, char *s2);
static int doex(char *earg);

int main(int argc, char**argv)
{
	ac = argc;
	av = argv;
	ap = 1;

	if (ac<=1)
		return 1;
	if (setjmp(env) != 0)
		return 1;
	if (expr())
		return doex(0);
	else	
		return 1;
}
/*
 * 获得下一个参数。mt 指示在读到列表尾部的时候是否为语法错误，
 * 1 返回空指针，0 提示错误并退出。
 */
char *nxtarg(int mt)
{
	if (ap>=ac) {
		if(mt) {
			ap++;
			return(0);
		}
		synbad("argument exprected","");
	}
	return(av[ap++]);
}
/*
 * expr ::= * e1 -o expr | expr
 */
int expr(void)
{
	int p1;

	p1 = e1();
	if (EQ(nxtarg(1), "-o"))
		return(p1 | expr());
	ap--;
	return(p1);
}
/*
 * e1 ::= e2 -a e1 | e1
 */
int e1(void)
{
	int p1;

	p1 = e2();
	if (EQ(nxtarg(1), "-a"))
		return (p1 & e1());
	ap--;
	return(p1);
}
/*
 * e2 ::= e3 | ! e3
 */
int e2(void)
{
	if (EQ(nxtarg(0), "!"))
		return(!e3());
	ap--;
	return(e3());
}
/*
 * e3 ::= ( expr ) | { command ... } | -op file | string -op string
 */
int e3(void)
{
	int p1,r;
	register char *a;
	char *p2;
	int int1, int2;
	int ccode;

	a=nxtarg(0);
	if(EQ(a, "(")) {
		p1 = expr();
		if(!EQ(nxtarg(0), ")")) 
			synbad(") exprected","");
		return(p1);
	}
	if(EQ(a, "{")) { /* 执行一个命令并等待退出状态 */
		if(fork()) /* 父进程执行部分 */ 
			wait(&ccode);
		else { /* 子进程执行部分 */
			if ((r=doex("}")) != 0 && r == 255) 
				synbad("} exprected","");
			else
				exit(r);
		}
		while((a=nxtarg(0)) && (!EQ(a,"}")));
		return(ccode? 0 : 1);
	}
	if(EQ(a, "-r"))
		return(tio(nxtarg(0), 0));
	if(EQ(a, "-w"))
		return(tio(nxtarg(0), 1));
	if(EQ(a, "-d"))
		return(ftype(nxtarg(0))==DIR);
	if(EQ(a, "-f"))
		return(ftype(nxtarg(0))==FIL);
	if(EQ(a, "-s"))
		return(fsizep(nxtarg(0)));
	if(EQ(a, "-t")) {
		if(ap>=ac)
			return(isatty(STDOUT_FILENO));
		else
			return(isatty(atoi(nxtarg(0))));
	}
	if(EQ(a, "-n"))
		return(!EQ(nxtarg(0), ""));
	if(EQ(a, "-z"))
		return(EQ(nxtarg(0), ""));

	p2 = nxtarg(1);
	if (p2==NULL)
		return(!EQ(a,""));
	if(EQ(p2, "="))
		return(EQ(nxtarg(0), a));
	if(EQ(p2, "!="))
		return(!EQ(nxtarg(0), a));
	if(EQ(a, "-l")) {
		int1=strlen(p2);
		p2=nxtarg(0);
	} else{
		int1=atoi(a);
	}

	int2 = atoi(nxtarg(0));
	if(EQ(p2, "-eq"))
		return(int1==int2);
	if(EQ(p2, "-ne"))
		return(int1!=int2);
	if(EQ(p2, "-gt"))
		return(int1>int2);
	if(EQ(p2, "-lt"))
		return(int1<int2);
	if(EQ(p2, "-ge"))
		return(int1>=int2);
	if(EQ(p2, "-le"))
		return(int1<=int2);

	synbad("unknown operator ",p2);
	return 0;
}
/*
 * 文件访问测试
 */
int tio(char *a, int  f)
{

	f = open(a, f);
	if (f>=0) {
		close(f);
		return(1);
	}
	return(0);
}
/*
 * 文件类型测试
 */
int ftype(char *f)
{
	struct stat statb;

	if(stat(f,&statb)<0)
		return(0);
	if((statb.st_mode&S_IFMT)==S_IFDIR)
		return(DIR);
	return(FIL);
}
/*
 * 文件大小测试
 */
int fsizep(char *f)
{
	struct stat statb;
	if(stat(f,&statb)<0)
		return(0);
	return(statb.st_size>0);
}
/*
 * 打印错误信息并退出
 */
void synbad(char *s1, char *s2)
{
	write(2, "if: ", 6);
	write(2, s1, strlen(s1));
	write(2, s2, strlen(s2));
	write(2, "\n", 1);
	longjmp(env,1);
}
/*
 * 执行命令
 * earg 是终止参数
 */
int doex(char *earg)
{
	char *p=av[ap];
	char **v=av+ap;

	if (p == NULL)
		return 0;
	if (earg != NULL) {
		while (*v != 0 && !EQ(*v,earg))
			v++;
		if (*v == NULL)
			return 255;
		else
			*v=NULL;
	}
	execvp(p, av+ap);
	return errno;
}
