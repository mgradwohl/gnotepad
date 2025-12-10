# Performance Tests

This directory contains comprehensive performance tests for GnotePad.

## Quick Start

```bash
# Build tests
cmake --build build/debug

# Run all performance tests
ctest --test-dir build/debug -L Performance

# Run specific test
./build/debug/GnotePadPerformance testLoadLargeFile100KB
```

## Test Categories

1. **Large File Handling** - Load/save performance for files 100KB to 1MB
2. **Encoding Conversion** - UTF-8 ↔ UTF-16 conversion performance
3. **UI Responsiveness** - Find, replace, undo/redo, zoom operations
4. **Stress Tests** - Massive insert/delete operations, continuous editing

## Documentation

See [PERFORMANCE_TESTING.md](PERFORMANCE_TESTING.md) for:
- Detailed test descriptions
- Performance thresholds and baselines
- How to interpret results
- Adding new performance tests

## Files

- `PerformanceTests.h` - Test class definition
- `performance.cpp` - Test implementations
- `PERFORMANCE_TESTING.md` - Complete documentation
- `README.md` - This file

## Key Features

- ✅ Automated test data generation (100KB, 500KB, 1MB files)
- ✅ Memory usage monitoring (Linux)
- ✅ Precise timing with QElapsedTimer
- ✅ Performance threshold verification
- ✅ CTest integration with labels

## Running Tests

### All Performance Tests
```bash
ctest --test-dir build/debug -L Performance
```

### Individual Test
```bash
./build/debug/GnotePadPerformance testEncodingConversionUtf8ToUtf16LE
```

### With Verbose Output
```bash
./build/debug/GnotePadPerformance -v2
```

## Performance Thresholds

| Operation | Threshold |
|-----------|-----------|
| Load 100KB | < 1000 ms |
| Load 500KB | < 3000 ms |
| Load 1MB | < 6000 ms |
| Encoding conversion | < 2000 ms |
| Find (10 ops) | < 1000 ms |
| Replace all | < 2000 ms |

See PERFORMANCE_TESTING.md for complete threshold list and rationale.

## CI/CD Integration

Performance tests are labeled with "Performance" and can be run separately:

```bash
# Run only performance tests
ctest --test-dir build/debug -L Performance --output-on-failure

# Exclude performance tests from regular runs
ctest --test-dir build/debug -LE Performance
```

## Contributing

When adding new features that may impact performance:

1. Add relevant performance test
2. Define appropriate threshold
3. Document in PERFORMANCE_TESTING.md
4. Verify test passes on target hardware
5. Update this README if needed

---

For detailed information, see [PERFORMANCE_TESTING.md](PERFORMANCE_TESTING.md).
