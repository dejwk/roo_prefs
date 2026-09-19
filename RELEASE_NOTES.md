# roo_prefs 1.3.2

- Updated Roo dependencies to `roo_backport` 1.2.4, `roo_logging` 1.5.10, and `roo_scheduler` 2.2.0 in Bazel and PlatformIO.
- Updated Bazel tooling to `rules_cc` 0.2.25, GoogleTest 1.18.0.bcr.1, and `roo_testing` 2.1.2.
- Updated the shared GitHub Actions CI workflow to `roo_testing` 2.1.2.
- Added consolidated release notes for previous releases.

---

# [roo_prefs 1.3.1](https://github.com/dejwk/roo_prefs/releases/tag/1.3.1)

Published 2026-08-29.

This maintenance release updates the host-development and CI tooling around `roo_prefs`.

### Highlights

- Added a runnable host-emulation target for the Basic preferences example:

  ```sh
  bazel run //examples/Basic
  ```

- Migrated host builds and tests to `roo_testing` 2.x Arduino ESP32 profiles.
- Updated GitHub Actions CI to use the shared Roo testing workflow, including pull-request and manual-run support.
- Centralized AddressSanitizer configuration.
- Updated Roo dependency versions:
  - `roo_backport` 1.2.3
  - `roo_logging` 1.5.8
  - `roo_scheduler` 2.1.10

### Notes

For host testing, use Bazelisk 1.21 or newer. The default profile is the Arduino ESP32 emulator:

```sh
bazel test ...
bazel test ... --config=asan
```

---

# [roo_prefs 1.3.0](https://github.com/dejwk/roo_prefs/releases/tag/1.3.0)

Published 2026-06-04.

* Improved support for strings. Writers accept std::string, Arduino ::String, and roo::string_view.
* Added explicit ArduinoString that internally uses Arduino ::String, rather than std::string, to represent the cached value.
* Better documentation: added a programming guide and more examples.
* Added support for custom serialization/deserialization.

**Full Changelog**: https://github.com/dejwk/roo_prefs/compare/1.2.11...1.3.0

---

# [roo_prefs 1.2.11](https://github.com/dejwk/roo_prefs/releases/tag/1.2.11)

Published 2026-03-18.

Fixing compilation errors on ESP8266.

**Full Changelog**: https://github.com/dejwk/roo_prefs/compare/1.2.10...1.2.11

---

# [roo_prefs 1.2.10](https://github.com/dejwk/roo_prefs/releases/tag/1.2.10)

Published 2026-02-26.

Added logging support for enum classes, so that they can be used in CHECK macros.

---

# [roo_prefs 1.2.9](https://github.com/dejwk/roo_prefs/releases/tag/1.2.9)

Published 2026-02-26.

Doxygen docs, and some (backwards-compatible and minor) API changes.

**Full Changelog**: https://github.com/dejwk/roo_prefs/compare/1.2.8...1.2.9

---

# [roo_prefs 1.2.8](https://github.com/dejwk/roo_prefs/releases/tag/1.2.8)

Published 2026-01-26.

Fixed tests after Bazel behavior changed.

**Full Changelog**: https://github.com/dejwk/roo_prefs/compare/1.2.7...1.2.8

---

# [roo_prefs 1.2.7](https://github.com/dejwk/roo_prefs/releases/tag/1.2.7)

Published 2026-01-06.

Updated the roo_logging dependency.

---

# [roo_prefs 1.2.6](https://github.com/dejwk/roo_prefs/releases/tag/1.2.6)

Published 2026-01-06.

Updated dependencies.

**Full Changelog**: https://github.com/dejwk/roo_prefs/compare/1.2.3...1.2.6

---

# [roo_prefs 1.2.3](https://github.com/dejwk/roo_prefs/releases/tag/1.2.3)

Published 2025-11-12.

Update dependencies only.

**Full Changelog**: https://github.com/dejwk/roo_prefs/compare/1.2.2...1.2.3

---

# [roo_prefs 1.2.2](https://github.com/dejwk/roo_prefs/releases/tag/1.2.2)

Published 2025-10-30.

Updated CI; added .gitignore; refreshed dependencies.

**Full Changelog**: https://github.com/dejwk/roo_prefs/compare/1.2.1...1.2.2

---

# [roo_prefs 1.2.1](https://github.com/dejwk/roo_prefs/releases/tag/1.2.1)

Published 2025-10-19.

Fixing build on ESP32C3.

---

# [roo_prefs 1.2.0](https://github.com/dejwk/roo_prefs/releases/tag/1.2.0)

Published 2025-10-18.

Added unit tests, an example, and a README file.

---

# [roo_prefs 1.1.0](https://github.com/dejwk/roo_prefs/releases/tag/1.1.0)

Published 2025-07-04.

* Added lazy perference writers, that update preferences after some period of inactivity (a few seconds), to avoid bursts of writes (e.g. when the user drags a slider).
* A few bugfixes.
* Better encapsulation of storage.
* Slightly improved log output.

---

# [roo_prefs 1.0.3](https://github.com/dejwk/roo_prefs/releases/tag/1.0.3)

Published 2024-12-29.

Fix: use a default value if failed to read initial state.

---

# [roo_prefs 1.0.2](https://github.com/dejwk/roo_prefs/releases/tag/1.0.2)

Published 2024-08-06.

Version numbering fix

---

