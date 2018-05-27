#include "MsaUiClient.h"

using namespace simpleipc::client;

rpc_call<MsaUiClient::BrowserResult> MsaUiClient::openBrowser(std::string const& url) {
    nlohmann::json data;
    data["url"] = url;
    return rpc_call<MsaUiClient::BrowserResult>(rpc("msa/ui/open_browser", data), [](nlohmann::json const& d) {
        BrowserResult result;
        auto prop = d["properties"];
        for (auto it = prop.begin(); it != prop.end(); ++it)
            result.properties[it.key()] = it.value();
        return result;
    });
}

rpc_call<MsaUiClient::PickAccountResult> MsaUiClient::pickAccount(std::vector<PickAccountItem> const& items) {
    nlohmann::json data;
    auto& accounts = data["accounts"] = nlohmann::json::array();
    for (auto const& item : items) {
        accounts.push_back(
                {
                        {"cid", item.cid},
                        {"username", item.username}
                });
    };

    return rpc_call<MsaUiClient::PickAccountResult>(rpc("msa/ui/pick_account", data), [](nlohmann::json const& d) {
        PickAccountResult result;
        result.cid = d.value("cid", "");
        result.add_account = d.value("add_account", false);
        return result;
    });
}