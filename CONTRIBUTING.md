# Contributing to DustCrate

Thank you for contributing! This document covers the branching model, CI
requirements, and deployment/rollback procedures.

---

## Branching Model (GitFlow)

| Branch | Purpose |
|--------|---------|
| `main` | Stable, tagged releases only.  Direct commits are **blocked**. |
| `develop` | Integration branch.  All features merge here first. |
| `feature/<name>` | New features or improvements. Branch from `develop`; PR back to `develop`. |
| `hotfix/<name>` | Urgent production fixes. Branch from `main`; PR to **both** `main` and `develop`. |
| `release/<version>` | Release prep (version bump, changelog).  Branch from `develop`; merge to `main` + `develop`. |

### Branch Protection Rules

Both `main` and `develop` require:
- At least **one approving review** before merge.
- All CI checks (lint, build, tests) must pass.
- No force-pushes.

---

## Development Setup

The fastest way to get a consistent build environment is via the provided
Dev Container:

```bash
# Open in VS Code with the Dev Containers extension
code .
# "Reopen in Container" when prompted
```

Or set up manually:

```bash
git submodule update --init --recursive   # fetch JUCE
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target DustCrate_Standalone
```

See `.devcontainer/setup.sh` for the full list of system dependencies.

---

## CI Pipeline

Every push and pull request runs three GitHub Actions jobs:

1. **lint** — `clang-format` checks all `Source/` and `Tests/` files against
   `.clang-format`.  Fix failures locally with:
   ```bash
   find Source Tests -name '*.cpp' -o -name '*.h' | \
     xargs clang-format -i --style=file
   ```

2. **build** — Matrix build on `macos-14` (VST3 + AU + Standalone) and
   `windows-latest` (VST3 + Standalone) using CMake + Ninja.  The test
   binary `DustCrateTests` is also built and executed.

3. **security** — CodeQL static-analysis scan for C++ vulnerabilities.

A PR may only be merged once all three jobs are green.

---

## Running Tests Locally

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target run_tests
```

Aim to keep non-UI logic test coverage at **≥ 70 %**.

---

## Code Style

- 4-space indentation, no tabs.
- Line length ≤ 100 characters.
- Enforce with `clang-format --style=file` (config in `.clang-format`).
- Document every public method in headers using Doxygen `/** */` comments.
  Regenerate docs locally with `doxygen Doxyfile`.

---

## Releases

Releases are created by pushing a version tag:

```bash
git tag -a v0.2.0 -m "Release v0.2.0"
git push origin v0.2.0
```

GitHub Actions will build signed installers for macOS and Windows and attach
them as assets to the GitHub Release automatically.

---

## Rollback Procedure

If a release is found to be broken in the wild, follow these steps:

### One-Button Rollback

1. **Identify the last stable tag** (e.g. `v0.1.0`) in
   [GitHub Releases](https://github.com/ncsound919/dustcrate/releases).

2. **Re-publish the previous release as Latest**:
   - Open the previous release on GitHub.
   - Click **Edit release**.
   - Check **Set as the latest release**.
   - Click **Update release**.

3. **Update the download link** on any distribution page to point to the
   previous release's installer assets.

4. **Draft a hotfix** branched from `main` at the previous tag:
   ```bash
   git checkout -b hotfix/fix-description v0.1.0
   # ... fix ...
   git push origin hotfix/fix-description
   ```
   Open a PR targeting `main` (and `develop`) with the fix.

5. **Communicate** to users via release notes that v0.x.x is the current
   stable version and that a hotfix is in progress.

> The previous installer remains available as a GitHub Release asset at all
> times — users can always downgrade by re-running it.

---

## Security

- Never commit secrets, credentials, or API keys.
- All HTTPS in transit; no plaintext connections.
- Report vulnerabilities privately via GitHub's
  [Security Advisories](https://github.com/ncsound919/dustcrate/security/advisories).
