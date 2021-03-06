#include "lua.hpp"

#include <memory>

namespace hammy {

class LuaTable;

class LuaValue {
    private:
        LuaValue(LuaValue const &) {}
    public:
        LuaValue(lua_State *L);
        ~LuaValue() throw();

        bool isNil();

        const char *asString(size_t *len = nullptr);
        lua_Integer asInteger(bool *is_num = nullptr);
        lua_Number asNumber(bool *is_num = nullptr);
        bool asBool();
        std::shared_ptr<LuaTable> asTable();

    private:
        lua_State *L_;
};

class LuaTable {
    public:
        LuaTable(lua_State *L);
        ~LuaTable() throw();

        std::shared_ptr<LuaValue> get(const char *field);
        inline std::shared_ptr<LuaValue> operator[](const char *field) {
            return this->get(field);
        }

    private:
        lua_State *L_;
};

}
