import subprocess
import numpy as np
import itertools
import re

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

def run_cache_sim(l1_size, l1_assoc, vc_entries, l2_size, l2_assoc, block_size):
    command = f"./cache_sim {l1_size} {l1_assoc} {block_size} {vc_entries} {l2_size} {l2_assoc} gcc_trace.txt"
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    output = result.stdout
    
    # Parse the output to extract required statistics
    l1_miss_rate = parse_output(output, "miss_rate") 
    aat = parse_output(output, "aat")
    edp = parse_output(output, "edp") 
    return l1_miss_rate, aat, edp

def generate_plot1_data():
    sizes = [2**i * 1024 for i in range(1, 11)]  # 2KB to 1MB
    assocs = [1, 2, 4, 8, -1]  # -1 represents fully associative
    data = {assoc: [] for assoc in assocs}
    
    for size in sizes:
        for assoc in assocs:
            l1_miss_rate, _, _ = run_cache_sim(size, assoc, 0, 0, 0, 32)
            data[assoc].append((np.log2(size), l1_miss_rate))
    
    return data

def generate_plot2_data():
    # Similar to plot1, but return AAT instead of miss rate
    sizes = [2**i * 1024 for i in range(1, 11)]  # 2KB to 1MB
    assocs = [1, 2, 4, 8, -1]  # -1 represents fully associative
    data = {assoc: [] for assoc in assocs}
    
    for size in sizes:
        for assoc in assocs:
            _, aat, _ = run_cache_sim(size, assoc, 0, 0, 0, 32)
            data[assoc].append((np.log2(size), aat))
    
    return data

def generate_plot3_data():
    sizes = [2**i * 1024 for i in range(1, 8)]  # 2KB to 128KB
    assocs = [1, 2, 4, 8, -1]  # -1 represents fully associative
    data = {assoc: [] for assoc in assocs}
    
    for size in sizes:
        for assoc in assocs:
            _, aat, _ = run_cache_sim(size, assoc, 0, 256*1024, 8, 32)
            data[assoc].append((np.log2(size), aat))
    
    return data

def generate_plot4_data():
    sizes = [2**i * 1024 for i in range(0, 6)]  # 1KB to 32KB
    block_sizes = [16, 32, 64, 128]
    data = {size: [] for size in sizes}
    
    for size in sizes:
        for block_size in block_sizes:
            l1_miss_rate, _, _ = run_cache_sim(size, 4, 0, 0, 0, block_size)
            data[size].append((np.log2(block_size), l1_miss_rate))
    
    return data

def generate_plot5_data():
    l1_sizes = [2**i * 1024 for i in range(0, 7)]  # 1KB to 64KB
    l2_sizes = [2**i * 1024 for i in range(5, 11)]  # 32KB to 1MB
    data = []
    
    for l1_size, l2_size in itertools.product(l1_sizes, l2_sizes):
        if l1_size < l2_size:
            _, aat, _ = run_cache_sim(l1_size, 4, 0, l2_size, 8, 32)
            data.append((np.log2(l1_size), np.log2(l2_size), aat))
    
    return data

def generate_plot6_data():
    # Similar to plot5, but return EDP instead of AAT
    l1_sizes = [2**i * 1024 for i in range(0, 7)]  # 1KB to 64KB
    l2_sizes = [2**i * 1024 for i in range(5, 11)]  # 32KB to 1MB
    data = []
    
    for l1_size, l2_size in itertools.product(l1_sizes, l2_sizes):
        if l1_size < l2_size:
            _, _, edp = run_cache_sim(l1_size, 4, 0, l2_size, 8, 32)
            data.append((np.log2(l1_size), np.log2(l2_size), edp))
    
    return data

def generate_plot7_data():
    sizes = [2**i * 1024 for i in range(0, 6)]  # 1KB to 32KB
    configs = [
        (1, 0), (1, 4), (1, 8), (1, 16),  # Direct-mapped with different VC entries
        (2, 0), (4, 0)  # 2-way and 4-way set-associative without VC
    ]
    data = {config: [] for config in configs}
    
    for size in sizes:
        for assoc, vc_entries in configs:
            _, aat, _ = run_cache_sim(size, assoc, vc_entries, 256*1024, 8, 32)
            data[(assoc, vc_entries)].append((np.log2(size), aat))
    
    return data

def generate_latex_plot(plot_number, data, xlabel, ylabel, title):
    latex_code = f"""
\\begin{{figure}}[htbp]
\\centering
\\begin{{tikzpicture}}
\\begin{{axis}}[
    width=0.8\\textwidth,
    height=0.6\\textwidth,
    xlabel={{{xlabel}}},
    ylabel={{{ylabel}}},
    title={{{title}}},
    legend pos=north east,
    legend style={{font=\\tiny}},
    grid=major
]
"""
    
    if plot_number in [1, 2, 3, 4, 7]:
        for label, points in data.items():
            x, y = zip(*points)
            latex_code += f"\\addplot coordinates {{{' '.join([f'({xi},{yi})' for xi, yi in zip(x, y)])}}};\n"
            latex_code += f"\\addlegendentry{{{label}}}\n"
    elif plot_number in [5, 6]:
        x, y, z = zip(*data)
        latex_code += f"\\addplot3[surf] coordinates {{{' '.join([f'({xi},{yi},{zi})' for xi, yi, zi in zip(x, y, z)])}}};\n"
    
    latex_code += """
\\end{axis}
\\end{tikzpicture}
\\caption{""" + title + """}
\\end{figure}
"""
    
    return latex_code

def main():
    plots = [
        (1, generate_plot1_data(), "log2(SIZE)", "L1 Miss Rate", "L1 Miss Rate vs Cache Size"),
        (2, generate_plot2_data(), "log2(SIZE)", "AAT", "Average Access Time vs Cache Size"),
        (3, generate_plot3_data(), "log2(L1 SIZE)", "AAT", "Average Access Time vs L1 Cache Size (with L2)"),
        (4, generate_plot4_data(), "log2(BLOCKSIZE)", "L1 Miss Rate", "L1 Miss Rate vs Block Size"),
        (5, generate_plot5_data(), "log2(L1 SIZE)", "log2(L2 SIZE)", "AAT vs L1 and L2 Cache Sizes"),
        (6, generate_plot6_data(), "log2(L1 SIZE)", "log2(L2 SIZE)", "EDP vs L1 and L2 Cache Sizes"),
        (7, generate_plot7_data(), "log2(L1 SIZE)", "AAT", "AAT vs L1 Cache Size (with Victim Cache)")
    ]
    
    latex_output = "\\documentclass{article}\n\\usepackage{pgfplots}\n\\pgfplotsset{compat=1.18}\n\\begin{document}\n"
    
    for plot_number, data, xlabel, ylabel, title in plots:
        latex_output += generate_latex_plot(plot_number, data, xlabel, ylabel, title)
    
    latex_output += "\\end{document}"
    
    with open("cache_simulation_plots.tex", "w") as f:
        f.write(latex_output)

if __name__ == "__main__":
    main()
