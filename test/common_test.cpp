
#include <gtest/gtest.h>
#include <random>
#include <map>
#include <cstring>
#include <array>
#include "gob_stdmap.hpp"
#include "allocator.hpp"

auto rng = std::default_random_engine {};

#if defined(ARDUINO)
#include <WString.h>
using string_t = String;
#else
#include <cstring>
using string_t = std::string;
#endif


TEST(goblib_stdmap, constructor)
{
    using pair_t = std::pair<int, int>;
    std::array<pair_t, 3> elms = {{{1, 2}, {3, 4}, {5, 6}}};

    {
        goblib::stdmap<int,int> gmap;
        EXPECT_TRUE(gmap.empty());
        EXPECT_EQ(gmap.size(), 0U);
        gmap.emplace(1,2);
        EXPECT_EQ(gmap.size(), 1U);

    }

    {
        auto gmap = goblib::stdmap<int,int>(std::less<int>(), MyAllocator<pair_t>());
        EXPECT_TRUE(gmap.empty());
        EXPECT_EQ(gmap.size(), 0U);
        gmap.emplace(1,2);
        EXPECT_EQ(gmap.size(), 1U);
    }

    {
        auto gmap = goblib::stdmap<int,int>(MyAllocator<pair_t>());
        EXPECT_TRUE(gmap.empty());
        EXPECT_EQ(gmap.size(), 0U);
        gmap.emplace(1,2);
        EXPECT_EQ(gmap.size(), 1U);
    }

    {
        goblib::stdmap<int,int> gmap(elms.begin(), elms.end(), std::less<int>(), MyAllocator<pair_t>());
        EXPECT_FALSE(gmap.empty());
        EXPECT_EQ(gmap.size(), elms.size());
        gmap.emplace(111,222);
        EXPECT_EQ(gmap.size(), elms.size() + 1);

        auto gmap2 = gmap;
        EXPECT_EQ(gmap2.size(), elms.size() + 1);

        auto gmap3 = goblib::stdmap<int,int>(gmap2, MyAllocator<pair_t>());
        EXPECT_EQ(gmap3.size(), elms.size() + 1);

        goblib::stdmap<int,int> gmap4(std::move(gmap));
        EXPECT_EQ(gmap4.size(), elms.size() + 1);        
        EXPECT_TRUE(gmap.empty());
        
        MyAllocator<pair_t> alloc;
        goblib::stdmap<int,int> gmap5(std::move(gmap2), alloc);
        EXPECT_EQ(gmap5.size(), elms.size() + 1);
        EXPECT_TRUE(gmap2.empty());
    }
    {
        std::initializer_list<pair_t> il= { {1,2}, {2,3}, {3, 4} };
        goblib::stdmap<int,int> gmap(il, std::less<int>(), MyAllocator<pair_t>());
        EXPECT_FALSE(gmap.empty());
        EXPECT_EQ(gmap.size(), 3);
        gmap.emplace(111,222);
        EXPECT_EQ(gmap.size(), 4);
    }
}

TEST(goblib_stdmap, assignment)
{
    using pair_t = std::pair<int, int>;
    std::initializer_list<pair_t> il= { {1,2}, {2,3}, {3, 4} };
    {
        goblib::stdmap<int,int> gmap(il);
        goblib::stdmap<int,int> gmap2;

        EXPECT_FALSE(gmap.empty());
        EXPECT_EQ(gmap.size(), 3);
        EXPECT_TRUE(gmap2.empty());
        
        gmap2 = gmap;
        EXPECT_FALSE(gmap2.empty());
        EXPECT_EQ(gmap2.size(), 3);

        gmap.emplace(555,555);
        gmap2 = std::move(gmap);
        EXPECT_TRUE(gmap.empty());
        EXPECT_FALSE(gmap2.empty());
        EXPECT_EQ(gmap2.size(), 4);

        gmap2 = il;
        EXPECT_FALSE(gmap2.empty());
        EXPECT_EQ(gmap2.size(), 3);

    }
}



namespace
{

struct Person
{
    int id;
    int age;
    string_t name;

    Person() : id(-1), age(-2), name("deadbeaf") {}
    Person(const int a, const int b, const char* c) : id(a), age(b), name(c) {}
    Person(const Person& x) : id(x.id), age(x.age), name(x.name) { ++count_copy_ctr; }
    Person(Person&& x) : id(x.id), age(x.age), name(std::move(x.name)) { ++count_move_ctr; }

    Person& operator=(const Person& x)
    {
        ++count_copy;
        id = x.id;
        age = x.age;
        name = x.name;
        return *this;
    }
    Person& operator=(Person&& x)
    {
        ++count_move;
        id = x.id;
        age = x.age;
        name = std::move(x.name);
        return *this;
    }
    static uint32_t count_copy;
    static uint32_t count_move;
    static uint32_t count_copy_ctr;
    static uint32_t count_move_ctr;
    
};
uint32_t Person::count_copy_ctr{};
uint32_t Person::count_move_ctr{};
uint32_t Person::count_copy{};
uint32_t Person::count_move{};

struct PersonLess
{
    bool operator()(const Person& a, const Person& b) const noexcept
    {
        return std::tie(a.id, a.age, a.name) < std::tie(b.id, b.age, b.name);
    }
};
//
}

TEST(goblib_stdmap, comp)
{
    {
        goblib::stdmap<int,char> m;
        auto comp = m.key_comp();
        EXPECT_TRUE(comp(1,2));
        EXPECT_FALSE(comp(3,2));
    }
    {
        goblib::stdmap<int,char> c;
        const auto& comp = c.value_comp();

        auto p1 = std::make_pair(1,'a');
        auto p2 = std::make_pair(2,'b');
        auto p3 = std::make_pair(3,'c');

        EXPECT_TRUE(comp(p1, p2));
        EXPECT_FALSE(comp(p3, p2));
    }
}

TEST(goblib_stdmap, allocator)
{
    {
        goblib::stdmap<int,char> m;
        std::pair<int,char>* p = m.get_allocator().allocate(2);

        p[0].second = 'a';
        p[1].second = 'b';

        EXPECT_EQ(p[0].second, 'a');
        EXPECT_EQ(p[1].second, 'b');

        m.get_allocator().deallocate(p, 2);
    }

    {
        goblib::stdmap<int,char,std::less<int>, MyAllocator<std::pair<int,char>>> m;
        std::pair<int,char>* p = m.get_allocator().allocate(2);

        p[0].second = 'a';
        p[1].second = 'b';

        EXPECT_EQ(p[0].second, 'a');
        EXPECT_EQ(p[1].second, 'b');

        m.get_allocator().deallocate(p, 2);
    }
}


TEST(goblib_stdmap, comapre_operator)
{
    {
        goblib::stdmap<int,char> m1;
        m1[0] = 'a';
        auto m2 = m1;
        EXPECT_EQ(m1, m2);
        m2[0] = 'b';
        EXPECT_NE(m1, m2);
    }
    {
        goblib::stdmap<char,int> m1, m2;
        m1.insert(std::make_pair('a',10));
        m1.insert(std::make_pair('b',20));
        m1.insert(std::make_pair('c',30));
        m2 = m1;

        EXPECT_LE(m1, m2);
        EXPECT_GE(m2, m1);

        m2.insert(std::make_pair('d',40));
        EXPECT_LT(m1, m2);
        EXPECT_LE(m1, m2);
        EXPECT_GT(m2, m1);
        EXPECT_GE(m2, m1);
    }
}

TEST(goblib_stdmap, user_object)
{
    //std::map<Person, int, PersonLess> gmap =
    goblib::stdmap<Person, int, PersonLess> gmap =
    {
        //        {Person{0,  118, "Alice"}, 3},
        //  {Person{10, 230, "Bob"}, 1},
        // {Person{20, 330, "Carol"}, 4},
    };

    //printf("%u/%u/%u/%u\n", Person::count_copy_ctr, Person::count_move_ctr, Person::count_copy, Person::count_move);

    gmap.emplace(Person(1, 11, "Alice"), 42);

    //printf("%u/%u/%u/%u\n", Person::count_copy_ctr, Person::count_move_ctr, Person::count_copy, Person::count_move);

    #if 0
    for(auto&& e : gmap)
    {
        printf("--> Person(%d,%d,%s) %d\n", e.first.id, e.first.age, e.first.name.c_str(), e.second);
    }
    #endif
    EXPECT_EQ(42, gmap[Person(1, 11, "Alice")]);

    //printf("%u/%u/%u/%u\n", Person::count_copy_ctr, Person::count_move_ctr, Person::count_copy, Person::count_move);
    gmap.emplace(std::piecewise_construct,
                 std::forward_as_tuple(2, 22, "Bob"),
                 std::forward_as_tuple(888));
    //printf("%u/%u/%u/%u\n", Person::count_copy_ctr, Person::count_move_ctr, Person::count_copy, Person::count_move);    
    EXPECT_EQ(888, gmap[Person(2,22,"Bob")]);
}


TEST(goblib_stdmap, Compare)
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

    char buf[16] = { 'K', 'i', 'l', 'o', '\0' }; // "Kilo"
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
    EXPECT_EQ(s_map[999], 112);
    EXPECT_EQ(s_map[999], v_map[999]);
    EXPECT_EQ(s_map.size(), v_map.size());

    // emplace_hint
    {
        auto se = s_map.end();
        s_map.emplace_hint(se, 12345, 6789);
        auto ve = v_map.end();
        v_map.emplace_hint(ve, 12345, 6789);
        EXPECT_EQ(s_map[12345], 6789);
        EXPECT_EQ(s_map[12345], v_map[12345]);
        EXPECT_EQ(s_map.size(), v_map.size());
    }
    
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
        sit = s_map.lower_bound(999999);
        vit = v_map.lower_bound(999999);
        EXPECT_EQ(sit, s_map.end());
        EXPECT_EQ(vit, v_map.end());
        sit = s_map.lower_bound(-999999);
        vit = v_map.lower_bound(-999999);
        EXPECT_EQ(sit, s_map.begin());
        EXPECT_EQ(vit, v_map.begin());

        // upper
        // exists
        sit = s_map.upper_bound(30);
        vit = v_map.upper_bound(30);
        EXPECT_EQ(sit->first,  vit->first);
        EXPECT_EQ(sit->second, vit->second);
        // not exists
        sit = s_map.lower_bound(999999);
        vit = v_map.lower_bound(999999);
        EXPECT_EQ(sit, s_map.end());
        EXPECT_EQ(vit, v_map.end());
        sit = s_map.lower_bound(-999999);
        vit = v_map.lower_bound(-999999);
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



