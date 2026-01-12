# Benchmark Testing Implementation Plan

## Overview
Implement professional-grade benchmarking using Google Benchmark framework with realistic order streams, latency percentile tracking, cache performance metrics, and stress testing to find system limits.

## Goals
1. **Stress test** the order book to find performance limits and breaking points
2. Use **statistically realistic** order distributions (Poisson arrivals, price clustering, power law sizes)
3. Measure **latency percentiles** (p50, p99, p99.9), **throughput**, and **cache performance**
4. Create **impressive, professional** benchmarks suitable for tech company portfolios

---

## Phase 1: Google Benchmark Integration

### 1.1 Update CMakeLists.txt
**File:** `CMakeLists.txt`

Add Google Benchmark via CMake FetchContent:
```cmake
# After project() declaration:
include(FetchContent)

FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3  # Latest stable release
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(benchmark)

# Create benchmark executable (separate from existing tests)
set(BENCH_SOURCE
    benchmarks/orderbook_bench.cpp
    src/orderbook.cpp
    src/order.cpp
)

add_executable(OrderBookBenchmark ${BENCH_SOURCE})
target_link_libraries(OrderBookBenchmark benchmark::benchmark)
```

**Why FetchContent?**
- No manual download required
- Beginner-friendly
- Automatic dependency management
- Industry standard approach

### 1.2 Create Benchmark Directory Structure
```
benchmarks/
├── orderbook_bench.cpp       # Main benchmark file
├── order_generator.hpp       # Realistic order stream generator
└── order_generator.cpp       # Generator implementation
```

---

## Phase 2: Realistic Order Stream Generator

### 2.1 Order Generator Design
**File:** `benchmarks/order_generator.hpp` and `.cpp`

**Components:**
1. **Price clustering** - Orders cluster around a "market price" using normal distribution
2. **Order size distribution** - Power law (most orders small, few large orders)
3. **Poisson arrivals** - Realistic time spacing between orders
4. **Cancel/modify rates** - Configurable percentage of orders get cancelled
5. **Buy/sell ratio** - Typically 50/50 but configurable

**Key Parameters:**
```cpp
struct MarketConfig {
    int32_t base_price = 10000;           // $100.00 center price
    double price_stddev = 100;             // Price volatility (±$1.00)
    double arrival_rate = 1000.0;          // Lambda for Poisson (orders/sec)
    double cancel_rate = 0.30;             // 30% of orders get cancelled
    uint32_t min_quantity = 1;
    uint32_t max_quantity = 10000;
    double power_law_alpha = 2.5;          // Controls size distribution
};
```

**Random Distributions Used:**
- `std::normal_distribution` - Price clustering
- `std::exponential_distribution` - Poisson arrivals (inter-arrival times)
- Custom power law - Order sizes
- `std::bernoulli_distribution` - Buy vs Sell, Cancel decisions

### 2.2 Generator API
```cpp
class OrderGenerator {
public:
    OrderGenerator(MarketConfig config, uint32_t seed);

    Order generate_order();              // Get next order
    bool should_cancel();                // Whether to cancel this order
    uint32_t select_cancel_target();     // Which order to cancel

    std::vector<Order> generate_batch(size_t count);
};
```

---

## Phase 3: Core Benchmarks with Google Benchmark

### 3.1 Basic Throughput Benchmarks
**File:** `benchmarks/orderbook_bench.cpp`

**Benchmarks to implement:**
1. `BM_AddOrder_NoMatch` - Add orders that don't match (builds book depth)
2. `BM_AddOrder_ImmediateMatch` - Orders that immediately match and trade
3. `BM_RemoveOrder` - Cancel orders (with varying book depths)
4. `BM_MatchMultipleLevels` - Large order sweeping through price levels
5. `BM_MixedWorkload` - Realistic mix of adds, matches, cancels

**Example benchmark structure:**
```cpp
static void BM_AddOrder_NoMatch(benchmark::State& state) {
    OrderBook book;
    OrderGenerator gen(MarketConfig{}, 42);

    // state.range(0) = number of existing orders in book
    for (int i = 0; i < state.range(0); i++) {
        Order o = gen.generate_order();
        book.add_order(o);
    }

    for (auto _ : state) {
        Order o = gen.generate_order();
        auto trades = book.add_order(o);
        benchmark::DoNotOptimize(trades);
    }

    state.SetItemsProcessed(state.iterations());
}

// Register with different book depths
BENCHMARK(BM_AddOrder_NoMatch)
    ->Arg(1000)      // 1K orders in book
    ->Arg(10000)     // 10K orders
    ->Arg(100000)    // 100K orders
    ->Arg(1000000);  // 1M orders (stress test)
```

### 3.2 Latency Percentile Tracking
Google Benchmark doesn't track percentiles by default, so we'll use custom statistics:

```cpp
static void BM_AddOrder_Latency(benchmark::State& state) {
    OrderBook book;
    std::vector<double> latencies;
    latencies.reserve(10000);

    for (auto _ : state) {
        state.PauseTiming();
        Order o = /* generate order */;
        state.ResumeTiming();

        auto start = std::chrono::high_resolution_clock::now();
        auto trades = book.add_order(o);
        auto end = std::chrono::high_resolution_clock::now();

        double ns = std::chrono::duration<double, std::nano>(end - start).count();
        latencies.push_back(ns);
        benchmark::DoNotOptimize(trades);
    }

    // Calculate percentiles
    std::sort(latencies.begin(), latencies.end());
    state.counters["p50_ns"] = latencies[latencies.size() * 0.50];
    state.counters["p99_ns"] = latencies[latencies.size() * 0.99];
    state.counters["p999_ns"] = latencies[latencies.size() * 0.999];
}
```

---

## Phase 4: Cache Performance Measurement

### 4.1 Using perf on macOS
macOS doesn't expose hardware performance counters as easily as Linux, but we can:
- Use Instruments.app for deep analysis (manual)
- Use `-fprofile-instr-generate` for PGO profiling
- Measure indirectly via microbenchmarks with varying data sizes

### 4.2 Cache-Aware Benchmarks
**Strategy:** Vary the size of the order book to exceed L1, L2, L3 cache sizes and observe performance degradation.

```cpp
// Test with different book depths to observe cache effects
BENCHMARK(BM_AddOrder_CacheTest)
    ->Arg(100)        // Fits in L1 cache (~32KB)
    ->Arg(1000)       // Fits in L2 cache (~256KB)
    ->Arg(10000)      // Fits in L3 cache (~8MB)
    ->Arg(100000)     // Exceeds L3, uses RAM
    ->Arg(1000000);   // Heavy RAM pressure
```

### 4.3 Stride Pattern Testing
Test random vs sequential access patterns:
```cpp
static void BM_RemoveOrder_RandomAccess(benchmark::State& state);
static void BM_RemoveOrder_SequentialAccess(benchmark::State& state);
```

---

## Phase 5: Stress Testing to Find Limits

### 5.1 Gradual Load Increase
Find the breaking point by gradually increasing:
1. **Order book depth** - How many orders before performance degrades?
2. **Price level count** - How many distinct price levels?
3. **Orders per price level** - How deep can queues get?
4. **Compaction frequency** - What happens with high cancel rates?

### 5.2 Stress Test Scenarios

**Scenario 1: Depth Stress Test**
```cpp
static void BM_Stress_BookDepth(benchmark::State& state) {
    // state.range(0) = target book depth (100K to 10M)
    // Measure throughput degradation as book grows
}
```

**Scenario 2: High Cancel Rate Stress**
```cpp
static void BM_Stress_HighCancelRate(benchmark::State& state) {
    // Test with 50%, 70%, 90% cancel rates
    // Measure compaction overhead
}
```

**Scenario 3: Price Fragmentation**
```cpp
static void BM_Stress_PriceFragmentation(benchmark::State& state) {
    // Create orders across 100, 1000, 10000 distinct price levels
    // Measure std::map traversal overhead
}
```

**Scenario 4: Memory Pressure**
```cpp
static void BM_Stress_MemoryPressure(benchmark::State& state) {
    // Track memory usage as book grows
    // Find RAM limits
}
```

### 5.3 Limit Detection
Output should identify:
- Maximum sustainable throughput (ops/sec)
- Maximum book depth before 10x slowdown
- Memory usage at various scales
- Point where compaction dominates runtime

---

## Phase 6: Output and Reporting

### 6.1 Google Benchmark Output
Default output includes:
```
--------------------------------------------------------------
Benchmark                    Time             CPU   Iterations
--------------------------------------------------------------
BM_AddOrder_NoMatch/1000    156 ns          156 ns      4480000
BM_AddOrder_NoMatch/10000   189 ns          189 ns      3710000
BM_AddOrder_NoMatch/100000  421 ns          421 ns      1662000
```

### 6.2 Custom Metrics
Add custom counters for:
- Orders/sec throughput
- Trades/sec executed
- Compaction frequency
- Memory allocated
- Latency percentiles

### 6.3 Comparison Mode
Google Benchmark supports comparing runs:
```bash
# Baseline run
./OrderBookBenchmark --benchmark_out=baseline.json --benchmark_out_format=json

# After optimization
./OrderBookBenchmark --benchmark_out=optimized.json --benchmark_out_format=json

# Compare
python3 -m pip install google-benchmark
compare.py benchmarks baseline.json optimized.json
```

---

## Implementation Order

1. ✅ **Update CMakeLists.txt** - Add Google Benchmark dependency
2. ✅ **Create basic benchmark file** - `benchmarks/orderbook_bench.cpp` with simple examples
3. ✅ **Implement OrderGenerator** - Realistic order stream generator
4. ✅ **Add throughput benchmarks** - Test basic operations at scale
5. ✅ **Add latency benchmarks** - Track p50/p99/p999
6. ✅ **Add cache benchmarks** - Varying data sizes
7. ✅ **Add stress tests** - Find system limits
8. ✅ **Document results** - Create BENCHMARKS.md with findings

---

## Critical Files to Modify

1. **CMakeLists.txt** - Add Google Benchmark integration
2. **benchmarks/orderbook_bench.cpp** (NEW) - Main benchmark suite
3. **benchmarks/order_generator.hpp** (NEW) - Order generator interface
4. **benchmarks/order_generator.cpp** (NEW) - Generator implementation
5. **BENCHMARKS.md** (NEW) - Document findings and system limits

---

## Verification Plan

After implementation, verify:
1. ✅ CMake builds successfully with Google Benchmark
2. ✅ `./OrderBookBenchmark` runs without errors
3. ✅ Output shows throughput in ops/sec
4. ✅ Latency percentiles are reasonable (< 1µs for basic ops)
5. ✅ Stress tests identify clear performance limits
6. ✅ Cache benchmarks show performance degradation as data grows
7. ✅ Results look professional and impressive for portfolio

---

## Expected Outcomes

Based on your current performance (~4M cancels/sec), realistic benchmarks should show:
- **Best case latency**: 50-200ns per operation (hot cache, small book)
- **Typical latency**: 200-500ns per operation (realistic book depth)
- **Worst case latency**: 1-10µs (cache misses, deep book, compaction)
- **Throughput**: 1-5M ops/sec sustained
- **Limit point**: Performance degrades significantly beyond 1-2M active orders

This professional benchmark suite will demonstrate:
- Understanding of realistic performance testing
- Knowledge of statistical analysis (percentiles, distributions)
- Awareness of hardware constraints (cache hierarchy)
- Industry-standard tools (Google Benchmark)
