#include "Manager.hpp"

#include <string>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <algorithm>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>

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

static int
loadConfig(int argc, char *argv[], boost::property_tree::ptree &config) {
    namespace po = boost::program_options;
    namespace fs = boost::filesystem;

    po::options_description desc{"Allowed options"};
    desc.add_options()
        ("help,h", "help message")
        ("config,c", po::value<std::string>(), "configuration file")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if(vm.count("help") || !vm.count("config")) {
        std::cerr << desc << "\n";
        return 1;
    }

    std::string config_file = vm["config"].as< std::string >();
    fs::path p(config_file);

    if(p.extension() == ".xml") {
        read_xml(config_file, config);
    }
    else if(p.extension() == ".json") {
        read_json(config_file, config);
    }
    else if(p.extension() == ".ini") {
        read_ini(config_file, config);
    }
    else if(p.extension() == ".info") {
        read_info(config_file, config);
    }
    else {
        std::ostringstream msg;
        msg << "Unknown configuration file format: " << p.extension();
        throw std::runtime_error(msg.str());
    }

    return 0;
}

static void
getFiles(std::vector<std::string> &files, std::vector<std::string> const &dirs) {
    namespace fs = boost::filesystem;

    for(std::string const &dir : dirs) {
        fs::path p(dir);
        if(fs::is_regular_file(p) && (p.extension() == ".lua") && file_size(p) > 0) {
            files.push_back(p.native());
        }
        else if (fs::is_directory(p)) {
            std::for_each(fs::directory_iterator(p), fs::directory_iterator(),
                [&](fs::directory_entry &de) {
                    fs::path p2 = de.path();
                    if(fs::is_regular_file(p2) && (p2.extension() == ".lua") && file_size(p2) > 0) {
                        files.push_back(p2.native());
                    }
                }
            );
        }
    }
}

static void
loadFiles(std::vector<std::string> const &files, hammy::Manager &m) {
    for(std::string const &file : files) {
        int fd = open(file.c_str(), O_RDONLY);
        if(fd < 0)
            THROW_ERRNO("open");

        struct stat statbuf;
        if ( fstat(fd, &statbuf) < 0 )
            THROW_ERRNO("fstat");

        void *data = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
        if (data == MAP_FAILED)
            THROW_ERRNO("mmap");

        m.preload((const char *)data, statbuf.st_size);

        if (munmap(data, statbuf.st_size))
            THROW_ERRNO("munmap");
    }
}

int
smain(int argc, char *argv[]) {
    boost::property_tree::ptree config;

    int rc = loadConfig(argc, argv, config);
    if(rc)
        return rc;

    std::vector<std::string> files;

    {
        std::vector<std::string> dirs;

        auto h_cfg = config.get_child("hammy");
        auto dr = h_cfg.equal_range("preload");
        for(auto it = dr.first; it != dr.second; ++it) {
            dirs.push_back(it->second.get_value<std::string>());
        }

        getFiles(files, dirs);
    }

    hammy::Application app{config};

    hammy::Manager m{app};

    loadFiles(files, m);

    m.run();

    return 0;
}

int
main(int argc, char *argv[]) {
    try {
        return smain(argc, argv);
    }
    catch(std::exception const &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    catch(...) {
        std::cerr << "Unknown exception" << std::endl;
        return 1;
    }
}
