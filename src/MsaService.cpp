#include "MsaService.h"

MsaService::MsaService(std::string const& path, std::string const& dataPath)
        : service(path), storageManager(dataPath), accountManager(storageManager), loginManager(&storageManager) {
    using namespace std::placeholders;
    add_handler_async("msa/sign_in", std::bind(&MsaService::handle_sign_in, this, _3, _4));
}

void MsaService::handle_sign_in(nlohmann::json const& data, rpc_handler::result_handler const& handler) {
    //
}
