#include "MsaUiClient.h"

using namespace simpleipc::client;

MsaUiClient::MsaUiClient(const std::string& path) : service_client(path) {}

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