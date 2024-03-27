/*!
  @file gob_stdmap.hpp
  @brief Main header for gob_stdmap

  @mainpage
  Memory-saving map container that is functionally compatible with std::map for C++11 or later
  
  @author GOB https://twitter.com/gob_52_gob

  @copyright 2024 GOB
  @copyright Licensed under the MIT license. See LICENSE file in the project root for full license information.
*/
#ifndef GOBLIB_STDMAP_HPP
#define GOBLIB_STDMAP_HPP

#include <vector>
#include <algorithm>

/*!
  @namespace goblib
  @brief Top level namespace of mine
 */
namespace goblib
{

/*!
  @class stdmap
  @brief std::map-like map class implemented with std::vector
  @tparam Key Type of key
  @tparam T Type of element
  @tparam Compare Function for compare
  @note Adding an element causes the element to be sorted by the value of key (for binary search)
  @warning Allocator is not supported
 */
template <typename Key, typename T, class Compare = std::less<Key>> class stdmap
{
  public:
    using key_type = Key;
    using mapped_type = T;
    using value_type = std::pair<Key, T>;
    using container_type = std::vector<value_type>;
    using size_type = typename container_type::size_type;
    using reference = value_type&;
    using const_reference =  const value_type&;
    using iterator = typename container_type::iterator;
    using const_iterator = typename container_type::const_iterator;
    using reverse_iterator = typename container_type::reverse_iterator;
    using const_reverse_iterator = typename container_type::const_reverse_iterator;
    using key_compare = Compare;
    
    stdmap() = default;

    ///@name Element access
    ///@{
    /*! @brief Access specified element with bounds checking */
    inline T& at(const key_type& key) { return _at(key); }
    //! @brief Access specified element with bounds checking
    inline const T& at(const key_type& key) const { return _at(key); }
    //! @brief Access or insert specified element
    T& operator[](const key_type& key)
    {
        auto it = find2(key);
        if (it == _v.end() || it->first != key) { it = _v.insert(it, std::make_pair(key, T())); }
        return it->second;
    }
    //! @brief Access or insert specified element
    T& operator[](key_type&& key)
    {
        auto it = find2(key);
        if (it == _v.end() || it->first != key) { it = _v.insert(it, std::make_pair(std::move(key), T())); }
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
    //! @brief Constructs element in-place
    template <class... Args> std::pair<iterator, bool> emplace(Args&&... args)
    {
        value_type val(std::forward<Args>(args)...);
        auto it = std::lower_bound(_v.begin(), _v.end(), val, [](const value_type& a, const value_type& b)
        {
            return key_compare()(a.first, b.first);
        });
        if(it == _v.end() || it->first != val.first) { it = _v.insert(it, std::move(val));  return {it, true}; }
        return {it, false};
    }
    //! @brief Erases element
    size_type erase(const key_type& key)
    {
#if 0
        auto it = std::remove_if(_v.begin(), _v.end(), [&key](const value_type& pair) { return pair.first == key; });
        size_type removed = std::distance(it, _v.end());
        _v.erase(it, _v.end());
        return removed;
#else
        auto it = find(key);
        if(it != _v.end() && it->first == key) { _v.erase(it, it + 1); return 1; }
        return 0;
#endif
    }
    //! @brief Erases element
    inline iterator erase(const_iterator position) { return _v.erase(position); }
    //! @brief Erases elements
    inline iterator erase(const_iterator first, const_iterator last) { return _v.erase(first, last); }
    ///@}

    ///@name Lookup
    ///@{
    /*! @brief Returns the number of elements matching specific key */
    inline size_type count(const key_type& key) const { return (size_type)(find(key) != _v.end());  }
    //! @brief Finds element with specific key
    inline const_iterator find(const key_type& key) const
    {
        auto it = std::lower_bound(_v.begin(), _v.end(), key, [](const value_type& pair, const key_type& k) { return key_compare()(pair.first, k); });
        return (it != _v.end() && it->first == key) ? it : _v.end();
    }
    //! @brief Finds element with specific key
    inline iterator find(const key_type& key)
    {
        auto it = std::lower_bound(_v.begin(), _v.end(), key, [](const value_type& pair, const key_type& k) { return key_compare()(pair.first, k); });
        return (it != _v.end() && it->first == key) ? it : _v.end();
    }
    ///@}

  private:
    T& _at(const key_type& key)
    {
        auto it = find(key);
        if (it != _v.end() && it->first == key) { return it->second; }
        // Compiling a C++ source file with exceptions enabled? (GCC macro)
        // See also https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
#if defined(__EXCEPTIONS)
        throw std::out_of_range("vmap.at");
#else
        assert(false && "out_of_range");
        abort();
#endif
    }

    inline iterator find2(const key_type& key)
    {
        return std::lower_bound(_v.begin(), _v.end(), key, [](const value_type& pair, const key_type& k) { return key_compare()(pair.first, k); });
    }

    container_type _v;
};
//
}
#endif
