#pragma once

#include <simpleipc/server/service.h>
#include <msa/simple_storage_manager.h>
#include <msa/login_manager.h>
#include <msa/account_manager.h>
#include <msa/token_response.h>
#include "MsaUiClient.h"
class MsaUiLauncher;

class MsaService : public simpleipc::server::service {

private:
    static std::string const PLATFORM_NAME;

    msa::SimpleStorageManager storageManager;
    msa::LoginManager loginManager;
    msa::AccountManager accountManager;
    MsaUiLauncher& uiLauncher;
    std::weak_ptr<MsaUiClient> uiClient;

    static std::shared_ptr<msa::LegacyToken> createLegacyTokenFromProperties(
            std::map<std::string, std::string> const& p);

    static nlohmann::json createTokenJson(msa::Token const& token);

    static nlohmann::json createTokenErrorInfoJson(msa::TokenErrorInfo const& errorInfo);


    std::shared_ptr<MsaUiClient> acquireUiClient();

public:
    MsaService(std::string const& path, std::string const& dataPath, MsaUiLauncher& uiLauncher);

    simpleipc::rpc_json_result handleGetAccounts();

    simpleipc::rpc_json_result handleAddAccount(nlohmann::json const& data);

    simpleipc::rpc_json_result handleRemoveAccount(nlohmann::json const& data);

    void handlePickAccount(nlohmann::json const& data, rpc_handler::result_handler const& handler);

    void handleRequestToken(nlohmann::json const& data, rpc_handler::result_handler const& handler);

};
