# Command-Line Parsing Tests

This directory contains tests for the command-line argument parsing functionality in GnotePad.

## Test Coverage

- `testQuitAfterInitParsing()` - Verifies `--quit-after-init` flag is parsed correctly
- `testHeadlessSmokeParsing()` - Verifies `--headless-smoke` alias is parsed correctly  
- `testNoFlagsParsing()` - Verifies normal operation without flags
- `testQuitAfterInitBehavior()` - Integration test verifying both option aliases work

## Running Tests

```bash
ctest --test-dir build -R cmdline
```
