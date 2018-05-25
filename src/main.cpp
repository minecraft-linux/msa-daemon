#include "MsaService.h"
#include "MsaUiLauncher.h"
#include "MsaUiHelper.h"
#include <FileUtil.h>
#include <EnvPathUtil.h>
#include <log.h>
#include <argparser.h>

using namespace argparser;
using namespace daemon_utils;

static std::string findMsaUI() {
#ifdef MSA_UI_APP_PATH
    std::string path = MSA_UI_APP_PATH;
    auto appDir = EnvPathUtil::getAppDir() + "/";
    for (size_t iof = 0; iof != std::string::npos; ) {
        size_t iof2 = path.find(':', iof);
        std::string el = (iof2 == std::string::npos ? path.substr(iof) : path.substr(iof, iof2 - iof));
        if (el[0] != '/')
            el = appDir + el;
        if (FileUtil::exists(el))
            return el;
        iof = iof2;
    }
#else
    Log::warn("Main", "No Msa UI path set on compile-time; will not be able to add new accounts");
#endif
    return std::string();
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
    MsaUiLauncher uiLauncher (findMsaUI(), msaUiService);
    MsaUiHelper uiHelper (uiLauncher);
    MsaService service(msaService, msaDataHome, uiHelper,
                       autoClose ? shutdown_policy::no_connections : shutdown_policy::never);
    service.run();
}