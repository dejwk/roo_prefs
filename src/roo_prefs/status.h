#pragma once

namespace roo_prefs {

/// Result of reading a value from storage.
enum class ReadResult { kOk, kNotFound, kWrongType, kError };

/// Result of writing a value to storage.
enum class WriteResult { kOk, kError };

/// Result of clearing a value from storage.
enum class ClearResult { kOk, kError };

/// @deprecated Use `ReadResult::kOk` instead.
constexpr ReadResult READ_OK = ReadResult::kOk;
/// @deprecated Use `ReadResult::kNotFound` instead.
constexpr ReadResult READ_NOT_FOUND = ReadResult::kNotFound;
/// @deprecated Use `ReadResult::kWrongType` instead.
constexpr ReadResult READ_WRONG_TYPE = ReadResult::kWrongType;
/// @deprecated Use `ReadResult::kError` instead.
constexpr ReadResult READ_ERROR = ReadResult::kError;

/// @deprecated Use `WriteResult::kOk` instead.
constexpr WriteResult WRITE_OK = WriteResult::kOk;
/// @deprecated Use `WriteResult::kError` instead.
constexpr WriteResult WRITE_ERROR = WriteResult::kError;

/// @deprecated Use `ClearResult::kOk` instead.
constexpr ClearResult CLEAR_OK = ClearResult::kOk;
/// @deprecated Use `ClearResult::kError` instead.
constexpr ClearResult CLEAR_ERROR = ClearResult::kError;

}  // namespace roo_prefs
