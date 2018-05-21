#include "MsaService.h"

MsaService::MsaService(std::string const& path, std::string const& dataPath)
        : service(path), storageManager(dataPath), accountManager(storageManager), loginManager(&storageManager) {
    using namespace std::placeholders;
    add_handler_async("msa/get_accounts", std::bind(&MsaService::handle_pick_account, this, _3, _4));

    add_handler_async("msa/pick_account", std::bind(&MsaService::handle_pick_account, this, _3, _4));
}

simpleipc::rpc_result MsaService::handle_get_accounts() {
    auto accounts = accountManager.getAccounts();
    nlohmann::json ret;
    auto& l = ret["accounts"] = {};
    for (auto const& account : accounts) {
        nlohmann::json acc;
        acc["cid"] = account.getCID();
        acc["username"] = account.getUsername();
        l.push_back(std::move(acc));
    }
    return simpleipc::rpc_result::response(std::move(ret));
}

void MsaService::handle_pick_account(nlohmann::json const& data, rpc_handler::result_handler const& handler) {
    //
}