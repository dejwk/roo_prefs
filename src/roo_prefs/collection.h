#pragma once

#include <memory>
#include <type_traits>

#include "roo_logging.h"
#include "roo_prefs/store/preferences_store.h"

namespace roo_prefs {

class Transaction;

using Store = PreferencesStore;

/// Collection corresponds to a preferences namespace. Use it to group related
/// preferences.
class Collection {
 public:
  Collection(const char* name)
      : store_(), name_(name), refcount_(0), read_only_(true) {}

  bool inTransaction() const { return refcount_ > 0; }

  /// Calls `visitor` once for every persisted key in this collection.
  ///
  /// The order is unspecified. The key view is valid only for the duration of
  /// the call to `visitor`. Return false from the visitor to stop early. Do not
  /// modify this collection while it is being enumerated.
  template <typename Visitor>
  EnumerateResult forEachKey(Visitor&& visitor) const {
    using VisitorType = typename std::remove_reference<Visitor>::type;
    return store_.enumerateKeys(
        name_,
        [](void* context, roo::string_view key) {
          return static_cast<bool>((*static_cast<VisitorType*>(context))(key));
        },
        const_cast<void*>(static_cast<const void*>(std::addressof(visitor))));
  }

 private:
  friend class Transaction;

  bool inc(bool read_only) {
    if (refcount_ == 0) {
      if (!store_.begin(name_, read_only)) {
        if (read_only) {
          LOG(WARNING) << "Failed to initialize preferences " << name_
                       << " for reading";
        } else {
          LOG(ERROR) << "Failed to initialize preferences " << name_
                     << " for writing";
        }
        return false;
      }
      read_only_ = read_only;
      ++refcount_;
      return true;
    }
    if (read_only_ && !read_only) return false;
    ++refcount_;
    return true;
  }

  void dec() {
    if (--refcount_ == 0) {
      store_.end();
    }
  }

  Store store_;
  const char* name_;
  int refcount_;
  bool read_only_;
};

}  // namespace roo_prefs
