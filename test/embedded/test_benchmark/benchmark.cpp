
#include <gtest/gtest.h>
#include <M5Unified.h>
#include <random>
#include <map>
#include "gob_stdmap.hpp"
#include <esp_system.h>

#if defined(ARDUINO)
#include <WString.h>
using string_t = String;
#else
#include <cstring>
using string_t = std::string;
#endif

namespace
{
auto rng = std::default_random_engine {};

struct Person
{
    int id;
    int age;
    string_t name;
    Person(const int a, const int b, const string_t&  c) : id(a), age(b), name(c) {}
};
bool operator<(const Person& a, const Person& b) noexcept
{
  return std::tie(a.id, a.age, a.name) < std::tie(b.id, b.age, b.name);
}

std::vector<Person> make_persons(const size_t sz)
{
    constexpr char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::vector<Person> v;
    
    for(size_t i = 0; i < sz; ++i)
    {
        string_t s;
        size_t len = 1 + (rng() % 7);
        for(size_t j = 0; j < len; ++j) { s += chars[rng() % sizeof(chars)]; }
        v.emplace_back(rng(), rng(), s);
    }
    std::shuffle(v.begin(),v.end(), rng);    
    return v;
}

std::vector<int> make_integers(const size_t sz)
{
    std::vector<int> v;
    for(size_t i=0;i < sz;++i) { v.emplace_back(esp_random()); };
    std::shuffle(v.begin(),v.end(), rng);
    return v;
}

class Profile
{
  public:
    explicit Profile(int64_t& out) : _start_at(esp_timer_get_time()), _save(out) {}
    ~Profile() {_save = esp_timer_get_time() - _start_at; }
  private:
    int64_t _start_at{};
    int64_t& _save;
};


template<typename Key> void benchmark(const size_t sz, std::function<std::vector<Key>(const size_t)> f)
{
    using smap_t = std::map<Key,int>;
    using gmap_t = goblib::stdmap<Key,int>;
    int64_t tm{};

    M5_LOGI("==== benchmark %zu elemnts ====", sz);
    std::vector<Key> elms = f(sz);

    // std::map
    auto mem = esp_get_free_heap_size();
    {
        smap_t m;
        // []=
        {
            Profile _(tm);
            for(auto& e : elms) { m[e] = rng(); }
        }
        M5_LOGI("std::map[]=: elapsed:%lld us mem:%u", tm, mem - esp_get_free_heap_size());
#if 0
        m.clear();
        // insert
        {
            Profile _(tm);
            for(auto& e : elms) { m.insert({e, rng()}); }
        }
        M5_LOGI("std::map.insert: elapsed:%lld", tm);
        m.clear();
        // emplace
        {
            Profile _(tm);
            for(auto& e : elms) { m.emplace(e, rng()); }
        }
        M5_LOGI("std::map.emplace: elapsed:%lld", tm);
#endif
        // find
        {
            Profile _(tm);
            for(auto& e : elms) { auto it = m.find(e); }
        }
        M5_LOGI("std::map.find: elapsed:%lld us", tm);
        // iteration
        {
            Profile _(tm);
            for(auto& e : m) { ++e.second; }
        }
        M5_LOGI("std::map.iteration: elapsed:%lld us", tm);
        // erase
        {
            Profile _(tm);
            for(auto it = elms.crbegin(); it != elms.crend(); ++it) { m.erase(*it); }
        }
        M5_LOGI("std::map.erase elapsed:%lld us", tm);
    }

    mem = esp_get_free_heap_size();
    {
        gmap_t m;
        m.reserve(sz); // goblib::stdmap dedicated Extension
        // []=
        {
            Profile _(tm);
            for(auto& e : elms) { m[e] = rng(); }
        }
        M5_LOGI("goblib::stdmap[]=: elapsed:%lld us mem:%u", tm, mem - esp_get_free_heap_size());
#if 0
        m.clear();
        // insert
        {
            Profile _(tm);
            for(auto& e : elms) { m.insert({e, rng()}); }
        }
        M5_LOGI("goblib::stdmap.insert: elapsed:%lld", tm);
        m.clear();
        // emplace
        {
            Profile _(tm);
            for(auto& e : elms) { m.emplace(e, rng()); }
        }
        M5_LOGI("goblib::stdmap.emplace: elapsed:%lld", tm);
#endif
        // find
        {
            Profile _(tm);
            for(auto& e : elms) { auto it = m.find(e); }
        }
        M5_LOGI("goblib::stdmap.find: elapsed:%lld us", tm);
        // iteration
        {
            Profile _(tm);
            for(auto& e : m) { ++e.second; }
        }
        M5_LOGI("goblib::stdmap.iteration: elapsed:%lld us", tm);
        // erase
        {
            Profile _(tm);
            for(auto it = elms.crbegin(); it != elms.crend(); ++it) { m.erase(*it); }
        }
        M5_LOGI("goblib::stdmap.erase elapsed:%lld us", tm);
    }
}
//
}

TEST(gob_stdmap, benchmark)
{
#if defined(ENABLE_BENCHMARK)
# pragma message "Execute benchmark"
    constexpr size_t szz[] = { 10, 100, 1000, 2000 };
    for(auto&& sz : szz)
    {
        benchmark<int>(sz, make_integers);
        benchmark<Person>(sz, make_persons);
    }

    // Using PSRAM
    auto board = M5.getBoard();
    if(board== m5::board_t::board_M5StackCoreS3 ||
       board== m5::board_t::board_M5StackCore2)
    {
        constexpr size_t szz[] = { 5000, 10000 };
        for(auto&& sz : szz)
        {
            benchmark<int>(sz, make_integers);
            benchmark<Person>(sz, make_persons);
        }
    }
#endif
}

