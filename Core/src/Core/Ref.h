#pragma once
#include <atomic>

namespace fg {

class RefBase {
  public:
    virtual ~RefBase() = default;

    void AddRef() const {
      m_StrongRef.fetch_add(1, std::memory_order_relaxed);
    }
    void RelRef() const {
      auto count = m_StrongRef.fetch_sub(1, std::memory_order_acq_rel);
      if (count == 1) {
        delete this;
      }
    }
    void AddWeakRef() const {
      m_WeakRef.fetch_add(1, std::memory_order_relaxed);
    }
    void RelWeakRef() const {
      m_WeakRef.fetch_sub(1, std::memory_order_acq_rel);
    }
    uint32_t RefCount() const {
      return m_StrongRef;
    }
    uint32_t WeakRefCount() const {
      return m_WeakRef;
    }
    void Release() {
      delete this;
    }

  private:
    mutable std::atomic<uint32_t> m_StrongRef = 0;
    mutable std::atomic<uint32_t> m_WeakRef = 0;
};

template <class T>
class Ref {
  public:
    Ref(T* ptr = nullptr) : m_Ptr(ptr) {
      AddStrong();
    }

    Ref(const Ref& other) : m_Ptr(other.m_Ptr) {
      AddStrong();
    }

    Ref(Ref&& other) noexcept : m_Ptr(other.m_Ptr) {
      other.m_Ptr = nullptr;
    }

    ~Ref() {
      ReleaseStrong();
    }

    Ref& operator=(T* ptr) {
      if (m_Ptr != ptr) {
        ReleaseStrong();
        m_Ptr = ptr;
        AddStrong();
      }
      return *this;
    }

    Ref& operator=(const Ref& other) {
      if (this != &other) {
        ReleaseStrong();
        m_Ptr = other.m_Ptr;
        AddStrong();
      }
      return *this;
    }

    Ref& operator=(Ref&& other) noexcept {
      if (this != &other) {
        ReleaseStrong();
        m_Ptr = other.m_Ptr;
        other.m_Ptr = nullptr;
      }
      return *this;
    }

    T* operator->() const {
      return m_Ptr;
    }
    T& operator*() const {
      return *m_Ptr;
    }
    T* get() const {
      return m_Ptr;
    }

    bool operator==(const Ref& other) const {
      return m_Ptr == other.m_Ptr;
    }
    bool operator!=(const Ref& other) const {
      return !(other == *this);
    }
    bool operator!=(T* ptr) const {
      return ptr != m_Ptr;
    }

    explicit operator bool() const {
      return m_Ptr != nullptr;
    }

  private:
    T* m_Ptr = nullptr;

    void AddStrong() {
      if (m_Ptr) {
        m_Ptr->AddRef();
      }
    }

    void ReleaseStrong() {
      if (!m_Ptr) {
        return;
      }

      m_Ptr->RelRef();
      m_Ptr = nullptr;
    }
};

template <typename T>
class WeakRef {
  public:
    WeakRef() = default;

    WeakRef(const Ref<T>& strongRef) {
      if (strongRef) {
        strongRef->AddWeakRef();
      }
      m_Ptr = strongRef.m_Ptr;
    }

    WeakRef(const WeakRef& other) : m_Ptr(other.m_Ptr) {
      if (m_Ptr) {
        m_Ptr->AddWeakRef();
      }
    }

    WeakRef(WeakRef&& other) noexcept : m_Ptr(other.m_Ptr) {
      other.m_Ptr = nullptr;
    }

    ~WeakRef() {
      ReleaseWeak();
    }

    WeakRef& operator=(const WeakRef& other) {
      if (this != &other) {
        ReleaseWeak();
        m_Ptr = other.m_Ptr;
        if (m_Ptr) {
          m_Ptr->AddWeakRef();
        }
      }
      return *this;
    }

    WeakRef& operator=(WeakRef&& other) noexcept {
      if (this != &other) {
        m_Ptr = other.m_Ptr;
        other.m_Ptr = nullptr;
      }
      return *this;
    }

    Ref<T> Lock() const {
      return Ref<T>(m_Ptr);
    }

    bool Expired() const {
      return m_Ptr && m_Ptr->RefCount() > 0;
    }

  private:
    T* m_Ptr = nullptr;

    void ReleaseWeak() {
      if (m_Ptr) {
        m_Ptr->RelWeakRef();
      }
      m_Ptr = nullptr;
    }
};

template <class T, class... Args>
Ref<T> MakeRef(Args&&... args) {
  return Ref<T>(new T(std::forward<Args>(args)...));
}

}  // namespace fg
