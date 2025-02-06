#ifndef TYPES_H_
#define TYPES_H_

typedef unsigned char uint8_t;
typedef unsigned short int uint16_t;
typedef long unsigned int uint32_t;

/* global variable to store error information */
struct error_descr_t {
        void *p_fct;
        char *p_text;
        int value;
};

extern struct error_descr_t error_descr;

void perror(void *p_fct, char *p_text, int value);

#endif
