#pragma once

#include <memory>
#include <type_traits>

#include "roo_logging.h"
#include "roo_prefs/store/preferences_store.h"
#include "roo_prefs/store/store.h"

namespace roo_prefs {

class Transaction;

/// Collection corresponds to a preferences namespace. Use it to group related
/// preferences.
class Collection {
 public:
  Collection(const char* name)
      : default_store_(),
        store_(&default_store_),
        name_(name),
        refcount_(0),
        read_only_(true) {}

  /// Creates a collection backed by the specified store. The store must
  /// outlive this collection and must not be attached to another collection
  /// concurrently.
  Collection(const char* name, Store& store)
      : default_store_(),
        store_(&store),
        name_(name),
        refcount_(0),
        read_only_(true) {}

  bool inTransaction() const { return refcount_ > 0; }

  /// Calls `visitor` once for every persisted key in this collection.
  ///
  /// The order is unspecified. The key view is valid only for the duration of
  /// the call to `visitor`. Return false from the visitor to stop early. Do not
  /// modify this collection while it is being enumerated.
  template <typename Visitor>
  EnumerateResult forEachKey(Visitor&& visitor) const {
    using VisitorType = typename std::remove_reference<Visitor>::type;
    return store_->enumerateKeys(
        name_,
        [](void* context, roo::string_view key) {
          return static_cast<bool>((*static_cast<VisitorType*>(context))(key));
        },
        const_cast<void*>(static_cast<const void*>(std::addressof(visitor))));
  }

 private:
  friend class Transaction;

  Store::BeginResult inc(bool read_only) {
    if (refcount_ == 0) {
      Store::BeginResult result = store_->begin(name_, read_only);
      if (result != Store::BeginResult::kOk) {
        if (result == Store::BeginResult::kNotFound) return result;
        if (read_only) {
          LOG(WARNING) << "Failed to initialize preferences " << name_
                       << " for reading";
        } else {
          LOG(ERROR) << "Failed to initialize preferences " << name_
                     << " for writing";
        }
        return result;
      }
      read_only_ = read_only;
      ++refcount_;
      return Store::BeginResult::kOk;
    }
    if (read_only_ && !read_only) return Store::BeginResult::kError;
    ++refcount_;
    return Store::BeginResult::kOk;
  }

  void dec() {
    if (--refcount_ == 0) {
      store_->end();
    }
  }

  PreferencesStore default_store_;
  Store* store_;
  const char* name_;
  int refcount_;
  bool read_only_;
};

}  // namespace roo_prefs
