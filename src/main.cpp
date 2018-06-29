#include "MsaService.h"
#include "MsaUiLauncher.h"
#include "MsaUiHelper.h"
#include <FileUtil.h>
#include <EnvPathUtil.h>
#include <log.h>
#include <argparser.h>

using namespace argparser;
using namespace daemon_utils;

static bool findMsaUI(std::string const& what, std::string& ret) {
#ifdef MSA_UI_APP_PATH
    if (EnvPathUtil::findInPath(what, ret, MSA_UI_APP_PATH, EnvPathUtil::getAppDir().c_str()))
        return true;
#endif
    if (EnvPathUtil::findInPath(what, ret))
        return true;
    return false;
}

static std::string findMsaUI() {
    std::string ret;
    if (findMsaUI("msa-ui-gtk", ret))
        return ret;;
    if (findMsaUI("msa-ui-qt", ret))
        return ret;
    Log::warn("Main", "Could not find MSA UI, it will not be possible to log in");
    return ret;
}

int main(int argc, const char** argv) {
    arg_parser args;
    arg<std::string> msaHome(args, "--dir", "-d", "Specifies the main directory where the service handle and data will be stored", EnvPathUtil::getDataHome() + "/msa");
    arg<bool> autoClose(args, "--auto-exit", "-x", "Automatically exits the daemon after all clients have disconnected", false);
    if (!args.parse(argc, argv))
        return 1;

    std::string msaService = msaHome.get() + "/service";
    std::string msaUiService = msaHome.get() + "/.ui_service";
    std::string msaDataHome = msaHome.get() + "/data";
    FileUtil::mkdirRecursive(msaDataHome);
    MsaUiLauncher uiLauncher (findMsaUI(), msaUiService, msaService);
    MsaUiHelper uiHelper (uiLauncher);
    MsaService service(msaService, msaDataHome, uiHelper,
                       autoClose ? shutdown_policy::no_connections : shutdown_policy::never);
    service.run();
}