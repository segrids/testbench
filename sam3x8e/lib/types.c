#include "types.h"

/* perror()
 *
 * short cut to write to error_descr
 */
void perror(void *p_fct, char *p_text, int value){
        error_descr.p_fct = p_fct;
        error_descr.p_text = p_text;
        error_descr.value = value;
}  
