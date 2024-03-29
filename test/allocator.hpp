
#ifndef GOBLIB_STDMAP_TEST_ALLOCATOR_HPP
#define GOBLIB_STDMAP_TEST_ALLOCATOR_HPP

#include <memory>
#if 0
template<typename T>
class MyAllocator : public std::allocator<T> {
public:
    using size_type = typename std::allocator<T>::size_type;
    using pointer = typename std::allocator<T>::pointer;
    using const_pointer = typename std::allocator<T>::const_pointer;
    using reference = typename std::allocator<T>::reference;
    using const_reference = typename std::allocator<T>::const_reference;

    MyAllocator() = default;
    MyAllocator(const MyAllocator& other) noexcept : std::allocator<T>(other) {}
    template<typename U> MyAllocator(const std::allocator<U>& alloc) noexcept : std::allocator<T>(alloc) {}

    pointer allocate(size_type n) {
        _usage += n;
        printf("%s:%zu\n", __func__, n);
        return std::allocator<T>::allocate(n);
    }

    void deallocate(pointer p, size_type n) noexcept {
        printf("%s:%zu\n", __func__, n);
        std::allocator<T>::deallocate(p, n);
        _usage -= n;
    }

    MyAllocator select_on_container_copy_construction() const {
        return *this;
    }

    std::allocator<T> get_allocator() const noexcept {
        return *this;
    }

    size_t usage() const { return _usage; }
    
  private:
    size_t _usage{};

};
#endif

#if 0
template<typename T>
struct MyAllocator : public std::allocator<T> {
    MyAllocator() {}
    template<class Other> MyAllocator( const MyAllocator<Other>& _Right ) {}


    inline typename std::allocator<T>::pointer allocate(typename std::allocator<T>::size_type n, typename std::allocator<void>::const_pointer = 0) {
        //    std::cout << "Allocating: " << n << " itens." << std::endl;
        printf("NEW;%zu\n", n);
        _usage += n;
        return reinterpret_cast<typename std::allocator<T>::pointer>(::operator new(n * sizeof (T))); 
    }

    inline void deallocate(typename std::allocator<T>::pointer p, typename std::allocator<T>::size_type n) {
        printf("DEL;%zu\n", n);
        _usage -= n;
        //    std::cout << "Dealloc: " <<  n << " itens." << std::endl;
        ::operator delete(p); 
    }

    template<typename U>
    struct rebind {
        typedef MyAllocator<U> other;
    };


    size_t usage() const { return _usage; }

    size_t _usage{};
    
};
#endif

#if 1
template<typename T>
struct MyAllocator : public std::allocator<T> {
    MyAllocator() {}
    template<class Other> MyAllocator( const MyAllocator<Other>&) {}

    using base_type = std::allocator<T>;
    using pointer = typename base_type::pointer;
    using size_type = typename base_type::size_type;
    
    inline typename base_type::pointer allocate(typename base_type::size_type n, typename base_type::const_pointer = nullptr) {
        //        printf("NEW;%zu\n", n);
        _usage += n;
        return reinterpret_cast<typename base_type::pointer>(::operator new(n * sizeof(T)));
    }

    inline void deallocate(typename base_type::pointer p, typename base_type::size_type n) {
        //printf("DEL;%zu\n", n);
        _usage -= n;
        ::operator delete(p);
    }

    template<typename U>
    struct rebind {
        typedef MyAllocator<U> other;
    };

   std::allocator<T> get_allocator() const noexcept {
        return *this;
    }

    
    size_t usage() const { return _usage * sizeof(T); }

    size_t _usage{};
};
#endif


#endif


