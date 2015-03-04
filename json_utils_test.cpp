
#include "json_utils.h"
#include <QFile>
#include <QTextStream>
#include <QString>

bool jsmn_test_parser(){

    jsmntok_t ljtok[100];
    memset(ljtok, 0, sizeof(ljtok));
    char ljarr[100];

    const char ljs[] = {

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

  json_manager ljman;
  jsmn_utils_init_man(&ljman, ljtok, 100, ljarr, sizeof(ljarr));
  return (0 < jsmn_utils_import(&ljman, ljs, sizeof(ljs)));
}

bool jsmn_test_composer(){

  jsmntok_t ljtok[100];
  char ljarr[1024];
  char ljs[1024];
  json_manager ljman;
  jsmn_utils_init_man(&ljman, ljtok, 100, ljarr, sizeof(ljarr));

  jsmn_utils_begin_child(&ljman, JSMN_OBJECT);
      jsmn_utils_new_token(&ljman, "numbers");
      jsmn_utils_begin_child(&ljman, JSMN_ARRAY);
        jsmn_utils_new_token(&ljman, 1.2, 8);
        jsmn_utils_new_token(&ljman, 45E-1, 8);
      jsmn_utils_end_child(&ljman);

      jsmn_utils_new_token(&ljman, "strings");
      jsmn_utils_begin_child(&ljman, JSMN_ARRAY);
        jsmn_utils_new_token(&ljman, "one", 8);
        jsmn_utils_new_token(&ljman, "oneone", 8);
        jsmn_utils_new_token(&ljman, "abcdefghijklmn", 8); //longer than expected - will see
      jsmn_utils_end_child(&ljman);

      jsmn_utils_new_token(&ljman, "rules");
      jsmn_utils_begin_child(&ljman, JSMN_ARRAY);
        jsmn_utils_begin_child(&ljman, JSMN_OBJECT);
          jsmn_utils_new_token(&ljman, "path");
          jsmn_utils_new_token(&ljman, 0.0, 2);
          jsmn_utils_new_token(&ljman, "dest");
          jsmn_utils_begin_child(&ljman, JSMN_ARRAY);
            jsmn_utils_new_token(&ljman, "+420725224100");
            jsmn_utils_new_token(&ljman, "+420725224101");
          jsmn_utils_end_child(&ljman);
        jsmn_utils_end_child(&ljman);
        jsmn_utils_begin_child(&ljman, JSMN_OBJECT);
          jsmn_utils_new_token(&ljman, "path");
          jsmn_utils_new_token(&ljman, 3, 2);
          jsmn_utils_new_token(&ljman, "dest");
          jsmn_utils_begin_child(&ljman, JSMN_ARRAY);
            jsmn_utils_new_token(&ljman, "prvni@jablocom.com");
            jsmn_utils_new_token(&ljman, "nahradni@jablocom.com");
          jsmn_utils_end_child(&ljman);
        jsmn_utils_end_child(&ljman);
        jsmn_utils_begin_child(&ljman, JSMN_OBJECT);
          jsmn_utils_new_token(&ljman, "path");
          jsmn_utils_new_token(&ljman, 3, 2);
          jsmn_utils_new_token(&ljman, "dest");
          jsmn_utils_begin_child(&ljman, JSMN_ARRAY);
            jsmn_utils_new_token(&ljman, "arc.alt.jablotron:10001");
            jsmn_utils_new_token(&ljman, "arc.main.jablotron:10000");
          jsmn_utils_end_child(&ljman);
        jsmn_utils_end_child(&ljman);
      jsmn_utils_end_child(&ljman);
  jsmn_utils_end_child(&ljman);

  jsmn_utils_export(&ljman, ljs, sizeof(ljs));

  QFile lfile("json_unit_test_out.txt");
  if (!lfile.open(QIODevice::WriteOnly | QIODevice::Text))
      return false;

  QTextStream lout(&lfile);
  lout << QString(ljs);


//  jsmn_parser ljrd;
//  jsmntok_t ljtrd[100];
//  jsmn_init(&ljrd);
//  jsmn_parse(&ljrd, ljarr, strlen(ljarr), ljtrd, 100);

  /*
  /todo - kontrola poctu polozek; musi sedet
  /todo - musi sedet typ polozek
  /todo - tam kde se to vleze odpovida i hodnota polozek
  */

  return false;
}

