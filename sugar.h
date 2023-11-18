#pragma once
#pragma GCC diagnostic error "-Wnonnull"

#define FORI(v, s, m) for (int v = (s); v < (m); v++)
#define FORS(v, s, m, step) for (int v = (s); v < (m); v+=(step))

#define FTYPE(x)\
_Generic(x,\
int:    #x"=%d",\
long:   #x"=%ld",\
short:  #x"=%hd",\
float:  #x"=%f",\
double: #x"=%lf",\
char:   #x"=%c",\
default: 0), (x)

#define FNTYPE(x)\
_Generic(x,\
int:    #x"=%d\n",\
long:   #x"=%ld\n",\
short:  #x"=%hd\n",\
float:  #x"=%f\n",\
double: #x"=%lf\n",\
char:   #x"=%c\n",\
default: 0), (x)

// Format for scanf, fscanf and sscanf
#define FSTYPE(x)\
_Generic(x,\
int*:    "%d",\
long*:   "%ld",\
short*:  "%hd",\
float*:  "%f",\
double*: "%lf",\
char*:   "%c",\
default: 0), (x)

#define DLOG(x) printf(FNTYPE(x))
