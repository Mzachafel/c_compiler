#ifndef MY_UTIL_H
#define MY_UTIL_H

char *newlabel(void);

void creatvarmap(void);
void pushvar(char *);
int popvar(char *);
void enterscope(void);
void exitscope(FILE *);
void pushlbl(char *, char *);
char *poplbl(int);
void exitloop(void);
void clearvarmap(void);

#endif
