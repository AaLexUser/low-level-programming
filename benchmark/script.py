import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import os
import sys
import subprocess

root_path = "/Users/aleksei/ITMO/LLP-2023-new/low-level-programming/cmake-build-benchmark/benchmark/"
def plot(df, x, y, title, xlabel, ylabel, filename):
    # Plot the data
    plt.figure(figsize=(10, 6))
    plt.scatter(df[x], df[y], s=15)
    plt.plot(df[x], df[y])
    plt.xlabel(xlabel)
    plt.ylabel(ylabel)
    plt.grid(True)
    plt.minorticks_on()
    plt.grid(which='minor', linestyle='-', color='#999999', alpha=0.2)
    plt.title(title)
    plt.savefig(filename)
    plt.show()

def linked_blocks_mem():
    # subprocess.run(["./benchmark/bench_linked_blocks_mem"])
    df = pd.read_csv(root_path + 'linked_blocks_mem.csv'
                     , sep=';')
    # Plot the data
    plot(df, 'BlocksCount', 'FileSize', 'Linked Blocks Memory Usage', 'Blocks count', 'File size',
         'linked_blocks_mem.png')

def linked_blocks_wide_mem():
    # subprocess.run(["./benchmark/bench_linked_blocks_wide_mem"])
    df = pd.read_csv(root_path + 'linked_blocks_wide_mem.csv'
                     , sep=';')
    # Plot the data
    plot(df, 'BlocksCount', 'FileSize', 'Linked Blocks Memory Usage', 'Blocks count', 'File size',
         'linked_blocks_wide_mem.png')

def linked_blocks_insert():
    # subprocess.run(["./benchmark/bench_linked_blocks_insert"])
    df = pd.read_csv(root_path + 'linked_blocks_insert.csv'
                     , sep=';')
    # Plot the data
    plot(df, 'BlocksCount', 'Time', 'Linked Blocks Insertion Time', 'Blocks count', 'Time mcs',
         'linked_blocks_insert.png')

def linked_blocks_wide_insert():
    # subprocess.run(["./benchmark/bench_linked_blocks_wide_insert"])
    df = pd.read_csv(root_path + 'linked_blocks_wide_insert.csv'
                     , sep=';')
    # Plot the data
    plot(df, 'BlocksCount', 'Time', 'Linked Blocks Wide Insertion Time', 'Blocks count', 'Time mcs',
         'linked_blocks_wide_insert.png')

def table_insert():
    # subprocess.run(["./benchmark/bench_table_insert"])
    df = pd.read_csv(root_path + 'table-insert.csv'
                     , sep=';')
    # Plot the data
    plot(df, 'Rows', 'Time', 'Table Insertion Time', 'Rows count', 'Time mcs',
         'table_insert.png')

def table_mem():
    # subprocess.run(["./benchmark/bench_table_mem"])
    df = pd.read_csv(root_path + 'table-mem.csv'
                     , sep=';')
    # Plot the data
    plot(df, 'Rows', 'FileSize', 'Table Memory Usage', 'Rows count', 'File size',
         'table_mem.png')

def table_update():
    # subprocess.run(["./benchmark/bench_table_update"])
    df = pd.read_csv(root_path + 'table-update.csv'
                     , sep=';')
    # Plot the data
    plot(df, 'Allocated', 'Time', 'Table Update Time', 'Rows count', 'Time mcs',
         'table_update.png')

def table_delete():
    # subprocess.run(["./benchmark/bench_table_delete"])
    df = pd.read_csv(root_path + 'table-delete.csv'
                     , sep=';')
    # Plot the data
    plot(df, 'Allocated', 'Time', 'Table Delete Time', 'Allocated', 'Time mcs',
         'table_delete.png')

def table_select():
    # subprocess.run(["./benchmark/bench_table_delete"])
    df = pd.read_csv(root_path + 'table-select.csv'
                     , sep=';')
    # Plot the data
    plot(df, 'Allocated', 'Time', 'Table Select Time', 'Allocated', 'Time mcs',
         'table_select.png')

def table_ram():
    # subprocess.run(["./benchmark/bench_table_mem"])
    df = pd.read_csv(root_path + 'table-ram.csv'
                     , sep=';')
    # Plot the data
    plot(df, 'Rows', 'RAMSize', 'RAM Memory Usage', 'Rows count', 'RAM size',
         'table_mem.png')



if __name__ == '__main__':
    # Run the benchmark
    build_dir = '/Users/aleksei/ITMO/LLP-2023/LLP-Lab1/cmake-build-benchmark/'
    os.chdir(build_dir)
    # subprocess.run(["cmake", "-DCMAKE_BUILD_TYPE=Debug", "-DBUILD_TESTING=OFF", "-DBUILD_BENCHMARK=ON" ".."])
    # subprocess.run(["cmake", "--build", "."])

    # Run the benchmark
    table_insert()