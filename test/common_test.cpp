
#include <gtest/gtest.h>
#include <random>
#include <map>
#include <cstring>
#include <array>
#include "gob_stdmap.hpp"

auto rng = std::default_random_engine {};

// Test compatibility between std:map and goblib::stdmap
TEST(goblib_stdmap, compatibility)
{
    std::map<int,int> s_map;
    goblib::stdmap<int,int> v_map;
    std::vector<int> v;

    for(int i=0;i<100;++i) { v.emplace_back(i); } 
    std::shuffle(v.begin(),v.end(), rng);

    // empty
    EXPECT_EQ(s_map.empty(), v_map.empty());
    
    // [],at
    for(auto& e : v)
    {
        s_map[e] = e * 2;
        v_map[e] = e * 2;
    }
    for(auto& e : v) { EXPECT_EQ(s_map[e], v_map[e]); }
    for(auto& e : v) { EXPECT_EQ(s_map.at(e), v_map.at(e)); }

    // size
    EXPECT_EQ(s_map.size(), v_map.size());

    // erase
    // by key
    auto pos = v.size() - 2;
    auto se = s_map.erase(pos);
    auto ve = v_map.erase(pos);
    EXPECT_EQ(se, ve);
    EXPECT_EQ(s_map.size(), v_map.size());
    EXPECT_EQ(s_map.find(pos), s_map.end());
    EXPECT_EQ(v_map.find(pos), v_map.end());
    // by iterator
    auto sit = std::next(s_map.find(3), 1); // [4]
    auto vit = std::next(v_map.find(3), 1); // [4]
    EXPECT_EQ(sit->first, vit->first);
    sit = s_map.find(4);
    auto sit2 = s_map.erase(sit);
    vit = v_map.find(4);
    auto vit2 = v_map.erase(vit);
    EXPECT_EQ(s_map.size(), v_map.size());
    EXPECT_EQ(sit2->first, vit2->first);
    EXPECT_EQ(s_map.find(4), s_map.end());
    EXPECT_EQ(v_map.find(4), v_map.end());
    sit = std::next(s_map.find(3), 1); // [5]
    vit = std::next(v_map.find(3), 1); // [5]
    EXPECT_EQ(sit->first, vit->first);
    // by iterators erase 10..19
    sit = s_map.find(10);
    sit2 = s_map.find(20);
    sit2 = s_map.erase(sit, sit2);
    vit = v_map.find(10);
    vit2 = v_map.find(20);
    vit2 = v_map.erase(vit, vit2);
    EXPECT_EQ(sit2->first, vit2->first);
    EXPECT_EQ(s_map.size(), v_map.size());
    for(int i=10;i<20;++i) // erased?
    {
        EXPECT_EQ(s_map.find(i), s_map.end()) << "S) i:" << i;
        EXPECT_EQ(v_map.find(i), v_map.end()) << "V) i:" << i;
    }
    EXPECT_EQ(s_map.find(20)->first, v_map.find(20)->first);

    // count
    EXPECT_EQ(s_map.count(1), s_map.count(1));
    EXPECT_EQ(s_map.count(15), s_map.count(15));
    EXPECT_EQ(s_map.count(-100), s_map.count(-100));

    // emplace
    // (key Not exists)
    auto rs = s_map.emplace(15, 30);
    auto rv = v_map.emplace(15, 30);
    EXPECT_EQ(rs.first->first, rv.first->first);   // iterator
    EXPECT_EQ(rs.second, true);
    EXPECT_EQ(rs.second, rv.second); // inserted? bool == true
    EXPECT_EQ(s_map.size(), v_map.size());
    // (key exists)
    rs = s_map.emplace(60, 120);
    rv = v_map.emplace(60, 120);
    EXPECT_EQ(rs.first->first, rv.first->first);   // iterator
    EXPECT_EQ(rs.second, false); // menas failed to emplace
    EXPECT_EQ(rs.second, rv.second); // inserted? bool == false
    EXPECT_EQ(s_map.size(), v_map.size());
    // (piecewise construct)
    s_map.emplace(std::piecewise_construct,
                  std::forward_as_tuple(999),
                  std::forward_as_tuple(112)); // empalce(999,112)
    v_map.emplace(std::piecewise_construct,
                  std::forward_as_tuple(999),
                  std::forward_as_tuple(112)); // empalce(999,112)
    EXPECT_EQ(s_map[999], v_map[999]);
    EXPECT_EQ(s_map.size(), v_map.size());
    
    // iterator
    {
        auto sbeg = s_map.begin();
        auto send = s_map.end();
        --send;
        auto vbeg = v_map.begin();
        auto vend = v_map.end();
        --vend;
        EXPECT_EQ(sbeg->first, vbeg->first);
        EXPECT_EQ(send->first, vend->first);
    }
    {
        auto sbeg = s_map.rbegin();
        auto send = s_map.rend();
        --send;
        auto vbeg = v_map.rbegin();
        auto vend = v_map.rend();
        --vend;
        EXPECT_EQ(sbeg->first, vbeg->first);
        EXPECT_EQ(send->first, vend->first);
    }

    // bound
    {
        // lower
        // exists
        auto sit = s_map.lower_bound(30);
        auto vit = v_map.lower_bound(30);
        EXPECT_EQ(sit->first,  vit->first);
        EXPECT_EQ(sit->second, vit->second);
        // not exists
        sit = s_map.lower_bound(10000);
        vit = v_map.lower_bound(10000);
        EXPECT_EQ(sit, s_map.end());
        EXPECT_EQ(vit, v_map.end());

        // upper
        // exists
        sit = s_map.upper_bound(30);
        vit = v_map.upper_bound(30);
        EXPECT_EQ(sit->first,  vit->first);
        EXPECT_EQ(sit->second, vit->second);
        // not exists
        sit = s_map.upper_bound(-33);
        vit = v_map.upper_bound(-33);
        EXPECT_EQ(sit, s_map.begin());
        EXPECT_EQ(vit, v_map.begin());

        // equal
        auto sp = s_map.equal_range(50);
        auto vp = v_map.equal_range(50);
        EXPECT_EQ(std::distance(sp.first, sp.second), std::distance(vp.first, vp.second));
        sp = s_map.equal_range(5000);
        vp = v_map.equal_range(5000);
        EXPECT_EQ(std::distance(sp.first, sp.second), std::distance(vp.first, vp.second));
        sp = s_map.equal_range(-5000);
        vp = v_map.equal_range(-5000);
        EXPECT_EQ(std::distance(sp.first, sp.second), std::distance(vp.first, vp.second));
    }
    
    // swap
    std::map<int,int> s_map2;
    goblib::stdmap<int,int> v_map2;

    s_map.swap(s_map2);
    v_map.swap(v_map2);
    EXPECT_TRUE(s_map.empty());
    EXPECT_FALSE(s_map2.empty());
    EXPECT_EQ(s_map.empty(), v_map.empty());
    EXPECT_EQ(s_map2.size(), v_map2.size());

    std::swap(s_map, s_map2);
    std::swap(v_map, v_map2);
    EXPECT_FALSE(s_map.empty());
    EXPECT_TRUE(s_map2.empty());
    EXPECT_EQ(s_map.empty(), v_map.empty());
    EXPECT_EQ(s_map2.empty(), v_map2.empty());
    EXPECT_EQ(s_map.size(), v_map.size());
    
    // clear
    s_map.clear();
    v_map.clear();
    EXPECT_EQ(s_map.empty(), v_map.empty());
}


#if 0
namespace
{
struct Foo
{
    Foo(const char* p) : _p(p) { printf("cstr\n"); ++cnt; }
    Foo(const Foo& f) : _p(f._p) { printf("copy cstr\n"); ++copy_con_cnt; }
    Foo(Foo&& f) : _p(std::move(f._p)) { printf("move cstr\n"); ++move_con_cnt; _p = nullptr; }
    Foo& operator=(const Foo& f) { _p = f._p; ++eq_cnt; return *this; }
    Foo& operator=(Foo&& f) { _p = std::move(f._p); ++move_eq_cnt; f._p = nullptr; return *this; }

        
    static uint32_t cnt;
    static uint32_t copy_con_cnt;
    static uint32_t move_con_cnt;
    static uint32_t eq_cnt;
    static uint32_t move_eq_cnt;

    bool operator<(const Foo& o) const { return strcmp(_p, o._p) < 0; }
    //        bool operator==(const Foo& o) const { return strcmp(_p, o._p) == 0; }
    //        bool operator!=(const Foo& o) const { return strcmp(_p, o._p) != 0; }
    const char* _p;
};
uint32_t Foo::cnt{};
uint32_t Foo::copy_con_cnt{};
uint32_t Foo::move_con_cnt{};
uint32_t Foo::eq_cnt{};
uint32_t Foo::move_eq_cnt{};
}


TEST(gobkib_stdmap, move)
{

    goblib::stdmap<Foo, int> mmap;

    mmap.emplace(Foo("ABC"), 42);
    //mmap[Foo("ABC")] = 1;
    printf("c:%u cc:%u mc:%u eq:%u me:%u",
           Foo::cnt, Foo::copy_con_cnt, Foo::move_con_cnt, Foo::eq_cnt, Foo::move_eq_cnt);

}
#endif


TEST(goblib_stdmap, compare)
{
    struct compare_str
    {
        bool operator()(const char* a, const char* b) const
        {
            // Strict weak order (operator<(x,x) is false)
            return strcmp(a,b) < 0;
        }
    };

    std::array<const char*, 26> org = {
        "Zulu", "Yankee", "X-ray", "Whiskey", "Victor", "Uniform",
        "Tango", "Sierra", "Romeo", "Quebec", "Papa", "Oscar",
        "November", "Mike", "Lima", "Kilo", "Juliett", "India",
        "Hotel", "Golf", "Foxtrot", "Echo", "Delta", "Charlie",
        "Bravo", "Alpha"
    };
    std::sort(org.begin(), org.end()); // order by address (上の並び順通りなるとは限らないので)
    
    std::array<const char*, 26> fcode = org;
    std::sort(fcode.begin(), fcode.end(), compare_str()); // A...Z にソート
    
    std::array<const char*, 26> shuffled = fcode;
    std::shuffle(shuffled.begin(),shuffled.end(), rng); // 混ぜる

    // fmap は文字列辞書順
    // badmap はポインタアドレス順
    goblib::stdmap<const char*, int, compare_str> fmap; // order by compare_str
    goblib::stdmap<const char*, int> badmap; // order by pointer

    for(auto& e : shuffled)
    {
        fmap.emplace(e, rng());
        badmap.emplace(e, rng());
    }
    //    for(auto&& e : badmap) { printf("> %s\n", e); }
    
    int idx = 0;
    for(auto& e : fmap)
    {
        EXPECT_STREQ(e.first, fcode[idx]);
        ++idx;
    }
    idx = 0;
    for(auto& e : badmap)
    {
        EXPECT_STREQ(e.first, org[idx]);
        ++idx;
    }

    char buf[16] = { 'K', 'i', 'l', 'o', '\0' };
    {
        auto sz = fmap.size();
        fmap.erase(buf); // 文字列比較なので消える
        EXPECT_EQ(sz - 1, fmap.size());
    }
    {
        auto sz = badmap.size();
        badmap.erase(buf); // アドレス比較なので消えない
        EXPECT_EQ(sz, badmap.size());
    }
}
