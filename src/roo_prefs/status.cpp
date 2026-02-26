#include "roo_prefs/status.h"

namespace roo_prefs {

const char* ReadResultToString(ReadResult result) {
  switch (result) {
    case ReadResult::kOk:
      return "ReadResult::kOk";
    case ReadResult::kNotFound:
      return "ReadResult::kNotFound";
    case ReadResult::kWrongType:
      return "ReadResult::kWrongType";
    case ReadResult::kError:
      return "ReadResult::kError";
  }
  return "ReadResult::kUnknown";
}

const char* WriteResultToString(WriteResult result) {
  switch (result) {
    case WriteResult::kOk:
      return "WriteResult::kOk";
    case WriteResult::kError:
      return "WriteResult::kError";
  }
  return "WriteResult::kUnknown";
}

const char* ClearResultToString(ClearResult result) {
  switch (result) {
    case ClearResult::kOk:
      return "ClearResult::kOk";
    case ClearResult::kError:
      return "ClearResult::kError";
  }
  return "ClearResult::kUnknown";
}

roo_logging::Stream& operator<<(roo_logging::Stream& os, ReadResult result) {
  return os << ReadResultToString(result);
}

roo_logging::Stream& operator<<(roo_logging::Stream& os, WriteResult result) {
  return os << WriteResultToString(result);
}

roo_logging::Stream& operator<<(roo_logging::Stream& os, ClearResult result) {
  return os << ClearResultToString(result);
}

}  // namespace roo_prefs
