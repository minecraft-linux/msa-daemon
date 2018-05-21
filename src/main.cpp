#include "FileUtil.h"
#include "MsaService.h"
#include <thread>

int main() {
    std::string msaHome = FileUtil::getDataHome() + "/msa";
    std::string msaService = msaHome + "/service";
    std::string msaDataHome = msaHome + "/data";
    FileUtil::mkdirRecursive(msaDataHome);
    MsaService service(msaService, msaDataHome);

    while (true) {
        std::this_thread::sleep_for(std::chrono::hours(1));
    }
}