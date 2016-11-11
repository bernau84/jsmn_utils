#include <stdio.h>
#include <stdarg.h>

#include "json_utils.h"


#ifndef JSMN_PARENT_LINKS
#error "json utils demands support of token parenthes"
#endif //JSMN_PARENT_LINKS

/**
 */
int jsmn_utils_init(json_manager *man, jsmntok_t *atok, size_t atok_sz,
                                  char *st, size_t st_sz) {

    if(!man || !atok || !atok_sz)
      return JSMN_ERROR_NOMEM;

    man->p = st;
    man->pos = 0;
    man->size = st_sz;

    man->tokens = atok;
    man->toknext = 0;
    man->toksize = atok_sz;

    man->toksuper = -1;  /*! not pointer - for comaptibility with parser */
    man->colsuper = 0;  

    // using alocated man to deserialize json string automaticaly
    if(man->p && man->p[0] && man->size){ //given data could be the valid json?

        //uses jasmine parser
        jsmn_parser parser = { //local copy instead of man::jsmn_parser
          man->pos,
          man->toknext,
          man->toksuper,
          0
        };

        int ret = jsmn_parse(&parser,
                             man->p, man->size,
                             man->tokens, man->toksize);

        //put back
        man->pos = parser.pos;
        man->toknext = parser.toknext;
        man->toksuper = parser.toksuper;

        return ret;
    }

    return 1;
}

/**
 */
#define JSON_TOKEN_PLACEHOLDER_CH 0x3
jsmntok_t *jsmn_utils_new_token(json_manager *man, size_t reserv, jsmntype_t type) {

    if(!man && !reserv)   //token without size is not supported
      return NULL;

    if(man->toknext >= man->toksize)
      return NULL;

    if((man->size - man->pos) < (reserv + 8))  //+ some reserve for delimiters and json control characters
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
 */
int jsmn_utils_fill_token(json_manager *man, jsmntok_t *token, const char *f, ... ) {

    if(!man || !token)
      return JSMN_ERROR_INVAL;

    int n, space;
    if(1 >= (space = (token->end - token->start)))
      return JSMN_ERROR_NOMEM;

    memset(&man->p[token->start], JSON_TOKEN_PLACEHOLDER_CH, space);  //prepare

    char prep[space/*256*/];
    va_list argptr;
    va_start(argptr, f);

    n = vsnprintf(prep, space, f, argptr); //n=<1, size-1>
    va_end(argptr);

    if(n <= 0)
        return 0;

    if(n >= space) //retezec byl kracen => delka bez ukoncovaciho retezce je space-1
        n = space - 1;

    memcpy(&man->p[token->start], prep, n);  //n is without \0
    return n;
}

/**
 */
int jsmn_utils_begin_child(json_manager *man, jsmntype_t type) {
	
    jsmntok_t *tok;

    if((type != JSMN_OBJECT) && (type != JSMN_ARRAY))
      return JSMN_ERROR_INVAL;

    const char *del = (JSMN_OBJECT == type) ? "{" : "[";

    if(NULL == (tok = jsmn_utils_new_token(man, 3, type)))
      return JSMN_ERROR_NOMEM;

    jsmn_utils_fill_token(man, tok, "%s\n", del);

    /* new superior token */
    man->toksuper = tok - man->tokens; //this index
    man->colsuper += 1;
    return 1;
}

/**
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
 */
int jsmn_utils_read_token(json_manager *man, jsmntok_t *token, const char *f, ... ){

    if(!man || !token)
      return JSMN_ERROR_INVAL;

    int n, space;
    if(1 >= (space = (token->end - token->start + 1))) //+1 for \0
      return JSMN_ERROR_NOMEM;

    va_list argptr;
    va_start(argptr, f);
    n = vsscanf(&man->p[token->start], f, argptr);
    va_end(argptr);

    return n;
}

/**
 */
jsmntok_t *jsmn_utils_get_token(json_manager *man, const char *xpath) {
   
  if(!man || !xpath)
    return NULL;

  int iparent = -1;  //supperior token index backup for searching
  int count = -1;  //length of serching string or order of item in array

  for(jsmntok_t *tok = &man->tokens[0]; tok < &man->tokens[man->toknext]; ){

      switch(*xpath){

        case 0:  //end of path? -> end of serching
            return tok;
        break;

        case '\\': //check ve stay on class / array
            if((tok->type != JSMN_ARRAY) && (tok->type != JSMN_OBJECT))
                return NULL;  //we cant search on primitive / stings

            iparent = tok - &man->tokens[0];  //save reference to parent object/toksuper
            count = -1; //enforce inititialization
            xpath += 1; //move on next instruction
            tok++;
        break;

        case '@': //iterrate through sub-tokens
            if(count < 0)
                if(1 != sscanf(xpath, "@%d", &count))  //how much itteration?
                    return NULL; //path error

            //in counter is remaining number of iterrations now
            //try to meet conditions of parenthesis and order
            if(tok->parent == iparent)
                count -= 1;

            if(count) tok ++;
        break;

        default:
            //find key sub-token
            if(count < 0){  //init search str len & check if it is not end of path

                const char *ep = strchr(xpath, '\\');
                count = (!ep) ? strlen(xpath) : ep - xpath;  //backup key size
            }

            if((tok->parent == iparent) && //is there affinity to reference object?
               (tok->type == JSMN_STRING) && //object must be string
               (count == (tok->end - tok->start)) &&  //and length must be equal
               (0 == memcmp(xpath, &man->p[tok->start], count))){ //and finally if strings match

                count = 0; //indicate we are done
            }

            tok ++;
        break;
      }

      //evaluation - previous iteration was successful
      if(count == 0) //prepare new search path nod
          while((*xpath != '\\') && (*xpath != 0))//move behind recent path/dir
              ++xpath;
  }

  return NULL;
}

/**
 */
int jsmn_utils_done(json_manager *man) {

    char *p_end = &man->p[man->pos];
    char *p_red = man->p;

    for(char *p = man->p; p < p_end; p++)
        if(*p != JSON_TOKEN_PLACEHOLDER_CH)
            *p_red++ = *p; //copy useful key chars

    if(p_red < p_end)
        *p_red++ = 0;

    /* reintit optionaly
     * jsmn_utils_done(json_manager *man,
     *              man->tokens, man->toksize,
     *              man->p, man->size);
     */

    return 1;
}


/**
 */
double jsmn_utils_read_double(json_manager *man, const char *path, double def){

    jsmntok_t *tok = jsmn_utils_get_token(man, path);
    if((NULL == tok) || (tok->type != JSMN_PRIMITIVE)) //unwanted type
      return def;

    double number;
    if(1 != sscanf(&man->p[tok->start], "%lf", &number))
      return def;

    return number;
}

/**
 */
int jsmn_utils_read_int(json_manager *man, const char *path, int def){

    jsmntok_t *tok = jsmn_utils_get_token(man, path);
    if((NULL == tok) || (tok->type != JSMN_PRIMITIVE)) //unwanted type
      return def;

    int number;
    if(1 != sscanf(&man->p[tok->start], "%d", &number))
      return def;

    return number;
}

/**
 */
const char *jsmn_utils_read_string(json_manager *man, const char *path) {

    jsmntok_t *tok = jsmn_utils_get_token(man, path);
    if((NULL == tok) || (tok->type != JSMN_STRING)) //unwanted type
      return NULL;

    return &man->p[tok->start];
}

/**
 */
jsmntok_t *jsmn_utils_new_string(json_manager *man, const char *str, size_t reserv) {

    jsmntok_t *tok = jsmn_utils_new_token(man, (reserv) ? reserv : strlen(str)+1, JSMN_STRING);
    if(tok) jsmn_utils_fill_token(man, tok, "%s", str);
    return tok;
}

/**
 */
jsmntok_t *jsmn_utils_new_double(json_manager *man, double val, size_t reserv) {

    jsmntok_t *tok = jsmn_utils_new_token(man, (reserv) ? reserv : 24, JSMN_PRIMITIVE);
    if(tok) jsmn_utils_fill_token(man, tok, "%lf", val);
    return tok;
}

/**
 */
jsmntok_t *jsmn_utils_new_int(json_manager *man, int val, size_t reserv) {

    jsmntok_t *tok = jsmn_utils_new_token(man, (reserv) ? reserv : 32, JSMN_PRIMITIVE);
    if(tok) jsmn_utils_fill_token(man, tok, "%d", val);
    return tok;
}

