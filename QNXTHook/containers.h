#pragma once
template <typename T>
class JagexList {
 public:
  JagexList(uint64_t ptr) : ptr_(ptr) {}

  class iterator
      : public std::iterator<std::input_iterator_tag, T, T, const T*, T> {
   public:
    uint64_t ptr_;

    explicit iterator(uint64_t ptr) : ptr_(ptr) {}
    iterator& operator++() {
      ptr_ += sizeof(T);
      return *this;
    }
    bool operator==(const iterator& other) const { return ptr_ == other.ptr_; }
    bool operator!=(const iterator& other) const { return ptr_ != other.ptr_; }

    T& operator*() const { return *reinterpret_cast<T*>(ptr_); }
  };

  const iterator begin() const {
    if (ptr_ == 0) {
      return iterator(0);
    }
    return iterator(*reinterpret_cast<uint64_t*>(ptr_ + 0x10));
  }

  const iterator end() const {
    if (ptr_ == 0) {
      return iterator(0);
    }
    return iterator(*reinterpret_cast<uint64_t*>(ptr_ + 0x18));
  }

  size_t size() { return std::distance(begin(), end()); }

  const uint64_t base_ptr() const { return ptr_; }

  const T& at(int index) const {
    /*if (index >= size()) {
            throw std::out_of_range("Index is out of range");
    }*/

    uint64_t first_ptr = *reinterpret_cast<uint64_t*>(ptr_ + 0x10);
    return *reinterpret_cast<T*>(first_ptr + index * sizeof(T));
  }

  const T& operator[](int index) const { return at(index); }

 private:
  const uint64_t ptr_;
};