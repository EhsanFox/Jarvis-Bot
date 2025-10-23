TerminalManager (lib/TerminalManager)
======================================

Purpose
-------
`TerminalManager` manages a list of `Command` objects and reads serial input to dispatch commands. It's the local interactive CLI for debugging and control.

Public API
----------
- `void addCommand(Command* cmd)` — register a `Command` instance.
- `void handleInput()` — called from the main loop to read and process serial input.

Behavior
--------
- `handleInput()` reads bytes from `Serial` until newline. Each line is parsed into a command and arguments. If a matching command is found, dependencies are attached using the `DependencyContainer` and the handler is run.

Parsing details
---------------
- Input is accumulated in a `String _buffer` until a `\n` is received. Carriage returns (`\r`) are ignored.
- `processLine()` tokenizes using `splitArgs()` which currently splits on spaces and does not support quoted tokens. Both command names and args are converted to lowercase in the current implementation — note the consequence for case-sensitive data like passwords.

Dependency injection
---------------------
- `TerminalManager` contains its own `DependencyContainer _deps`. Call `terminal.addDependency("wifi", wifiManager)` or similar in `main.cpp` to make services available to commands.
- Before a command handler is run, `processLine()` calls `cmd->attachDependencies(&_deps)` so handlers can call `cmd->use<T>("key")`.

Examples
--------
Register a command and a dependency:

```cpp
terminal.addDependency("config", &config);
terminal.addCommand(infoCommand);
```

Limitations and improvements
---------------------------
- Quoting and case: implement quoted-argument parsing and preserve argument case for secrets.
- Feedback: `processLine()` prints `Unknown command` for unregistered commands. Consider providing suggestions or a `help` command listing registered commands.
- Blocking handlers: If a command performs lengthy work, spawn a task to avoid blocking the main loop.

Testing
-------
- Unit test `processLine()` by supplying mock input and verifying that the correct command's handler is called and receives expected args. Consider extracting parsing into a pure function for easier tests.

`TerminalManager` manages a list of `Command` objects and reads serial input to dispatch commands. It's the local interactive CLI for debugging and control.

API
---
- `void addCommand(Command* cmd)` — register a `Command` instance.
- `void handleInput()` — called from the main loop to read and process serial input.

Behavior
--------
- `handleInput()` reads bytes from `Serial` until newline. Each line is parsed into a command and arguments. If a matching command is found, dependencies are attached using the `DependencyContainer` and the handler is run.

Usage
-----
Register commands in `main.cpp` (example):

terminal.addCommand(infoCommand);
terminal.addDependency("wifi", wifiManager);

Then call `terminal.handleInput()` each loop iteration.

Notes
-----
- Commands run synchronously and may print to `Serial`. Keep long tasks off the main thread.
