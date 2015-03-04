

#include <stdio.h>
#include <stdarg.h>

#include "json_utils.h"


#ifndef JSMN_PARENT_LINKS
#error "json utils demands support of token parenthes"
#endif //JSMN_PARENT_LINKS

/**
 * Init json man
 *  @param write - man to be inited
 *  @param atok - array of tokens
 *  @param atok_sz - size of array of tokens
 *  @param st - byte storage
 *  @param st_sz - byte-size of storage
 */
int jsmn_utils_init_man(json_manager *man, jsmntok_t *atok, size_t atok_sz, 
                                  char *st, size_t st_sz) {

    if(!man || !atok || !st || !atok_sz || !st_sz)
      return JSMN_ERROR_NOMEM;

    man->p = st;
    man->pos = 0;
    man->size = st_sz;

    man->tokens = atok;
    man->toknext = 0;
    man->toksize = atok_sz;

    man->toksuper = -1;  /*! not pointer - for comaptibility with parser */
    man->colsuper = 0;  

    return 1;
}

/**
 * Allocates a fresh unused token from the token pull
 *   Prepares place for token, white space delimiters and json parsing chracters for choosen type of token
 *    reserved space fo srtring is quoted at boundaries
 *    placeholde character will be excluded before exporting to json serialization
 */

#define JSON_TOKEN_PLACEHOLDER_CH 0x3
static jsmntok_t *jsmn_utils_new_token(json_manager *man, size_t reserv, jsmntype_t type) {

    if(!man && !reserv)   //token without size is not supported
      return NULL;

    if(man->toknext >= man->toksize)
      return NULL;

    if((man->size - man->pos) < (man->colsuper + 8))  //+ some reserve for delimiters and json control characters
      return NULL;


    //helpers value for determine framing of next token
    jsmntype_t  sup = (man->toksuper < 0) ? JSMN_OBJECT  : man->tokens[man->toksuper].type;  //object by default
    int         ord = (man->toksuper < 0) ? man->toknext : man->tokens[man->toksuper].size;  //number of objects
    bool        key = (sup == JSMN_OBJECT) && (0 == (ord & 0x1));  //token before was odd in object structure

    if(key && (type == JSMN_PRIMITIVE))
      return NULL;  // fatal - consistency error!!

    /*! \warning - we need master token first (for counting inferior purpose)
     * mandatoru for automatic decision of new token type - key (if even) or value (if odd)
     */
    if((0 == man->toknext) && (type != JSMN_OBJECT) && (type != JSMN_ARRAY))
      //jsmn_utils_begin_child(man, JSMN_OBJECT);     //\todo - who close it?! 
      return NULL;

    //if it is odd token of superior object of any token of array ',' is needed
    if((key) || (sup == JSMN_ARRAY)){

        if(ord){  //not for first key

            man->p[man->pos++] = ',';  //value delimiter
            man->p[man->pos++] = '\n';  //always with new line
        }

        memset(&man->p[man->pos], '\t', man->colsuper); //tabulators for better legibility, rem if no tabulators between rows
        man->pos += man->colsuper;
    }

    if(type == JSMN_STRING)
     man->p[man->pos++] = '\"';

    //init token
    jsmntok_t *tok = &man->tokens[man->toknext++];

    //jsmn_fill_token()
    tok->size = 0;  //number of inferiors
    tok->type = type;
    tok->start = man->pos;
    tok->end = man->pos + reserv;
    tok->parent = man->toksuper;

    //prepare with EOT by default (end of text) char
    memset(&man->p[tok->start], JSON_TOKEN_PLACEHOLDER_CH, reserv);
    man->pos = tok->end;

    if(type == JSMN_STRING){
      man->p[man->pos++] = '\"';

      if(key){

        man->p[man->pos++] = ':';
        man->p[man->pos++] = ' ';
      }
    }

    if(tok->parent >= 0){  //superior exists?

      man->tokens[man->toksuper].end = man->pos; //shift his range
      man->tokens[man->toksuper].size += 1; //increase number od child tokens
    }

    return tok;
}

/**
 * Update general token in storage
 *    @param val - pointer atribute for printf
 *    @param format - formating string for printf
 */
int jsmn_utils_fill_token(json_manager *man, jsmntok_t *token, const char *f, ... ) {

    if(!man || !token)
      return JSMN_ERROR_INVAL;

    int n, space;
    if(1 >= (space = (token->end - token->start + 1))) //+1 for \0
      return JSMN_ERROR_NOMEM;

    char prep[256]; //prep[size] variable size doesnt work with vsnprintf ?!

    va_list argptr;
    va_start(argptr, f);
    n = vsnprintf(prep, space, f, argptr); //n=<1, size-1>
    va_end(argptr);

    if(n <= 0)
        return 0;

    memcpy(&man->p[token->start], prep, n);  //n is without \0
    return n;
}


/**
 * Begin inferior section
 *    @return -1 - no free token
 *            >0 - associated token index
 */
int jsmn_utils_begin_child(json_manager *man, jsmntype_t type) {
	
    jsmntok_t *tok;

    if((type != JSMN_OBJECT) && (type != JSMN_ARRAY))
      return JSMN_ERROR_INVAL;

    const char *del = (JSMN_OBJECT == type) ? "{\n" : "[\n";

    if(NULL == (tok = jsmn_utils_new_token(man, 2, type)))
      return JSMN_ERROR_NOMEM;

    jsmn_utils_fill_token(man, tok, "%s", del);

    /* new superior token */
    man->toksuper = tok - man->tokens; //this index
    man->colsuper += 1;
    return 1;
}


/**
 * End inferior section
 */
int jsmn_utils_end_child(json_manager *man) {

    if((man->colsuper <= 0) && (man->toksuper < 0))
      return JSMN_ERROR_INVAL;

    jsmntok_t *stok = &man->tokens[man->toksuper];

    man->toksuper = man->tokens[man->toksuper].parent;  //go higher
    man->colsuper -= 1;

    if((man->size - man->pos) < (man->colsuper+2))
      return JSMN_ERROR_NOMEM;

    /*! close json class - enclosure doesn't own any token */
    man->p[man->pos++] = '\n';
    memset(&man->p[man->pos], '\t', man->colsuper); //tabulators for better legibility
    man->pos += man->colsuper;
    man->p[man->pos++] = (JSMN_OBJECT == stok->type) ? '}' : ']';
    stok->end = man->pos; 
    return 1;
}


/**
 * Allocates a fresh string token
 */
jsmntok_t *jsmn_utils_new_token(json_manager *man, const char *str, size_t reserv) {

    jsmntok_t *tok = jsmn_utils_new_token(man, (reserv) ? reserv : strlen(str), JSMN_STRING);
    if(tok) jsmn_utils_fill_token(man, tok, "%s", str);
    return tok;
}

/**
 * Allocates a fresh primitive token
 */
jsmntok_t *jsmn_utils_new_token(json_manager *man, float val, size_t reserv) {

    jsmntok_t *tok = jsmn_utils_new_token(man, reserv, JSMN_PRIMITIVE);
    if(tok) jsmn_utils_fill_token(man, tok, "%g", val);
    return tok;
}

/**
 * Update JSMN_STRING token in storage
 */
int jsmn_utils_fill_token(json_manager *man, jsmntok_t *token, const char *val) {

    return jsmn_utils_fill_token(man, token, val, 0);
}
 
/**
 * Update JSMN_PRIMITIVE token in storage
 */
int jsmn_utils_fill_token(json_manager *man, jsmntok_t *token, float val) {
    
    return jsmn_utils_fill_token(man, token, "%g", val);
}

/**
 * Return apropriate token from token list
 *    @param man - tokens io manager
 *    @param path - defines orders and keys in way to the token
 *      each level is separated with '\' keys suppose to be string, order in array is preceded with @ 
 *         anonymous object denotes empty "\\" path section

 *  for example - "\config\users\@5\name" //acess the key token itself
 *              - "\config\rf\@5\address\@1" //acess the value of key
 */
jsmntok_t *jsmn_utils_get_token(json_manager *man, const char *path) {
    
  const char *ap = path;
   
  if(!man)
    return NULL;

  for(jsmntok_t *tok = &man->tokens[0]; (*ap) && (tok < &man->tokens[man->toknext]); tok++){

      switch(*ap){
        case '@': {

          int shift = 0;
          if(1 == sscanf(&ap[1], "%d", &shift))
            tok += shift;
          ap = strchr(ap+1, '\\');
        } break;
      
        case '\\': {
        
          ap += 1;
          const char *ep = strchr(ap+1, '\\');
          if(!ep) ep += strlen(ap);
          if(memcmp(ap, &man->p[tok->start], ep - ap)){
            
            ap = ep;
            if(0 == *ep){ //the end of path

              return tok;
            }
          }
        } break;
      
        default:
        break;
      }
  }

  return NULL;
}

/**
 * Return apropriate value of primitive
 *    see jsmn_utils_get_token for usage
 *    @return - JASON_PRIM_NAN if not exist or failure
 *    \todo - rework to template (?)
 */
float jsmn_utils_read_primitive(json_manager *man, const char *path, float def = float(0.0/0)){
    
    float number;
  
    if(!man)
      return def;

    jsmntok_t *tok = jsmn_utils_get_token(man, path);
    
    if((NULL == tok) || (tok->type != JSMN_PRIMITIVE)) //undesired type
      return def;  

    if(1 != sscanf(&man->p[tok->start], "%g", &number))
      return def;

    return number;
}

/**
 * Return apropriate string
 *    see jsmn_utils_get_token for usage
 */
const char *jsmn_utils_read_string(json_manager *man, const char *path) {
    
    jsmntok_t *tok = jsmn_utils_get_token(man, path);
    
    if((NULL == tok) || (tok->type != JSMN_STRING)) //undesired type
      return NULL;  

    return &man->p[tok->start];
}

/**
 * Using previously alocated man to deserialize json string 
 * \note - embedd jsmn_parse()
 */
int jsmn_utils_import(json_manager *man, const char *js, size_t js_len) {

   if(!man)
      return NULL;
  
   jsmn_parser parser = { //local copy instead of man::jsmn_parser
      man->pos,
      man->toknext,
      man->toksuper,
   };

   return jsmn_parse(&parser, js, js_len, man->tokens, man->toksize);
}

/**
 * Generate json serialization (it is almost preprared, just exclude 
 * non-printable chars act as placeholder
 */
int jsmn_utils_export(json_manager *man, char *js, size_t js_len) {

    char *j = js;
    for(char *c = man->p; ((j - js) < js_len) && (c < &man->p[man->pos]); c++)
      if(*c != JSON_TOKEN_PLACEHOLDER_CH) *j++ = *c;

    return j - js;
}


/**
 * Space optimalization - exculde placeholder in token to shring 
 *    size to minimal; intern buffer us than identical to json
 *    serialization
 */
int jsmn_utils_shrink(json_manager *man, char *js, size_t js_len) {

  return 0;

  for(jsmntok_t *tok = &man->tokens[0]; tok < &man->tokens[man->toknext]; tok++){
      /*! \todo - iterate troughe tokens and recount start and end pointers to 
       *    new (optimized) length
       */
  }
}

