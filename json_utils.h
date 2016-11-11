#ifndef _JSON_MNGR_H_
#define _JSON_MNGR_H_

#include <stdio.h>
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
/**
 * Init json man
 *  @param write - man to be inited
 *  @param atok - array of tokens
 *  @param atok_sz - size of array of tokens
 *  @param st - byte storage
 *  @param st_sz - byte-size of storage
 */
extern int jsmn_utils_init(json_manager *man,            //context for reader/writer
                               jsmntok_t *atok, size_t atok_sz,  //tokens
                               char *st, size_t st_sz);          //raw json data
/**
 * Begin inferior section
 *    @return -1 - no free token
 *            >0 - associated token index
 */
extern int jsmn_utils_begin_child(json_manager *man, jsmntype_t type);

/**
 * End inferior section
 */
extern int jsmn_utils_end_child(json_manager *man);

/**
 * Update general token in storage
 *    @param val - pointer atribute for printf
 *    @param format - formating string for printf
 */
extern int jsmn_utils_fill_token(json_manager *man, jsmntok_t *token, const char *f, ... );

/**
 * Return number of succ. scaned parameters
 *   uses vsscanf() inside
 */
extern int jsmn_utils_read_token(json_manager *man, jsmntok_t *token, const char *f, ... );

/**
 * Space optimalization/shrink data only - exculde placeholder in token to shring
 *    size to minimal on json data buffer directly; token array is not useful again!!
 *
 * \note this is lazy verzion - exclude placeholders characters only
 * if you wish to continue with editing tokens you have to call
 * parser again
 */
extern int jsmn_utils_done(json_manager *man);

/**
 * Allocates a fresh unused token from the token pull
 *   Prepares place for token, white space delimiters and json parsing chracters for choosen type of token
 *    reserved space fo srtring is quoted at boundaries
 *    placeholde character will be excluded before exporting to json serialization
 */
extern jsmntok_t *jsmn_utils_new_token(json_manager *man, size_t reserv, jsmntype_t type);

/**
 * Return apropriate token from token list
 *    @param man - tokens io manager
 *    @param path - defines orders and keys in way to the token
 *      each level is separated with '\' keys suppose to be string, order in array is preceded with @
 *         anonymous object denotes empty "\\" path section

 *  for example - "\config\users\@5\name" //acess the key token itself
 *              - "\config\rf\@5\address\@1" //acess the value of key
 */
extern jsmntok_t *jsmn_utils_get_token(json_manager *man, const char *path);

/**
 * Allocates a fresh string token
 */
extern jsmntok_t *jsmn_utils_new_string(json_manager *man, const char *str, size_t reserv = 0);

/**
 * Allocates a fresh primitive token
 */
extern jsmntok_t *jsmn_utils_new_double(json_manager *man, double val, size_t reserv = 0);

/**
 * Allocates a fresh primitive token
 */
extern jsmntok_t *jsmn_utils_new_int(json_manager *man, int val, size_t reserv = 0);

/**
 * Update JSMN_STRING token in storage
 */
inline int jsmn_utils_fill_token(json_manager *man, jsmntok_t *token, const char *val) {

    return jsmn_utils_fill_token(man, token, val, 0);
}

/**
 * Update JSMN_PRIMITIVE token in storage
 */
inline int jsmn_utils_fill_token(json_manager *man, jsmntok_t *token, int val) {

    return jsmn_utils_fill_token(man, token, "%d", val);
}

/**
 * Update JSMN_PRIMITIVE token in storage
 */
inline int jsmn_utils_fill_token(json_manager *man, jsmntok_t *token, double val) {

    return jsmn_utils_fill_token(man, token, "%lf", val);
}

/**
 * Return apropriate value of primitive
 *    see jsmn_utils_get_token for usage
 *    @return - def if not exist or failure
 */
extern double jsmn_utils_read_double(json_manager *man, const char *path, double def);

/**
 * Return apropriate value of primitive
 *    see jsmn_utils_get_token for usage
 *    @return - def if not exist or failure
 */
extern int jsmn_utils_read_int(json_manager *man, const char *path, int def);

/**
 * Return apropriate string
 *    see jsmn_utils_get_token for usage
 */
extern const char *jsmn_utils_read_string(json_manager *man, const char *path);


#endif //_JSON_MNGR_H_

