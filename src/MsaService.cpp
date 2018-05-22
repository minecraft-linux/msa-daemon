#include <rapidxml.hpp>
#include <msa/network/server_config.h>
#include <msa/network/request_utils.h>
#include <base64.h>
#include "MsaService.h"

using namespace simpleipc;
using namespace msa::network;

std::string const MsaService::PLATFORM_NAME = "android2.1.0504.0524";

MsaService::MsaService(std::string const& path, std::string const& dataPath)
        : service(path), storageManager(dataPath), accountManager(storageManager), loginManager(&storageManager) {
    using namespace std::placeholders;
    add_handler("msa/get_accounts", std::bind(&MsaService::handle_get_accounts, this));
    add_handler("msa/add_account", std::bind(&MsaService::handle_add_account, this, _3));
    add_handler("msa/remove_account", std::bind(&MsaService::handle_remove_account, this, _3));

    add_handler_async("msa/pick_account", std::bind(&MsaService::handle_pick_account, this, _3, _4));
    add_handler_async("msa/request_token", std::bind(&MsaService::handle_request_token, this, _3, _4));
}

rpc_json_result MsaService::handle_get_accounts() {
    auto accounts = accountManager.getAccounts();
    nlohmann::json ret;
    auto& l = ret["accounts"] = nlohmann::json::array();
    for (auto const& account : accounts) {
        nlohmann::json acc;
        acc["cid"] = account.getCID();
        acc["username"] = account.getUsername();
        l.push_back(std::move(acc));
    }
    return rpc_json_result::response(std::move(ret));
}

rpc_json_result MsaService::handle_add_account(nlohmann::json const& data) {
    std::string username = data["username"];
    std::string cid = data["cid"];
    std::string tokenData = data["token"];
    rapidxml::xml_document<char> doc;
    doc.parse<0>(&tokenData[0]);
    auto token = msa::token_pointer_cast<msa::LegacyToken>(msa::Token::fromXml(doc));
    try {
        accountManager.addAccount(username, cid, token);
    } catch (msa::AccountAlreadyExistsException& e) {
        return rpc_json_result::error(-101, e.what());
    }
    return rpc_json_result::response(nullptr);
}

simpleipc::rpc_json_result MsaService::handle_remove_account(nlohmann::json const& data) {
    std::string cid = data["cid"];
    try {
        auto account = accountManager.findAccount(cid);
        if (!account)
            throw msa::NoSuchAccountException();
        accountManager.removeAccount(*account);
    } catch (msa::NoSuchAccountException& e) {
        return rpc_json_result::error(-100, e.what());
    }
    return rpc_json_result::response(nullptr);
}

void MsaService::handle_pick_account(nlohmann::json const& data, rpc_handler::result_handler const& handler) {
    if (!uiClient) {
        handler(rpc_json_result::error(-200, "Internal error (No UI client)"));
        return;
    }
    std::string baseUrl = ServerConfig::ENDPOINT_INLINE_CONNECT_PARTNER;
    std::vector<std::pair<std::string, std::string>> p;
    p.emplace_back("platform", PLATFORM_NAME);
    if (data.count("client_id") > 0)
        p.emplace_back("client_id", data["client_id"]);
    if (data.count("cobrandid") > 0)
        p.emplace_back("cobrandid", data["cobrandid"]);
    std::string params = RequestUtils::encodeUrlParams(p);
    uiClient->openBrowser(baseUrl + "&" + params).call([this, handler](rpc_result<MsaUiClient::BrowserResult> r) {
        if (!r.success()) {
            handler(rpc_json_result::error(r.error_code(), r.error_text()));
            return;
        }
        auto const& properties = r.data().properties;
        std::string cid = properties.at("CID");
        std::string username = properties.at("Username");
        auto token = legacy_token_from_properties(properties);

        auto account = accountManager.addAccount(username, cid, token);
        handler(rpc_json_result::response({{"cid", cid}}));
    });
}

std::shared_ptr<msa::LegacyToken> MsaService::legacy_token_from_properties(
        std::map<std::string, std::string> const& p) {
    std::string da_token = p.at("DAToken");
    std::string da_session_key = p.at("DASessionKey");
    std::string da_start_time_s = p.at("DAStartTime");
    msa::Token::TimePoint da_start_time;
    std::string da_expires_s = p.at("DAExpires");
    msa::Token::TimePoint da_expires;

    struct tm tm;
    if (strptime(da_start_time_s.c_str(), "%FT%TZ", &tm))
        da_start_time = std::chrono::system_clock::from_time_t(timegm(&tm));
    if (strptime(da_expires_s.c_str(), "%FT%TZ", &tm))
        da_expires = std::chrono::system_clock::from_time_t(timegm(&tm));

    return std::shared_ptr<msa::LegacyToken>(new msa::LegacyToken({}, da_start_time, da_expires, da_token,
                                                                  Base64::decode(da_session_key)));
}

void MsaService::handle_request_token(nlohmann::json const& data, rpc_handler::result_handler const& handler) {
    std::string cid = data["cid"];
    std::string client_id = data.value("client_id", std::string());
    auto const& scope_json = data["scope"];
    msa::SecurityScope scope = {scope_json["address"], scope_json.value("policy_ref", std::string())};
    bool silent = data.value("silent", false);

    auto account = accountManager.findAccount(cid);
    if (!account) {
        handler(rpc_json_result::error(-100, "No such account"));
        return;
    }

    auto ret = account->requestTokens(loginManager, {scope}, client_id);
    if (ret.size() == 0) {
        handler(rpc_json_result::error(-200, "Internal error (no token returned from internal route)"));
        return;
    }
    auto& token = ret.begin()->second;
    if (token.hasError() && !token.getError()->inlineAuthUrl.empty()) {
        if (silent) {
            handler(rpc_json_result::error(-102, "Must show UI to acquire token (silent mode is requested)"));
        } else {
            // TODO: open browser
        }
    } else if (token.hasError()) {
        handler(rpc_json_result::error(-110, "Server error", token_error_info_to_json(*token.getError())));
    } else {
        handler(rpc_json_result::response(token_to_json(*token.getToken())));
    }
}