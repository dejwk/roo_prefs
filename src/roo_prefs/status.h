#pragma once

namespace roo_prefs {

/// Result of reading a value from storage.
enum ReadResult { READ_OK, READ_NOT_FOUND, READ_WRONG_TYPE, READ_ERROR };

/// Result of writing a value to storage.
enum WriteResult { WRITE_OK, WRITE_ERROR };

/// Result of clearing a value from storage.
enum ClearResult { CLEAR_OK, CLEAR_ERROR };

}  // namespace roo_prefs