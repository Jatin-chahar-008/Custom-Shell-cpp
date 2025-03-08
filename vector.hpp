#include <stdexcept>
#include <initializer_list>
#include <utility>

template<typename T>
class Vector {
private:
    T* data;              // Pointer to the storage
    size_t capacity_;     // Total allocated space
    size_t size_;         // Number of elements currently stored

    // Helper function to reallocate memory
    void reallocate(size_t new_capacity) {
        T* new_data = new T[new_capacity];
        
        // Copy existing elements to new storage
        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data[i]);
        }
        
        // Delete old storage and update pointers
        delete[] data;
        data = new_data;
        capacity_ = new_capacity;
    }

public:
    // Iterator types
    typedef T* iterator;
    typedef const T* const_iterator;

    // Constructors
    Vector() : data(nullptr), capacity_(0), size_(0) {}
    
    explicit Vector(size_t count, const T& value = T()) : data(nullptr), capacity_(0), size_(0) {
        reserve(count);
        for (size_t i = 0; i < count; ++i) {
            push_back(value);
        }
    }
    
    Vector(std::initializer_list<T> init) : data(nullptr), capacity_(0), size_(0) {
        reserve(init.size());
        for (const auto& item : init) {
            push_back(item);
        }
    }
    
    // Copy constructor
    Vector(const Vector& other) : data(nullptr), capacity_(0), size_(0) {
        reserve(other.size_);
        for (size_t i = 0; i < other.size_; ++i) {
            push_back(other.data[i]);
        }
    }
    
    // Move constructor
    Vector(Vector&& other) noexcept : data(other.data), capacity_(other.capacity_), size_(other.size_) {
        other.data = nullptr;
        other.capacity_ = 0;
        other.size_ = 0;
    }
    
    // Destructor
    ~Vector() {
        clear();
        delete[] data;
    }
    
    // Copy assignment operator
    Vector& operator=(const Vector& other) {
        if (this != &other) {
            clear();
            reserve(other.size_);
            for (size_t i = 0; i < other.size_; ++i) {
                push_back(other.data[i]);
            }
        }
        return *this;
    }
    
    // Move assignment operator
    Vector& operator=(Vector&& other) noexcept {
        if (this != &other) {
            clear();
            delete[] data;
            
            data = other.data;
            size_ = other.size_;
            capacity_ = other.capacity_;
            
            other.data = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }
    
    // Element access
    T& operator[](size_t index) {
        return data[index];
    }
    
    const T& operator[](size_t index) const {
        return data[index];
    }
    
    T& at(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Vector::at: index out of range");
        }
        return data[index];
    }
    
    const T& at(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Vector::at: index out of range");
        }
        return data[index];
    }
    
    T& front() {
        return data[0];
    }
    
    const T& front() const {
        return data[0];
    }
    
    T& back() {
        return data[size_ - 1];
    }
    
    const T& back() const {
        return data[size_ - 1];
    }
    
    T* data_ptr() {
        return data;
    }
    
    const T* data_ptr() const {
        return data;
    }
    
    // Iterators
    iterator begin() {
        return data;
    }
    
    const_iterator begin() const {
        return data;
    }
    
    const_iterator cbegin() const {
        return data;
    }
    
    iterator end() {
        return data + size_;
    }
    
    const_iterator end() const {
        return data + size_;
    }
    
    const_iterator cend() const {
        return data + size_;
    }
    
    // Capacity
    bool empty() const {
        return size_ == 0;
    }
    
    size_t size() const {
        return size_;
    }
    
    size_t capacity() const {
        return capacity_;
    }
    
    void reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            reallocate(new_capacity);
        }
    }
    
    void shrink_to_fit() {
        if (size_ < capacity_) {
            reallocate(size_);
        }
    }
    
    // Modifiers
    void clear() {
        for (size_t i = 0; i < size_; ++i) {
            data[i].~T();
        }
        size_ = 0;
    }
    
    iterator insert(const_iterator pos, const T& value) {
        size_t index = pos - begin();
        if (index > size_) {
            throw std::out_of_range("Vector::insert: iterator out of range");
        }
        
        if (size_ == capacity_) {
            size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            reserve(new_capacity);
        }
        
        // Move elements after insertion point
        for (size_t i = size_; i > index; --i) {
            data[i] = std::move(data[i - 1]);
        }
        
        // Insert new element
        data[index] = value;
        ++size_;
        
        return begin() + index;
    }
    
    iterator insert(const_iterator pos, T&& value) {
        size_t index = pos - begin();
        if (index > size_) {
            throw std::out_of_range("Vector::insert: iterator out of range");
        }
        
        if (size_ == capacity_) {
            size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            reserve(new_capacity);
        }
        
        // Move elements after insertion point
        for (size_t i = size_; i > index; --i) {
            data[i] = std::move(data[i - 1]);
        }
        
        // Insert new element
        data[index] = std::move(value);
        ++size_;
        
        return begin() + index;
    }
    
    template<typename... Args>
    iterator emplace(const_iterator pos, Args&&... args) {
        size_t index = pos - begin();
        if (index > size_) {
            throw std::out_of_range("Vector::emplace: iterator out of range");
        }
        
        if (size_ == capacity_) {
            size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            reserve(new_capacity);
        }
        
        // Move elements after insertion point
        for (size_t i = size_; i > index; --i) {
            data[i] = std::move(data[i - 1]);
        }
        
        // Construct new element in place
        new (&data[index]) T(std::forward<Args>(args)...);
        ++size_;
        
        return begin() + index;
    }
    
    iterator erase(const_iterator pos) {
        return erase(pos, pos + 1);
    }
    
    iterator erase(const_iterator first, const_iterator last) {
        if (first > last || first < begin() || last > end()) {
            throw std::out_of_range("Vector::erase: iterators out of range");
        }
        
        size_t start_idx = first - begin();
        size_t end_idx = last - begin();
        size_t count = end_idx - start_idx;
        
        // Move elements after the erased range
        for (size_t i = start_idx; i + count < size_; ++i) {
            data[i] = std::move(data[i + count]);
        }
        
        // Call destructors for the removed elements
        for (size_t i = size_ - count; i < size_; ++i) {
            data[i].~T();
        }
        
        size_ -= count;
        return begin() + start_idx;
    }
    
    void push_back(const T& value) {
        if (size_ == capacity_) {
            size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            reserve(new_capacity);
        }
        data[size_] = value;
        ++size_;
    }
    
    void push_back(T&& value) {
        if (size_ == capacity_) {
            size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            reserve(new_capacity);
        }
        data[size_] = std::move(value);
        ++size_;
    }
    
    template<typename... Args>
    T& emplace_back(Args&&... args) {
        if (size_ == capacity_) {
            size_t new_capacity = capacity_ == 0 ? 1 : capacity_ * 2;
            reserve(new_capacity);
        }
        new (&data[size_]) T(std::forward<Args>(args)...);
        return data[size_++];
    }
    
    void pop_back() {
        if (size_ > 0) {
            --size_;
            data[size_].~T();
        }
    }
    
    void resize(size_t count, const T& value = T()) {
        if (count < size_) {
            // Destroy excessive elements
            for (size_t i = count; i < size_; ++i) {
                data[i].~T();
            }
        } else if (count > size_) {
            // Ensure we have enough capacity
            if (count > capacity_) {
                reserve(count);
            }
            
            // Construct new elements
            for (size_t i = size_; i < count; ++i) {
                data[i] = value;
            }
        }
        size_ = count;
    }
    
    void swap(Vector& other) noexcept {
        std::swap(data, other.data);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
};

// Non-member functions
template<typename T>
bool operator==(const Vector<T>& lhs, const Vector<T>& rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    
    for (size_t i = 0; i < lhs.size(); ++i) {
        if (!(lhs[i] == rhs[i])) {
            return false;
        }
    }
    
    return true;
}

template<typename T>
bool operator!=(const Vector<T>& lhs, const Vector<T>& rhs) {
    return !(lhs == rhs);
}

template<typename T>
void swap(Vector<T>& lhs, Vector<T>& rhs) noexcept {
    lhs.swap(rhs);
}