/*
 * 这个 shell 是 UNIX v6 的 sh 在 POSIX 环境下的重新实现。
 * UNIX v6 是受 BSD 许可证保护的自由软件，其中 sh 的原作者是 Ken Thompson。
 * 余对这个 shell 的语法做了细节修改，对代码做了重写和注释。
 * 余对这个程序和相关文档不做任何担保，放弃一切权利，不承担任何责任和义务。
 *
 * 最近修订: 2006-07-13   寒蝉退士(http://mhss.cublog.cn)
 * 做的工作主要有: K&R C -> ANSI C，unix v6/v7 -> POSIX，去掉了进程记帐和 ^，
 * 增加了 $?、umask 和 exec，去除了 goto 语句，增加了中文注释。
 * 上次修订: 2004-09-06
 */

/*
 * Copyright(C) Caldera International Inc. 2001-2002. All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met: 
 *
 *     Redistributions of source code and documentation must retain the above
 *     copyright notice, this list of conditions and the following disclaimer.
 *     Redistributions in binary form must reproduce the above copyright notice, 
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution. 
 *
 *     All advertising materials mentioning features or use of this software
 *     must display the following acknowledgement: 
 *
 *     This product includes software developed or owned by Caldera International, Inc. 
 *
 *     Neither the name of Caldera International, Inc. nor the names of other
 *     contributors may be used to endorse or promote products derived from this
 *     software without specific prior written permission. 
 * 
 * USE OF THE SOFTWARE PROVIDED FOR UNDER THIS LICENSE BY CALDERA INTERNATIONAL, INC. 
 * AND CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL CALDERA INTERNATIONAL, INC. BE LIABLE FOR 
 * ANY DIRECT, INDIRECT INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF 
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE. 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/inter.h>
#include <posix.h>
#include <none/scntl.h>
#include <none/fcntl.h>

#define	QUOTE 0x80 /* 引用标志位，限制了字符集为 7 位 ASCII */

/* 语法树节点字段定义 */
#define	DTYP 0 /* type: 节点类型 */
#define	DLEF 1 /* left: 左子节点或输入重定向文件描述符 */
#define	DRIT 2 /* right: 右子节点或输出重定向文件描述符 */
#define	DFLG 3 /* flag: 标志位，语法树节点属性 */
#define	DSPR 4 /* space: 在简单命令的时候原版本是空位现在是参数个数，
                * parentheses: 在复合命令时指向子语法树 */
#define	DCOM 5 /* command: 参数字列表 */

/* 类型定义，用于 DTYP */
#define	TCOM 1 /* command: 简单命令 */
#define	TPAR 2 /* parentheses: 复合命令 */
#define	TFIL 3 /* filter: 过滤器 */
#define	TLST 4 /* list: 命令列表 */

/* 标志定义，用于 DFLG */
#define	FAND 0x01 /* and: & 后台执行 */
#define	FCAT 0x02 /* catenate: >> 添加方式的输出重定向 */
#define	FPIN 0x04 /* pipe in: 命令从管道获得输入 */
#define	FPOU 0x08 /* pipe out: 命令向管道发送输出 */
#define	FPAR 0x10 /* parentheses: 复合命令中最后一个命令 */
#define	FINT 0x20 /* interrupt: 后台进程忽略中断信号 */
#define	FPRS 0x40 /* print string: 打印后台进程 pid */

char	*dolp = NULL; /* dollar p: 指向命令行参数的指针 */
char	**dolv; /* dollar v: 命令行参数列表 */
int 	dolc; /* dollar c: 命令行参数个数 */
char	pidp[11]; /* 保存 sh 自身进程 pid 的字符串 */
char	*promp; /* 提示符 */
int 	peekc = 0; /* 预读字符 */
int 	gflg = 0; /* 全局标志，两种用途：缓冲区溢出，或包含通配符 */
int 	error; /* 语法分析发现错误标志 */
int 	setintr = 0; /* 设置中断信号忽略的标志 */
char	*arginp = NULL; /* 指向包含要执行的命令的参数，执行之后退出 */
int 	onelflg = 0; /* one line flag: 读取一行命令执行并退出标志 */
#ifdef NONE_PID
uid_t	uid; /* sh 进程的真实 uid */
#endif
#ifdef  NONE_JUMP
jmp_buf	jmpbuf; /* 跨函数跳转用来保存当前状态的缓冲区 */
#endif

int 	exitstat = 0; /* 执行命令的终止状态 */
char	exitp[4]; /* 保存终止状态的字符串 */

char	*linep, *elinep; /* 操纵行缓冲区的指针 */
char	**argp, **eargp; /* 操纵字列表存储空间的指针 */
int 	*treep, *treeend; /* 操纵语法树节点存储空间的指针 */

#define	LINSIZ 1024 /* _POSIX_ARG_MAX */
#define	ARGSIZ LINSIZ/20 /* 假定平均每个字有 20 个字符 */
#define	TRESIZ ARGSIZ*2
char	line[LINSIZ]; /* 命令行缓冲区 */
char	*args[ARGSIZ]; /* 字列表 */
int 	trebuf[TRESIZ]; /* 存储语法树节点 */

/* 诊断消息 */
#define NSIGMSG 18
char *mesg[NSIGMSG] = {
	NULL,
	"Hangup", /* SIGHUP 1 */
	NULL, /* SIGINT 2 */
	"Quit", /* SIGQUIT 3 */
	"Illegal instruction", /* SIGILL 4 */
	"Trace trap", /* SIGTRAP 5 */
	"Abort", /* SIGABRT 6 */
	"Signal 7", /* SIGEMT 7 */
	"Floating point exception", /* SIGFPE 8 */
	"Killed", /* SIGKILL 9 */
	"Signal 10", /* SIGBUS 10 */	
	"Segmentation violation", /* SIGSEGV 11 */
	"Bad argument to system call", /* SIGSYS 12 */
	NULL, /* SIGPIPE 13 */
	"Alarm clock", /* SIGALRM 14 */
	"Software termination signal from kill", /* SIGTERM 15 */
	"Signal 16", /* 16 */
	"Child process terminated or stopped",/* SIGSTOP 17 */
};

void lexscan(void);
void word(void);
int  nextc(void);
int  readc(void);
int  *parse(void);
int  *cmdlist(char **p1, char **p2);
int  *pipeline(char **p1, char **p2);
int  *command(char **p1, char **p2);
int  *tree(int n);
void execute(int *t, int *pf1, int *pf2);
int  redirect(int *t);
int  builtin(int *t);
int  execcmd(int *t);
int  texec(char *f, char **t);
void scan(char **t, int (*f)());
int  tglob(int c);
int  trim(int c);
void err(char *s);
void prs(char *as);
void prc(char c);
void prn(int n);
void sprn(int n, char *s);
int  any(int c, char *as);
int  equal(char *as1, char *as2);
int  pwait(pid_t i);

#if 0
extern int glob(int argc, char *argv[]);
#endif

int main(int argc, char **argv)
{
	int *t; /* 语法树 */

	object_t f;	/* 文件描述符 */
#ifdef  NONE_DUP
	/* 关闭打开的文件 */
	for(f=STDERR_FILENO; f<sysconf(_SC_OPEN_MAX); f++)
		close(f);
	/* 复制标准输出到标准错误输出 */
	if((f=dup(STDOUT_FILENO)) != STDERR_FILENO)
		close(f);
#endif

#ifdef NONE_PID
	/* 获得进程标识(32位整数)，并转换成字符串，临时借用 dolc 变量 */
	dolc = (int)getpid();
	sprn(dolc, pidp);
	/* 判断当前时候是否根用户，设置正确的提示符 */
	if((uid = getuid()) == 0) /* 根用户 */
		promp = "# ";
	else
		promp = "% ";
	setuid(uid);
	setgid(getgid());
#else
    promp = "% ";
#endif
    (void)f;
	if(argc > 1) {	/* 有参数或选项 */
		promp = 0;	/* 设置为非交互模式 */
		if (*argv[1]=='-') { /* 有选项 */
			**argv = '-';  /* 把参数 0 的第一个字符设置为 - */
			if (argv[1][1]=='c' && argc>2) /* -c 把下一个参数作为命令执行 */
				arginp = argv[2];
			else if (argv[1][1]=='t') /* -t: 从标准输入读入一行执行并退出 */
				onelflg = 2;
		} else { /* 有命令文件 */
			/* 在标准输入上打开包含要执行的命令的文件 */
			f = open(argv[1], O_RDONLY);
			if(f < 0) { /* 打不开指定文件 */
				prs(argv[1]);
				err(": cannot open");
			}
            dup2(f,STDIN_FILENO);
		}
	}
	if(**argv == '-') { /* 有选项 */
		setintr++;
		/* 设置中断信号处理程序为忽略信号 */
#ifdef  NONE_SIGNAL
		signal(SIGQUIT, SIG_IGN);
		signal(SIGINT, SIG_IGN);
#endif
	}
	dolv = argv + 1; /* 参数列表指针右移 */
	dolc = argc - 1; /* 参数数目减少 */

	/* 主循环: 扫描命令行，分析和执行语法树 */
	for(;;) {
		error = 0;
		gflg = 0;
		if(promp != 0)	/* 交互模式运行 */
			prs(promp);	
		lexscan();
		if(gflg != 0) {/* 发生命令行字符溢出 */
			err("Command line overflow");
			continue;
		}
		t = parse();
		if(error != 0) {/* 语法分析发现错误 */
			err("syntax error");
			continue;
		}
		execute(t,NULL,NULL);
	}
	return 0;
}

/*
 * 词法扫描
 */
void lexscan(void)
{
	register char *cp;
	register int c;
	/* 初始化行缓冲区、字列表空间和相关指针 */
	argp = args+1; /* 空出一个位置 */
	eargp = args+ARGSIZ-5;
	linep = line;
	elinep = line+LINSIZ-5;

	/* 过滤掉注释行 */
	do c = nextc();
	while (c == ' ' || c == '\t');
	if (c == '#')
		while ((c = nextc()) != '\n');
	peekc = (char) c; /* 送回最后的换行符 */
	
	/* 把命令行扫描到字列表中 */
	do {
		cp = linep; /* cp 指向当前要读入的字在行缓冲区中的位置 */
		word(); /* 读入一个字到行缓冲区中 */
	} while(*cp != '\n'); /* 循环直到读到换行符 */
}

/*
 * 从输入中读出一个字到行缓冲区中，并增加一个字列表元素
 * 被调于 lexscan() 
 * 调用 nextc() readc()
 */
void word(void)
{
	register int c, c1;
	
	/* 字列表的当前元素指针指向当前要读入的字在行缓冲区中的位置 */
	*argp++ = linep;

	/* 忽略字前空白 */
	do c = nextc();
	while (c == ' ' || c == '\t');
	/* 处理 shell 的元字符和换行符 */
	if(any(c, ";&<>()|\n")) {	 
		*linep++ = (char) c; /* 把这个元字符或换行符写到行缓冲区中 */
		*linep++ = '\0'; /* 终结这个字符串 */
		return;	
	}
	/* 读到普通字符 */
	peekc = c; /* 送回这个普通字符 */
	for(;;) {
		c = nextc();
		if(any(c, " \t;&<>()|\n")) { /* 读到空白、元字符或换行符 */
			peekc = (char) c; /* 送回这个字符 */
			*linep++ = '\0'; /* 终结这个字符串 */
			return;
		}
		if(c == '\'' || c == '"') {/* 读到单引号或双引号 */
			c1 = c; /* 略过这个单引号或双引号 */
			while((c=readc()) != c1) {
				if(c == '\n') {
					error++;	/* 引用没有在本行完结 */
					peekc = (char) c; /* 送回这个字符 */
					return;
				}
				/* 对引号包围的字符设置引用标志位，并写到行缓冲区中 */
				*linep++ = (char) c|QUOTE;
			}
			continue; /* 略过匹配的单引号或双引号 */ 
		}
		*linep++ = (char) c; /* 把这个普通字符写入行缓冲区 */
	}
}

/*
 * 从输入中读出一个字符，带有变量替换
 * 被调于 word()
 * 调用 readc()
 */
int nextc(void)
{
	int c;

	if(peekc) { /* 已经预读了一个字符 */
		c = peekc;
		peekc = 0;
		return c;
	}
	if(argp > eargp) { /* 字列表空间溢出 */
		argp -= 10;
		while((c=nextc()) != '\n'); /* 忽略多出来的所有字符 */
		argp += 10;
		err("Too many args");
		gflg++;
		return c;
	}
	if(linep > elinep) {  /* 行缓冲区空间溢出 */
		linep -= 10;
		while((c=nextc()) != '\n'); /* 忽略多出来的所有字符 */
		linep += 10;
		err("Too many characters");
		gflg++;
		return c;
	}
	if(dolp == NULL) { /* 当前未处理 $ */
		c = readc();
		if(c == '\\') { /* 转义符 */
			c = readc();
			if(c == '\n') /* 行接续 */
				return ' ';
			return(c|QUOTE); /* 引用这个字符 */
		}
		if(c == '$') { /* 变量替换 */
			c = readc();
			if(c>='0' && c<='9') { /* 位置参数 */
				if(c-'0' < dolc)
					dolp = dolv[c-'0'];
			}
			if(c == '$') { /* 进程标识 */
				dolp = pidp;
			}
			if(c == '?') { /* 退出状态 */
				sprn(exitstat, exitp);
				dolp = exitp;
			}
		}
	}
	if (dolp != NULL) { /* 当前处理 $ */
		c = *dolp++;
		if(c != '\0')
			return c;
		dolp = NULL;
	}
	return c&~QUOTE; /* 清除引用位 */
}

/*
 * 从输入中读取一个字符
 * 被调于 word() nextc()
 */
int readc(void)
{
	char cc;
	int c;

	if (arginp) { /* 有 -c 选项，onelflg == 0 */
		/* 从 arginp 中读取字符 */
		if ((c = (int)*arginp++) == 0) { /* 没有要作为命令的参数 */
			arginp = NULL; 
			onelflg++; /* 设置下次执行本函数的时候退出 */
			c = '\n';
		}
		return c;
	}
	if (onelflg==1)
		exit(0);
	if(read(STDIN_FILENO, &cc, 1) != 1) /* 读一个字符 */
		exit(0);
	/* 有 -t 选项，onelflg == 2，从标准输入读入了换行符 */
	if (cc=='\n' && onelflg)
		onelflg--; /* 设置下次执行本函数的时候退出 */
	return (int)cc;
}

/*
 * 语法分析
 */
int * parse(void)
{
	/* 初始化语法树节点空间和相关指针 */
	treep = trebuf;
	treeend = &trebuf[TRESIZ];
	char **p = args+1; /* 相应的留出空位 */

#ifdef  NONE_JUMP
	if (setjmp(jmpbuf) != 0)
	    /* 没有空间分配语法树节点的了 */
		return NULL;
#endif

	/* 忽略前导的换行符 */
	while(p != argp && (int)**p == '\n')
		p++;
	/* args 指向是字列表的第一个元素，
	 * argp 指向是字列表的最后一个元素后面的一个元素 */
	return  cmdlist(p, argp);
}

/* 
 *  命令列表
 *	cmdlist:
 *		empty
 *		pipeline
 *		pipeline & cmdlist
 *		pipeline ; cmdlist
 */
int * cmdlist(char **p1, char **p2)
{
	register char **p;
	int *t, *t1;
	int l;

	if (p1 == p2)
		return NULL; /* 空命令列表 */

	l = 0; /* 嵌套层数 */
	for(p=p1; p!=p2; p++)
		switch(**p) {

		case '(':
			l++;
			break;

		case ')':
			l--;
			if(l < 0)
				error++;
			break;

		/* 找到一个列表分隔符 */
		case '&':
		case ';':
		case '\n':
			if(l == 0) { /* 如果这个列表分隔符不在圆括号包围内 */
				l = **p;
				t = tree(4);
				t[DTYP] = TLST; /* 类型是命令列表 */
				t[DLEF] = (int)pipeline(p1, p); 
				t[DFLG] = 0;  /* 命令列表节点不设置任何标志位 */
				if(l == '&') { /* 需要后台处理 */
					t1 = (int *)t[DLEF];
					/* 设置左子节点标志 */
					t1[DFLG] |= FAND|FPRS|FINT;	
					/* 
					 * 后台运行的这些标志位只设置到在管道线节点上，
					 * 所以在执行的时候需要通过继承来下传到各个命令节点上。
					 */
				}
				/* 认定多余的列表分隔符为语法错误 */
				if (any((int)*(p+1),";&")) {
					error++;
					return NULL;
				}
				t[DRIT] = (int)cmdlist(p+1, p2); 
				return t;
			}
		}
	/* 没有找到列表分隔符，这个命令列表出现在圆括号内 */
	if(l == 0)
		return pipeline(p1, p2);
	error++;
	return NULL;
}

/*
 *  管道线 
 *	pipeline:
 *		command
 *		command | pipeline
 */
int * pipeline(char **p1, char **p2)
{
	register char **p;
	int l, *t;

	l = 0; /* 嵌套层数 */
	for(p=p1; p!=p2; p++)
		switch(**p) {

		case '(':
			l++;
			break;

		case ')':
			l--;
			break;

		/* 找到一个管道符号 */
		case '|':
			if(l == 0) { /* 如果管道符号不在括号包为包围内 */
				t = tree(4);
				t[DTYP] = TFIL;	/* 类型是管道线 */
				t[DLEF] = (int)command(p1, p);
				t[DRIT] = (int)pipeline(p+1, p2);
				t[DFLG] = 0; /* 标志位由上级的 cmdlist 设置 */
				return t;
			}
		}
	/* 没有找到管道符号，是管道线末端或简单命令 */
	return command(p1, p2);
}

/*
 *  命令
 *	command:
 *		( cmdlist ) [ < in  ] [ > out ]
 *		word word* [ < in ] [ > out ]
 */
int * command(char **p1, char **p2)
{
	register char **p;
	char **lp = NULL, **rp = NULL; 
	int *t;
	int n = 0, l = 0, i = 0, o = 0, c, flg = 0;

	if(**p2 == ')')	/* 这个命令是在圆括号包围中的命令列表的最后一个命令 */
		flg |= FPAR; /* 这个命令要在 subshell 的进程内执行 */

	for(p=p1; p!=p2; p++)
		switch(c = **p) {

		case '(':
			if(l == 0) {
				if(lp != NULL)
					error++;
				lp = p+1; /* 最外层圆括号内列表的第一个字 */
			}
			l++;
			break;

		case ')':
			l--;
			if(l == 0)
				rp = p; /* 最外层圆括号内的最后一个字后面的')' */
			break;

		case '>':
			p++;
			if(p!=p2 && **p=='>') /*  >> 重定向 */
				flg |= FCAT;
			else
				p--;
		case '<':
			if(l == 0) {
				p++; /* 重定向的文件描述符 */
				if(p == p2) { /* 重定向符号之后没有字符了 */
					error++;
					p--;
				}
				if(any(**p, "<>("))  /* 重定向文件名开头的字符不合法 */
					error++;
				if(c == '<') {
					if(i != 0)
						error++;
					i = (int)*p; /* 保存输入重定向文件的全路径名 */
				} else { /* > 或 >> */
					if(o != 0)
						error++;
					o = (int)*p; /* 保存输出重定向文件的全路径名 */
				}
				break;
			}

		default: 
			if(l == 0) /* 这个普通的字不在圆括号包围内 */
				p1[n++] = *p;
			/* 形成参数列表，前移挤掉了重定向符号和相应的文件，并计数 */
		}

	if(lp != 0) { /* 有圆括号 */
		if(n != 0)  /* 还有不在圆括号中的普通的字 */
			error++;
		t = tree(5); /* 有 DSPR 字段 */
		t[DTYP] = TPAR; /* 类型是复合命令 */
		t[DSPR] = (int)cmdlist(lp, rp);
	} else { /* 简单命令 */
		if(n == 0)	/* 没有命令名字 */
			error++; 
		t = tree(6);
		/* 在语法树节点中有 DSPR，DCOM 字段 */
		t[DTYP] = TCOM; /* 类型是简单命令 */
		t[DSPR] = n; /* 字列表元素数目 */
		t[DCOM] = (int)p1; /* 字列表 */
	}
	t[DFLG] = flg; /* 根据情况设置的 FPAR 或 FCAT 标志 */
	t[DLEF] = i; /* 输入重定向文件的全路径名 */
	t[DRIT] = o; /* 输出重定向文件的全路径名 */
	return t;
}

/*
 * 分配指定大小的树结点
 * 被调于 cmdlist() pipeline() command()
 */
int * tree(int n)
{
	int *t;

	t = treep;
	treep += n;
	if (treep>treeend) { /* 语法树节点空间不够 */
		prs("Command line overflow\n");
		error++;
#ifdef NONE_JUMP
		longjmp(jmpbuf,1);
#else
        return NULL;
#endif
	}
	return t;
}

/* 
 * 管道线语法树的执行，pf1 流入管道，pf2 流出管道
 * a | b | c 管道线的语法树：
 *     TFIL1
 *     /   \
 *  TCOM1 TFIL2 
 *    |   /   \
 *    a TCOM2 TCOM3
 *        |     |
 *        b     c
 * 执行序列：
 * execute(TFIL1, NULL, NULL);
 * execute(TOCM1, NULL, pv1);	TOCM1.TFLG: FPOU;	父进程打开着 pv1
 * execute(TFIL2, pv1, NULL);	TFIL2.TFLG: FPIN;
 * execute(TCOM2, pv1, pv2);	TCOM2.TFLG: FPIN, FPOU;	父进程关闭了 pv1, 打开着 pv2
 * execute(TCOM3, pv2, NULL);	TCOM3.TFLG: FPIN;	父进程关闭了 pv2
 */
void execute(int *t, int *pf1, int *pf2) {
    (void)pf1;
    (void)pf2;
	int i, f; 
#ifdef NONE_PIPE
    int pv[2];
#endif
	register int *t1;

	if(t == 0)
		return;
	switch(t[DTYP]) {
	case TLST:	/* 命令列表类型 */
		f = t[DFLG];	
		if((t1 = (int *)t[DLEF]) != NULL)
			/* 向子节点下传 FINT 标志的当前状态 */
			t1[DFLG] |= f&FINT;	
		execute(t1,NULL,NULL);
		if((t1 = (int *)t[DRIT]) != NULL)
			t1[DFLG] |= f&FINT;
		execute(t1,NULL,NULL);
		/*
		 * 命令列表位于复合命令中当中的时候，TLST 节点
		 * 只从上层 TPAR 节点继承 FINT 标志。 
		 */
		return;

	case TFIL:	/* 管道线类型 */
		f = t[DFLG];
#ifdef  NONE_PIPE
		pipe(pv); /* 建立管道 */
		t1 = (int *)t[DLEF];
		/* 向左子节点下传 FPIN FINT FPRS 标志的当前状态，
		 * 并设置它的 FPOU 标志  */
		t1[DFLG] |= FPOU | (f&(FPIN|FINT|FPRS));
		execute(t1, pf1, pv); /* 命令输出流入新建的管道 */
		t1 = (int *)t[DRIT];
		/* 向右子节点下传 FPOU FINT FAND FPRS 标志的当前状态，
		 * 并设置它的 FPIN 标志  */
		t1[DFLG] |= FPIN | (f&(FPOU|FINT|FAND|FPRS));
		execute(t1, pv, pf2); /* 命令输入来自新建的管道 */
#endif
		/*
		 * 只有管道线末端的命令能从上层节点继承到 FAND 标志
		 */
		return;

	case TCOM: /* 简单命令类型入口 */
		/* 筛选出内置命令 */
		if (builtin(t))
			return;

	case TPAR: /* 复合命令类型切入点 */
		f = t[DFLG];
		i = 0;
		/* 
		 * 为简单命令、复合命令整体和复合命令除最后一个命令之外的所有命令
		 * 建立子进程，复合命令最后一个命令使用为复合命令整体建立的子进程
		 */
		if((f&FPAR) == 0) 
			i = fork(); 
		if(i == -1) { /* 进程复制失败 */
			err("try again");
			return;
		}
		/*
		 * 父进程执行部分
		 */
		if(i != 0) { 
			if((f&FPIN) != 0) { /* 子进程有流入管道 */
				/* 在读管道的子进程没有建立之前，父进程保持打开这个管道
				 * 当读管道的子进程已经建立之后，父进程关闭管道读出写入端 */
#ifdef  NONE_PIPE
				close(pf1[0]); 
				close(pf1[1]);
#endif
			}
			if((f&FPRS) != 0) { /* 需要打印子进程 pid */
				prn(i);
				prs("\n");
			}
			if((f&FAND) != 0) { /* 后台命令不需要等待子进程 */
				exitstat = 0;
				return;
			}
			if((f&FPOU) == 0) /* 子进程是前台的简单命令、或管道线末端的命令 */
				exitstat = pwait(i); /*  等待子进程终止 */
			return;
		}
		/*
		 * 子进程执行部分
		 */
		if (redirect(t)) /* 重定向标准输入输出 */
			exit(1);
		if((f&FPIN) != 0) { /* 有流入管道 */
#if  defined(NONE_PIPE) && defined(NONE_PIPE)
			close(STDIN_FILENO);
			/* 接入管道读出端 */
			dup(pf1[0]);
			close(pf1[0]);
			/* 关闭管道写入端 */
			close(pf1[1]);
#endif
		}
		if((f&FPOU) != 0) {	/* 有流出管道 */
#if  defined(NONE_PIPE) && defined(NONE_PIPE)
			/* 接入管道写入端 */
			close(STDOUT_FILENO);
			dup(pf2[1]);
			close(pf2[0]);
			/* 关闭管道读出端 */
			close(pf2[1]);
#endif
		}
		/* 后台进程没有做重定向其标准输入 */
		if((f&FINT)!=0 && t[DLEF]==0 && (f&FPIN)==0) {
#if  defined(NONE_PIPE) && defined(NONE_PIPE)
			close(STDIN_FILENO);
			open("/dev/null", O_RDONLY);
#endif
		}
		/* 是未设置中断信号忽略标志的前台命令，
		 * 并在非交互模式下已经设置中断信号处理程序为忽略信号 */
		if((f&FINT) == 0 && setintr) {
			/* 恢复中断信号处理程序为缺省 */
#ifdef  NONE_SIGNAL
			signal(SIGINT, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);
#endif
		}
		/* 
		 * 复合命令在子 shell 中执行，父 shell 节点中的 FAND, FPRS 标志位作用于
		 * 子 shell 本身的进程，子 shell 中的节点不继承 FAND, FPRS 标志位。
		 */
		if(t[DTYP] == TPAR) { 
			if((t1 = (int *)t[DSPR]) != NULL)
				/* 向子节点下传 FINT 标志的当前状态 */
				t1[DFLG] |= f&FINT;
			execute(t1,NULL,NULL);
			exit(1);
		}
		i=execcmd(t); /* 执行命令 */
		exit(i);
	}
}

/*
 * 重定向标准输入输出
 */
int redirect(int *t)
{
	int i;
#ifdef NONE_CREATE
	int f = t[DFLG];
#endif
	
	if(t[DLEF] != 0) { /* 有输入重定向 */
		/* 重定向标准输入 */
		close(STDIN_FILENO);
		i = open((char *)t[DLEF], 0/*! O_RDONLY !*/);
		if(i < 0) {
			prs((char *)t[DLEF]);
			err(": cannot open");
			return 1;
		}
	}
#ifdef  NONE_CREATE
	if(t[DRIT] != 0) { /* 有输出重定向 */
		if((f&FCAT) != 0) { /* >> 重定向 */
			i = open((char *)t[DRIT], O_WRONLY);
			if(i >= 0)
				lseek(i, 0, SEEK_END);
		} else {
			i = creat((char *)t[DRIT], 0666);
			if(i < 0) {
				prs((char *)t[DRIT]);
				err(": cannot create");
				return 1;
			}
		}
		/* 重定向标准输出 */
		close(STDOUT_FILENO);
		dup(i);
		close(i);
	}
#endif
	return 0;
}

/*
 * 内置命令处理
 */
int builtin(int *t)
{
	register char *cp1, *cp2;
	int c, i = 0;
	int ac = t[DSPR];
	char **av = (char**)(t[DCOM]);

	cp1 = av[0];
	if(equal(cp1, "cd") || equal(cp1, "chdir")) {
		if(ac == 2) { /* 有一个参数 */
#ifdef NONE_CDDIR
			if(chdir(av[1]) < 0)
				err("chdir: bad directory");
#endif
		} else
			err("chdir: arg count");
		return 1;
	}
	if(equal(cp1, "shift")) {
		if(dolc < 1) {
			prs("shift: no args\n");
			return 1;
		}
		dolv[1] = dolv[0]; /* 命令文件名右移替代了第一个位置参数 */
		dolv++;
		dolc--;
		return 1;
	}
	if(equal(cp1, "login")) {
#ifdef NONE_EXECV
		if(promp != 0) {
			av[ac] = NULL;
			execv("/bin/login", av);
		}
#endif
		prs("login: cannot execute\n");
		return 1;
	}
	if(equal(cp1, "newgrp")) {
#ifdef  NONE_EXECV
		if(promp != 0) {
			av[ac] = NULL;
			execv("/bin/newgrp", av);
		}
#endif
		prs("newgrp: cannot execute\n");
		return 1;
	}
	if(equal(cp1, "wait")) {
		pwait(-1);	/* 等待所有子进程 */
		return 1;
	}
	if(equal(cp1, ":"))
		return 1;
	if(equal(cp1, "exit")) {
		if (ac == 2) { /* 有一个参数 */
			cp2 = av[1];
			while ((c = *cp2++) >= '0' && c <= '9')
				i = (i*10)+c-'0';
		}
		else if (ac > 2) {
			err("exit: arg count");
			return 1;
		}
		if(promp == 0) 
			lseek(STDIN_FILENO, 0, SEEK_END);
		exit(i);
	}
	if(equal(cp1,"exec")) {
		if (redirect(t)) {	/* 重定向标准输入输出 */
			exitstat = 1;
			return 1;
		}
		if (ac > 1) { /* 有参数 */
			t[DSPR] = ac-1;
			t[DCOM] = (int)(av+1);
			exitstat=execcmd(t);
			return 1;
		}
		exitstat = 0;
		return 1;
	}
#ifdef  NONE_UMASK
	if(equal(cp1, "umask"))	{
		if (ac == 2) { /* 有一个参数 */
			cp2 = av[1];
			while ((c = *cp2++) >= '0' && c <= '7')
				i = (i<<3)+c-'0';
			umask(i);
		}
		else if (ac > 2) 
			err("umask: arg count");
		else {
			umask(i = umask(0));
			prc('0');
			for (c = 6; c >= 0; c-=3)
				prc(((i>>c)&07)+'0');
			prc('\n');
		}
		return 1;
	}
#endif
	return 0;
}

/* 
 * 执行语法树节点中的命令, 带有文件名展开
 * 被调于 execute() builtin()
 * 调用 glob() texec()
 */
int execcmd(int *t)
{
	int ac = t[DSPR];	
	char **av = (char**)(t[DCOM]);

	av[ac] = NULL; /* 终结字列表 */
#if 0
	gflg = 0;
	scan(av, tglob);	/* 测试参数是否有通配符 */
	if(gflg) {
		av[-1]="/etc/glob";
		return glob(ac+1, av-1);
	}
#endif
	scan(av, trim); /* 清除引用标志位 */
	return texec(av[0], av);
}

/*
 * exec 和进行错误消息打印
 * 被调于 execcmd() glob()
 */
int texec(char* f, char **t)
{

	/* 由于不设置进程的环境，可方便的使用 execvp() 而不用 execve() */
	execvp(f, t);
    printf("v6sh : %s command not found.\n",f);
#if 0
	extern int errno;
	if (errno==EACCES) { /* 没有访问权限 */
		prs(t[0]);
		err(": permission denied");
	}
	if (errno==ENOEXEC) { /* 不是二进制可执行文件 */
		prs("No shell!\n");
	}
	if (errno==ENOMEM) { /* 不能分配内存 */
		prs(t[0]);
		err(": too large");
	}
	if (errno==ENOENT) {
		prs(t[0]);
		err(": not found");
		return 127; /* 指示没有找到文件 */
	}
#endif
	return 126;
}

/*
 * 扫描字列表
 */
void scan(char **t, int (*f)())
{
	register char *p, c;

	while((p = *t++) != NULL)
		while((c = *p) != 0)
			*p++ = (*f)(c);
}

/*
 * 检测是否包含通配符
 */
int tglob(int c)
{
	if(any(c, "[?*"))
		gflg = 1;
	return c;
}

/*
 * 清除引用标志位
 */
int trim(int c)
{
	return c&~QUOTE;
}

/*
 * 打印错误输出，在非交互模式下退出
 */
void err(char *s)
{
	prs(s);
	prs("\n");
	if(promp == 0) {
		lseek(STDIN_FILENO, 0, SEEK_END);
		exit(1);
	}
}

/*
 * 写一个字符串
 */
void prs(char *as)
{
	register char *s;

	s = as;
	while(*s)
		prc(*s++);
}

/*
 * 写一个字符
 */
void prc(char c)
{
	write(STDERR_FILENO, &c, 1);
}

/*
 * 写一个数
 */
void prn(int n)
{
	register int a;

	if((a=n/10) != 0)
		prn(a);
	prc((n%10)+'0');
}

/*
 * 写一个数到字符串
 */
void sprn(int n, char *s)
{
	int i,j;
	for (i=1000000000; n<i && i>1; i/=10); /* 32 位字长 */
	for (j=0; i>0; j++,n%=i,i/=10)
		s[j] =(char)(n/i + '0');
	s[j] = '\0';
}

/*
 * 判断一个字符是否在一个字符串中，字符'\0'存在于任何字符串中
 */
int any(int c, char *as)
{
	register char *s;

	s = as;
	while(*s)
		if(*s++ == c)
			return 1;
	return 0;
}

/*
 * 判断两个字符串是否相等
 */
int equal(char *as1, char *as2)
{
	register char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while(*s1++ == *s2)
		if(*s2++ == '\0')
			return 1;
	return 0;
}

/*
 * i>0 等待指定 pid 的进程终止，i<0 等待所有子进程终止，
 * 如果异常终止，则打印诊断信息
 */
#ifdef  wait
#undef  wait
#define wait(x) run(MM_PID,MIF_WAIT,0,0,0)
#endif
int pwait(pid_t i)
{
	pid_t p;
#ifdef  NONE_WAIT
	int e = 0, s;
#endif

	if(i == 0)
		return 0;
	do{
		p = wait(&s);
		if(p == -1) /* 没有子进程需要等待了 */
			break;
#ifdef  NONE_WAIT
		e = WTERMSIG(s); /* 取子进程终止状态 */
		if(e >= NSIGMSG || mesg[e] != NULL) { /*被信号终止并有相应的诊断消息*/
			if(p != i) {
				prn(p);
				prs(": ");
			}
			if (e < NSIGMSG)
				prs(mesg[e]);
			else {
				prs("Signal ");
				prn(e);
			}
		/*	if(WCOREDUMP(s))
		 *		prs(" -- Core dumped"); */
			err("");
		}
		if(i == p) /* 终止的子进程是要等待的子进程 */
			break;
#endif
	}while(0);
#ifdef  NONE_WAIT
	if (WIFEXITED(s))
		return WEXITSTATUS(s);
	else
		return e | 0x80; /* 异常终止返回大于 128 的 8 位整数 */ 
#else
    return p;
#endif
}
