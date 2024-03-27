
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
    // by iterators
    sit = s_map.find(10);
    sit2 = s_map.find(20);
    sit2 = s_map.erase(sit, sit2);
    vit = v_map.find(10);
    vit2 = v_map.find(20);
    vit2 = v_map.erase(vit, vit2);
    EXPECT_EQ(sit2->first, vit2->first);
    EXPECT_EQ(s_map.size(), v_map.size());
    for(int i=10;i<20;++i)
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
    // Not exists
    auto rs = s_map.emplace(15, 30);
    auto rv = v_map.emplace(15, 30);
    EXPECT_EQ(rs.first->first, rv.first->first);   // iterator
    EXPECT_EQ(rs.second, rv.second); // inserted? bool
    EXPECT_EQ(s_map.size(), v_map.size());
    // Exists
    rs = s_map.emplace(60, 120);
    rv = v_map.emplace(60, 120);
    EXPECT_EQ(rs.first->first, rv.first->first);   // iterator
    EXPECT_EQ(rs.second, rv.second); // inserted? bool
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
    
    // clear
    s_map.clear();
    v_map.clear();
    EXPECT_EQ(s_map.empty(), v_map.empty());
}


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

    // ポインタ順だと Z...A
    std::array<const char*, 26> fcode = {
        "Zulu", "Yankee", "X-ray", "Whiskey", "Victor", "Uniform",
        "Tango", "Sierra", "Romeo", "Quebec", "Papa", "Oscar",
        "November", "Mike", "Lima", "Kilo", "Juliett", "India",
        "Hotel", "Golf", "Foxtrot", "Echo", "Delta", "Charlie",
        "Bravo", "Alpha"
    };
    std::sort(fcode.begin(), fcode.end(), compare_str()); // A...Z にソート
    
    std::array<const char*, 26> shuffled = fcode;
    std::shuffle(shuffled.begin(),shuffled.end(), rng); // 混ぜる

#if 0
    for(auto& e : fcode) { printf("%p\n", e); }
    printf("shuffled ---\n");
    for(auto& e : shuffled) { printf("%s\n", e); }
#endif
    
    goblib::stdmap<const char*, int, compare_str> fmap; // order by compare_str
    goblib::stdmap<const char*, int> badmap; // order by pointer
    for(auto& e : shuffled)
    {
        fmap.emplace(e, rng());
        badmap.emplace(e, rng());
    }

#if 0
    printf("valid ---\n");
    for(auto& e : fmap)   { printf("%s\n", e.first); }
    printf("bad ---\n");
    for(auto& e : badmap) { printf("%s\n", e.first); }
#endif
    
    int idx = 0;
    for(auto& e : fmap)
    {
        EXPECT_STREQ(e.first, fcode[idx]);
        ++idx;
    }
    idx = 0;
    for(auto& e : badmap)
    {
        EXPECT_STRNE(e.first, fcode[idx]);
        ++idx;
    }
}

// embedded のみへ
TEST(goblib_stdmap, benchmark)
{


}
