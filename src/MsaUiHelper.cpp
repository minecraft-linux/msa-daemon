#include "MsaUiHelper.h"

void MsaUiHelper::handleThread() {
    std::unique_lock<std::mutex> lock (cbs_mutex);
    std::weak_ptr<MsaUiClient> weakService;
    do {
        if (!this->cbs.empty()) {
            auto cbs = std::move(this->cbs);

            lock.unlock();
            std::shared_ptr<MsaUiClient> service = weakService.lock();
            if (!service) {
                service = std::shared_ptr<MsaUiClient>(new MsaUiClient(launcher), [this](MsaUiClient* client) {
                    cb_cv.notify_all();
                    delete client;
                });
                weakService = service;
            }

            for (auto& cb : cbs)
                cb(service);
            lock.lock();
        }
        cb_cv.wait(lock);
    } while (!weakService.expired() && this->cbs.empty());
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
        thread = std::thread(&MsaUiHelper::handleThread, this);
    }
}