#ifndef MY_UTIL_H
#define MY_UTIL_H

char *newlabel(void);

void creatvarmap(void);
void pushvar(char *);
int popvar(char *);
void clearvarmap(void);

#endif
