/*!
  @file gob_stdmap.hpp
  @brief Main header for gob_stdmap

  @mainpage
  Memory-saving map container that is functionally compatible with std::map for C++11 or later
  @sa https://en.cppreference.com/w/cpp/container/map
  
  @author GOB https://twitter.com/gob_52_gob
  @author https://github.com/GOB52

  @copyright 2024 GOB
  @copyright Licensed under the MIT license. See LICENSE file in the project root for full license information.
*/
#ifndef GOBLIB_STDMAP_HPP
#define GOBLIB_STDMAP_HPP

#include <vector>
#include <algorithm>
#include <memory>

/*!
  @namespace goblib
  @brief Top level namespace of mine
 */
namespace goblib
{

///@cond
// Extend Compare to allow value_type comparisons
template<typename K, typename T, class Compare>
struct compare_less : public Compare
{
    using key_type = K;
    using value_type = std::pair<key_type, T>;

    compare_less() {}
    explicit compare_less(const Compare& cmp) : Compare(cmp) {}
    
    inline bool operator()(const key_type& a,   const key_type& b)   const { return Compare::operator()(a, b); }
    inline bool operator()(const key_type& a,   const value_type& b) const { return Compare::operator()(a, b.first); }
    inline bool operator()(const value_type& a, const key_type& b)   const { return Compare::operator()(a.first, b); }
    inline bool operator()(const value_type& a, const value_type& b) const { return Compare::operator()(a.first, b.first); }
};
///@endcond

/*!
  @class stdmap
  @brief std::map-like map class implemented with std::vector
  @sa https://en.cppreference.com/w/cpp/container/map
  @tparam Key Type of key
  @tparam T Type of element
  @tparam Compare Function for compare [optional]
  @tparam Allocator Custom allocator [optional]
  @note Adding an element causes the element to be sorted by the value of key (for binary search)
  @warning Functions of std::map on C++11 are defined.
  @warning Those on C++14 or later are not defined.
  @warning For example, std::map::insert_or_assign (C++ 17 or later)
  
  <b>Differences from std::map (because using std::vector)</b>
   - <b>Allocator and value_type are different from std::map </b>
   - <b>Iterator lifetime is different from std::map</b>
*/
template <
    typename Key,
    typename T,
    class Compare = std::less<Key>,
    class Allocator = std::allocator<std::pair<Key, T>> // std::allocator<std::pair<const Key, T>> in std::map
          >
class stdmap : private std::vector<std::pair<Key,T>, Allocator>, private compare_less<Key, T, Compare>
{
  public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = typename std::pair<Key, T>; // typename std::pair<const Key, T> in std::map 
    using container_type = std::vector<std::pair<Key, T>, Allocator>;
    using size_type = typename container_type::size_type;
    using reference = value_type&;
    using const_reference =  const value_type&;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;
    using reverse_iterator = typename container_type::reverse_iterator;
    using const_reverse_iterator = typename container_type::const_reverse_iterator;
    using key_compare = Compare;
    ///@cond
    class ValueCompare : private key_compare
    {
        friend class stdmap;
      protected:
        explicit ValueCompare(key_compare pred) : key_compare(pred) {}
      public:
        bool operator()(const value_type& lhs, const value_type& rhs) const
        { return key_compare::operator()(lhs.first, rhs.first); }
    };
    ///@endcond
    using value_compare = ValueCompare;
    using allocator_type = Allocator;
    using difference_type = typename std::iterator_traits<iterator>::difference_type;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;

    using compare_type = compare_less<Key, T, Compare>;
    
    ///@name Constructor
    ///@{
    stdmap() : stdmap(Compare()) {}
    explicit stdmap(const Compare& comp, const Allocator& alloc = Allocator())
            : container_type(alloc), compare_type(comp) {}
    explicit stdmap(const Allocator& alloc) : stdmap(Compare(), alloc) {}
    template <class InputIterator> stdmap(InputIterator first, InputIterator last, const Compare& comp = Compare(), const Allocator& alloc = Allocator()) : container_type(alloc), compare_type(comp) { insert(first, last); }
    stdmap(const stdmap& x) : container_type(x), compare_type(x) {}
    stdmap(const stdmap& x, const Allocator& alloc) : container_type(x.begin(), x.end(), alloc), compare_type(x) {}
    stdmap(stdmap&& x) : container_type(std::move(x)), compare_type() {}
    stdmap(stdmap&& x, const Allocator& alloc) : container_type(std::move(x), alloc), compare_type() {}
    stdmap(std::initializer_list<value_type> init, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
            : stdmap(init.begin(), init.end(), comp, alloc) {}
    ///@}

    ///@name Assignment
    ///@{
    stdmap& operator=(const stdmap& o)
    {
        *(container_type*)this = o;
        *(compare_type*)this = o;
        return *this;
    }
    stdmap& operator=(stdmap&& o)
    {
        *(container_type*)this = std::move(o);
        *(compare_type*)this = std::move(o);
        return *this;
    }
    stdmap& operator=(std::initializer_list<value_type> il)
    {
        clear();
        insert(il.begin(), il.end());
        return *this;
    }
    ///@}

    //! @brief Returns the associated allocator
    inline allocator_type get_allocator() const { return container_type::get_allocator(); }
    
    ///@name Element access
    ///@{
    /*! @brief Access specified element with bounds checking */
    inline T& at(const key_type& key) { return at_impl(key); }
    //! @brief Access specified element with bounds checking
    inline const T& at(const key_type& key) const { return at_impl(key); }
    //! @brief Access or insert specified element
    mapped_type& operator[](const key_type& key)
    {
        return insert(value_type(key, mapped_type())).first->second;
    }
    //! @brief Access or insert specified element
    mapped_type& operator[](key_type&& key)
    {
        return insert(value_type(std::move(key), mapped_type())).first->second;
    }
    ///@}

    ///@name Iterators
    ///@{
    inline iterator begin() noexcept{ return container_type::begin(); } //!< @brief Returns an iterator to the beginning
    inline const_iterator begin() const noexcept{ return container_type::begin(); } //!< @brief Returns an iterator to the beginning
    inline const_iterator cbegin() const noexcept{ return container_type::cbegin(); } //!< @brief Returns an iterator to the beginning
    inline iterator end() noexcept{ return container_type::end(); } //!< @brief Returns an iterator to the end
    inline const_iterator end() const noexcept{ return container_type::end(); } //!< @brief Returns an iterator to the end
    inline const_iterator cend() const noexcept{ return container_type::cend(); } //!< @brief Returns an iterator to the end
    inline reverse_iterator rbegin() noexcept { return container_type::rbegin(); } //!< @brief Returns a reverse iterator to the beginning
    inline const_reverse_iterator rbegin() const noexcept { return container_type::rbegin(); } //!< @brief Returns a reverse iterator to the beginning
    inline const_reverse_iterator crbegin() const noexcept{ return container_type::crbegin(); } //!< @brief Returns a reverse iterator to the beginning
    inline reverse_iterator rend() noexcept { return container_type::rend(); } //!< @brief Returns a reverse iterator to the end
    inline const_reverse_iterator rend() const noexcept { return container_type::rend(); } //!< @brief Returns a reverse iterator to the end
    inline const_reverse_iterator crend() const noexcept { return container_type::crend(); } //!< @brief Returns a reverse iterator to the end
    ///@}

    ///@name Capacity
    ///@{
    inline bool empty() const noexcept { return container_type::empty(); } //!< @brief Checks whether the container is empty
    inline size_type size() const noexcept { return container_type::size(); } //!< @brief Returns the number of elements
    inline size_type max_size() const noexcept { return container_type::max_size; } //!< @brief Returns the maximum possible number of elements
    ///@}

    ///@name Modifiers
    ///@{
    inline void clear() noexcept { container_type::clear(); } //!< @brief Clears the contents
    //! @brief Inserts element
    std::pair<iterator, bool> insert(const value_type& x)
    {
        bool inserted{};
        auto it = lower_bound(x.first);
        if(it == container_type::end() || this->operator()(x.first, it->first))
        {
            it = container_type::insert(it, x);
            inserted = true;
        }
        return { it, inserted };
    }
    /*!
      @brief Inserts element
      @warning Equivalent to emplace(std::forward<P>(value)) and only participates in overload resolution if std::is_constructible<value_type, P&&>::value == true.
    */
    template <class P> std::pair<iterator, bool> insert(P&& x)
    {
        return emplace(std::forward<P>(x));
    }
    //! @brief Inserts element
    inline iterator insert(const_iterator position, const value_type& x)
    {
        // Is valid position?
        if( (position == container_type::begin() || this->operator()(*(position-1), x)) &&
            (position == container_type::end()    || this->operator()(x, *position)) )
        {
            return container_type::insert(position, x);
        }
        // Invalid, then insert by x.key position
        return insert(x).first;
    }
    /*!
      @brief Inserts element
      @warning Equivalent to emplace_hint(hint, std::forward<P>(value)) and only participates in overload resolution if std::is_constructible<value_type, P&&>::value == true.
    */
    template <class P> iterator insert(const_iterator position, P&& x)
    {
        return emplace_hint(position, std::forward<P>(x));
    }
    /*!
      @brief Inserts elementst
      @warning Inserts elements from range [first, last). If multiple elements in the range have keys that compare equivalent, it is unspecified which element is inserted
    */
    template <class InputIterator> void insert(InputIterator first, InputIterator last)
    {
        while(first != last) { insert(*first++);  }
    }
    //! @brief Inserts elements
    inline void insert(std::initializer_list<value_type> init)
    {
        insert(init.begin(), init.end());
    }
    /*!
      @brief Constructs element in-place
      @warning Note that even if no elements are inserted, an object of type value_type may be constructed,
      @warning and as a result the argument args may have been modified by the target of the move.
      @todo Like std::map, make move once if possible. (Not so with this implementation)
     */
    template <class... Args> std::pair<iterator, bool> emplace(Args&&... args)
    {
        bool emplaced{};
        // Resolved correctly with or without std::piecewise_contruct
        value_type val(std::forward<Args>(args)...);
        auto it = lower_bound(val.first);
        if(it == container_type::end() || this->operator()(val, it->first))
        {
            it = container_type::emplace(it, std::move(val));
            emplaced = true;
        }
        return { it, emplaced };
    }
    /*!
      @brief Constructs elements in-place using a hint
      @note If the return value is different from the hint, it may be considered inserted.
      @warning Note that even if no elements are inserted, an object of type value_type may be constructed,
      @warning and as a result the argument args may have been modified by the target of the move.
    */
    template <class... Args> iterator emplace_hint(const_iterator hint, Args&&... args)
    {
        // Resolved correctly with or without std::piecewise_contruct
        value_type val(std::forward<Args>(args)...);
        auto it = hint == container_type::end() ? hint : lower_bound(val.first);
        if(it == container_type::end() || this->operator()(val, it->first))
        {
            return container_type::emplace(it, std::move(val));
        }
        return container_type::begin() + (hint - container_type::cbegin()); // Same position as hint
    }
    //! @brief Erases element
    size_type erase(const key_type& key)
    {
        auto it = find(key);
        if(it != container_type::end()) { container_type::erase(it, it + 1); return 1; }
        return 0;
    }
    //! @brief Erases element
    inline iterator erase(const_iterator position) { return container_type::erase(position); }
    //! @brief Erases elements
    inline iterator erase(const_iterator first, const_iterator last) { return container_type::erase(first, last); }
    //! @brief Swaps the contents
    inline void swap(stdmap<Key, T, Compare, Allocator>& o)
    {
        container_type::swap((container_type&)o);
        std::swap((compare_type&)*this, (compare_type&)o);
    }
    ///@}

    ///@name Lookup
    ///@{
    /*! @brief Returns the number of elements matching specific key */
    inline size_type count(const key_type& key) const { return (size_type)(find(key) != container_type::end());  }
    //! @brief Finds element with specific key
    inline const_iterator find(const key_type& key) const
    {
        auto it = lower_bound(key);
        return it != container_type::end() && this->operator()(key, it->first) ? container_type::end() : it;
    }
    //! @brief Finds element with specific key
    inline iterator find(const key_type& key)
    {
        auto it = lower_bound(key);
        return it != container_type::end() && this->operator()(key, it->first) ? container_type::end() : it;
    }
    //! @brief Returns an iterator to the first element not less than the given key
    inline       iterator lower_bound(const key_type& key)       { return std::lower_bound(begin(), end(), key, (compare_type&)*this); }
    //! @brief Returns an iterator to the first element not less than the given key
    inline const_iterator lower_bound(const key_type& key) const { return std::lower_bound(begin(), end(), key, (compare_type&)*this); }
    //! @brief Returns an iterator to the first element greater than the given key
    inline       iterator upper_bound(const key_type& key)       { return std::upper_bound(container_type::begin(), container_type::end(), key, (compare_type&)*this); }
    //! @brief Returns an iterator to the first element greater than the given key
    inline const_iterator upper_bound(const key_type& key) const { return std::upper_bound(container_type::begin(), container_type::end(), key, (compare_type&)*this); }
    //! @brief Returns range of elements matching a specific key
    std::pair<iterator, iterator> equal_range(const key_type& key) { return std::equal_range(begin(), end(), key, (compare_type&)*this); }
    //! @brief Returns range of elements matching a specific key
    std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const { return std::equal_range(begin(), end(), key, (compare_type&)*this); }
    ///@}

    ///@name Observers
    ///@{
#if 0
    inline key_compare key_comp() const { return _compare_key; } //!< @brief Returns the function that compares keys
    inline value_compare value_comp() const { return _compare_value; } //!< @brief Returns the function that compares keys in objects of type value_type
#else
        key_compare key_comp() const
        { return *this; }

        value_compare value_comp() const
        {
            const key_compare& comp = *this;
            return value_compare(comp);
        }
#endif
    ///@}
    
    ///@name Dedicated Extension
    ///@{
    inline void reserve(size_type n) { container_type::reserve(n); } //!< @brief Reserves storage
    ///@}
    
  private:
    mapped_type& at_impl(const key_type& key)
    {
        auto it = find(key);
        if (it != container_type::end()) { return it->second; }
        // Compiling a C++ source file with exceptions enabled? (GCC macro)
        // See also https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
#if defined(__EXCEPTIONS)
        throw std::out_of_range("gob_stdmap.at");
#else
        assert(false && "out_of_range");
        abort();
#endif
    }
};

///@name Compare
/// @related goblib::stdmap
///@{
template<typename Key, typename T, class Compare = std::less<Key>,class Allocator = std::allocator<std::pair<Key, T>>>
bool operator==(const stdmap<Key,T,Compare, Allocator>& a, const stdmap<Key,T,Compare, Allocator>& b)
{
    using value_type = typename stdmap<Key,T,Compare, Allocator>::value_type;
    const auto& cmp = a.key_comp();
    return a.size() == b.size()
            && std::equal(a.begin(), a.end(), b.begin(),
                          [&cmp](const value_type& x, const value_type& y)
                          {
                              return (!(cmp(x.first, y.first) || cmp(y.first, x.first))) // Are same keys?
                                      && x.second == y.second; // Are same values?
                          });
}
template<typename Key, typename T, class Compare = std::less<Key>,class Allocator = std::allocator<std::pair<Key, T>>>
bool operator!=(const stdmap<Key,T,Compare, Allocator>& a, const stdmap<Key,T,Compare, Allocator>& b)
{
    return !(a == b);
}
template<typename Key, typename T, class Compare = std::less<Key>,class Allocator = std::allocator<std::pair<Key, T>>>
bool operator<(const stdmap<Key,T,Compare, Allocator>& a, const stdmap<Key,T,Compare, Allocator>& b)
{
    using value_type = typename stdmap<Key,T,Compare, Allocator>::value_type;
    auto cmp = a.key_comp();
    return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(),
                                        [&cmp](const value_type& x, const value_type& y)
                                        {
                                            return (!(cmp(x.first, y.first) || cmp(y.first, x.first)))
                                                    ? x.second < y.second // Compare by value if the keys are the same
                                                    : cmp(x.first, y.first); // Is x less y??
                                        });
}
template<typename Key, typename T, class Compare = std::less<Key>,class Allocator = std::allocator<std::pair<Key, T>>>
bool operator>(const stdmap<Key,T,Compare, Allocator>& a, const stdmap<Key,T,Compare, Allocator>& b)
{
    return b < a;
}
template<typename Key, typename T, class Compare = std::less<Key>,class Allocator = std::allocator<std::pair<Key, T>>>
bool operator<=(const stdmap<Key,T,Compare, Allocator>& a, const stdmap<Key,T,Compare, Allocator>& b)
{
    return !(a > b);
}
template<typename Key, typename T, class Compare = std::less<Key>,class Allocator = std::allocator<std::pair<Key, T>>>
bool operator>=(const stdmap<Key,T,Compare, Allocator>& a, const stdmap<Key,T,Compare, Allocator>& b)
{
    return !(a < b);
}
///@}

//goblib
}

namespace std
{
/*!
  @related goblib::stdmap
  @brief Specializes the std::swap algorithm for goblib::stdmap
*/
template <class Key, class T, class Compare, class Allocator>
inline void swap(goblib::stdmap<Key,T,Compare,Allocator>& x, goblib::stdmap<Key,T,Compare,Allocator>& y)
{
    x.swap(y);
}
//std
}

#endif
