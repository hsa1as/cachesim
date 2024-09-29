import subprocess
import re
import numpy as np
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

def run_cache_sim(l1_size, l1_assoc, vc_entries, l2_size, l2_assoc, blocksize=32):
    command = f"./cache_sim {l1_size} {l1_assoc} {blocksize} {vc_entries} {l2_size} {l2_assoc} gcc_trace.txt"
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    return result.stdout

def parse_output(output, metric):
    if "Cacti failed" in output:
        return None
    output.replace(" ","")
    output.replace("\t","")
    patterns = {
        "miss_rate": r"combined L1\+VC miss rate:\s*([\d.]+)",
        "aat": r"average access time:\s*([\d.]+)",
        "edp": r"energy-delay product:\s*([\d.]+)",
        "area": r"total area:\s*([\d.]+)"
    }
    match = re.search(patterns[metric], output)
    return float(match.group(1)) if match else None

def generate_plot1():
    sizes = [2**i * 1024 for i in range(1, 11)]  # 2KB to 1MB
    assocs = [1, 2, 4, 8, -1]  # -1 represents fully-associative
    assoc_labels = ["Direct-mapped", "2-way", "4-way", "8-way", "Fully-associative"]
    
    miss_rates = {assoc: [] for assoc in assocs}
    valid_sizes = {assoc: [] for assoc in assocs}
    
    for size in sizes:
        for assoc in assocs:
            actual_assoc = size // 32 if assoc == -1 else assoc  # For fully-associative
            output = run_cache_sim(size, actual_assoc, 0, 0, 1)
            miss_rate = parse_output(output, "miss_rate")
            if miss_rate is not None:
                miss_rates[assoc].append(miss_rate)
                valid_sizes[assoc].append(size)
    
    plt.figure(figsize=(10, 6))
    for assoc, label in zip(assocs, assoc_labels):
        plt.plot(np.log2(valid_sizes[assoc]), miss_rates[assoc], marker='o', label=label)
    
    plt.xlabel('log2(SIZE)')
    plt.ylabel('L1 Miss Rate')
    plt.title('Plot #1: L1 Miss Rate vs Cache Size for Different Associativities')
    plt.legend()
    plt.grid(True)
    plt.savefig('plot1.png')
    plt.close()

def generate_plot2():
    sizes = [2**i * 1024 for i in range(1, 11)]  # 2KB to 1MB
    assocs = [1, 2, 4, 8, -1]  # -1 represents fully-associative
    assoc_labels = ["Direct-mapped", "2-way", "4-way", "8-way", "Fully-associative"]
    
    aats = {assoc: [] for assoc in assocs}
    valid_sizes = {assoc: [] for assoc in assocs}
    
    for size in sizes:
        for assoc in assocs:
            actual_assoc = size // 32 if assoc == -1 else assoc  # For fully-associative
            output = run_cache_sim(size, actual_assoc, 0, 0, 1)
            aat = parse_output(output, "aat")
            if aat is not None:
                aats[assoc].append(aat)
                valid_sizes[assoc].append(size)
    
    plt.figure(figsize=(10, 6))
    for assoc, label in zip(assocs, assoc_labels):
        plt.plot(np.log2(valid_sizes[assoc]), aats[assoc], marker='o', label=label)
    
    plt.xlabel('log2(SIZE)')
    plt.ylabel('Average Access Time (AAT)')
    plt.title('Plot #2: AAT vs Cache Size for Different Associativities')
    plt.legend()
    plt.grid(True)
    plt.savefig('plot2.png')
    plt.close()

def generate_plot3():
    sizes = [2**i * 1024 for i in range(1, 8)]  # 2KB to 128KB
    assocs = [1, 2, 4, 8, -1]  # -1 represents fully-associative
    assoc_labels = ["Direct-mapped", "2-way", "4-way", "8-way", "Fully-associative"]
    
    aats = {assoc: [] for assoc in assocs}
    valid_sizes = {assoc: [] for assoc in assocs}
    
    for size in sizes:
        for assoc in assocs:
            actual_assoc = size // 32 if assoc == -1 else assoc  # For fully-associative
            output = run_cache_sim(size, actual_assoc, 0, 256*1024, 8)
            aat = parse_output(output, "aat")
            if aat is not None:
                aats[assoc].append(aat)
                valid_sizes[assoc].append(size)
    
    plt.figure(figsize=(10, 6))
    for assoc, label in zip(assocs, assoc_labels):
        plt.plot(np.log2(valid_sizes[assoc]), aats[assoc], marker='o', label=label)
    
    plt.xlabel('log2(L1 SIZE)')
    plt.ylabel('Average Access Time (AAT)')
    plt.title('Plot #3: AAT vs L1 Cache Size with L2 Cache')
    plt.legend()
    plt.grid(True)
    plt.savefig('plot3.png')
    plt.close()

def generate_plot4():
    sizes = [2**i * 1024 for i in range(0, 6)]  # 1KB to 32KB
    blocksizes = [16, 32, 64, 128]
    
    miss_rates = {size: [] for size in sizes}
    valid_blocksizes = {size: [] for size in sizes}
    
    for size in sizes:
        for blocksize in blocksizes:
            output = run_cache_sim(size, 4, 0, 0, 1, blocksize)
            miss_rate = parse_output(output, "miss_rate")
            if miss_rate is not None:
                miss_rates[size].append(miss_rate)
                valid_blocksizes[size].append(blocksize)
    
    plt.figure(figsize=(10, 6))
    for size in sizes:
        plt.plot(np.log2(valid_blocksizes[size]), miss_rates[size], marker='o', label=f'{size//1024}KB')
    
    plt.xlabel('log2(BLOCKSIZE)')
    plt.ylabel('L1 Miss Rate')
    plt.title('Plot #4: L1 Miss Rate vs Block Size for Different Cache Sizes')
    plt.legend()
    plt.grid(True)
    plt.savefig('plot4.png')
    plt.close()

def generate_plot5():
    l1_sizes = [2**i * 1024 for i in range(0, 7)]  # 1KB to 64KB
    l2_sizes = [2**i * 1024 for i in range(5, 11)]  # 32KB to 1MB
    
    aats = []
    valid_l1_sizes = []
    valid_l2_sizes = []
    
    for l1_size in l1_sizes:
        for l2_size in l2_sizes:
            if l1_size < l2_size:
                output = run_cache_sim(l1_size, 4, 0, l2_size, 8)
                aat = parse_output(output, "aat")
                if aat is not None:
                    aats.append(aat)
                    valid_l1_sizes.append(l1_size)
                    valid_l2_sizes.append(l2_size)
    
    fig = plt.figure(figsize=(12, 8))
    ax = fig.add_subplot(111, projection='3d')
    ax.scatter(np.log2(valid_l2_sizes), np.log2(valid_l1_sizes), aats)
    
    ax.set_xlabel('log2(L2 SIZE)')
    ax.set_ylabel('log2(L1 SIZE)')
    ax.set_zlabel('Average Access Time (AAT)')
    ax.set_title('Plot #5: AAT vs L1 and L2 Cache Sizes')
    plt.savefig('plot5.png')
    plt.close()

def generate_plot6():
    l1_sizes = [2**i * 1024 for i in range(0, 7)]  # 1KB to 64KB
    l2_sizes = [2**i * 1024 for i in range(5, 11)]  # 32KB to 1MB
    
    edps = []
    valid_l1_sizes = []
    valid_l2_sizes = []
    
    for l1_size in l1_sizes:
        for l2_size in l2_sizes:
            if l1_size < l2_size:
                output = run_cache_sim(l1_size, 4, 0, l2_size, 8)
                edp = parse_output(output, "edp")
                if edp is not None:
                    edps.append(edp)
                    valid_l1_sizes.append(l1_size)
                    valid_l2_sizes.append(l2_size)
    
    fig = plt.figure(figsize=(12, 8))
    ax = fig.add_subplot(111, projection='3d')
    ax.scatter(np.log2(valid_l2_sizes), np.log2(valid_l1_sizes), edps)
    
    ax.set_xlabel('log2(L2 SIZE)')
    ax.set_ylabel('log2(L1 SIZE)')
    ax.set_zlabel('Energy-Delay Product (EDP)')
    ax.set_title('Plot #6: EDP vs L1 and L2 Cache Sizes')
    plt.savefig('plot6.png')
    plt.close()

def generate_plot7():
    sizes = [2**i * 1024 for i in range(0, 6)]  # 1KB to 32KB
    configurations = [
        (1, 0),   # Direct-mapped, no VC
        (1, 4),   # Direct-mapped, 4-entry VC
        (1, 8),   # Direct-mapped, 8-entry VC
        (1, 16),  # Direct-mapped, 16-entry VC
        (2, 0),   # 2-way, no VC
        (4, 0),   # 4-way, no VC
    ]
    labels = [
        "Direct-mapped, no VC",
        "Direct-mapped, 4-entry VC",
        "Direct-mapped, 8-entry VC",
        "Direct-mapped, 16-entry VC",
        "2-way, no VC",
        "4-way, no VC"
    ]
    
    aats = {config: [] for config in configurations}
    valid_sizes = {config: [] for config in configurations}
    
    for size in sizes:
        for config in configurations:
            assoc, vc_entries = config
            output = run_cache_sim(size, assoc, vc_entries, 256*1024, 8)
            aat = parse_output(output, "aat")
            if aat is not None:
                aats[config].append(aat)
                valid_sizes[config].append(size)
    
    plt.figure(figsize=(12, 6))
    for config, label in zip(configurations, labels):
        plt.plot(np.log2(valid_sizes[config]), aats[config], marker='o', label=label)
    
    plt.xlabel('log2(L1 SIZE)')
    plt.ylabel('Average Access Time (AAT)')
    plt.title('Plot #7: AAT vs L1 Cache Size for Different Configurations')
    plt.legend()
    plt.grid(True)
    plt.savefig('plot7.png')
    plt.close()

if __name__ == "__main__":
    generate_plot1()
    generate_plot2()
    generate_plot3()
    generate_plot4()
    generate_plot5()
    generate_plot6()
    generate_plot7()
    print("All plots have been generated successfully.")
