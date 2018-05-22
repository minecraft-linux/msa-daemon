#pragma once

#include <simpleipc/client/service_client.h>
#include <simpleipc/client/rpc_call.h>
#include <msa/legacy_token.h>

class MsaUiClient : public simpleipc::client::service_client {

public:
    MsaUiClient(const std::string& path);

public:
    struct BrowserResult {
        std::string cid;
        std::string username;
        std::string token;
    };

    simpleipc::client::rpc_call<BrowserResult> openBrowser(std::string const& url);

};