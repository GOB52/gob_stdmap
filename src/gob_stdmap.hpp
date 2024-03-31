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

#if 0
///@cond
template<typename First, typename... Rest>
typename std::enable_if<!std::is_same<First, std::piecewise_construct_t>::value, First>::type
get_first_arg(First&& first, Rest&&...) {
    return std::forward<First>(first);
}
template<typename... Args>
auto get_first_arg(std::piecewise_construct_t, Args&&... args) -> decltype(get_first_arg(std::forward<Args>(args)...))
{
    return get_first_arg(std::forward<Args>(args)...);
}
///@endcond
#endif

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
class stdmap
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
    struct ValueCompare
    {
      protected:
        explicit ValueCompare(key_compare c) : cmp(c) {}

      public:
        using result_type = bool;
        using first_argument_type = value_type;
        using second_argument_type = value_type;
        inline bool operator()(const value_type& a, const value_type& b) const
        {
            return cmp(a.first, b.first);
        }
        key_compare cmp{};
        friend class stdmap<Key, T, Compare, Allocator>;
    };
    ///@endcond
    using value_compare = ValueCompare;
    using allocator_type = Allocator;
    using difference_type = typename std::iterator_traits<iterator>::difference_type;
    using pointer = typename std::allocator_traits<Allocator>::pointer;
    using const_pointer = typename std::allocator_traits<Allocator>::const_pointer;

    ///@name Constructor
    ///@{
    stdmap() : stdmap(Compare()) {}
    explicit stdmap(const Compare& comp, const Allocator& alloc = Allocator()) : _v(alloc), _compare_key(comp), _compare_value(_compare_key) {}
    explicit stdmap(const Allocator& alloc) : stdmap(Compare(), alloc) {}
    template <class InputIterator> stdmap(InputIterator first, InputIterator last, const Compare& comp = Compare(), const Allocator& alloc = Allocator()) : _v(first, last, alloc), _compare_key(comp), _compare_value(_compare_key) {}
    stdmap(const stdmap& x) : _v(x._v), _compare_key(x._compare_key), _compare_value(_compare_key) {}
    stdmap(const stdmap& x, const Allocator& alloc) : _v(x._v.begin(), x._v.end(), alloc), _compare_key(x._compare_key), _compare_value(_compare_key) {}
    stdmap(stdmap&& x) : _v(std::move(x._v)), _compare_key(std::move(x._compare_key)), _compare_value(std::move(x._compare_value)) {}
    stdmap(stdmap&& x, const Allocator& alloc) : _v(std::move(x._v), alloc), _compare_key(std::move(x._compare_key)), _compare_value(x._compare_value) {}
    stdmap(std::initializer_list<value_type> init, const Compare& comp = Compare(), const Allocator& alloc = Allocator())
            : stdmap(init.begin(), init.end(), comp, alloc) {}
    ///@}

    ///@name Assignment
    ///@{
    stdmap& operator=(const stdmap& o)
    {
        _v = container_type(o._v.begin(), o._v.end(), o._v.get_allocator());
        _compare_key = o._compare_key;
        _compare_value = value_compare(_compare_key);
        return *this;
    }
    stdmap& operator=(stdmap&& o)
    {
        _v = std::move(o._v);
        _compare_key = std::move(o._compare_key);
        _compare_value = std::move(o._compare_value);
        return *this;
    }
    stdmap& operator=(std::initializer_list<value_type> il)
    {
        _v = il;
        return *this;
    }
    ///@}

    //! @brief Returns the associated allocator
    inline allocator_type get_allocator() const { return _v.get_allocator(); }
    
    ///@name Element access
    ///@{
    /*! @brief Access specified element with bounds checking */
    inline T& at(const key_type& key) { return at_impl(key); }
    //! @brief Access specified element with bounds checking
    inline const T& at(const key_type& key) const { return at_impl(key); }
    //! @brief Access or insert specified element
    T& operator[](const key_type& key)
    {
        auto it = lower_bound(key);
        if (it == _v.end() || ne_key(it->first, key)) { it = _v.insert(it, std::make_pair(key, T())); }
        return it->second;
    }
    //! @brief Access or insert specified element
    T& operator[](key_type&& key)
    {
        auto it = lower_bound(key);
        if (it == _v.end() || ne_key(it->first, key)) { it = _v.insert(it, std::make_pair(std::move(key), T())); }
        return it->second;
    }
    ///@}

    ///@name Iterators
    ///@{
    inline iterator begin() noexcept{ return _v.begin(); } //!< @brief Returns an iterator to the beginning
    inline const_iterator begin() const noexcept{ return _v.begin(); } //!< @brief Returns an iterator to the beginning
    inline const_iterator cbegin() const noexcept{ return _v.cbegin(); } //!< @brief Returns an iterator to the beginning
    inline iterator end() noexcept{ return _v.end(); } //!< @brief Returns an iterator to the end
    inline const_iterator end() const noexcept{ return _v.end(); } //!< @brief Returns an iterator to the end
    inline const_iterator cend() const noexcept{ return _v.cend(); } //!< @brief Returns an iterator to the end
    inline reverse_iterator rbegin() noexcept { return _v.rbegin(); } //!< @brief Returns a reverse iterator to the beginning
    inline const_reverse_iterator rbegin() const noexcept { return _v.rbegin(); } //!< @brief Returns a reverse iterator to the beginning
    inline const_reverse_iterator crbegin() const noexcept{ return _v.crbegin(); } //!< @brief Returns a reverse iterator to the beginning
    inline reverse_iterator rend() noexcept { return _v.rend(); } //!< @brief Returns a reverse iterator to the end
    inline const_reverse_iterator rend() const noexcept { return _v.rend(); } //!< @brief Returns a reverse iterator to the end
    inline const_reverse_iterator crend() const noexcept { return _v.crend(); } //!< @brief Returns a reverse iterator to the end
    ///@}

    ///@name Capacity
    ///@{
    inline bool empty() const noexcept { return _v.empty(); } //!< @brief Checks whether the container is empty
    inline size_type size() const noexcept { return _v.size(); } //!< @brief Returns the number of elements
    inline size_type max_size() const noexcept { return _v.max_size; } //!< @brief Returns the maximum possible number of elements
    ///@}

    ///@name Modifiers
    ///@{
    inline void clear() noexcept { _v.clear(); } //!< @brief Clears the contents
    //! @brief Inserts element
    std::pair<iterator, bool> insert(const value_type& x)
    {
        auto it = lower_bound(x.first);
        if(it == _v.end() || ne_key(it->first, x.first))
        {
            it = _v.insert(it, x);
            return {it, true };
        }
        return { it, false };
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
        auto it = position == _v.end() ? position : lower_bound(x.first);
        if(it == _v.end() || ne_key(it->first, x.first))
        {
            return _v.insert(it, x);
        }
        return _v.begin() + (it - _v.cbegin());
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
        auto it = first;
        while(it != last)
        {
            insert(*it);
            ++it;
        }
    }
    //! @brief Inserts elements
    inline void insert(std::initializer_list<value_type> init)
    {
        insert(init.begin(), init.end());
    }
#if 0
    /*!
      @brief Constructs element in-place
      @warning Note that even if no elements are inserted, an object of type value_type may be constructed,
      @warning and as a result the argument args may have been modified by the target of the move.
     */
    template <class... Args> std::pair<iterator, bool> emplace(std::piecewise_construct_t, Args&&... args)
    {
        const key_type key { std::get<0>(get_first_arg(std::piecewise_construct,std::forward<Args>(args)...)) };
        auto it = std::lower_bound(_v.begin(), _v.end(), key, [this](const value_type& a, const key_type& b)
        {
            return this->_compare_key(a.first, b);
        });
        if(it == _v.end() || ne_key(it->first, key))
        {
            it = _v.emplace(it, std::piecewise_construct, std::forward<Args>(args)...);
            return {it, true};
        }
        return {it, false};

    }
#endif
    /*!
      @brief Constructs element in-place
      @warning Note that even if no elements are inserted, an object of type value_type may be constructed,
      @warning and as a result the argument args may have been modified by the target of the move.
      @todo Like std::map, make move once if possible. (Not so with this implementation)
     */
    template <class... Args> std::pair<iterator, bool> emplace(Args&&... args)
    {
#if 0
        const key_type key { get_first_arg(std::forward<Args>(args)...) };
        auto it = std::lower_bound(_v.begin(), _v.end(), key, [this](const value_type& a, const key_type& b)
        {
            return this->_compare_key(a.first, b);
        });
        if(it == _v.end() || ne_key(it->first, key))
        {
            it = _v.emplace(it, std::forward<Args>(args)...);
            return {it, true};
        }
        return {it, false};
#else
        // Resolved correctly with or without std::piecewise_contruct
        value_type val(std::forward<Args>(args)...);
        auto it = std::lower_bound(_v.begin(), _v.end(), val, [this](const value_type& a, const value_type& b)
        {
            return this->_compare_value(a, b);
        });
        if(it == _v.end() || ne_key(it->first, val.first))
        {
            return { _v.emplace(it, std::move(val)), true };
        }
        return {it, false};
#endif
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
        auto it = hint == _v.end() ? hint : lower_bound(val.first);
        if(it == _v.end() || ne_key(it->first, val.first))
        {
            return _v.emplace(it, std::move(val));
        }
        return _v.begin() + (hint - _v.cbegin()); // Same position as hint
    }
    //! @brief Erases element
    size_type erase(const key_type& key)
    {
        auto it = find(key);
        if(it != _v.end() && eq_key(it->first, key)) { _v.erase(it, it + 1); return 1; }
        return 0;
    }
    //! @brief Erases element
    inline iterator erase(const_iterator position) { return _v.erase(position); }
    //! @brief Erases elements
    inline iterator erase(const_iterator first, const_iterator last) { return _v.erase(first, last); }
    //! @brief Swaps the contents
    inline void swap(stdmap<Key, T, Compare, Allocator>& o)
    {
        _v.swap(o._v);
        std::swap(_compare_key,o._compare_key);
        std::swap(_compare_value, o._compare_value);
    }
    ///@}

    ///@name Lookup
    ///@{
    /*! @brief Returns the number of elements matching specific key */
    inline size_type count(const key_type& key) const { return (size_type)(find(key) != _v.end());  }
    //! @brief Finds element with specific key
    inline const_iterator find(const key_type& key) const
    {
        auto it = std::lower_bound(_v.begin(), _v.end(), key,
                                   [this](const value_type& pair, const key_type& k)
                                   {
                                       return this->_compare_key(pair.first, k);
                                   });
        return (it != _v.end() && eq_key(it->first, key)) ? it : _v.end();
    }
    //! @brief Finds element with specific key
    inline iterator find(const key_type& key)
    {
        auto it = std::lower_bound(_v.begin(), _v.end(), key,
                                   [this](const value_type& pair, const key_type& k)
                                   {
                                       return this->_compare_key(pair.first, k);
                                   });
        return (it != _v.end() && eq_key(it->first, key)) ? it : _v.end();
    }
    //! @brief Returns an iterator to the first element not less than the given key
    inline       iterator lower_bound(const key_type& key)       { return lower_bound_impl(key); }
    //! @brief Returns an iterator to the first element not less than the given key
    inline const_iterator lower_bound(const key_type& key) const { return lower_bound_impl(key); }
    //! @brief Returns an iterator to the first element greater than the given key
    inline       iterator upper_bound(const key_type& key)       { return upper_bound_impl(key); }
    //! @brief Returns an iterator to the first element greater than the given key
    inline const_iterator upper_bound(const key_type& key) const { return upper_bound_impl(key); }
    //! @brief Returns range of elements matching a specific key
    std::pair<iterator, iterator> equal_range(const key_type& key)
    {
        return { lower_bound(key), upper_bound(key) };
    }
    //! @brief Returns range of elements matching a specific key
    std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
    {
        return { lower_bound(key), upper_bound(key) };
    }
    ///@}

    ///@name Observers
    ///@{
    inline key_compare key_comp() const { return _compare_key; } //!< @brief Returns the function that compares keys
    inline value_compare value_comp() const { return _compare_value; } //!< @brief Returns the function that compares keys in objects of type value_type
    ///@}

    ///@name Dedicated Extension
    ///@{
    inline void reserve(size_type n) { _v.reserve(n); } //!< @brief Reserves storage
    ///@}
    
  private:
    T& at_impl(const key_type& key)
    {
        auto it = find(key);
        if (it != _v.end() && eq_key(it->first, key)) { return it->second; }
        // Compiling a C++ source file with exceptions enabled? (GCC macro)
        // See also https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
#if defined(__EXCEPTIONS)
        throw std::out_of_range("gob_stdmap.at");
#else
        assert(false && "out_of_range");
        abort();
#endif
    }

    inline iterator lower_bound_impl(const key_type& key)
    {
        return std::lower_bound(_v.begin(), _v.end(), key,
                                [this](const value_type& pair, const key_type& k)
                                {
                                    return this->_compare_key(pair.first, k);
                                });
    }
    inline iterator upper_bound_impl(const key_type& key)
    {
        return std::upper_bound(_v.begin(), _v.end(), key,
                                [this](const key_type& k, const value_type& pair)
                                {
                                    return this->_compare_key(k, pair.first);
                                });
    }
    
    container_type _v;


  protected:
    ///@cond    
    key_compare _compare_key;
    value_compare _compare_value;
    inline bool ne_key(const key_type& a, const key_type& b) const { return _compare_key(a,b) || _compare_key(b,a); }
    inline bool eq_key(const key_type& a, const key_type& b) const { return !ne_key(a, b); }
    //@endcond
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
