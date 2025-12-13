#include "stdlib.h"

void memcpy(void *dest, void *src, int bytes){
    char *d = dest;
    const char *s =src;
    while(bytes--){
        *d++=*s++;
    }
}

int memcmp(void *ptr1, void *ptr2, unsigned int size){
    const unsigned char *p1 = ptr1;
    const unsigned char *p2 = ptr2;
    while(size!=0){
        if(*p1>*p2) {return 1; }
        else if(*p1<*p2) {return -1; }

        p1++;
        p2++;
        size--;
    }
    return 0;

}

char* itoa(int n, int base){
    static char buf[32];
    int j=0, i=0, isneg=0;

    if(n==0)
        buf[j++]='0';

    if(n<0){
        isneg=1;
        n=-n;
    }

    while(n){
        buf[j++]=(n%base>9) ? (char)('a' + n%base-10) : (char)('0' + n%base);
        n/=base;
    }

    switch(base){
        case 16:
            buf[j++]='x';
            buf[j++]='0';
            break;
        case 8:
            buf[j++]='0';
            break;
        case 2:
            buf[j++]='b';
            buf[j++]='0';
    }

    if(isneg)
        buf[j++]='-';
    
    buf[j]='\0';
    j--;

    while(i<j){
        isneg=buf[j];
        buf[j]=buf[i];
        buf[i]=isneg;
        i++;
        j--;
    }

    return buf;
}

char* utoa(unsigned int n, int base){
    static char buf[32];
    int j=0, i=0, temp;

    if(n==0)
        buf[j++]='0';

    while(n){
        buf[j++]=(n%base>9) ? (char)('a' + n%base-10) : (char)('0' + n%base);
        n/=base;
    }

    switch(base){
        case 16:
            buf[j++]='x';
            buf[j++]='0';
            break;
        case 8:
            buf[j++]='0';
            break;
        case 2:
            buf[j++]='b';
            buf[j++]='0';
    }

    buf[j]='\0';
    j--;

    while(i<j){
        temp=buf[j];
        buf[j]=buf[i];
        buf[i]=temp;
        i++;
        j--;
    }

    return buf;
}
