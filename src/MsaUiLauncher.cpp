#include "MsaUiLauncher.h"
#include "MsaUiClient.h"

std::shared_ptr<MsaUiClient> MsaUiLauncher::createClient() {
    std::shared_ptr<MsaUiClient> ret (new MsaUiClient(open()));
    ret->send_hello_message();
    return ret;
}