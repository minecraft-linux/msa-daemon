#include <rapidxml.hpp>
#include "MsaService.h"

using namespace simpleipc;

MsaService::MsaService(std::string const& path, std::string const& dataPath)
        : service(path), storageManager(dataPath), accountManager(storageManager), loginManager(&storageManager) {
    using namespace std::placeholders;
    add_handler("msa/get_accounts", std::bind(&MsaService::handle_get_accounts, this));
    add_handler("msa/add_account", std::bind(&MsaService::handle_add_account, this, _3));
    add_handler("msa/remove_account", std::bind(&MsaService::handle_remove_account, this, _3));

    add_handler_async("msa/pick_account", std::bind(&MsaService::handle_pick_account, this, _3, _4));
}

rpc_result MsaService::handle_get_accounts() {
    auto accounts = accountManager.getAccounts();
    nlohmann::json ret;
    auto& l = ret["accounts"] = nlohmann::json::array();
    for (auto const& account : accounts) {
        nlohmann::json acc;
        acc["cid"] = account.getCID();
        acc["username"] = account.getUsername();
        l.push_back(std::move(acc));
    }
    return rpc_result::response(std::move(ret));
}

rpc_result MsaService::handle_add_account(nlohmann::json const& data) {
    std::string username = data["username"];
    std::string cid = data["cid"];
    std::string tokenData = data["token"];
    rapidxml::xml_document<char> doc;
    doc.parse<0>(&tokenData[0]);
    auto token = msa::token_pointer_cast<msa::LegacyToken>(msa::Token::fromXml(doc));
    try {
        accountManager.addAccount(username, cid, token);
    } catch (msa::AccountAlreadyExistsException& e) {
        return rpc_result::error(-101, e.what());
    }
    return rpc_result::response(nullptr);
}

simpleipc::rpc_result MsaService::handle_remove_account(nlohmann::json const& data) {
    std::string cid = data["cid"];
    try {
        auto account = accountManager.findAccount(cid);
        if (!account)
            throw msa::NoSuchAccountException();
        accountManager.removeAccount(*account);
    } catch (msa::NoSuchAccountException& e) {
        return rpc_result::error(-100, e.what());
    }
    return rpc_result::response(nullptr);
}

void MsaService::handle_pick_account(nlohmann::json const& data, rpc_handler::result_handler const& handler) {
    //
}