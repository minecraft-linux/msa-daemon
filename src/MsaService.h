#pragma once

#include <simpleipc/server/service.h>
#include <msa/simple_storage_manager.h>
#include <msa/login_manager.h>
#include <msa/account_manager.h>
#include <msa/token_response.h>
#include "MsaUiClient.h"

class MsaService : public simpleipc::server::service {

private:
    static std::string const PLATFORM_NAME;

    msa::SimpleStorageManager storageManager;
    msa::LoginManager loginManager;
    msa::AccountManager accountManager;
    std::shared_ptr<MsaUiClient> uiClient;

    static std::shared_ptr<msa::LegacyToken> legacy_token_from_properties(std::map<std::string, std::string> const& p);

    static nlohmann::json token_to_json(msa::Token const& token);

    static nlohmann::json token_error_info_to_json(msa::TokenErrorInfo const& errorInfo);

public:
    MsaService(std::string const& path, std::string const& dataPath);

    void setUiClient(std::shared_ptr<MsaUiClient> client) {
        uiClient = client;
    }

    simpleipc::rpc_json_result handle_get_accounts();

    simpleipc::rpc_json_result handle_add_account(nlohmann::json const& data);

    simpleipc::rpc_json_result handle_remove_account(nlohmann::json const& data);

    void handle_pick_account(nlohmann::json const& data, rpc_handler::result_handler const& handler);

    void handle_request_token(nlohmann::json const& data, rpc_handler::result_handler const& handler);

};
