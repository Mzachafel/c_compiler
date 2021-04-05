#ifndef MY_UTIL_H
#define MY_UTIL_H

char *newlabel(void);

void creatvarmap(void);
void pushvar(char *, int);
int popvar(char *);
void pushfunc(char *, int);
void popfunc(char *, int);
void enterscope(void);
void exitscope(FILE *);
void pushlbl(char *, char *);
char *poplbl(int);
int checkfuncname(char *);
void exitloop(void);
void clearvarmap(void);

#endif
