#pragma once

/// Umbrella header for the roo_prefs module.
///
/// Provides preference collections, transactions, and typed accessors.

#include "roo_prefs/collection.h"
#include "roo_prefs/pref.h"
#include "roo_prefs/status.h"
#include "roo_prefs/store/preferences_store.h"
#include "roo_prefs/transaction.h"

/// Basic usage:
///
/// @code
/// roo_prefs::Collection c("col-name");  // Statically declared.
///
/// {
///   roo_prefs::Transaction t(c);
///   t.store().writeI32("pref-name", val);
/// }
/// @endcode
///
/// In the snippet above, `Preferences::begin()` / `Preferences::end()` are
/// called automatically by transaction constructor/destructor.
///
/// If you want in-memory caching, prefer typed `Pref` classes over direct
/// transactions.
///
/// Transactions are reference-counted and reentrant. Nested usage only calls
/// `begin()` / `end()` once.
///
/// @code
/// void foo(const std::string& key, int val) {
///   roo_prefs::Transaction t(c);
///   t.store().writeI32(key.c_str(), val);
/// }
///
/// {
///   roo_prefs::Transaction t(c);
///   foo("a", 1);
///   foo("b", 2);
/// }
/// @endcode
