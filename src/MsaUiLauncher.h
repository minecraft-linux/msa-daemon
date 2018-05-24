#pragma once

#include <daemon_utils/daemon_launcher.h>
#include <FileUtil.h>

class MsaUiClient;

class MsaUiLauncher : protected daemon_utils::daemon_launcher {

private:
    std::string executable_path;

protected:
    std::vector<std::string> get_arguments() override {
        return {executable_path, "-p", service_path};
    }

    std::string get_cwd() override {
        return FileUtil::getParent(executable_path);
    }

public:
    MsaUiLauncher(std::string const& executable_path, std::string const& service_path) :
            daemon_launcher(service_path), executable_path(executable_path) {}


    MsaUiClient* createClient();

};