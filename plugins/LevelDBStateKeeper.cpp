#include "StateKeeper.hpp"

#include <leveldb/db.h>

#include <stdexcept>

namespace hammy {

class LevelDBStateKeeper : public StateKeeper {
        leveldb::DB* db_;
        leveldb::Options options_;

    public:
        LevelDBStateKeeper(std::string const &filename)
            : db_(nullptr)
        {
            options_.create_if_missing = true;
            leveldb::Status status = leveldb::DB::Open(options_, filename, &db_);
            if(!status.ok()) {
                std::string msg{"leveldb open error: "};
                msg += status.ToString();
                throw std::runtime_error(msg);
            }
        }

        virtual ~LevelDBStateKeeper() throw() {
            if(db_ != nullptr)
                delete db_;
        }

        virtual void set(std::string host, std::string metric, std::string buffer) {
            leveldb::Status s = db_->Put(leveldb::WriteOptions(), metric, buffer);
            if(!s.ok()) {
                std::string msg{"leveldb put error: "};
                msg += s.ToString();
                throw std::runtime_error(msg);
            }
        }

        virtual void get(std::string host, std::string metric, std::string *buffer) {
            leveldb::Status s = db_->Get(leveldb::ReadOptions(), metric, buffer);
            if(s.IsNotFound())
                *buffer = std::string{};

            if(!s.ok()) {
                std::string msg{"leveldb get error: "};
                msg += s.ToString();
                throw std::runtime_error(msg);
            }
        }
};

}

extern "C" hammy::Plugin *plugin_factory(boost::property_tree::ptree &config) {
    std::string filename = config.get<std::string>("dbfile");

    return dynamic_cast<hammy::Plugin *>(new hammy::LevelDBStateKeeper{filename});
}
