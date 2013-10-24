#include "StateKeeper.hpp"

#include <leveldb/db.h>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <stdexcept>
#include <functional>

namespace hammy {

class LevelDBStateKeeper : public StateKeeper {
        leveldb::DB* db_;
        leveldb::Options options_;

        boost::asio::io_service &io_;
        boost::asio::io_service io2_;

        boost::thread worker_th_;

    public:
        LevelDBStateKeeper(boost::asio::io_service &io, std::string const &filename)
            : db_(nullptr)
            , io_(io)
            , worker_th_(std::bind(&LevelDBStateKeeper::worker_f, this))
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

        virtual void get(std::string host, std::string metric, std::string *buffer, PluginCallback callback) {
            io2_.dispatch(
                    [=]() {
                        wrap_f(std::bind(&LevelDBStateKeeper::doGet, this, host, metric, buffer), callback);
                    }
                );
        }

        virtual void set(std::string host, std::string metric, std::string buffer, PluginCallback callback) {
            io2_.dispatch(
                    [=]() {
                        wrap_f(std::bind(&LevelDBStateKeeper::doSet, this, host, metric, buffer), callback);
                    }
                );
        }

    private:
        void worker_f() {
            io2_.run();
        }

        void wrap_f(std::function<void()> f, PluginCallback callback) {
            try {
                f();
                io_.dispatch(std::bind(callback, Error()));
            }
            catch(std::exception const &e) {
                io_.dispatch(std::bind(callback, Error(e)));
            }
            catch(...) {
                io_.dispatch(std::bind(callback, Error("<unknown error>")));
            }
        }

        void doSet(std::string host, std::string metric, std::string buffer) {
            leveldb::Status s = db_->Put(leveldb::WriteOptions(), metric, buffer);
            if(!s.ok()) {
                std::string msg{"leveldb put error: "};
                msg += s.ToString();
                throw std::runtime_error(msg);
            }
        }

        void doGet(std::string host, std::string metric, std::string *buffer) {
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

extern "C" hammy::Plugin *plugin_factory(boost::asio::io_service &io, boost::property_tree::ptree &config) {
    std::string filename = config.get<std::string>("dbfile");

    return dynamic_cast<hammy::Plugin *>(new hammy::LevelDBStateKeeper{io, filename});
}
