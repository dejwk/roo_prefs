#pragma once

#include "roo_prefs/collection.h"

namespace roo_prefs {

/// Ref-counted RAII for managing access to Preference namespaces. Allows
/// orchestrating access to the Store.
class Transaction {
 public:
  enum class Mode { kReadWrite, kReadOnly };

  Transaction(Collection& collection, Mode mode = Mode::kReadWrite)
      : collection_(collection) {
    begin_result_ = collection_.inc(mode == Mode::kReadOnly);
  }

  [[deprecated("Use Transaction(Collection&, Transaction::Mode) instead")]]
  Transaction(Collection& collection, bool read_only)
      : Transaction(collection,
                    read_only ? Mode::kReadOnly : Mode::kReadWrite) {}

  ~Transaction() {
    if (active()) collection_.dec();
  }

  bool active() const { return begin_result_ == Store::BeginResult::kOk; }

  Store::BeginResult beginResult() const { return begin_result_; }

  Store& store() { return *collection_.store_; }

 private:
  Collection& collection_;
  Store::BeginResult begin_result_;
};

}  // namespace roo_prefs
