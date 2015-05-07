#include <QCoreApplication>

extern bool jsmn_test_composer();
extern bool jsmn_test_parser();
extern bool jsmn_test_readout();

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    bool res = true;
    if((res = jsmn_test_parser()) == false){

        while(1);
    } else if((res = jsmn_test_composer()) == false){

        while(1);
    } else if((res = jsmn_test_readout()) == false){

        while(1);
    }

    return a.exec();
}
