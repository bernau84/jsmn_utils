#ifndef __JSMN_H_
#define __JSMN_H_

#ifdef __cplusplus
#include <cstring>
extern "C" {
#endif


#define JSMN_PARENT_LINKS   //chained token
#define JSMN_LEVEL   //level / deep of parsing 	
#define JSMN_STRICT     //json strict format

/**
 * JSON type identifier. Basic types are:
 * 	o Object
 * 	o Array
 * 	o String
 * 	o Other primitive: number, boolean (true/false) or null
 */
typedef enum {
	JSMN_PRIMITIVE = 0,
	JSMN_OBJECT = 1,
	JSMN_ARRAY = 2,
	JSMN_STRING = 3
} jsmntype_t;

typedef enum {
	/* Not enough tokens were provided */
	JSMN_ERROR_NOMEM = -1,
	/* Invalid character inside JSON string */
	JSMN_ERROR_INVAL = -2,
	/* The string is not a full JSON packet, more bytes expected */
    JSMN_ERROR_PART = -3
} jsmnerr_t;

/**
 * JSON token description.
 * @param		type	type (object, array, string etc.)
 * @param		start	start position in JSON data string
 * @param		end		end position in JSON data string
 */
typedef struct {
	jsmntype_t type;
	int start;
	int end;
	int size;
#ifdef JSMN_PARENT_LINKS
	int parent;
#endif
} jsmntok_t;

/**
 * JSON parser. Contains an array of token blocks available. Also stores
 * the string being parsed now and current position in that string
 */
typedef struct {
	unsigned int pos; /* offset in the JSON string */
	unsigned int toknext; /* next token to allocate */
	int toksuper; /* superior token node, e.g parent object or array */
#ifdef JSMN_LEVEL
	int level;
#endif //JSMN_LEVEL		
} jsmn_parser;

/**
 * Create JSON parser over an array of tokens
 */
void jsmn_init(jsmn_parser *parser);

/**
 * Run JSON parser. It parses a JSON data string into and array of tokens, each describing
 * a single JSON object.
 *
 * \todo predelat tak aby to podporovalo maximalni hloubku do ktere se zanori
 * pro ucely juniperu to bude slouzit tak ze chceme parsovat jen seznam rpc nikoli obsah parametru
 * setrime tak tokeny
 */
jsmnerr_t jsmn_parse(jsmn_parser *parser, const char *js, size_t len,
        jsmntok_t *tokens, unsigned int num_tokens);


/**
 * Run JSON parser with restricted deep of parsing
 *
 * \todo predelat tak aby to podporovalo maximalni hloubku do ktere se zanori
 * pro ucely juniperu to bude slouzit tak ze chceme parsovat jen seznam rpc nikoli obsah parametru
 * setrime tak tokeny
 */
jsmnerr_t jsmn_parse_restricted(jsmn_parser *parser, const char *js, size_t len,
        jsmntok_t *tokens, unsigned int num_tokens, unsigned max_level);

#ifdef __cplusplus
}
#endif

#endif /* __JSMN_H_ */
