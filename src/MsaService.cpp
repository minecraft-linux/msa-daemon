#include "MsaService.h"
#include <rapidxml.hpp>
#include <msa/network/server_config.h>
#include <msa/network/request_utils.h>
#include <base64.h>
#include <msa/compact_token.h>
#include "MsaUiHelper.h"
#include "MsaErrors.h"

using namespace simpleipc;
using namespace msa::network;

std::string const MsaService::PLATFORM_NAME = "android2.1.0504.0524";

MsaService::MsaService(std::string const& path, std::string const& dataPath, MsaUiHelper& uiHelper,
                       daemon_utils::shutdown_policy shutdownPolicy)
        : auto_shutdown_service(path, shutdownPolicy), storageManager(dataPath), accountManager(storageManager),
          loginManager(&storageManager), uiHelper(uiHelper) {
    using namespace std::placeholders;
    add_handler("msa/get_accounts", std::bind(&MsaService::handleGetAccounts, this));
    add_handler("msa/add_account", std::bind(&MsaService::handleAddAccount, this, _3));
    add_handler("msa/remove_account", std::bind(&MsaService::handleRemoveAccount, this, _3));

    add_handler_async("msa/pick_account", std::bind(&MsaService::handlePickAccount, this, _3, _4));
    add_handler_async("msa/add_account_browser", std::bind(&MsaService::handleAddAccountWithBrowser, this, _3, _4));
    add_handler_async("msa/request_token", std::bind(&MsaService::handleRequestToken, this, _3, _4));
}

rpc_json_result MsaService::handleGetAccounts() {
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

rpc_json_result MsaService::handleAddAccount(nlohmann::json const& data) {
    std::string username = data["username"];
    std::string cid = data["cid"];
    std::string puid = data["puid"];
    std::string tokenData = data["token"];
    rapidxml::xml_document<char> doc;
    doc.parse<0>(&tokenData[0]);
    auto token = msa::token_pointer_cast<msa::LegacyToken>(msa::Token::fromXml(doc));
    try {
        accountManager.addAccount(username, cid, puid, token);
    } catch (msa::AccountAlreadyExistsException& e) {
        return rpc_json_result::error(MsaErrors::AccountAlreadyExists, e.what());
    }
    return rpc_json_result::response(nullptr);
}

simpleipc::rpc_json_result MsaService::handleRemoveAccount(nlohmann::json const& data) {
    std::string cid = data["cid"];
    try {
        auto account = accountManager.findAccount(cid);
        if (!account)
            throw msa::NoSuchAccountException();
        accountManager.removeAccount(*account);
    } catch (msa::NoSuchAccountException& e) {
        return rpc_json_result::error(MsaErrors::NoSuchAccount, e.what());
    }
    return rpc_json_result::response(nullptr);
}

void MsaService::handlePickAccount(nlohmann::json const& data, rpc_handler::result_handler const& handler) {
    std::vector<MsaUiClient::PickAccountItem> items;
    for (auto const& acc : accountManager.getAccounts())
        items.push_back({acc.getCID(), acc.getUsername()});
    if (items.empty()) {
        handleAddAccountWithBrowser(data, handler);
        return;
    }
    uiHelper.pickAccount(items, [this, handler, data](rpc_result<MsaUiClient::PickAccountResult> r) {
        if (!r.success()) {
            handler(rpc_json_result::error(r.error_code(), r.error_text()));
            return;
        }
        if (r.data().add_account) {
            handleAddAccountWithBrowser(data, handler);
        } else {
            handler(rpc_json_result::response({{"cid", r.data().cid}}));
        }
    });
}

void MsaService::handleAddAccountWithBrowser(nlohmann::json const& data, rpc_handler::result_handler const& handler) {
    std::string baseUrl = ServerConfig::ENDPOINT_INLINE_CONNECT_PARTNER;
    std::vector<std::pair<std::string, std::string>> p;
    p.emplace_back("platform", PLATFORM_NAME);
    if (data.count("client_id") > 0)
        p.emplace_back("client_id", data["client_id"].get<std::string>());
    if (data.count("cobrandid") > 0)
        p.emplace_back("cobrandid", data["cobrandid"].get<std::string>());
    std::string params = RequestUtils::encodeUrlParams(p);
    uiHelper.openBrowser(baseUrl + "&" + params, [this, handler](rpc_result<MsaUiClient::BrowserResult> r) {
        if (!r.success()) {
            handler(rpc_json_result::error(r.error_code(), r.error_text()));
            return;
        }
        auto const& properties = r.data().properties;
        std::string cid = properties.at("CID");
        std::string puid = properties.at("PUID");
        std::string username = properties.at("Username");
        auto token = createLegacyTokenFromProperties(properties);

        auto account = accountManager.addAccount(username, cid, puid, token);
        handler(rpc_json_result::response({{"cid", cid}}));
    });
}

std::shared_ptr<msa::LegacyToken> MsaService::createLegacyTokenFromProperties(
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

void MsaService::handleRequestToken(nlohmann::json const& data, rpc_handler::result_handler const& handler) {
    std::string cid = data["cid"];
    std::string client_id = data.value("client_id", std::string());
    auto const& scope_json = data["scope"];
    msa::SecurityScope scope = {scope_json["address"], scope_json.value("policy_ref", std::string())};
    bool silent = data.value("silent", false);

    auto account = accountManager.findAccount(cid);
    if (!account) {
        handler(rpc_json_result::error(MsaErrors::NoSuchAccount, "No such account"));
        return;
    }

    auto ret = account->requestTokens(loginManager, {scope}, client_id);
    if (ret.size() == 0) {
        handler(rpc_json_result::error(MsaErrors::InternalError, "Internal error (no token returned from internal route)"));
        return;
    }
    auto& token = ret.begin()->second;
    if (token.hasError() && !token.getError()->inlineAuthUrl.empty()) {
        if (silent) {
            handler(rpc_json_result::error(MsaErrors::MustShowUI, "Must show UI to acquire token (silent mode is requested)"));
        } else {
            uiHelper.openBrowser(token.getError()->inlineAuthUrl, [this, account, data, handler]
                    (rpc_result<MsaUiClient::BrowserResult> r) {
                if (!r.success()) {
                    handler(rpc_json_result::error(r.error_code(), r.error_text()));
                    return;
                }
                auto const& properties = r.data().properties;
                if (properties.count("PUID") > 0 && properties.count("Username") > 0 &&
                    properties.count("DAToken") > 0 && properties.count("DASessionKey") > 0) {
                    std::string resPuid = properties.at("PUID");
                    if (resPuid != account->getPUID() &&
                        !(account->getPUID().empty() && properties.at("CID").empty())) {
                        handler(rpc_json_result::error(MsaErrors::InternalError,
                                "Internal error (login returned a different account)"));
                        // TODO: add a new account from here maybe?
                        return;
                    }
                    std::string username = properties.at("Username");
                    auto newToken = createLegacyTokenFromProperties(properties);
                    account->updateDetails(username, std::move(newToken));
                }
                handleRequestToken(data, handler);
            });
        }
    } else if (token.hasError()) {
        handler(rpc_json_result::error(MsaErrors::TokenAcquisitionServerError, "Server error",
                                       createTokenErrorInfoJson(*token.getError())));
    } else {
        assert(token.getToken() != nullptr);
        handler(rpc_json_result::response(createTokenJson(*token.getToken())));
    }
}

nlohmann::json MsaService::createTokenJson(msa::Token const& token) {
    nlohmann::json ret;
    ret["scope"]["address"] = token.getSecurityScope().address;
    ret["created"] = std::chrono::duration_cast<std::chrono::milliseconds>(token.getCreatedTime().time_since_epoch()).count();
    ret["expires"] = std::chrono::duration_cast<std::chrono::milliseconds>(token.getExpiresTime().time_since_epoch()).count();
    if (token.getType() == msa::TokenType::Legacy) {
        auto& legacyToken = msa::token_cast<msa::LegacyToken>(token);
        ret["type"] = "urn:passport:legacy";
        ret["xml_data"] = Base64::encode(legacyToken.getXmlData());
        ret["binary_secret"] = Base64::encode(legacyToken.getBinarySecret());
    } else if (token.getType() == msa::TokenType::Compact) {
        auto& compactToken = msa::token_cast<msa::CompactToken>(token);
        ret["type"] = "urn:passport:compact";
        ret["binary_token"] = compactToken.getBinaryToken();
    }
    return ret;
}

nlohmann::json MsaService::createTokenErrorInfoJson(msa::TokenErrorInfo const& errorInfo) {
    nlohmann::json ret;
    ret["req_status"] = errorInfo.reqStatus;
    ret["error_status"] = errorInfo.errorStatus;
    ret["flow_url"] = errorInfo.flowUrl;
    ret["inline_auth_url"] = errorInfo.inlineAuthUrl;
    ret["inline_end_auth_url"] = errorInfo.inlineEndAuthUrl;
    return ret;
}
