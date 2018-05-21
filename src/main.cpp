#include "FileUtil.h"
#include "MsaService.h"

int main() {
    std::string msaHome = FileUtil::getDataHome() + "/msa";
    std::string msaService = msaHome + "/service";
    std::string msaDataHome = msaHome + "/data";
    FileUtil::mkdirRecursive(msaDataHome);
    MsaService service(msaService, msaDataHome);
}