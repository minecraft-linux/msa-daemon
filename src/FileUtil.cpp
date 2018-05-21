#include "FileUtil.h"

#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <stdexcept>

std::string FileUtil::getHomeDir() {
    char* env = getenv("HOME");
    if (env != nullptr)
        return env;

    struct passwd pwd;
    long int bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
    if (bufsize == -1)
        bufsize = 16384;
    char* buf = new char[bufsize];
    struct passwd *result;
    getpwuid_r(getuid(), &pwd, buf, (size_t) bufsize, &result);
    if (result == nullptr) {
        delete[] buf;
        throw std::runtime_error("getpwuid failed");
    }
    std::string ret(result->pw_dir);
    delete[] buf;
    return ret;
}

std::string FileUtil::getDataHome() {
    char* env = getenv("XDG_DATA_HOME");
    if (env != nullptr)
        return env;
    return getHomeDir() + "/.local/share";
}

bool FileUtil::exists(std::string const& path) {
    return access(path.c_str(), F_OK) == 0;
}

bool FileUtil::isDirectory(std::string const& path) {
    struct stat s;
    stat(path.c_str(), &s);
    return S_ISDIR(s.st_mode);
}

std::string FileUtil::getParent(std::string const& path) {
    auto iof = path.rfind('/');
    if (iof == std::string::npos)
        return std::string();
    while (iof > 0 && path[iof - 1] == '/')
        iof--;
    return path.substr(0, iof);
}

void FileUtil::mkdirRecursive(std::string const& path) {
    if (isDirectory(path))
        return;
    if (exists(path))
        throw std::runtime_error(std::string("File exists and is not a directory: ") + path);
    mkdirRecursive(getParent(path));
    if (mkdir(path.c_str(), 0744) != 0)
        throw std::runtime_error(std::string("mkdir failed, path = ") + path);
}