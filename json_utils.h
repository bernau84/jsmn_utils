#ifndef _JSON_MNGR_H_
#define _JSON_MNGR_H_

#include <stdarg.h>

#include  "jsmn.h"

/**
 * JSON manager. In the way of structure it inherits jsmn parser and adds ability for write
 * and acess values of tokens.
 *
 * Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string.
 * Provides new unalocated tokens and correposnding record space if avaiable
 */

typedef struct {

    //reader properties
    //struct jsmn_parser;  /* anonymous structure for read access */ NOT ALLOWED IN C++
    //so instead again all properties from  jsmn_parser
    unsigned int pos; /* offset in the JSON string */
    unsigned int toknext; /* next token to allocate */
    int toksuper; /* superior token node, e.g parent object or array */
    unsigned int colsuper; /* depth of superior node */

        //writer properties
	char *p;  /* pointer of data in the storage */
	unsigned int size; /* overal length of storage */

        jsmntok_t *tokens;
	unsigned int toksize;  /* number of tokens */
} json_manager;



//exports
extern int         jsmn_utils_init_man(json_manager *man,
                                       jsmntok_t *atok, size_t atok_sz,
                                       char *st, size_t st_sz);

extern int         jsmn_utils_begin_child(json_manager *man, jsmntype_t type);
extern int         jsmn_utils_end_child(json_manager *man);

extern jsmntok_t  *jsmn_utils_new_token(json_manager *man, float val, size_t reserv = sizeof(0.0/0));
extern jsmntok_t  *jsmn_utils_new_token(json_manager *man, const char *str, size_t reserv = 0);

extern int         jsmn_utils_fill_token(json_manager *man, jsmntok_t *token, const char *val);
extern int         jsmn_utils_fill_token(json_manager *man, jsmntok_t *token, float val);
extern int         jsmn_utils_fill_token(json_manager *man, jsmntok_t *token, const char *f, ... );

extern jsmntok_t  *jsmn_utils_get_token(json_manager *man, const char *path);
extern float       jsmn_utils_read_primitive(json_manager *man, const char *path, float def);
extern const char *jsmn_utils_read_string(json_manager *man, const char *path); 

extern int         jsmn_utils_import(json_manager *man, const char *js, size_t js_len);
extern int         jsmn_utils_export(json_manager *man, char *js, size_t js_len);

//aliases
#define jsmn_utils_begin_array(MAN) jsmn_utils_begin_child(MAN, JSMN_ARRAY)
#define jsmn_utils_begin_object(OBJ) jsmn_utils_begin_child(OBJ, JSMN_OBJECT)

#endif //_JSON_MNGR_H_

