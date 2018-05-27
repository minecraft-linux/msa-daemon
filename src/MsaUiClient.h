#pragma once

#include <simpleipc/client/service_client.h>
#include <simpleipc/client/rpc_call.h>
#include <msa/legacy_token.h>
#include <daemon_utils/launchable_service_client.h>

class MsaUiClient : public daemon_utils::launchable_service_client {

public:
    explicit MsaUiClient(daemon_utils::daemon_launcher& launcher) : launchable_service_client(launcher) {}


    struct BrowserResult {
        std::map<std::string, std::string> properties;
    };

    simpleipc::client::rpc_call<BrowserResult> openBrowser(std::string const& url);


    struct PickAccountItem {
        std::string cid;
        std::string username;
    };
    struct PickAccountResult {
        std::string cid;
        bool add_account;
    };

    simpleipc::client::rpc_call<PickAccountResult> pickAccount(std::vector<PickAccountItem> const& items);

};