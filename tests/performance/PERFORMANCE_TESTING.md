# GnotePad Performance Testing Guide

This document describes the performance testing framework for GnotePad, including test methodology, benchmarks, and usage instructions.

## Overview

The performance test suite is designed to ensure GnotePad maintains responsive UI and efficient resource usage when handling large files, complex operations, and intensive workflows. The tests measure:

- **Load/Save Performance**: Time to load and save files of various sizes
- **Encoding Conversion**: Performance of UTF-8 ↔ UTF-16 conversions
- **UI Responsiveness**: Scrolling, zooming, and editing operations
- **Resource Usage**: Memory consumption during operations
- **Stress Testing**: Behavior under intensive editing workflows

## Test Categories

### 1. Large File Handling Tests

These tests measure performance when working with large files:

| Test | File Size | Purpose | Threshold |
|------|-----------|---------|-----------|
| `testLoadLargeFile100KB` | 100 KB | Basic load performance | < 1000 ms |
| `testLoadLargeFile500KB` | 500 KB | Medium file load | < 3000 ms |
| `testLoadLargeFile1MB` | 1 MB | Large file load | < 6000 ms |
| `testSaveLargeFile` | 500 KB | Save performance | < 3000 ms |

**What is tested:**
- Time to load file into editor
- Memory allocation during load
- Document rendering time
- Save operation performance

**Success criteria:**
- Load time scales linearly with file size
- No UI freezing during operations
- Memory usage is proportional to file size

### 2. Encoding Conversion Performance

Tests measure the performance of character encoding conversions:

| Test | Operation | Threshold |
|------|-----------|-----------|
| `testEncodingConversionUtf8ToUtf16LE` | UTF-8 → UTF-16 LE | < 2000 ms |
| `testEncodingConversionUtf8ToUtf16BE` | UTF-8 → UTF-16 BE | < 2000 ms |
| `testEncodingRoundTripPerformance` | UTF-8 → UTF-16 LE → UTF-16 BE → UTF-8 | < 6000 ms |
| `testMemoryUsageDuringEncoding` | 1 MB file encoding | < 50 MB |

**What is tested:**
- Conversion speed for different encodings
- Content preservation during conversions
- Memory overhead of encoding operations
- Round-trip consistency

**Success criteria:**
- Conversions complete within threshold
- No data loss during encoding changes
- Memory usage remains bounded

### 3. UI Responsiveness Tests

These tests ensure the UI remains responsive during common operations:

| Test | Operation | Threshold |
|------|-----------|-----------|
| `testScrollPerformanceLargeFile` | 100 scroll steps | < 500 ms |
| `testFindPerformanceLargeFile` | 10 find operations | < 1000 ms |
| `testReplacePerformanceLargeFile` | Replace all matches | < 2000 ms |
| `testUndoRedoStackPerformance` | 100 undo + 100 redo | < 10000 ms |
| `testZoomOperationsPerformance` | 20 zoom operations | < 4000 ms |

**What is tested:**
- Scrolling smoothness on large documents
- Search/replace speed
- Undo/redo stack efficiency
- Zoom rendering performance

**Success criteria:**
- Operations complete within interactive timeframes
- No visible lag or stuttering
- UI remains responsive throughout

### 4. Stress Tests

Intensive workload tests to identify performance limits:

| Test | Workload | Purpose |
|------|----------|---------|
| `testMassiveInsertOperations` | 1000 line inserts | Insert efficiency |
| `testMassiveDeleteOperations` | 500 line deletions | Delete efficiency |
| `testContinuousEditing` | 50 mixed operations | Real-world editing |
| `testDocumentModificationPerformance` | 1000 modifications | Flag tracking |

**What is tested:**
- Behavior under extreme editing loads
- Performance degradation over time
- Memory leaks during intensive use
- Recovery from stress conditions

## Performance Thresholds

The test suite uses the following performance thresholds (defined in `PerformanceTests.cpp`):

```cpp
namespace PerformanceThresholds
{
    constexpr int LOAD_100KB_MS = 1000;      // Max time to load 100KB file
    constexpr int LOAD_500KB_MS = 3000;      // Max time to load 500KB file
    constexpr int LOAD_1MB_MS = 6000;        // Max time to load 1MB file
    constexpr int SAVE_LARGE_MS = 3000;      // Max time to save large file
    constexpr int ENCODING_MS = 2000;        // Max time for encoding conversion
    constexpr int SCROLL_100_LINES_MS = 500; // Max time to scroll 100 lines
    constexpr int FIND_MS = 1000;            // Max time for find operation
    constexpr int REPLACE_MS = 2000;         // Max time for replace operation
    constexpr int UNDO_REDO_MS = 100;        // Max time for single undo/redo
    constexpr int ZOOM_MS = 200;             // Max time for zoom operation
}
```

These thresholds are based on:
- **User perception**: Operations under 100ms feel instant, under 1000ms feel responsive
- **File size**: Larger files get proportionally longer thresholds
- **Complexity**: More complex operations (encoding, replace-all) get longer thresholds

### Adjusting Thresholds

Thresholds may need adjustment based on:
- Target hardware (slower systems may need higher thresholds)
- Qt version (newer versions may have different performance characteristics)
- Operating system (platform-specific optimizations)
- User feedback (perceived performance issues)

To adjust a threshold, edit the value in `tests/performance/performance.cpp` and document the reason for the change.

## Running Performance Tests

### Build the Test Suite

```bash
# Configure and build
cmake -S . -B build/debug -G Ninja \
  -DCMAKE_CXX_COMPILER=clang++ \
  -DCMAKE_BUILD_TYPE=Debug
cmake --build build/debug

# Or use VS Code task: "Build Debug"
```

### Run All Performance Tests

```bash
# Run via CTest
ctest --test-dir build/debug -R "Performance"

# Or run the executable directly
./build/debug/GnotePadPerformance
```

### Run Individual Tests

```bash
# Run a specific test by name
./build/debug/GnotePadPerformance testLoadLargeFile100KB

# Run multiple specific tests
./build/debug/GnotePadPerformance testLoadLargeFile100KB testEncodingConversionUtf8ToUtf16LE
```

### Verbose Output

```bash
# Run with verbose output to see timing details
./build/debug/GnotePadPerformance -v2

# Run with maximum verbosity
./build/debug/GnotePadPerformance -v2 -maxwarnings 0
```

## Interpreting Results

### Sample Output

```
********* Start testing of PerformanceTests *********
Config: Using QtTest library 6.5.0, Qt 6.5.0
PASS   : PerformanceTests::initTestCase()
INFO   : PerformanceTests::testLoadLargeFile100KB() Load time for 100KB file: 245 ms
PASS   : PerformanceTests::testLoadLargeFile100KB()
INFO   : PerformanceTests::testLoadLargeFile500KB() Load time for 500KB file: 892 ms
PASS   : PerformanceTests::testLoadLargeFile500KB()
...
Totals: 20 passed, 0 failed, 0 skipped
********* Finished testing of PerformanceTests *********
```

### Performance Metrics

Each test outputs timing information:
- **Load time**: Time from file open to UI ready
- **Save time**: Time from save initiation to file written
- **Operation time**: Time for specific operations (find, replace, etc.)
- **Memory usage**: Memory delta before/after operations (Linux only)

### Success/Failure Criteria

- **PASS**: Operation completed within threshold time
- **FAIL**: Operation exceeded threshold or encountered error
- **INFO**: Actual timing measurement (for trend analysis)

### What to Do If Tests Fail

1. **Check system load**: Ensure no other intensive processes are running
2. **Review threshold**: Is the threshold appropriate for the hardware?
3. **Profile the code**: Use profiling tools to identify bottlenecks
4. **Optimize**: Address performance issues identified by profiling
5. **Document**: If optimization is not feasible, document why and adjust threshold

## Memory Monitoring

Memory usage is tracked on Linux systems via `/proc/self/status`:

```cpp
qint64 PerformanceTests::getCurrentMemoryUsage() const
{
#if defined(Q_OS_LINUX)
    // Read VmRSS from /proc/self/status
    // ...
#endif
    return 0; // Not available on other platforms
}
```

**Memory Test Example:**
```
INFO   : PerformanceTests::testMemoryUsageDuringEncoding() Memory usage before: 45832 KB
INFO   : PerformanceTests::testMemoryUsageDuringEncoding() Memory usage after load: 48124 KB
INFO   : PerformanceTests::testMemoryUsageDuringEncoding() Memory usage after save: 49001 KB
INFO   : PerformanceTests::testMemoryUsageDuringEncoding() Memory delta: 3169 KB
PASS   : PerformanceTests::testMemoryUsageDuringEncoding()
```

## Benchmark Baselines

These are reference timings from the development environment (Ubuntu 22.04, Intel i7, 16GB RAM):

| Operation | Baseline | Target |
|-----------|----------|--------|
| Load 100KB | ~250 ms | < 1000 ms |
| Load 500KB | ~900 ms | < 3000 ms |
| Load 1MB | ~2000 ms | < 6000 ms |
| UTF-8 → UTF-16 (200KB) | ~400 ms | < 2000 ms |
| Find (10 ops, 200KB) | ~150 ms | < 1000 ms |
| Replace All (100KB) | ~800 ms | < 2000 ms |
| 100 Undo/Redo | ~2000 ms | < 10000 ms |
| Scroll 100 steps | ~200 ms | < 500 ms |

**Note**: Baselines are for reference only. Actual performance varies by hardware, OS, and Qt version.

## Test Data Generation

Performance tests automatically generate test files on demand:

```cpp
void PerformanceTests::generateTestFile(const QString& path, qint64 sizeInBytes)
{
    // Generates realistic text content with line breaks
    // Includes paragraph text and line numbering
}
```

Generated files contain:
- Repeating Lorem ipsum paragraphs
- Line numbers for tracking
- Realistic line breaks
- Sizes: 100KB, 500KB, 1MB as needed

## Integration with CI/CD

### Automated Testing

Performance tests can be integrated into CI pipelines:

```bash
# Run performance tests in CI
ctest --test-dir build/debug -R "Performance" --output-on-failure
```

### Regression Detection

To detect performance regressions:

1. **Baseline**: Run tests on main branch and record timings
2. **Compare**: Run tests on feature branch
3. **Alert**: Flag tests that exceed baseline by >20%

Example CI script:
```bash
#!/bin/bash
# Run performance tests and check for regressions
./build/debug/GnotePadPerformance -o results.txt
# Parse results.txt and compare to baseline.txt
# Exit with error if regression detected
```

## Adding New Performance Tests

### 1. Add Test Method

Add to `tests/performance/PerformanceTests.h`:

```cpp
private slots:
    void testMyNewPerformanceFeature();
```

### 2. Implement Test

Add to `tests/performance/performance.cpp`:

```cpp
void PerformanceTests::testMyNewPerformanceFeature()
{
    // Setup
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    
    // Measure performance
    QElapsedTimer timer;
    timer.start();
    
    // ... operation to measure ...
    
    const qint64 operationTime = timer.elapsed();
    qInfo() << "My operation time:" << operationTime << "ms";
    
    // Verify threshold
    QVERIFY2(operationTime < MY_THRESHOLD_MS,
             qPrintable(QString("Time %1ms exceeds threshold").arg(operationTime)));
}
```

### 3. Update CMakeLists.txt

Add test name to the test list in `tests/CMakeLists.txt`.

### 4. Document

Add test description to this document under appropriate category.

## Best Practices

### Test Design

1. **Isolation**: Each test should be independent
2. **Cleanup**: Use `QTemporaryDir` for temporary files
3. **Realistic**: Use realistic data and workflows
4. **Measurable**: Clearly define what's being measured
5. **Repeatable**: Tests should produce consistent results

### Performance Optimization

When optimizing based on test results:

1. **Profile first**: Use profiling tools to identify actual bottlenecks
2. **Measure impact**: Run tests before and after optimization
3. **Consider trade-offs**: Balance performance vs code complexity
4. **Document changes**: Explain performance improvements in commit messages

### Threshold Selection

When defining performance thresholds:

1. **User-centered**: Based on perceived responsiveness
2. **Hardware-aware**: Account for minimum supported hardware
3. **Operation-specific**: Complex operations get higher thresholds
4. **Evidence-based**: Use profiling data and user feedback

## Troubleshooting

### Tests Run Slowly

- Ensure running in offscreen mode: `QT_QPA_PLATFORM=offscreen`
- Check for background processes consuming resources
- Build in Release mode for more realistic timings
- Consider hardware limitations

### Inconsistent Results

- Run multiple times and average results
- Check for thermal throttling on mobile hardware
- Disable power management features
- Close other applications

### Memory Tests Don't Work

- Memory monitoring is Linux-only currently
- Requires reading `/proc/self/status`
- Will show "Memory monitoring not available" on other platforms

## Future Enhancements

Planned improvements to the performance test suite:

1. **Async Operation Testing**: Test file I/O with progress indicators
2. **Multi-document Performance**: Test tab switching and concurrent documents
3. **Real-time Monitoring**: Live performance dashboard during development
4. **Historical Tracking**: Database of performance metrics over time
5. **Platform-specific Tests**: Windows and macOS specific optimizations
6. **GPU Acceleration**: Test rendering performance with different backends

## References

- [Qt Performance Tips](https://doc.qt.io/qt-6/performance.html)
- [QTest Benchmarking](https://doc.qt.io/qt-6/qtest-overview.html#benchmarking)
- [GnotePad Architecture](../docs/ARCHITECTURE.md)

## Support

For performance-related issues or questions:

1. Review this documentation
2. Check test output for specific timing information
3. Profile the application to identify bottlenecks
4. Consult Qt documentation for optimization strategies

---

**Last Updated**: December 2024  
**Version**: 1.0  
**Status**: Active Development
