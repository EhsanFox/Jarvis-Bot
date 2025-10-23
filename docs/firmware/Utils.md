Utils (lib/Utils)
===================

Purpose
-------
Small helper utilities used across the firmware. Currently the module exposes `splitArgs`, a small tokenizer used by CLI command handlers.

Function: splitArgs
-------------------
- Signature: `std::vector<String> splitArgs(const String &input)`
- Behavior: splits the input string on spaces, collapses multiple spaces and returns a vector of non-empty tokens. It does not support quotes or escaped spaces.

Implementation note
-------------------
The simple implementation iterates characters and accumulates a token until a space is encountered. It intentionally avoids dynamic allocations in the hot path beyond the token vector.

Example
-------
```cpp
auto tokens = splitArgs("wifi connect MySSID MyPass");
// tokens = {"wifi", "connect", "MySSID", "MyPass"}
```

Limitations and recommended improvements
---------------------------------------
- Quoted arguments: if you need to support spaces in arguments (for example passwords with spaces or file paths), implement a tokenizer that handles `"` and `'` quoting and escape sequences.
- Case preservation: current commands convert tokens to lowercase for matching; the helper preserves case — be mindful where conversion happens.

Testing
-------
- Unit tests should cover normal tokenization, leading/trailing spaces, multiple spaces and empty input.
