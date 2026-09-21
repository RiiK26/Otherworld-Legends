#pragma once

namespace Unity
{
  template<typename T>
  struct il2cppArray: il2cppObject
  {
    using value_type     = T;
    using iterator       = T*;
    using const_iterator = const T*;

    il2cppArrayBounds* m_pBounds    = nullptr;
    uintptr_t          m_uMaxLength = 0;
    T*                 m_pValues    = nullptr;

    inline uintptr_t GetData() const { return reinterpret_cast<uintptr_t>(&m_pValues); }

    inline T* Data() { return reinterpret_cast<T*>(&m_pValues); }

    inline const T* Data() const { return reinterpret_cast<const T*>(&m_pValues); }

    inline uintptr_t Size() const { return m_uMaxLength; }

    inline bool Empty() const { return m_uMaxLength == 0; }

    inline iterator begin() { return Data(); }

    inline const_iterator begin() const { return Data(); }

    inline const_iterator cbegin() const { return Data(); }

    inline iterator end() { return Data() + Size(); }

    inline const_iterator end() const { return Data() + Size(); }

    inline const_iterator cend() const { return Data() + Size(); }

    inline T& operator[](unsigned int m_uIndex) { return Data()[m_uIndex]; }

    inline const T& operator[](unsigned int m_uIndex) const { return Data()[m_uIndex]; }

    inline T& At(unsigned int m_uIndex) { return operator[](m_uIndex); }

    inline const T& At(unsigned int m_uIndex) const { return operator[](m_uIndex); }

    inline void Insert(T* m_pArray, uintptr_t m_uSize, uintptr_t m_uIndex = 0)
    {
      if ((m_uSize + m_uIndex) >= m_uMaxLength) {
        if (m_uIndex >= m_uMaxLength)
          return;

        m_uSize = m_uMaxLength - m_uIndex;
      }

      for (uintptr_t u = 0; m_uSize > u; ++u)
        operator[](u + m_uIndex) = m_pArray[u];
    }

    inline void Fill(T m_tValue)
    {
      for (uintptr_t u = 0; m_uMaxLength > u; ++u)
        operator[](u) = m_tValue;
    }

    inline void RemoveAt(unsigned int m_uIndex)
    {
      if (m_uIndex >= m_uMaxLength)
        return;

      for (uintptr_t u = m_uIndex; (u + 1U) < m_uMaxLength; ++u)
        operator[](static_cast<unsigned int>(u)) = operator[](static_cast<unsigned int>(u + 1U));

      --m_uMaxLength;
    }

    inline void RemoveRange(unsigned int m_uIndex, unsigned int m_uCount)
    {
      if (m_uCount == 0)
        m_uCount = 1;

      unsigned int m_uTotal = m_uIndex + m_uCount;
      if (m_uTotal >= m_uMaxLength)
        return;

      for (uintptr_t u = m_uIndex; (u + m_uCount) < m_uMaxLength; ++u)
        operator[](static_cast<unsigned int>(u)) = operator[](static_cast<unsigned int>(u + m_uCount));

      m_uMaxLength -= m_uCount;
    }

    inline void RemoveAll()
    {
      if (m_uMaxLength > 0) {
        memset(GetData(), 0, sizeof(T) * m_uMaxLength);
        m_uMaxLength = 0;
      }
    }
  };

  // Defined here because its basically same shit
  template<typename T>
  struct il2cppList: il2cppObject
  {
    il2cppArray<T>* m_pListArray;

    inline il2cppArray<T>* ToArray() { return m_pListArray; }

    inline const il2cppArray<T>* ToArray() const { return m_pListArray; }

    inline typename il2cppArray<T>::iterator begin() { return m_pListArray ? m_pListArray->begin() : nullptr; }

    inline typename il2cppArray<T>::const_iterator begin() const
    {
      return m_pListArray ? m_pListArray->begin() : nullptr;
    }

    inline typename il2cppArray<T>::const_iterator cbegin() const
    {
      return m_pListArray ? m_pListArray->cbegin() : nullptr;
    }

    inline typename il2cppArray<T>::iterator end() { return m_pListArray ? m_pListArray->end() : nullptr; }

    inline typename il2cppArray<T>::const_iterator end() const { return m_pListArray ? m_pListArray->end() : nullptr; }

    inline typename il2cppArray<T>::const_iterator cend() const
    {
      return m_pListArray ? m_pListArray->cend() : nullptr;
    }

    inline uintptr_t Size() const { return m_pListArray ? m_pListArray->Size() : 0; }

    inline bool Empty() const { return !m_pListArray || m_pListArray->Empty(); }
  };

  template<typename T>
  inline typename il2cppArray<T>::iterator begin(il2cppArray<T>* m_pArray)
  {
    return m_pArray ? m_pArray->begin() : nullptr;
  }

  template<typename T>
  inline typename il2cppArray<T>::const_iterator begin(const il2cppArray<T>* m_pArray)
  {
    return m_pArray ? m_pArray->begin() : nullptr;
  }

  template<typename T>
  inline typename il2cppArray<T>::iterator end(il2cppArray<T>* m_pArray)
  {
    return m_pArray ? m_pArray->end() : nullptr;
  }

  template<typename T>
  inline typename il2cppArray<T>::const_iterator end(const il2cppArray<T>* m_pArray)
  {
    return m_pArray ? m_pArray->end() : nullptr;
  }

  template<typename T>
  inline typename il2cppArray<T>::iterator begin(il2cppList<T>* m_pList)
  {
    return m_pList ? m_pList->begin() : nullptr;
  }

  template<typename T>
  inline typename il2cppArray<T>::const_iterator begin(const il2cppList<T>* m_pList)
  {
    return m_pList ? m_pList->begin() : nullptr;
  }

  template<typename T>
  inline typename il2cppArray<T>::iterator end(il2cppList<T>* m_pList)
  {
    return m_pList ? m_pList->end() : nullptr;
  }

  template<typename T>
  inline typename il2cppArray<T>::const_iterator end(const il2cppList<T>* m_pList)
  {
    return m_pList ? m_pList->end() : nullptr;
  }
}  // namespace Unity
