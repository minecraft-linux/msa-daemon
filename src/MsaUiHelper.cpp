#include "MsaUiHelper.h"
#include <iostream>

thread_local std::shared_ptr<MsaUiClient> MsaUiHelper::service;

void MsaUiHelper::handleThread() {
    std::unique_lock<std::mutex> lock (cbs_mutex);
    std::weak_ptr<MsaUiClient> weakService;
    do {
        if (!this->cbs.empty()) {
            auto cbs = std::move(this->cbs);

            lock.unlock();
            service = weakService.lock();
            if (!service) {
                for (size_t i = 0; i < 5; i++) {
                    try {
                        service = std::shared_ptr<MsaUiClient>(new MsaUiClient(launcher), [this](MsaUiClient* client) {
                            delete client;
                            cb_cv.notify_all();
                        });
                        weakService = service;
                        break;
                    } catch (const std::exception& ex) {
                        std::cout << "MsaUiHelper Failed to start login UI\nYou are unable to login\n";
                        std::this_thread::sleep_for(std::chrono::seconds(5));
                    }
                }
                if (!service) {
                    break;
                }
            }

            for (auto& cb : cbs) {
                cb(service);
            }
            lock.lock();
            service.reset();
        }
        cb_cv.wait(lock);
    } while (!weakService.expired() || !this->cbs.empty());
    thread_running = false;
}

void MsaUiHelper::post(CallbackFunc func) {
    std::unique_lock<std::mutex> lock (cbs_mutex);
    cbs.push_back(std::move(func));
    if (thread_running) {
        lock.unlock();
        cb_cv.notify_all();
    } else {
        thread_running = true;
        if (thread.joinable())
            thread.join();
        thread = std::thread(&MsaUiHelper::handleThread, this);
    }
}