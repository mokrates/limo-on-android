typedef struct _MYSTREAM {
  unsigned long length;
  unsigned long pos;
  char *startptr;
} MYSTREAM;

MYSTREAM *myopen(void *arr,  int length);
int myread(char *dest, int n, MYSTREAM *f);
void myclose(MYSTREAM *f);
