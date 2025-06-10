#pragma once
#include <atomic>
#include <type_traits>
#include "Assert.h"

namespace fg {

template <class T>
class WeakRef;

template <class T>
class Ref;

class RefBase;

class ReferenceCounter {
  public:
    uint32_t AddStrong() const {
      return m_StrongRef.fetch_add(1, std::memory_order_relaxed);
    }
    uint32_t RelRef() const {
      return m_StrongRef.fetch_sub(1, std::memory_order_acq_rel);
    }
    uint32_t AddWeakRef() const {
      return m_WeakRef.fetch_add(1, std::memory_order_relaxed);
    }
    uint32_t RelWeakRef() const {
      return m_WeakRef.fetch_sub(1, std::memory_order_acq_rel);
    }
    uint32_t GetStrongReferenceCount() const {
      return m_StrongRef;
    }
    uint32_t GetWeakReferenceCount() const {
      return m_WeakRef;
    }

  private:
    mutable std::atomic<uint32_t> m_StrongRef = 0;
    mutable std::atomic<uint32_t> m_WeakRef = 0;
};

class RefBase {
  public:
    RefBase(ReferenceCounter* counter) : m_RefCounter(counter) {
    }
    virtual ~RefBase() = default;

    ReferenceCounter* GetRefCounter() const {
      return m_RefCounter;
    }
    void AddRef() const {
      m_RefCounter->AddStrong();
    }
    void RelRef() const {
      m_RefCounter->RelRef();
    }
    void AddWeakRef() const {
      m_RefCounter->AddWeakRef();
    }
    void RelWeakRef() const {
      m_RefCounter->RelWeakRef();
    }
    uint32_t RefCount() const {
      return m_RefCounter->GetStrongReferenceCount();
    }
    uint32_t WeakRefCount() const {
      return m_RefCounter->GetWeakReferenceCount();
    }
    void Release() {
      delete this;
    }

  private:
    ReferenceCounter* m_RefCounter;
};

template <class T>
class Ref {
  public:
    Ref(T* ptr = nullptr) : m_Ptr(ptr) {
      if (m_Ptr) {
        m_RefCounter = m_Ptr->GetRefCounter();
        AddStrong();
      }
    }

    Ref(const Ref& other) : m_Ptr(other.m_Ptr) {
      if (other) {
        m_RefCounter = other.m_RefCounter;
        AddStrong();
      }
    }

    Ref(Ref&& other) noexcept : m_Ptr(other.m_Ptr), m_RefCounter(other.m_RefCounter) {
      other.m_Ptr = nullptr;
      other.m_RefCounter = nullptr;
    }

    ~Ref() {
      ReleaseStrong();
    }

    Ref& operator=(T* ptr) {
      if (m_Ptr != ptr) {
        ReleaseStrong();
        m_Ptr = ptr;
        if (ptr) {
          m_RefCounter = ptr->GetRefCounter();
        }
        AddStrong();
      }
      return *this;
    }

    Ref& operator=(const Ref& other) {
      if (this != &other) {
        ReleaseStrong();
        m_Ptr = other.m_Ptr;
        m_RefCounter = other.m_RefCounter;
        AddStrong();
      }
      return *this;
    }

    Ref& operator=(Ref&& other) noexcept {
      if (this != &other) {
        // ReleaseStrong();
        m_Ptr = other.m_Ptr;
        m_RefCounter = other.m_RefCounter;
        other.m_Ptr = nullptr;
        other.m_RefCounter = nullptr;
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
      return m_RefCounter != nullptr &&
             m_RefCounter->GetStrongReferenceCount() != 0;  //|| m_Ptr != nullptr;
    }

  private:
    T* m_Ptr = nullptr;
    ReferenceCounter* m_RefCounter = nullptr;

    void AddStrong() {
      if (m_RefCounter) {
        m_RefCounter->AddStrong();
      }
    }

    void ReleaseStrong() {
      if (!m_Ptr) {
        return;
      }
      FOO_ASSERT(m_RefCounter != nullptr, "Some thing happened here");
      uint32_t count = m_RefCounter->RelRef();
      if (count == 1) {
        delete m_Ptr;
      }
      if (m_RefCounter->GetWeakReferenceCount() == 0 &&
          m_RefCounter->GetStrongReferenceCount() == 0) {
        delete m_RefCounter;
      }
      m_RefCounter = nullptr;
      m_Ptr = nullptr;
    }
    void AddWeakRef() {
      if (m_RefCounter) {
        m_Ptr->AddWeakRef();
      }
    }
    void RelWeakRef() {
      if (m_RefCounter) {
        m_Ptr->RelWeakRef();
      }
    }
    friend class WeakRef<T>;
};

template <typename T>
class WeakRef {
  public:
    WeakRef() : m_Ptr(nullptr), m_RefCounter(nullptr) {
    }

    WeakRef(const Ref<T>& strongRef) {
      if (strongRef) {
        m_RefCounter = strongRef.m_RefCounter;
        m_RefCounter->AddWeakRef();
      }
      m_Ptr = strongRef.m_Ptr;
    }

    WeakRef(const WeakRef& other) : m_Ptr(other.m_Ptr) {
      if (m_Ptr) {
        m_RefCounter = other.m_RefCounter;
        m_RefCounter->AddWeakRef();
      }
    }

    WeakRef(WeakRef&& other) noexcept : m_Ptr(other.m_Ptr), m_RefCounter(other.m_RefCounter) {
      other.m_Ptr = nullptr;
      other.m_RefCounter = nullptr;
    }

    ~WeakRef() {
      ReleaseWeak();
    }

    WeakRef& operator=(const WeakRef& other) {
      if (this != &other) {
        ReleaseWeak();
        m_Ptr = other.m_Ptr;
        m_RefCounter = other->m_RefCounter;
        if (m_RefCounter) {
          m_RefCounter->AddWeakRef();
        }
      }
      return *this;
    }

    WeakRef& operator=(WeakRef&& other) noexcept {
      if (this != &other) {
        m_Ptr = other.m_Ptr;
        m_RefCounter = other.m_RefCounter;
        other.m_Ptr = nullptr;
        other.m_RefCounter = nullptr;
      }
      return *this;
    }

    Ref<T> Lock() const {
      if (!m_RefCounter) {
        return nullptr;
      }
      if (m_RefCounter->GetStrongReferenceCount() == 0) {
        return nullptr;
      }
      return Ref<T>(m_Ptr);
    }

    bool Expired() const {
      if (!m_RefCounter) {
        return true;
      }
      return m_RefCounter->GetStrongReferenceCount() > 0;
    }

  private:
    T* m_Ptr = nullptr;
    ReferenceCounter* m_RefCounter = nullptr;
    void ReleaseWeak() {
      if (m_RefCounter) {
        m_RefCounter->RelWeakRef();
        if (m_RefCounter->GetWeakReferenceCount() == 0 &&
            m_RefCounter->GetStrongReferenceCount() == 0) {
          delete m_RefCounter;
        }
      }
      m_Ptr = nullptr;
      m_RefCounter = nullptr;
    }
};

template <class T, class... Args>
Ref<T> MakeRef(Args&&... args) {
  return Ref<T>(new T(new ReferenceCounter {}, std::forward<Args>(args)...));
}

}  // namespace fg
