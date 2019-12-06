#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <cstddef>
#include <iostream>
#include <new>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

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
    virtual void do_contains_and_further(
        const std::string& stem,
        const std::string& suffixes,
        wordsearch_solver::Result& result_out) const = 0;
    virtual bool do_contains(const std::string& key) const = 0;
    virtual bool do_further(const std::string& key) const = 0;
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

    void do_contains_and_further(
        const std::string& stem,
        const std::string& suffixes,
        wordsearch_solver::Result& result_out) const override
    {
      return m_data.contains_and_further(stem, suffixes, result_out);
    }

    bool do_contains(const std::string& key) const override
    {
      return m_data.contains(key);
    }

    bool do_further(const std::string& key) const override
    {
      return m_data.further(key);
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

  bool further(const std::string& key) const
  {
    return m_impl->do_further(key);
  }

  void contains_and_further(
      const std::string& stem,
      const std::string& suffixes,
      wordsearch_solver::Result& result_out) const
  {
    return m_impl->do_contains_and_further(stem, suffixes, result_out);
  }

private:
  std::unique_ptr<concept_t> m_impl;
};

} // wordsearch_solver

#endif // DICTIONARY_H
