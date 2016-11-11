#include "json_utils.h"

bool jsmn_test_parser(){

   char ljs[] = {

          "\"nubers\": [\n"
            "1.0, 2.1, 3.2, 3.4, 4.5, 5.6\n"
          "],\n"
          "\"strings\": [\n"
            "\"one\", \"oneone\", \"oneoneone\", \"oneoneoneoneoneone\"\n"
          "],\n"
          "\"rules\": [\n"
              "{\n"
                  "\"path\": 0,\n"
                  "\"dest\": [ \"+420725224100\", \"+420725224101\" ]\n"
              "},\n"
              "{\n"
                  "\"path\": 1,\n"
                  "\"dest\": [ \"+420725224100\" ]\n"
              "},\n"
              "{\n"
                  "\"path\": 1,\n"
                  "\"dest\": [ \"+420725224200\" ]\n"
              "},\n"
              "{\n"
                  "\"path\": 2,\n"
                  "\"dest\": [ \"+420725224101\" ]\n"
              "},\n"
              "{\n"
                  "\"path\": 3,\n"
                  "\"dest\": [ \"prvni@jablocom.com\", \"nahradni@jablocom.com\" ]\n"
              "},\n"
              "{\n"
                  "\"path\": 4,\n"
                  "\"dest\": [ \"arc.main.jablotron:10000\", \"arc.alt.jablotron:10001\" ]\n"
              "}\n"
          "]\n"
    };

    jsmntok_t ljtok[100];
    json_manager ljman;
    return (0 < jsmn_utils_init(&ljman, ljtok, 100, ljs, sizeof(ljs)));
}

bool jsmn_test_readout(){

    char rpcList[] =
            "[\r\n"
                "\t{\r\n"
                    "\t\t\"c\": 10010,\r\n"
                    "\t\t\"n\": \"jun-ack\",\r\n"
                    "\t\t\"t\": \"2015-04-12T03:04:05+01:00\",\r\n"
                    "\t\t\"p\": [1234, \"text\", 1.125],\r\n"
                    "\t\t\"ref\": 10009,\r\n"
                    "\t\t\"ack\": false\r\n"
                "\t},\r\n"
                "\t{\r\n"
                    "\t\t\"c\": 10011,\r\n"
                    "\t\t\"n\": \"jun-badRpc\",\r\n"
                    "\t\t\"t\": \"2015-04-12T03:04:05+01:00\",\r\n"
                    "\t\t\"p\": [\"mime-parse\", -55],\r\n"
                    "\t\t\"ref\": 10008,\r\n"
                    "\t\t\"pri\": 12,\r\n"
                    "\t\t\"ack\": false\r\n"
                "\t}\r\n"
            "]\r\n,"
            "\"offtopic\": \"debug-comments\"\r\n";

    jsmntok_t ljtok[100];
    json_manager ljman;

    jsmn_utils_init(&ljman, ljtok, 100, rpcList, sizeof(rpcList));

    if(10010 != jsmn_utils_read_int(&ljman, "\\@1\\c", 0))
        return false;
    if(memcmp(jsmn_utils_read_string(&ljman, "\\@1\\n"), "jun-ack", 7))
        return false;
    if(memcmp(jsmn_utils_read_string(&ljman, "\\@1\\p\\@2"), "text", 4))
        return false;
    if(1234 != jsmn_utils_read_int(&ljman, "\\@1\\p\\@1", 0))
        return false;
    if(1.125 != jsmn_utils_read_double(&ljman, "\\@1\\p\\@3", 0.0))
        return false;
    if(10011 != jsmn_utils_read_int(&ljman, "\\@2\\c", 0))
        return false;
    if(memcmp(jsmn_utils_read_string(&ljman, "\\@2\\n"), "jun-badRpc", 10))
        return false;
    if(memcmp(jsmn_utils_read_string(&ljman, "\\@2\\p\\@1"), "mime-parse", 10))
        return false;
    if(-55 != jsmn_utils_read_int(&ljman, "\\@2\\p\\@2", 0))
        return false;
    if(12 != jsmn_utils_read_int(&ljman, "\\@2\\pri", 0))
        return false;
    if(memcmp(jsmn_utils_read_string(&ljman, "offtopic"), "debug-comments", 14))
        return false;
    return true;
}

bool jsmn_test_composer(){

  json_manager ljman;
  jsmntok_t ljtok[100];
  char ljarr[1024] = "";

  jsmn_utils_init(&ljman, ljtok, 100, ljarr, sizeof(ljarr));

  jsmn_utils_begin_child(&ljman, JSMN_OBJECT);
      jsmn_utils_new_string(&ljman, "numbers");
      jsmn_utils_begin_child(&ljman, JSMN_ARRAY);
        jsmn_utils_new_double(&ljman, 1.2, 8);
        jsmn_utils_new_double(&ljman, 45E-1, 8); //5
      jsmn_utils_end_child(&ljman);

      jsmn_utils_new_string(&ljman, "strings");
      jsmn_utils_begin_child(&ljman, JSMN_ARRAY);
        jsmn_utils_new_string(&ljman, "one", 8);
        jsmn_utils_new_string(&ljman, "oneone", 8);
        jsmn_utils_new_string(&ljman, "abcdefghijklmn", 8); //10 //longer than expected - will see
      jsmn_utils_end_child(&ljman);

      jsmn_utils_new_string(&ljman, "rules");
      jsmn_utils_begin_child(&ljman, JSMN_ARRAY);
        jsmn_utils_begin_child(&ljman, JSMN_OBJECT);
          jsmn_utils_new_string(&ljman, "path");
          jsmn_utils_new_int(&ljman, 0, 2);
          jsmn_utils_new_string(&ljman, "dest");
          jsmn_utils_begin_child(&ljman, JSMN_ARRAY);
            jsmn_utils_new_string(&ljman, "+420725224100");
            jsmn_utils_new_string(&ljman, "+420725224101"); //19
          jsmn_utils_end_child(&ljman);
        jsmn_utils_end_child(&ljman);
        jsmn_utils_begin_child(&ljman, JSMN_OBJECT);
          jsmn_utils_new_string(&ljman, "path");
          jsmn_utils_new_int(&ljman, 3, 2);
          jsmn_utils_new_string(&ljman, "dest");
          jsmn_utils_begin_child(&ljman, JSMN_ARRAY);
            jsmn_utils_new_string(&ljman, "prvni@jablocom.com");
            jsmn_utils_new_string(&ljman, "nahradni@jablocom.com");  //27
          jsmn_utils_end_child(&ljman);
        jsmn_utils_end_child(&ljman);
        jsmn_utils_begin_child(&ljman, JSMN_OBJECT);
          jsmn_utils_new_string(&ljman, "path");
          jsmn_utils_new_int(&ljman, 2, 2);
          jsmn_utils_new_string(&ljman, "dest");
          jsmn_utils_begin_child(&ljman, JSMN_ARRAY);
            jsmn_utils_new_string(&ljman, "arc.alt.jablotron:10001");
            jsmn_utils_new_string(&ljman, "arc.main.jablotron:10000"); //34
          jsmn_utils_end_child(&ljman);
        jsmn_utils_end_child(&ljman);
      jsmn_utils_end_child(&ljman);
  jsmn_utils_end_child(&ljman);

  unsigned rem = ljman.toknext;
  jsmn_utils_done(&ljman);

//  QFile lfile("json_unit_test_out.txt");  //export json data (for human control)
//  if (!lfile.open(QIODevice::WriteOnly | QIODevice::Text))
//      return false;

//  QTextStream lout(&lfile);
//  lout << QString(ljarr);

  //parse shrink ljarr again
  jsmn_utils_init(&ljman, ljtok, 100, ljarr, sizeof(ljarr));

  //number of tokens should remain unchanged
  if(rem != ljman.toknext)
      return false;

  return true;
}

