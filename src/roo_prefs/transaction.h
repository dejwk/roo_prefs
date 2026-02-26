#pragma once

#include "roo_prefs/collection.h"

namespace roo_prefs {

/// Ref-counted RAII for managing access to Preference namespaces. Allows
/// orchestrating access to the Store.
class Transaction {
 public:
  Transaction(Collection& collection, bool read_only = false)
      : collection_(collection) {
    active_ = collection_.inc(read_only);
  }

  ~Transaction() {
    if (active_) collection_.dec();
  }

  bool active() const { return active_; }

  Store& store() { return collection_.store_; }

 private:
  Collection& collection_;
  bool active_;
};

}  // namespace roo_prefs
