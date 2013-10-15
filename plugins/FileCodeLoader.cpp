#include "CodeLoader.hpp"
#include "Context.hpp"

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <limits>
#include <stdexcept>
#include <vector>
#include <map>
#include <set>
#include <tuple>

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

// Warning! Not thread-safe!
#define THROW_ERRNO(comment) { \
        std::ostringstream msg; \
        msg << "Error " << errno << " " << comment << ": " << strerror(errno); \
        throw std::runtime_error(msg.str()); \
    }

namespace hammy {

class FileCodeLoader : public CodeLoader {
    public:
        FileCodeLoader() {}

        virtual ~FileCodeLoader() throw() {}

        virtual std::vector<std::string> loadTrigger(std::string const &host, std::string const &metric) {
            std::vector<std::string> res;

            return res;
        }

        void load(std::string const &path) {
            namespace fs = boost::filesystem;

            fs::path root{path};
            if(!is_directory(root))
                throw std::runtime_error(path + " is not a directory");

            std::for_each(fs::directory_iterator(root), fs::directory_iterator(),
                [&](fs::directory_entry &de) {
                    fs::path p = de.path();
                    if(is_directory(p)) {
                        std::for_each(fs::directory_iterator(p), fs::directory_iterator(),
                            [&](fs::directory_entry &de2) {
                                fs::path p2 = de2.path();
                                if(fs::is_regular_file(p2) && (p2.extension() == ".lua") && file_size(p2) > 0) {
                                    std::string filename = p2.filename().native();
                                    std::string metric_name = filename.substr(0, filename.length() - 4);

                                    loadFile(p.filename().native(), metric_name, p2.native());
                                }
                            });
                    }
                });
        }

    private:
        std::map<std::string, std::map<std::string, std::string>> code_;
        std::map<std::string, std::map<std::string, std::set<std::tuple<std::string, std::string>>>> deps_;

    private:
        void loadFile(std::string const &host, std::string const &metric, std::string const &path) {
            Context c;

            int fd = open(path.c_str(), O_RDONLY);
            if(fd < 0)
                THROW_ERRNO("open");

            struct stat statbuf;
            if ( fstat(fd, &statbuf) < 0 )
                THROW_ERRNO("fstat");

            void *data = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
            if (data == MAP_FAILED)
                THROW_ERRNO("mmap");

            c.loadModule("module", (const char *)data, statbuf.st_size);

            code_[host][metric] = std::string((const char *)data, statbuf.st_size);

            if (munmap(data, statbuf.st_size))
                THROW_ERRNO("munmap");

            if(!c["module"]->asTable()->get("depends")->isNil()) {
                for(int i = 1; i < std::numeric_limits<decltype(i)>::max(); ++i) {
                    if(c["module"]->asTable()->get("depends")->asTable()->get(std::to_string(i).c_str())->isNil())
                        break;
                    std::string dep = c["module"]->asTable()->get("depends")->asTable()->get(std::to_string(i).c_str())->asString();
                    auto dep_sp = splitDep(dep);
                    addDep(std::get<0>(dep_sp), std::get<1>(dep_sp), host, metric);
                }
            }
        }

        std::tuple<std::string, std::string> splitDep(std::string const &dep) {
            std::vector<std::string> spv;
            boost::algorithm::split(spv, dep, boost::algorithm::is_any_of(":"), boost::token_compress_on);
            if(spv.size() < 2)
                throw std::runtime_error(std::string("Invalid dependency: ") + dep);
            return std::tuple<std::string, std::string>(spv[0], spv[1]);
        }

        void addDep(std::string const &host1, std::string const &metric1, std::string const &host2, std::string const &metric2) {
            deps_[host1][host2].insert(std::tuple<std::string, std::string>(host2, metric2));
        }
};

}

extern "C" hammy::Plugin *plugin_factory(boost::property_tree::ptree &config) {
    std::string path = config.get<std::string>("codepath");

    auto res = new hammy::FileCodeLoader;
    try {
        res->load(path);
    }
    catch(...) {
        delete res;
        throw;
    }

    return dynamic_cast<hammy::Plugin *>(res);
}
