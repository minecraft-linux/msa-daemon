#pragma once

#include <simpleipc/server/service.h>
#include <msa/simple_storage_manager.h>
#include <msa/login_manager.h>
#include <msa/account_manager.h>

class MsaService : public simpleipc::server::service {

private:
    msa::SimpleStorageManager storageManager;
    msa::LoginManager loginManager;
    msa::AccountManager accountManager;

public:
    MsaService(std::string const& path, std::string const& dataPath);

    simpleipc::rpc_result handle_get_accounts();

    void handle_pick_account(nlohmann::json const& data, rpc_handler::result_handler const& handler);

};
