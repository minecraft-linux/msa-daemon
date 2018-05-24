#pragma once

#include <daemon_utils/daemon_launcher.h>

class MsaUiClient;

class MsaUiLauncher : protected daemon_utils::daemon_launcher {

private:
    std::string executable_path;

protected:
    std::vector<std::string> get_arguments() override {
        return {executable_path, "-p", service_path};
    }

public:
    MsaUiLauncher(std::string const& executable_path, std::string const& service_path) :
            daemon_launcher(service_path), executable_path(executable_path) {}


    std::shared_ptr<MsaUiClient> createClient();

};