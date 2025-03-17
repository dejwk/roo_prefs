#pragma once

#include "roo_prefs/status.h"
#include "roo_prefs/store/preferences_store.h"
#include "roo_prefs/collection.h"
#include "roo_prefs/transaction.h"
#include "roo_prefs/pref.h"

// Basic usage:
//
// roo_prefs::Collection c("col-name");  // Statically declared.
//
// {
//   roo_prefs::Transaction t(c);
//   t.store().writeI32("pref-name", val);
// }
//
// In the snippet above, Preferences::begin()/end() will be called automatically
// by the transaction constructor/destructor.
//
// If you would like to use automated in-memory caching, use typed 'Pref'
// classes instead of creating transactions directly.
//
// Note: transactions have reference-counting (and are reentrant). In other
// words, this works, and only calls the begin()/end() pair once:
//
// void foo(const string& key, int val) {
//   roo_prefs::Transaction t(c);
//   t.writeI32(key, val);
// }
//
// // ...
// {
//   roo_prefs::Transaction t(c);
//   foo("a", 1);
//   foo("b", 2);
// }
