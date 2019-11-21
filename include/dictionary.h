#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <filesystem>
#include <iostream>
#include <string>
#include <set>

#include "wordsearch_solver_defs.h"

namespace wordsearch_solver
{

// https://mropert.github.io/2017/11/30/polymorphic_ducks/
class Dictionary
{
  public:
  struct concept_t
  {
    virtual ~concept_t()
    {
      std::cout << __PRETTY_FUNCTION__ << "\n";
    }
    virtual bool do_contains(const std::string& key) const = 0;
    virtual bool do_contains_prefix(const std::string& prefix) const = 0;
  };
  template <typename T>
  struct model_t : public concept_t
  {
    model_t() = default;
    model_t(const T& v) : m_data(v) {}
    model_t(T&& v) : m_data(std::move(v)) {}
    ~model_t()
    {
      std::cout << __PRETTY_FUNCTION__ << "\n";
    }

    bool do_contains(const std::string& key) const override
    {
      return m_data.contains(key);
    }

    bool do_contains_prefix(const std::string& prefix) const override
    {
      return m_data.contains_prefix(prefix);
    }

    T m_data;
  };
private:
  Dictionary() = default;

public:
  Dictionary(const Dictionary&) = delete;
  Dictionary(Dictionary&&) = default;

  template <typename T>
  Dictionary(T&& impl)
    : m_impl(new model_t<std::decay_t<T>>(std::forward<T>(impl)))
  {
    std::cout << "Called ->>> " << __PRETTY_FUNCTION__ << "\n";
  }

  Dictionary& operator=(const Dictionary&) = delete;
  Dictionary& operator=(Dictionary&&) = default;

  // -Weffc++ whines at return "Dictionary&" but not this. Possible look into
  template <typename T>
  decltype(auto) operator=(T&& impl)
  {
    std::cout << "Called " << __PRETTY_FUNCTION__ << "\n";
    m_impl.reset(new model_t<std::decay_t<T>>(std::forward<T>(impl)));
    return *this;
  }

  bool contains(const std::string& key) const
  {
    return m_impl->do_contains(key);
  }

  bool contains_prefix(const std::string& prefix) const
  {
    return m_impl->do_contains_prefix(prefix);
  }

  // TODO: remove, this is for debug
  auto underlying() const
  {
    return m_impl.get();
  }

private:
  std::unique_ptr<concept_t> m_impl;
};

} // wordsearch_solver

#endif // DICTIONARY_H
