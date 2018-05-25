#include "MsaService.h"
#include "MsaUiLauncher.h"
#include "MsaUiHelper.h"
#include <FileUtil.h>
#include <EnvPathUtil.h>
#include <log.h>

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

int main() {
    std::string msaHome = EnvPathUtil::getDataHome() + "/msa";
    std::string msaService = msaHome + "/service";
    std::string msaUiService = msaHome + "/.ui_service";
    std::string msaDataHome = msaHome + "/data";
    FileUtil::mkdirRecursive(msaDataHome);
    MsaUiLauncher uiLauncher (findMsaUI(), msaUiService);
    MsaUiHelper uiHelper (uiLauncher);
    MsaService service(msaService, msaDataHome, uiHelper, daemon_utils::shutdown_policy::no_connections);
    service.run();
}