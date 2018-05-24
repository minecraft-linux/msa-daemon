#include "MsaService.h"
#include "MsaUiLauncher.h"
#include "MsaUiHelper.h"
#include <thread>
#include <FileUtil.h>
#include <EnvPathUtil.h>

int main() {
    std::string msaHome = EnvPathUtil::getDataHome() + "/msa";
    std::string msaService = msaHome + "/service";
    std::string msaUiService = msaHome + "/.ui_service";
    std::string msaDataHome = msaHome + "/data";
    FileUtil::mkdirRecursive(msaDataHome);
    MsaUiLauncher uiLauncher (MSA_UI_APP_PATH, msaUiService);
    MsaUiHelper uiHelper (uiLauncher);
    MsaService service(msaService, msaDataHome, uiHelper);

    while (true) {
        std::this_thread::sleep_for(std::chrono::hours(1));
    }
}