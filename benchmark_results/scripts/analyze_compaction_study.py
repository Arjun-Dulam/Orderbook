#!/usr/bin/env python3
"""
Compaction Ratio Performance Analysis
Parses Google Benchmark results and generates performance graphs.
"""

import re
import matplotlib.pyplot as plt
from collections import defaultdict
import sys
import os

def parse_benchmark_results(file_path):
    """Parse Google Benchmark output and extract performance data."""
    data = defaultdict(lambda: defaultdict(dict))
    current_ratio = None
    
    with open(file_path, 'r') as f:
        for line in f:
            # Extract current compaction ratio
            ratio_match = re.search(r'Testing Compaction Ratio:\s+([\d.]+)', line)
            if ratio_match:
                current_ratio = float(ratio_match.group(1))
                continue
            
            # Parse benchmark lines
            # Format: BM_AddOrder_No_Match/1000   337 ns   337 ns   2202227 items_per_second=2.97052M/s
            bench_match = re.search(
                r'(BM_\w+)/(\d+)\s+\d+\s+ns\s+\d+\s+ns\s+\d+\s+items_per_second=([\d.]+)([MK])/s',
                line
            )
            if bench_match and current_ratio is not None:
                benchmark_name = bench_match.group(1)
                depth = int(bench_match.group(2))
                throughput = float(bench_match.group(3))
                unit = bench_match.group(4)
                
                # Convert to millions/sec
                if unit == 'K':
                    throughput /= 1000
                
                data[benchmark_name][current_ratio][depth] = throughput
    
    return data

def create_graphs(data, output_dir):
    """Generate performance visualization graphs."""
    
    # Ensure output directory exists
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)
        print(f"Created directory: {output_dir}")
    
    for benchmark_name, ratios in data.items():
        # Extract all unique depths and ratios
        all_depths = sorted(set(depth for ratio_data in ratios.values() for depth in ratio_data.keys()))
        all_ratios = sorted(ratios.keys())
        
        # Graph 1: Throughput vs Compaction Ratio (separate line per depth)
        plt.figure(figsize=(12, 7))
        for depth in all_depths:
            ratio_vals = []
            throughput_vals = []
            for ratio in all_ratios:
                if depth in ratios[ratio]:
                    ratio_vals.append(ratio)
                    throughput_vals.append(ratios[ratio][depth])
            
            if ratio_vals:
                label = f"{depth:,} orders" if depth > 0 else "cold start"
                plt.plot(ratio_vals, throughput_vals, marker='o', linewidth=2, label=label)
        
        plt.xlabel('Compaction Ratio', fontsize=12)
        plt.ylabel('Throughput (M ops/sec)', fontsize=12)
        plt.title(f'{benchmark_name}: Throughput vs Compaction Ratio', fontsize=14, fontweight='bold')
        plt.legend(loc='best', fontsize=10)
        plt.grid(True, alpha=0.3)
        plt.tight_layout()
        
        output_file = f"{output_dir}/{benchmark_name}_ratio_comparison.png"
        plt.savefig(output_file, dpi=300, bbox_inches='tight')
        print(f"✓ Saved: {output_file}")
        plt.close()
        
        # Graph 2: Throughput vs Depth (separate line per ratio)
        plt.figure(figsize=(12, 7))
        for ratio in all_ratios:
            depths = []
            throughputs = []
            for depth in all_depths:
                if depth in ratios[ratio]:
                    depths.append(depth)
                    throughputs.append(ratios[ratio][depth])
            
            if depths:
                plt.plot(depths, throughputs, marker='o', linewidth=2, label=f"Ratio {ratio:.2f}")
        
        plt.xlabel('Order Book Depth', fontsize=12)
        plt.ylabel('Throughput (M ops/sec)', fontsize=12)
        plt.title(f'{benchmark_name}: Throughput vs Depth', fontsize=14, fontweight='bold')
        plt.xscale('log')
        plt.legend(loc='best', fontsize=10)
        plt.grid(True, alpha=0.3)
        plt.tight_layout()
        
        output_file = f"{output_dir}/{benchmark_name}_depth_comparison.png"
        plt.savefig(output_file, dpi=300, bbox_inches='tight')
        print(f"✓ Saved: {output_file}")
        plt.close()

def print_summary(data):
    """Print summary statistics."""
    print("\n" + "="*60)
    print("PERFORMANCE SUMMARY")
    print("="*60)
    
    for benchmark_name, ratios in data.items():
        print(f"\n{benchmark_name}:")
        print("-" * 60)
        
        # Find best ratio for each depth
        all_depths = sorted(set(depth for ratio_data in ratios.values() for depth in ratio_data.keys()))
        
        for depth in all_depths:
            best_ratio = None
            best_throughput = 0
            
            for ratio, depths_data in ratios.items():
                if depth in depths_data and depths_data[depth] > best_throughput:
                    best_throughput = depths_data[depth]
                    best_ratio = ratio
            
            depth_label = f"{depth:,} orders" if depth > 0 else "cold start"
            print(f"  {depth_label:20} → Best: Ratio {best_ratio:.2f} ({best_throughput:.2f} M/s)")

if __name__ == "__main__":
    # Configuration
    results_file = "benchmark_results/results/compaction_ratios"
    output_dir = "benchmark_results/results/"
    
    print("Compaction Ratio Performance Analysis")
    print("=" * 60)
    
    # Parse results
    print(f"\nParsing: {results_file}")
    data = parse_benchmark_results(results_file)
    
    if not data:
        print("ERROR: No benchmark data found in results file!")
        sys.exit(1)
    
    print(f"Found {len(data)} benchmark types")
    
    # Generate graphs
    print(f"\nGenerating graphs → {output_dir}/")
    create_graphs(data, output_dir)
    
    # Print summary
    print_summary(data)
    
    print("\n" + "="*60)
    print("Analysis complete!")
    print("="*60)
