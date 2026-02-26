#pragma once

namespace roo_prefs {

/// Result of reading a value from storage.
enum class ReadResult { kOk, kNotFound, kWrongType, kError };

/// Result of writing a value to storage.
enum class WriteResult { kOk, kError };

/// Result of clearing a value from storage.
enum class ClearResult { kOk, kError };

}  // namespace roo_prefs