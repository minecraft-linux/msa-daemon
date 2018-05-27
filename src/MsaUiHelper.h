#pragma once

#include <mutex>
#include <thread>
#include <functional>
#include <condition_variable>
#include "MsaUiClient.h"
#include "MsaUiLauncher.h"
#include "MsaErrors.h"

class MsaUiHelper {

private:
    using CallbackFunc = std::function<void (std::shared_ptr<MsaUiClient>)>;

    MsaUiLauncher& launcher;
    std::thread thread;
    std::mutex cbs_mutex;
    std::condition_variable cb_cv;
    std::vector<CallbackFunc> cbs;
    bool thread_running = false;

    static thread_local std::shared_ptr<MsaUiClient> service;

    void handleThread();

public:
    explicit MsaUiHelper(MsaUiLauncher& launcher) : launcher(launcher) {}

    ~MsaUiHelper() {
        if (thread.joinable())
            thread.join();
    }

    void post(CallbackFunc func);

    template <typename T, typename F>
    void postRpc(F func, simpleipc::client::rpc_result_callback<T> cb) {
        // service is passed here to avoid getting it freed, and ending up in us reconnecting to the service needlessly
        auto currentService = service;
        post([func, cb, currentService](std::shared_ptr<MsaUiClient> c) {
            if (!c) {
                cb(simpleipc::rpc_result<T>::error(MsaErrors::InternalUIStartError, "Failed to start UI subservice"));
                return;
            }
            func(c.get()).call([cb, c](simpleipc::rpc_result<T> res) {
                service = c;
                cb(std::move(res));
                service.reset();
            });
        });
    }

    void openBrowser(std::string const& url, simpleipc::client::rpc_result_callback<MsaUiClient::BrowserResult> cb) {
        postRpc(std::bind(&MsaUiClient::openBrowser, std::placeholders::_1, url), std::move(cb));
    }

    void pickAccount(std::vector<MsaUiClient::PickAccountItem> const& items,
                     simpleipc::client::rpc_result_callback<MsaUiClient::PickAccountResult> cb) {
        postRpc(std::bind(&MsaUiClient::pickAccount, std::placeholders::_1, items), std::move(cb));
    }

};