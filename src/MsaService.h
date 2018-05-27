#pragma once

#include <daemon_utils/auto_shutdown_service.h>
#include <msa/simple_storage_manager.h>
#include <msa/login_manager.h>
#include <msa/account_manager.h>
#include <msa/token_response.h>
#include "MsaUiClient.h"

class MsaUiHelper;

class MsaService : public daemon_utils::auto_shutdown_service {

private:
    static std::string const PLATFORM_NAME;

    msa::SimpleStorageManager storageManager;
    msa::LoginManager loginManager;
    msa::AccountManager accountManager;
    MsaUiHelper& uiHelper;

    static std::shared_ptr<msa::LegacyToken> createLegacyTokenFromProperties(
            std::map<std::string, std::string> const& p);

    static nlohmann::json createTokenJson(msa::Token const& token);

    static nlohmann::json createTokenErrorInfoJson(msa::TokenErrorInfo const& errorInfo);

public:
    MsaService(std::string const& path, std::string const& dataPath, MsaUiHelper& uiHelper,
               daemon_utils::shutdown_policy shutdownPolicy);

    simpleipc::rpc_json_result handleGetAccounts();

    simpleipc::rpc_json_result handleAddAccount(nlohmann::json const& data);

    simpleipc::rpc_json_result handleRemoveAccount(nlohmann::json const& data);

    void handlePickAccount(nlohmann::json const& data, rpc_handler::result_handler const& handler);

    void handleAddAccountWithBrowser(nlohmann::json const& data, rpc_handler::result_handler const& handler);

    void handleRequestToken(nlohmann::json const& data, rpc_handler::result_handler const& handler);

};
