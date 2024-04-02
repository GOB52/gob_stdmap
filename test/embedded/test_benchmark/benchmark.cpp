
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


template<class M> void benchmark_map(M& m, std::vector<typename M::key_type>& elms, const char* tag)
{
    M5_LOGW("---- %s ----", tag);
    auto mem = esp_get_free_heap_size();
    int64_t elapsed{};

    // insert
    {
        Profile _(elapsed);
        for(auto& e : elms) { m.insert({e, rng()}); }
    }
    M5_LOGI("usgae: %u", mem - esp_get_free_heap_size());
    M5_LOGI("   insert: %lld", elapsed);
    // find
    {
        Profile _(elapsed);
        for(auto& e : elms) { (void)m.find(e); }
    }
    M5_LOGI("     find: %lld", elapsed);
    // iteration
    {
        Profile _(elapsed);
        for(auto& e : m) { ++e.second; }
    }
    M5_LOGI("iteration: %lld", elapsed);
    // erase
    {
        Profile _(elapsed);
        for(auto it = elms.cbegin(); it != elms.cend(); ++it) { m.erase(*it); }
    }
    M5_LOGI("    erase: %lld", elapsed);
}

template<typename Key> void benchmark(const size_t sz, std::function<std::vector<Key>(const size_t)> f, const char* tag)
{
    using smap_t = std::map<Key,int>;
    using gmap_t = goblib::stdmap<Key,int>;
    std::vector<Key> elms = f(sz);

    M5_LOGE("==== benchmark [%s] %zu elemnts ====", tag, sz);
    smap_t s;
    benchmark_map(s, elms, "std::map");
    gmap_t g;
    benchmark_map(g, elms, "goblib::stdmap");
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
        benchmark<int>(sz, make_integers,   "key:int");
        benchmark<Person>(sz, make_persons, "Key:struct Person");
    }

    // Using PSRAM
    auto board = M5.getBoard();
    if(board== m5::board_t::board_M5StackCoreS3 ||
       board== m5::board_t::board_M5StackCore2)
    {
        constexpr size_t szz[] = { 5000, 10000 };
        for(auto&& sz : szz)
        {
            benchmark<int>(sz, make_integers,   "key:int");
            benchmark<Person>(sz, make_persons, "Key:struct Person");
        }
    }
#endif
}

