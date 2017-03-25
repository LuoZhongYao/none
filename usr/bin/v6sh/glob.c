/* 
 *  globals params
 *
 *  在 params 中的 "*" 匹配正则表达式 ".*"
 *  在 params 中的 "?" 匹配正则表达式 "."
 *  在 params 中的 "[...]" 匹配字符类
 *  在 params 中的 "[!...]" 匹配字符类的补集。
 *  在 params 中的 "[...a-z...]" 匹配 a 到 z。
 *
 *  执行命令并带有按如下规则构造的参数列表:
 *      如果 param 不包含 "*"，"["，或 "?"，则按原样使用它
 *      如果包含，则在当前目录中找到匹配这个 param 的所有文件，
 *      排序并使用它们。
 */

#include <stdlib.h>
#if 0
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <setjmp.h>
#endif
#include <posix.h>

#define	QUOTE 0x80	/* 引用标志位 */

#define	STRSIZ 1024 //_POSIX_ARG_MAX
static char ab[STRSIZ];	/* 生成的字符串的存储空间 */
static char *string;	
static char *ava[STRSIZ/2];	/* 生成的参数列表 */
static char **av;
static int  ncoll;
#if 0
static jmp_buf env; 
#endif

int glob(int argc, char *argv[]);
extern int texec(char* f, char **t);

static void expand(char *as);
//static void sort(char **oav);
//static int compare(char *as1, char *as2);
//static int match(char *s, char *p);
static void toolong(void);
static int amatch(char *as, char *ap);
static int umatch(char *s, char *p);
static char* cat(char *as1, char *as2);

int glob(int argc, char *argv[])
{
	string = ab;
	av = ava;

	if (argc < 2) {
		write(STDERR_FILENO, "Arg count\n", 10);
		return 1;
	}
	argv++;
	*av++ = *argv; /* 指向第一个参数，它是要执行的文件的名字 */

#if 0
	if (setjmp(env) != 0)
		return 1;
#endif
	while (--argc >= 2) 
		expand(*++argv);  /* 展开余下的所有的参数 */
	return texec(ava[0], ava);
}

void expand(char *as)
{
	register char *s, *cs;

	ncoll = 0;
	s = cs = as;
	/* 把 cs 定位到第一个通配符 */
	while (*cs!='*' && *cs!='?' && *cs!='[') {
		if (*cs++ == 0) { /* 没有找到通配符 */
			*av++ = cat(s, "");
			return; 
		}
	}
#if 0
	DIR	*dp;
	struct dirent * dirp; 
	char **oav;
	for (;;) {
		if (cs==s) { /* 在通配符之前没有'/' */
			dp = opendir(".");
			s = "";
			break;
		}
		if (*--cs == '/') {
			*cs = 0; /* 把参数分开为目录和文件名两个字符串 */
			dp = opendir((s==cs)? "/": s);
			*cs++ = QUOTE;	/* 做标记，在后面的 cat 操作中恢复成斜杠 */
			break;
		}
	}
	if (dp == NULL) {
		write(STDERR_FILENO, "No directory\n", 13);
		longjmp(env,1);
	}
	oav = av;
	while ((dirp = readdir(dp)) != NULL) {
		if (match(dirp->d_name, cs)) {
			*av++ = cat(s, dirp->d_name);
			ncoll++;
		}
	}
	if (!ncoll) /* 没有匹配 */
		*av++ = cat(s, cs); /* 保持参数为原状 */
	else /* 排序匹配的文件名字 */
		sort(oav);
	closedir(dp);
#endif

}
/* 排序，采用插入排序算法  */
#if 0
void sort(char **oav)
{
	register char **p1, **p2, *c;

	p1 = oav;
	while (++p1 < av) {
		c = *p1;
		p2 = p1;
		while (--p2 >= oav && compare(*p2, c) > 0)
			*(p2+1) = *p2;
		*(p2+1) = c;
	}
}
#endif

/* 打印错误消息并退出 */
void toolong(void)
{
	write(2, "Arg list too long\n", 18);
#if 0
	longjmp(env,1);
#endif
}
/*匹配判断例程 */
#if 0
int match(char *s, char *p)
{
	if (*s=='.' && *p!='.') /* 对 . 开头的文件的处理 */
		return 0;
	return amatch(s, p);
}
#endif
/* 对字符的匹配 */
int amatch(char *as, char *ap)
{
	register char *s, *p;
	register int scc;
	int c, cc, ok, lc;
	int neg = 0;

	s = as;
	p = ap;
	if ((scc = *s++) != 0) /* 文件名未终结 */
		/* 传递给 glob 的参数中的字符可能设置了引用位，
		这些字符在 shell 命令行中位于" "或' ' 中 */
		if ((scc &= ~QUOTE) == 0) /* 如果 scc 清除了引用位之后是 0 */
			scc = QUOTE; /* 重新把它设置为 QUOTE */
	switch (c = *p++) {

	case '[':	/* 处理字符类 */
		if (*p=='!') { /* 下个字符是 ! */
			neg = 1;
			p++;
		}
		ok = 0;
		lc = 0xFFFF;//INT_MAX;
		while ((cc = *p++) != 0) {
			if (cc==']') {
				if ((ok && !neg) || (!ok && neg)) 
					return amatch(s, p); /* 如果匹配，继续比较后面的字符 */
				else
					return 0;
			} else if (cc=='-') {
				if (lc<=scc && scc<=(c = *p++)) /* 在范围内 */
					ok++;
			} else
				if (scc == (lc=cc))
					ok++;
		}
		return 0;

	default:
		if (c!=scc)
			return 0;

	case '?':
		if (scc)
			return amatch(s, p);
		return 0;

	case '*':
		return umatch(--s, p);

	case '\0':
		return !scc;
	}
}
/* 对闭包的匹配 */
int umatch(char *s, char *p)
{
	if(*p==0) /* 模式以 * 结束 */
		return 1;
	while(*s) /* 文件名中有字符匹配模式中 * 后面的字符 */
		if (amatch(s++,p))
			return 1;
	return 0;
}
/* 比较两个字符串 */
int compare(char *as1, char *as2)
{
	register char *s1, *s2;

	s1 = as1;
	s2 = as2;
	while (*s1++ ==  *s2)
		if (*s2++ == 0)
			return 0;
	return (*--s1 - *s2);
}
/* 联接两个字符串到 string 指向的数组中并返回它，
 * 副作用是 string 指针右移，指向的数组空间减小 */
char* cat(char *as1, char *as2)
{
	register char *s1, *s2;
	register int c;

	s2 = string;
	s1 = as1;
	while ((c = *s1++) != 0) {
		if (s2 > &ab[STRSIZ])
			toolong();
		c &= ~QUOTE; /* 清除引用位 */
		if (c==0) { /* 原先是路径分隔符 */
			*s2++ = '/'; 
			break;
		}
		*s2++ = c;
	}

	s1 = as2;
	do {
		if (s2 > &ab[STRSIZ])
			toolong();
		*s2++ = c = *s1++;
	} while (c);

	s1 = string;
	string = s2;
	return s1;
}
