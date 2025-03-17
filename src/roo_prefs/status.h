#pragma once

namespace roo_prefs {

enum ReadResult { READ_OK, READ_NOT_FOUND, READ_WRONG_TYPE, READ_ERROR };
enum WriteResult { WRITE_OK, WRITE_ERROR };
enum ClearResult { CLEAR_OK, CLEAR_ERROR };

}