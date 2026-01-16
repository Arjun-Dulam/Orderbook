# Stock Exchange Orderbook

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.21+-green.svg)](https://cmake.org/)
[![Performance](https://img.shields.io/badge/Throughput-3.4M%20orders%2Fs-orange.svg)](#performance)

A high-performance limit order matching engine achieving **3.4 million orders per second**. Built as a deep dive into low-latency systems engineering, this project demonstrates the impact of data structure selection, memory access patterns, and compiler optimizations on real-world performance.

The journey from initial implementation to production-ready performance involved a **1,000x improvement** (3K → 3.4M ops/sec) through systematic profiling and optimization.

---

## Table of Contents

- [Architecture](#architecture)
- [Performance](#performance)
- [Building & Running](#building--running)
- [Project Structure](#project-structure)
- [Future Work](#future-work)

---

## Architecture

### Data Structure Design

The orderbook uses a three-tier indexing strategy optimized for different access patterns:

```
┌──────────────────────────────────────────────────────────────────┐
│                          OrderBook                               │
├──────────────────────────────────────────────────────────────────┤
│                                                                  │
│   bids: std::map<price, std::vector<Order>>                      │
│         └── Sorted high→low, FIFO within each price level        │
│                                                                  │
│   asks: std::map<price, std::vector<Order>>                      │
│         └── Sorted low→high, FIFO within each price level        │
│                                                                  │
│   order_lookup: std::unordered_map<order_id, OrderLocation>      │
│                 └── O(1) lookup for cancellations                │
│                                                                  │
└──────────────────────────────────────────────────────────────────┘
```

### Design Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| **Price-level container** | `std::map` | Automatic ordering for best-price iteration; O(log n) is acceptable given typical spread |
| **Orders at price level** | `std::vector` | Cache locality dominates; contiguous memory means CPU prefetching works. Lists scatter nodes across heap = cache miss on every traversal |
| **Order lookup** | `std::unordered_map` | Transforms O(n) cancellation scan into O(1) hash lookup |
| **Deletion strategy** | Lazy deletion + compaction | Defers expensive vector reorganization; amortizes cost across many operations |

### Matching Algorithm

The engine implements **price-time priority** (standard FIFO matching):

```cpp
// Simplified matching flow
while (incoming_order.quantity > 0) {
    // 1. Find best counter-side price
    auto best_price = (is_buy) ? asks.begin() : prev(bids.end());

    // 2. Check price compatibility
    if (!prices_cross(incoming_order.price, best_price)) break;

    // 3. Match against orders at this level (FIFO)
    for (auto& resting_order : orders_at_price) {
        if (resting_order.deleted) continue;

        uint32_t fill_qty = min(incoming.qty, resting.qty);
        execute_trade(incoming, resting, fill_qty);

        if (resting.qty == 0) mark_deleted(resting);
    }

    // 4. Clean up empty price levels
    if (level_empty) erase_price_level(best_price);
}
```

### Lazy Deletion & Compaction

Rather than immediately removing filled/cancelled orders (expensive vector reorganization), orders are marked `deleted_or_filled` and cleaned up in batches:

```cpp
#define COMPACTION_RATIO 0.35  // Trigger when 35% of orders are deleted

void compact_orderbook() {
    // Uses std::remove_if to batch-remove deleted orders
    // Updates order_lookup indices after shifts
    // Erases empty price levels from maps
}
```

This trades memory for latency—deleted orders occupy space temporarily but avoid per-deletion overhead on the hot path.

---

## Performance

### Benchmark Results

> **Note:** Fill in your specific benchmark results below after running `./build/OrderBookBenchmark`

| Metric | Value | Notes |
|--------|-------|-------|
| **Throughput (no match)** | `___` M orders/sec | Pure insertion, no matching |
| **Throughput (with matching)** | `___` M orders/sec | Realistic mixed workload |
| **P50 Latency** | `___` ns | Median per-order latency |
| **P99 Latency** | `___` ns | Tail latency |
| **P999 Latency** | `___` ns | Extreme tail |
| **Max Tested Depth** | 15M orders | Orderbook size during benchmarks |

### Benchmark Suite

Four benchmarks measure different aspects of performance:

1. **BM_AddOrder_No_Match** — Insertion throughput without matching
2. **BM_AddOrder_Latency** — Per-order latency distribution (P50/P99/P999)
3. **BM_RemoveOrder_VaryDepth** — Cancellation performance at various depths
4. **BM_MatchingPerformance** — Realistic trading simulation with order overlap

### Optimization Journey

The 1,000x performance improvement came from three key optimizations:

| Optimization | Impact | Before → After |
|--------------|--------|----------------|
| **Compiler flags** | ~100x | Debug → `-O3 -march=native -flto` |
| **O(1) order lookup** | ~10x | Linear scan → Hash map |
| **Pass-by-reference** | ~2x | Copy semantics → Reference semantics |

**Critical insight:** The initial 12M ops/sec measurement was *too optimistic*—it didn't account for compaction overhead in realistic workloads. Refined benchmarking with proper statistical validation yielded the accurate 3.4M figure.

### Compiler Flags

Performance is extremely sensitive to optimization flags:

```cmake
# Production (benchmarks)
-O3 -march=native -DNDEBUG -flto

# Debug (tests with sanitizers)
-g -O0 -fsanitize=address -fsanitize=undefined
```

Running without `-O3` results in **100x+ slowdown**.

---

## Building & Running

### Requirements

- C++20 compiler (GCC 10+, Clang 10+, Apple Clang 12+)
- CMake 3.21+
- Git (for fetching dependencies)

### Build

```bash
# Configure
cmake -B build

# Compile
cmake --build build
```

### Run Benchmarks

```bash
./build/OrderBookBenchmark
```

Example output:
```
-----------------------------------------------------------------
Benchmark                       Time             CPU   Iterations
-----------------------------------------------------------------
BM_AddOrder_No_Match/0       294 ns          294 ns      2384021
BM_AddOrder_No_Match/1000    301 ns          301 ns      2325581
...
```

### Run Tests

```bash
./build/OrderBookTests
```

39 unit tests covering:
- Order/Trade construction
- Matching engine (partial fills, price-time priority, multi-level matching)
- Order removal and lookup consistency
- Compaction correctness
- Edge cases (negative prices, large quantities, high volume)

---

## Project Structure

```
Stock Exchange/
├── include/
│   ├── order.hpp           # Order, Trade classes, Side enum
│   └── orderbook.hpp       # OrderBook interface
├── src/
│   ├── order.cpp           # Utility functions
│   └── orderbook.cpp       # Matching engine implementation
├── benchmarks/
│   ├── orderbook_bench.cpp # Google Benchmark suite
│   └── order_generator.*   # Realistic order generation
├── tests/
│   └── test.cpp            # Google Test suite
├── benchmark_results/
│   └── scripts/            # Analysis tools
└── CMakeLists.txt          # Build configuration
```

### Key Files

| File | Description |
|------|-------------|
| `orderbook.cpp` | Core matching algorithm (~150 lines of critical path) |
| `orderbook_bench.cpp` | Four benchmarks with statistical validation |
| `order_generator.cpp` | Realistic order generation with configurable distributions |

---

## Future Work

Potential enhancements for production use:

- **Thread Safety** — Lock-free data structures or fine-grained locking for concurrent access
- **Memory Pooling** — Custom allocators to reduce allocation overhead and improve cache behavior
- **Advanced Order Types** — Iceberg orders, stop-loss, fill-or-kill, immediate-or-cancel
- **Network Layer** — FIX protocol integration, WebSocket API for real-time updates
- **Persistence** — Write-ahead logging for crash recovery
- **Monitoring** — Latency histograms, throughput metrics, alerting integration

---

## Acknowledgments

Built with:
- [Google Benchmark](https://github.com/google/benchmark) — Microbenchmarking framework
- [Google Test](https://github.com/google/googletest) — Unit testing framework
