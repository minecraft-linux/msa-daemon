#include "MsaUiLauncher.h"
#include "MsaUiClient.h"

MsaUiClient* MsaUiLauncher::createClient() {
    MsaUiClient* ret = new MsaUiClient(open());
    ret->send_hello_message();
    return ret;
}