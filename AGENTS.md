Document Type: Reference
Status: Reference

# AGENTS.md

<small>Last updated: 2026-05-08T23:23:44+02:00</small>

## Agent Working Rules (AGENTS) - SpoolSense

### 0. Project Overview

- Project name: SpoolSense
- Primary technology / language: C++ / ESP-IDF
- Target environment: ESP32 / M5Stack Core ESP32 / ESP-IDF
- Repository / codebase structure: Embedded firmware based on ESP-IDF + Markdown documentation
- Last rules update: 2026-05-08

This document defines the collaboration rules for the SpoolSense project.

Technical project details are stored in:

```text
README.md
```

---

## 0.1 Development Environment

The project is currently developed in:

- `C++`
- `ESP-IDF`
- `Arduino` compatibility layers where required by specific components

Current project configuration is confirmed by:

- `CMakeLists.txt`
- `main/CMakeLists.txt`
- component manifests and build files under `components/*/CMakeLists.txt`
- base configuration:
  - `sdkconfig.defaults`
- firmware entry point:
  - `main/src/main.cpp`
- project documentation:
  - `README.md`

The project is currently developed on:

- `ESP-IDF`

The code architecture should remain aligned with the native ESP-IDF environment while keeping Arduino-specific dependencies isolated.

Requirements:

- minimize dependence on Arduino-specific constructs
- separate application logic from hardware abstraction
- limit global state
- design modules to remain compatible with the future ESP-IDF architecture

Code should comply with:

- embedded C++ best practices
- modular architecture
- limitations of the ESP32 / ESP-IDF environment
- local ESP-IDF project requirements

---

## 1. Definitions

- **Working environment** - the project directory on this machine.
- **Commit message** - a change description according to section 4.
- **"— fragment"** - denotes a presented excerpt of a function / class, with at least 3-5 lines of context.
- **ISO 8601** - for example `2026-05-07T18:45:12+02:00`.

---

## 2. General Work Rules

1. Communication is conducted in Polish and in Markdown format.
2. On chat, show only the code fragments or summaries that are actually needed.
3. Mark large functions and large classes as "— fragment".
4. Keep alignment with:
   - the existing project architecture
   - the current code style
   - modularity of the project
   - modern embedded C++ practices
5. Make changes iteratively.
6. Do not push changes without explicit user approval.
7. Before adding new helpers, check:
   - `helpers.md`
   - existing utility modules
8. When modifying documentation (`*.md`), update:

   ```html
   <small>Last updated: YYYY-MM-DDThh:mm:ss±hh:mm</small>
   ```

9. All dates must be given in the project time zone, i.e. Central European time (`Europe/Warsaw`, `CET/CEST`), unless a document requires a different zone.
10. `README.md` contains the technical details of the project. Do not duplicate them in other documents unless the file is a translation or a short summary.
11. Code and documentation must be maintained according to software engineering best practices.
12. A modular architecture that is easy to extend is preferred.
13. Firmware code must be written according to best practices for:
    - embedded C++
    - ESP-IDF
    - ESP32
14. Design the code to stay compatible with native ESP-IDF architecture and isolate Arduino-dependent code.

---

## 3. Role Split

### ChatGPT

ChatGPT is responsible for:

- project architecture analysis
- technical consulting
- implementation quality analysis
- analysis of CODEX output
- proposing development directions
- project and documentation support

ChatGPT:

- does not perform actual Git commits
- does not replace the implementation process carried out by CODEX
- acts as an advisory and analytical role

---

### CODEX

CODEX is responsible for:

- code implementation
- system behavior testing
- firmware development
- refactoring
- keeping code consistent
- performing commits
- preparing commit descriptions
- updating technical documentation

CODEX:

- implements changes according to project requirements
- is responsible for technical correctness of the implementation
- performs functional tests
- follows the project Git workflow

---

## 4. Change and Commit Management

### Commit Standard

The project requires the use of:

- Conventional Commits

Format:

```text
type(scope): short description
```

Examples:

```text
feat(rfid): add PN532 tag parsing
fix(hx711): prevent unstable weight readings
refactor(core): split hardware abstraction layer
docs(readme): update wiring section
test(rfid): add hardware initialization tests
chore(ci): update ESP-IDF configuration
```

---

### Allowed Commit Types

| Type     | Meaning                                  |
|----------|------------------------------------------|
| feat     | new feature                              |
| fix      | bug fix                                  |
| refactor | refactoring without behavior change      |
| docs     | documentation changes                    |
| test     | tests                                    |
| chore    | technical / maintenance changes          |
| build    | build system changes                     |
| ci       | CI/CD configuration                      |
| perf     | performance optimizations                |

---

### Commit Rules

1. A commit must be:
   - logical
   - consistent
   - small and readable
   - easy to review

2. One commit should represent:
   - one logical change

3. Do not mix in one commit:
   - refactoring
   - new features
   - documentation changes
   - formatting changes

4. WIP commits:
   - allowed locally
   - should not reach `main`

5. Before finalization:
   - squash WIP commits
   - prepare the final logical commit

6. Commit messages:
   - write them in English
   - use present tense
   - no period at the end
   - be as specific as possible

7. Do not generate automatic changelogs.

8. Prepare a detailed commit description only when the user explicitly asks for it.

9. Skip:
   - `.reports/junit.xml`
   - `.reports/tests.html`

10. Documentation commits (`*.md`) may use a simplified description if the scope is clearly documentation-only.

---

## 5. Git Workflow

The project uses a task-branch workflow.

### Rules

1. One branch = one task.
2. Do not work directly on:

   ```text
   main
   ```

3. Local WIP commits are allowed.
4. Before finalization:
   - squash WIP commits
   - prepare a final logical commit
5. Commit messages must be:
   - readable
   - technical
   - unambiguous

---

## 6. README.md Updates

Update `README.md` only when there are:

- major functional changes
- hardware changes
- configuration changes
- new startup / run procedures

Do not update README for:

- minor presentation changes
- cosmetic refactors
- local tests

---

## 7. Code Testing

1. New features must have tests, if testability is technically possible.
2. After every change, run the appropriate test suite.
3. Do not propose further changes if tests are failing.
4. Long-running tests require user approval.
5. Prefer real hardware, and when it is not available use lightweight test doubles: stubs, fakes, or interface simulators. Use full hardware mocks only when they are needed to verify higher-level logic.
6. Hardware tests may be skipped when hardware is unavailable.
7. Store test artifacts in:

   ```text
   .run/TESTS
   ```

8. Firmware tests should account for:

   - embedded environment constraints
   - real ESP32 / M5Stack hardware
   - I2C / SPI / UART communication
   - device initialization stability

9. Do not replace hardware tests with full hardware mocks if the test can be executed on real hardware or with a lighter substitute test.

---

## 8. Directory Organization

### py_tools/

Shared tools between projects.

Modifications:

- only with user approval

---

### tools/

Local project tools.

May be:

- moved to `py_tools/`
- standardized after stabilization

---

### workspace/

Experimental / sandbox directory.

Code should be moved to the target project structure after stabilization.

---

## 9. Code Style and Linting

Code must pass:

- `clang-format`
- `cppcheck`
- local project rules

### Requirements

1. ASCII-only in source code.
2. Include directives only at the top of the file.
3. Follow C++ naming conventions:

   ```text
   files, functions, local variables: lower_snake_case
   classes and structs: PascalCase
   enum class types: PascalCase
   constants: kCamelCase or UPPER_SNAKE_CASE, according to the local module style
   ```

4. Keep compatibility with:
   - modern C++
   - project modularity
5. Use one blank line at the end of the file.
6. No trailing whitespace.
7. Prefer `[[maybe_unused]]` for unused variables and parameters. If that is not enough, use an explicit cast to `void`.

---

### Embedded / Arduino / ESP-IDF Ready

1. Prefer:
   - classes and modules instead of large procedural files
   - separation of HAL and business logic
   - minimizing dynamic memory allocation
   - avoiding blocking `delay()` calls outside test code

2. Avoid:
   - global mutable state without justification
   - excessive use of `String`
   - tight coupling between modules

3. Preferred tools and patterns:
   - `constexpr`
   - `enum class`
   - `std::array`
   - RAII where it makes sense

4. Hardware logic should be isolated from application logic.

5. Code should be as easy as possible to migrate to:
   - ESP-IDF
   - FreeRTOS
   - ESP32 component architecture

6. Avoid excessive dependence on:
   - Arduino framework-specific classes
   - global singletons
   - logic placed directly in `loop()`

---

## 10. C++ Module Header

Each manually maintained `.cpp` and `.hpp` module should start with a comment header.

### Required Structure

```cpp
/**
 * Short summary sentence.
 *
 * Features (EN):
 * - ...
 *
 * Funkcje (PL):
 * - ...
 *
 * File: relative/path.cpp
 */
```

Exceptions:

- automatically generated files
- very small test files, if the header hurts readability
- technical shims and adapters, if they are only a temporary experimental element

---

## 11. CODEX Workflow

1. For simple changes:
   - no lengthy preambles
2. For larger changes:
   - use a plan
3. Rule conflicts:
   - report them immediately
4. Destructive operations:
   - require user approval
5. Tests:
   - mandatory after functional changes

---

## 12. Rule Precedence

1. The document closest to the modified file applies.
2. Local rules have higher priority.
3. Rule conflicts must be reported to the user.

---

## 13. Project Documentation

Project documentation is divided into:

| Document    | Purpose                      |
|-------------|------------------------------|
| README.md   | technical details            |
| AGENTS.md   | collaboration and workflow   |

---

## 13.1 Documentation Language

This file is the reference version.

The working Polish version is:

- `AGENTS_PL.md`

Naming convention:

| Type                    | Example        |
|-------------------------|----------------|
| reference version (EN)  | `AGENTS.md`    |
| working version (PL)    | `AGENTS_PL.md` |

Rules:

1. The English version is the reference source.
2. The Polish version remains the working draft and the basis for refinement.
3. Technical documentation for API, architecture, and firmware should ultimately be written in English.
4. User documentation may have multilingual versions.

---

## 14. Summary

SpoolSense is developed according to:

- modular architecture
- software engineering best practices
- an orderly Git workflow
- clear documentation rules

The split of responsibilities between ChatGPT and CODEX is intended to provide:

- high implementation quality
- better architecture analysis
- easier maintenance
- more predictable development

The project should be treated as embedded firmware currently developed with ESP-IDF on the ESP32 platform, with Arduino compatibility layers kept isolated where needed.
