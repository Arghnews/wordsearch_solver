#ifndef UTILITY_LRU_CACHE_HPP
#define UTILITY_LRU_CACHE_HPP

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

#include <cassert>
#include <cstddef>
#include <functional>
#include <list>
#include <ostream>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace utility {

/** std::pair neater replacement with appropriate names */
template <class A, class B> struct ValueIteratorPair {
  A value;
  B iter;
};

/** Quick and dirty attempt at implementing an LRU cache.
 * Performance @b sucks, and it hasn't been tuned at all.
 *
 * @todo change into actual impl class + wrapper on top for copy_constructible
 * trait to work properly, from underlying key and value types
 * https://stackoverflow.com/a/27073263/8594193
 *
 * Inspired by
 * https://blog.janestreet.com/what-a-jane-street-dev-interview-is-like/
 *
 * Design choice/issue 0: Currently we store two copies of keys, as otherwise
 * we have a circular dependency problem defining what the map and the list
 * types should be. A possible alternative, if keys are non-copyable or
 * copying them is expensive, would be to store a pointer to the key in the
 * list instead.
 */
template <class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class LruCache {

public:
  using Key = KeyType;
  using Value = ValueType;

  /** Lookup a value given a @p key
   *
   * @param[in] key Key to lookup
   * @returns Pointer to Value, or `nullptr` if the value is not in the cache
   *
   * @note Would prefer to return `std::optional<T&>`, consider using boost for
   * that if so
   */
  const Value* lookup(const Key& key) {
    // fmt::print("Looking up {}\n", key);
    auto it = cache_.find(key);
    if (it == cache_.end()) {
      // fmt::print("lookup did not find {}\n", key);
      ++misses_;
      return {};
    }

    ++hits_;
    auto& cached_it = it->second.iter;
    if (cached_it != used_.begin()) {
      // fmt::print("Setting {} to front of used_ sz: {}\n", key, used_.size());
      used_.erase(cached_it);
      used_.push_front(&it->first);
      cached_it = used_.begin();
    }

    // fmt::print("Returning cached {}\n", it->second.first);
    return &it->second.value;
  }

  /** Insert a key @p k and a value @p v. May evict least recently used item
   * from cache to make space, if cache is at capacity()
   *
   * @param[in] k Key
   * @param[in] v Value
   */
  const Value& insert(const Key& k, const Value& v) {
    return this->insert_impl(k, v);
  }
  /** @overload */
  const Value& insert(Key&& k, const Value& v) {
    return this->insert_impl(std::move(k), v);
  }
  /** @overload */
  const Value& insert(const Key& k, Value&& v) {
    return this->insert_impl(k, std::move(v));
  }
  /** @overload */
  const Value& insert(Key&& k, Value&& v) {
    return this->insert_impl(std::move(k), std::move(v));
  }

  /** Remove a key @p k from the cache, if it exists */
  void erase(const Key& k) {
    // Won't work with move only types, but not a problem
    const auto it = cache_.find(k);
    if (it == cache_.end()) {
      return;
    }
    assert(it != cache_.end() && "Trying to erase from cache non-existant key");
    used_.erase(it->second.second);
    cache_.erase(it);
  }

private:
  // Yes the perfect forwarding insert_impl avoids the duplication of the above.
  // But the above gives nicer errors. Still unsure how much I like it

  template <class K, class V> const Value& insert_impl(K&& key, V&& val) {
    const auto it = cache_.find(key);
    if (it != cache_.end()) {
      assert(val == it->second.value &&
             "We assume a key always maps to the same val");
      // fmt::print("Insert doing nothing, already contains key {}\n", key);
      // Assumes a key always maps to the same val
      return it->second.value;
    }

    if (used_.size() >= capacity_) {
      assert(capacity_ > 0);
      const auto* remove_key = used_.back();
      // fmt::print("Removing key {}\n", *remove_key);
      cache_.erase(*remove_key);
      used_.pop_back();
    }

    // Double yuck. The map element needs to store an iterator to the list
    // element. The list element needs to store a pointer to the key element in
    // the map. So insert the key and value into the map. This way we can take
    // the address of the key to put onto the front of the list. Then that that
    // new list iterator to the front of the list, and set the map's iterator to
    // that.
    // using T = typename Map::mapped_type;
    // auto&& vall = std::move(val);
    // auto it2 = ListIteratorType{};
    // T t{std::move(vall), {}};
    auto [i, b] = cache_.emplace(
        std::forward<K>(key),
        typename Map::mapped_type{std::forward<V>(val), ListIteratorType{}});
    // auto [i, b] = cache_.emplace(key, val, nullptr});
    assert(b);
    static_cast<void>(b);
    used_.push_front(&i->first);
    i->second.iter = used_.begin();
    return i->second.value;
  }

  using List = std::list<const Key*>;
  using ListIteratorType = typename List::iterator;
  using Map =
      std::unordered_map<Key, ValueIteratorPair<Value, ListIteratorType>, Hash,
                         std::equal_to<void>>;

public:
  bool empty() const { return cache_.empty(); }
  std::size_t size() const { return cache_.size(); }
  std::size_t capacity() const { return capacity_; }
  void clear() {
    used_.clear();
    cache_.clear();
  }

  LruCache(LruCache&& cache) = default;

  // https://stackoverflow.com/a/5695855/8594193
  friend void swap(LruCache& c0, LruCache& c1) {
    // fmt::print("I'm swapped!\n");
    using std::swap;
    swap(c0.used_, c1.used_);
    swap(c0.cache_, c1.cache_);
    swap(c0.capacity_, c1.capacity_);
    swap(c0.print_hitrate_on_destruction_, c1.print_hitrate_on_destruction_);
    swap(c0.hits_, c1.hits_);
    swap(c0.misses_, c1.misses_);
  }

  /** Copy constructor
   *
   * @note To properly make this copy-constructible and behave well with traits,
   * will need to template the whole class on this. So for now, just define a
   * copy constructor and leave it https://stackoverflow.com/q/27073082/8594193
   */
  LruCache(const LruCache& other)
      : capacity_(other.capacity_),
        print_hitrate_on_destruction_(other.print_hitrate_on_destruction_),
        hits_(other.hits_), misses_(other.misses_) {

    static_assert(
        std::is_copy_constructible_v<Key>,
        "Copy constructor for LruCache disabled as Key is not copyable");
    static_assert(
        std::is_copy_constructible_v<Value>,
        "Copy constructor for LruCache disabled as Value is not copyable");

    // fmt::print("Copy cons\n");
    cache_.reserve(other.size());
    // Insert in reverse so that our LRU order matches the original
    for (auto it = other.used_.rbegin(); it != other.used_.rend(); ++it) {
      const auto& other_k = **it;
      // Can't use other.lookup as not const (would change lru order)
      const auto& other_v = other.cache_.at(other_k).value;
      this->insert(other_k, other_v);
    }
    assert(other.used_.size() == used_.size());
    auto other_it = other.used_.begin();
    for ([[maybe_unused]] const auto* key : used_) {
      assert(key != *other_it++ && "Pointers should not be the same");
      assert(*key == **other_it++ && "Keys should be identical after copy");
    }
    assert(used_.size() == cache_.size());
    assert(capacity() == other.capacity());
  }

  LruCache& operator=(LruCache other) {
    // fmt::print("Copy assignment op called\n");
    swap(*this, other);
    return *this;
  }

  // See https://en.cppreference.com/w/cpp/container/unordered_map
  // We can use a default constructor for std::unordered_map here as "References
  // and pointers to either key or data stored in the container are only
  // invalidated by erasing that element, even when the corresponding iterator
  // is invalidated."
  LruCache() = default;

  explicit LruCache(const std::size_t capacity) : capacity_(capacity) {
    cache_.reserve(capacity);
  }

  explicit LruCache(const std::size_t capacity,
                    const bool print_hitrate_on_destruction)
      : capacity_(capacity),
        print_hitrate_on_destruction_(print_hitrate_on_destruction) {
    cache_.reserve(capacity);
  }

  ~LruCache() {
    if (hits_ > 0 || misses_ > 0) {
      fmt::print("LruCache destructor summary: Hits {} / misses {} - {}%\n",
                 hits_, misses_,
                 static_cast<double>(hits_) / static_cast<double>(misses_) *
                     100.0);
    }
  }

private:
  template <class K, class V>
  friend std::ostream& operator<<(std::ostream& os,
                                  const LruCache<K, V>& cache);

  List used_ = {};
  Map cache_ = {};
  std::size_t capacity_ = 64;

  bool print_hitrate_on_destruction_ = false;
  std::size_t hits_ = 0;
  std::size_t misses_ = 0;
};

template <class K, class V>
std::ostream& operator<<(std::ostream& os, const LruCache<K, V>& cache) {
  fmt::memory_buffer b{};
  for (const auto& [k, v] : cache.cache_) {
    fmt::format_to(b, "{}: {}, ", k, v.value);
  }
  if (b.size() > 2) {
    b.resize(b.size() - 2);
  }
  return os << fmt::to_string(b);
}

} // namespace utility

#endif // UTILITY_LRU_CACHE_HPP
