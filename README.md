# Orderbook

[![C++20](https://img.shields.io/badge/C%2B%2B-20-blue.svg)](https://isocpp.org/)
[![CMake](https://img.shields.io/badge/CMake-3.21+-green.svg)](https://cmake.org/)

A high-performance limit order matching engine achieving **2.22 million orders per second**.

---

## Table of Contents

- [Architecture](#architecture)
- [Performance](#performance)
- [Building & Running](#building--running)
- [Project Structure](#project-structure)
- [Future Work](#future-work)
- [Technologies Used](#technologies-used)
- [Contact Me](#contact-me)

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
| **Orders at price level** | `std::vector` | Cache locality dominates; contiguous memory means CPU prefetching works.|
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
    if (!prices_compatible(incoming_order.price, best_price)) break;

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
#define COMPACTION_RATIO 0.75  // Trigger when 75% of orders are deleted

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

| Metric | Value | Notes |
|--------|-------|-------|
| **Throughput (no match)** | 3.37M orders/sec | Pure insertion, no matching (at 15M depth) |
| **Throughput (with matching)** | 2.22M orders/sec | Realistic mixed workload (at 15M depth) |
| **P50 Latency** | 251 ns | Median per-order latency (at 15M depth) |
| **P99 Latency** | 739 ns | Tail latency (at 15M depth) |
| **P999 Latency** | 3.41 µs | Extreme tail (at 15M depth) |
| **Max Tested Depth** | 15M orders | Orderbook size during benchmarks |

### Benchmark Suite

Four benchmarks measure different aspects of performance:

1. **BM_AddOrder_No_Match** — Insertion throughput without matching
2. **BM_AddOrder_Latency** — Per-order latency distribution (P50/P99/P999)
3. **BM_RemoveOrder_VaryDepth** — Cancellation performance at various depths
4. **BM_MatchingPerformance** — Realistic trading simulation with order overlap

---

## Building & Running

### Requirements

- C++20 compiler
- CMake 3.21+

### Build

```bash
cmake -B build
cmake --build build
```

### Run Benchmarks

```bash
./build/OrderBookBenchmark
```

Example output:
```
-----------------------------------------------------------------------------------------
Benchmark                       Time             CPU   Iterations   UserCounters
-----------------------------------------------------------------------------------------
BM_AddOrder_No_Match/0       294 ns          294 ns      2384021    items_per_second=3.676767M/s
BM_AddOrder_No_Match/1000    301 ns          301 ns      2325581    items_per_second=3.676767M/s
           |                    |               |           |                        |                   
           |                    |               |           |                        |                   
           |                    |               |           |                        |                   
```

### Run Unit Tests

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

---

## Future Work

- **Multithreading** - Synchronize across multiple orderbooks for different symbols.
- **Memory Pooling** — Custom allocators to reduce allocation overhead and improve cache behavior
- **Advanced Order Types** — Market Orders, Stop-Loss/Stop-Limit
- **Network Layer** — gRPC for order submission over network.

---

## Technologies Used

Built with:
- [Google Benchmark](https://github.com/google/benchmark) — Microbenchmarking framework
- [Google Test](https://github.com/google/googletest) — Unit testing framework

## Contact Me

Have any suggestions, critiques, or improvements? Please reach out!

Email: [adulam3@gatech.edu](mailto:adulam3@gatech.edu) 
