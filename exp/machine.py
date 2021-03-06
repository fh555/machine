#!/usr/bin/python
# encoding: utf-8

###################################################################################
# MACHINE EXPERIMENTS
###################################################################################

import time
import argparse
import csv
import logging
import math
import os
import subprocess
import pprint
import matplotlib
import numpy as np
import re
import shutil
import datetime
import uuid
import multiprocessing
import itertools

matplotlib.use('Agg')
import pylab
import matplotlib.pyplot as plot
from matplotlib.backends.backend_pdf import PdfPages
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LinearLocator

###################################################################################
# LOGGING CONFIGURATION
###################################################################################
LOG = logging.getLogger(__name__)
LOG_handler = logging.StreamHandler()
LOG_formatter = logging.Formatter(
    fmt='%(asctime)s [%(funcName)s:%(lineno)03d] %(levelname)-5s: %(message)s',
    datefmt='%m-%d-%Y %H:%M:%S'
)
LOG_handler.setFormatter(LOG_formatter)
LOG.addHandler(LOG_handler)
LOG.setLevel(logging.INFO)

MAJOR_STRING = "++++++++++++++++++++++++++++\n"
MINOR_STRING = "----------------"
SUB_MINOR_STRING = "******"

###################################################################################
# OUTPUT CONFIGURATION
###################################################################################

BASE_DIR = os.path.dirname(__file__)
OPT_GRAPH_HEIGHT = 150
OPT_GRAPH_WIDTH = 400

# Make a list by cycling through the colors you care about
# to match the length of your data.
NUM_COLORS = 5

COLOR_MAP_1 = ( '#556270', '#4ECDC4', '#C44D58', '#FF6B6B', '#C7F464')
COLOR_MAP_2 = ( '#418259', '#bd5632', '#e1a94c', '#7d6c5b', '#364d38', '#c4e1c6')
COLOR_MAP_3 = ( '#262626', '#FECEA8', '#67abb8', '#f15b40')
COLOR_MAPS = (COLOR_MAP_1, COLOR_MAP_2, COLOR_MAP_3)

OPT_GRID_COLOR = '#EEEEEE' # gray
OPT_LEGEND_SHADOW = False
OPT_MARKERS = (['o', 's', 'v', "^", "h", "v", ">", "x", "d", "<", "|", "", "|", "_"])

PATTERNS_1 = ([ "////", "o", "\\\\" , ".", "\\\\\\"])
PATTERNS_2 = ([ "\\\\" , ".", "////", "o", "\\\\\\"])
PATTERNS_3 = ([ "////", "o", "\\\\" , ".", "\\\\\\"])

PATTERNS = (PATTERNS_1, PATTERNS_2, PATTERNS_3)

OPT_STACK_COLORS = ('#2b3742', '#c9b385', '#610606', '#1f1501')
OPT_LINE_STYLES= ('-', ':', '--', '-.')

# SET STYLE AND FONT

# bmh, ggplot, fivethirtyeight
# seaborn-deep, seaborn-pastel and seaborn-white
OPT_STYLE = 'bmh'

OPT_LABEL_WEIGHT = 'bold'
OPT_LINE_WIDTH = 3.0
OPT_MARKER_SIZE = 5.0

AXIS_LINEWIDTH = 1.3
BAR_LINEWIDTH = 1.2

LABEL_FONT_SIZE = 16
TICK_FONT_SIZE = 14
TINY_FONT_SIZE = 10
LEGEND_FONT_SIZE = 18

SMALL_LABEL_FONT_SIZE = 10
SMALL_LEGEND_FONT_SIZE = 10

XAXIS_MIN = 0.25
XAXIS_MAX = 3.75

# SET STYLE
#plot.style.use(OPT_STYLE)

# SET TYPE1 FONTS
matplotlib.rcParams['ps.useafm'] = True
matplotlib.rcParams['pdf.use14corefonts'] = True
matplotlib.rcParams['text.usetex'] = True
matplotlib.rcParams['text.latex.preamble']= [
    r'\usepackage{helvet}',    # set the normal font here
    r'\usepackage{sansmath}',  # load up the sansmath so that math -> helvet
    r'\sansmath'               # <- tricky! -- gotta actually tell tex to use!
]
matplotlib.rcParams['font.serif'] = 'Ubuntu'
matplotlib.rcParams['font.monospace'] = 'Ubuntu Mono'

LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE, weight='bold')
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)
TINY_FP = FontProperties(style='normal', size=TINY_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE, weight='bold')

SMALL_LABEL_FP = FontProperties(style='normal', size=SMALL_LABEL_FONT_SIZE, weight='bold')
SMALL_LEGEND_FP = FontProperties(style='normal', size=SMALL_LEGEND_FONT_SIZE, weight='bold')

YAXIS_TICKS = 5
YAXIS_ROUND = 1000.0

EXIT_FAILURE = 1

###################################################################################
# CONFIGURATION
###################################################################################

## PROGRAM DIRS
BASE_DIR = os.path.dirname(os.path.realpath(__file__))

BUILD_DIR = BASE_DIR + "/../build/test/"
BENCHMARK_DIR = BASE_DIR + "/../traces/"
PROGRAM_NAME = BUILD_DIR + "machine"
JOBS = []

## HIERARCHY TYPES
HIERARCHY_TYPE_NVM = 1
HIERARCHY_TYPE_DRAM_NVM = 2
HIERARCHY_TYPE_DRAM_DISK = 3
HIERARCHY_TYPE_NVM_DISK = 4
HIERARCHY_TYPE_DRAM_NVM_DISK = 5

HIERARCHY_TYPES_STRINGS = {
    1 : "nvm",
    2 : "dram-nvm",
    3 : "dram-disk",
    4 : "nvm-disk",
    5 : "dram-nvm-disk"
}

HIERARCHY_TYPES = [
#    HIERARCHY_TYPE_NVM,
#    HIERARCHY_TYPE_DRAM_NVM,
    HIERARCHY_TYPE_DRAM_DISK,
    HIERARCHY_TYPE_NVM_DISK,
    HIERARCHY_TYPE_DRAM_NVM_DISK
]

## SIZE TYPES
SIZE_TYPE_1 = 1
SIZE_TYPE_2 = 2
SIZE_TYPE_3 = 3
SIZE_TYPE_4 = 4

SIZE_TYPES = [
    SIZE_TYPE_1,
    SIZE_TYPE_2,
    SIZE_TYPE_3,
    SIZE_TYPE_4
]

## SIZE RATIO TYPES
SIZE_RATIO_TYPE_1 = 1
SIZE_RATIO_TYPE_2 = 2
SIZE_RATIO_TYPE_3 = 3
SIZE_RATIO_TYPE_4 = 4

SIZE_RATIO_TYPES = [
    SIZE_RATIO_TYPE_1,
    SIZE_RATIO_TYPE_2,
    SIZE_RATIO_TYPE_3,
    SIZE_RATIO_TYPE_4
]

## LATENCY TYPES
LATENCY_TYPE_1 = 1
LATENCY_TYPE_2 = 2
LATENCY_TYPE_3 = 3
LATENCY_TYPE_4 = 4
LATENCY_TYPE_5 = 5
LATENCY_TYPE_6 = 6
LATENCY_TYPE_7 = 7
LATENCY_TYPE_8 = 8

LATENCY_TYPES = [
    LATENCY_TYPE_1,
    LATENCY_TYPE_2,
    LATENCY_TYPE_3,
    LATENCY_TYPE_4,
    LATENCY_TYPE_5,
    LATENCY_TYPE_6,
    LATENCY_TYPE_7,
    LATENCY_TYPE_8
]

LATENCY_TYPES_STRINGS = {
    1 : "1x-1x",
    2 : "2x-2x",
    3 : "2x-4x",
    4 : "2x-8x",
    5 : "4x-4x",
    6 : "4x-8x",
    7 : "10x-20x",
    8 : "20x-40x"
}

## CACHING TYPES
CACHING_TYPE_FIFO = 1
CACHING_TYPE_LFU = 2
CACHING_TYPE_LRU = 3
CACHING_TYPE_ARC = 4

CACHING_TYPES_STRINGS = {
    1 : "fifo",
    2 : "lfu",
    3 : "lru",
    4 : "arc"
}

CACHING_TYPES = [
    CACHING_TYPE_FIFO,
    CACHING_TYPE_LFU,
    CACHING_TYPE_LRU,
    CACHING_TYPE_ARC
]

## DISK MODE TYPES
DISK_MODE_TYPE_SSD = 1
DISK_MODE_TYPE_HDD = 2

## BENCHMARK TYPES
BENCHMARK_TYPE_EXAMPLE = 0
BENCHMARK_TYPE_TPCC = 1
BENCHMARK_TYPE_YCSB = 2
BENCHMARK_TYPE_VOTER = 3
BENCHMARK_TYPE_YCSB_READ = 4
BENCHMARK_TYPE_YCSB_INSERT = 5
BENCHMARK_TYPE_CHBENCHMARK = 6
BENCHMARK_TYPE_AUCTIONMARK = 7
BENCHMARK_TYPE_SMALLBANK = 8

BENCHMARK_TYPES_STRINGS = {
    0 : "example",
    1 : "tpcc",
    2 : "ycsb",
    3 : "voter",
    4 : "ycsb-read",
    5 : "ycsb-insert",
    6 : "chbenchmark",
    7 : "auctionmark",
    8 : "smallbank"
}

BENCHMARK_TYPES_DIRS = {
    0 : BENCHMARK_DIR + BENCHMARK_TYPES_STRINGS[0] + ".txt",
    1 : BENCHMARK_DIR + BENCHMARK_TYPES_STRINGS[1] + ".txt",
    2 : BENCHMARK_DIR + BENCHMARK_TYPES_STRINGS[2] + ".txt",
    3 : BENCHMARK_DIR + BENCHMARK_TYPES_STRINGS[3] + ".txt",
    4 : BENCHMARK_DIR + BENCHMARK_TYPES_STRINGS[4] + ".txt",
    5 : BENCHMARK_DIR + BENCHMARK_TYPES_STRINGS[5] + ".txt",
    6 : BENCHMARK_DIR + BENCHMARK_TYPES_STRINGS[6] + ".txt",
    7 : BENCHMARK_DIR + BENCHMARK_TYPES_STRINGS[7] + ".txt",
    8 : BENCHMARK_DIR + BENCHMARK_TYPES_STRINGS[8] + ".txt",
}

BENCHMARK_TYPES = [
#    BENCHMARK_TYPE_EXAMPLE,
    BENCHMARK_TYPE_TPCC,
    BENCHMARK_TYPE_VOTER,
    BENCHMARK_TYPE_YCSB,
    BENCHMARK_TYPE_CHBENCHMARK,
    BENCHMARK_TYPE_AUCTIONMARK
]

## OUTPUT

THROUGHPUT_OFFSET = 0

## DEFAULTS

SCALE_FACTOR = 50

DEFAULT_DURATION = 10
DEFAULT_HIERARCHY_TYPE = HIERARCHY_TYPE_NVM
DEFAULT_SIZE_TYPE = SIZE_TYPE_2
DEFAULT_SIZE_RATIO_TYPE = SIZE_RATIO_TYPE_3
DEFAULT_CACHING_TYPE = CACHING_TYPE_LRU
DEFAULT_LATENCY_TYPE = LATENCY_TYPE_2
DEFAULT_DISK_MODE_TYPE = DISK_MODE_TYPE_SSD
DEFAULT_BENCHMARK_TYPE = BENCHMARK_TYPE_TPCC
DEFAULT_MIGRATION_FREQUENCY = 3
DEFAULT_OPERATION_COUNT = 100000 * SCALE_FACTOR
DEFAULT_SUMMARY_FILE = "outputfile.txt"

## EXPERIMENTS
LATENCY_EXPERIMENT = 1
SIZE_EXPERIMENT = 2
CACHE_EXPERIMENT = 3
SIZE_RATIO_EXPERIMENT = 4
HARD_DISK_EXPERIMENT = 5

## SUMMARY FILES
LATENCY_EXPERIMENT_SUMMARY = "latency.txt"
SIZE_EXPERIMENT_SUMMARY = "size.txt"
CACHE_EXPERIMENT_SUMMARY = "cache.txt"
SIZE_RATIO_EXPERIMENT_SUMMARY = "size_ratio.txt"
HARD_DISK_EXPERIMENT_SUMMARY = "hard_disk.txt"

## EVAL DIRS
LATENCY_DIR = BASE_DIR + "/results/latency"
SIZE_DIR = BASE_DIR + "/results/size"
CACHE_DIR = BASE_DIR + "/results/cache"
SIZE_RATIO_DIR = BASE_DIR + "/results/size_ratio"
HARD_DISK_DIR = BASE_DIR + "/results/hard_disk"

## PLOT DIRS
LEGEND_PLOT_DIR = BASE_DIR + "/images/legend/"
LATENCY_PLOT_DIR = BASE_DIR + "/images/latency/"
SIZE_PLOT_DIR = BASE_DIR + "/images/size/"
CACHE_PLOT_DIR = BASE_DIR + "/images/cache/"
SIZE_RATIO_PLOT_DIR = BASE_DIR + "/images/size_ratio/"
HARD_DISK_PLOT_DIR = BASE_DIR + "/images/hard_disk/"

## LATENCY EXPERIMENT

LATENCY_EXP_BENCHMARK_TYPES = BENCHMARK_TYPES
LATENCY_EXP_HIERARCHY_TYPES = HIERARCHY_TYPES
LATENCY_EXP_SIZE_TYPES = [DEFAULT_SIZE_TYPE]
LATENCY_EXP_LATENCY_TYPES = LATENCY_TYPES
LATENCY_EXP_CACHING_TYPES = [DEFAULT_CACHING_TYPE]

## SIZE EXPERIMENT

SIZE_EXP_BENCHMARK_TYPES = BENCHMARK_TYPES
SIZE_EXP_HIERARCHY_TYPES = HIERARCHY_TYPES
SIZE_EXP_SIZE_TYPES = SIZE_TYPES
SIZE_EXP_LATENCY_TYPES = [DEFAULT_LATENCY_TYPE]
SIZE_EXP_CACHING_TYPES = [DEFAULT_CACHING_TYPE]

## CACHE EXPERIMENT

CACHE_EXP_BENCHMARK_TYPES = BENCHMARK_TYPES
CACHE_EXP_HIERARCHY_TYPES = [HIERARCHY_TYPE_DRAM_NVM_DISK]
CACHE_EXP_SIZE_TYPES = [DEFAULT_SIZE_TYPE]
CACHE_EXP_LATENCY_TYPES = [DEFAULT_LATENCY_TYPE]
CACHE_EXP_CACHING_TYPES = CACHING_TYPES

## SIZE RATIO EXPERIMENT

SIZE_RATIO_EXP_BENCHMARK_TYPES = BENCHMARK_TYPES
SIZE_RATIO_EXP_HIERARCHY_TYPES = HIERARCHY_TYPES
SIZE_RATIO_EXP_SIZE_RATIO_TYPES = SIZE_RATIO_TYPES
SIZE_RATIO_EXP_LATENCY_TYPES = [DEFAULT_LATENCY_TYPE]
SIZE_RATIO_EXP_CACHING_TYPES = [DEFAULT_CACHING_TYPE]

## HARD DISK EXPERIMENT

HARD_DISK_EXP_BENCHMARK_TYPES = BENCHMARK_TYPES
HARD_DISK_EXP_HIERARCHY_TYPES = HIERARCHY_TYPES
HARD_DISK_EXP_SIZE_TYPES = [DEFAULT_SIZE_TYPE]
HARD_DISK_EXP_LATENCY_TYPES = LATENCY_TYPES
HARD_DISK_EXP_CACHING_TYPES = [DEFAULT_CACHING_TYPE]
HARD_DISK_EXPERIMENT_DISK_MODE_TYPE = DISK_MODE_TYPE_HDD

## CSV FILES

LATENCY_CSV = "latency.csv"
SIZE_CSV = "size.csv"
CACHE_CSV = "cache.csv"
SIZE_RATIO_CSV = "size_ratio.csv"
HARD_DISK_CSV = "hard_disk.csv"

###################################################################################
# UTILS
###################################################################################

def chunks(l, n):
    """ Yield successive n-sized chunks from l.
    """
    for i in range(0, len(l), n):
        yield l[i:i + n]

def loadDataFile(path):
    file = open(path, "r")
    reader = csv.reader(file)

    data = []

    row_num = 0
    for row in reader:
        row_data = []
        column_num = 0
        for col in row:
            row_data.append(float(col))
            column_num += 1
        row_num += 1
        data.append(row_data)

    return data

def next_power_of_10(n):
    return (10 ** math.ceil(math.log(n, 10)))

def get_upper_bound(n):
    return (math.ceil(n / YAXIS_ROUND) * YAXIS_ROUND)

## MAKE GRID
def makeGrid(ax):
    for axis in ['top','bottom','left','right']:
            ax.spines[axis].set_linewidth(AXIS_LINEWIDTH)
    ax.set_axisbelow(True)

## SAVE GRAPH
def saveGraph(fig, output, width, height):
    size = fig.get_size_inches()
    dpi = fig.get_dpi()
    LOG.debug("Current Size Inches: %s, DPI: %d" % (str(size), dpi))

    new_size = (width / float(dpi), height / float(dpi))
    fig.set_size_inches(new_size)
    new_size = fig.get_size_inches()
    new_dpi = fig.get_dpi()
    LOG.debug("New Size Inches: %s, DPI: %d" % (str(new_size), new_dpi))

    pp = PdfPages(output)
    fig.savefig(pp, format='pdf', bbox_inches='tight')
    pp.close()
    LOG.info("OUTPUT: %s", output)

def get_result_file(base_result_dir, result_dir_list, result_file_name):

    # Start with result dir
    final_result_dir = base_result_dir + "/"

    # Add each entry in the list as a sub-dir
    for result_dir_entry in result_dir_list:
        final_result_dir += result_dir_entry + "/"

    # Create dir if needed
    if not os.path.exists(final_result_dir):
        os.makedirs(final_result_dir)

    # Add local file name
    result_file_name = final_result_dir + result_file_name

    #pprint.pprint(result_file_name)
    return result_file_name

###################################################################################
# LEGEND
###################################################################################

def create_legend_hierarchy_type_line(color_mode):
    fig = pylab.figure()
    ax1 = fig.add_subplot(111)

    LOG.info("Creating hierarchy type line");

    LEGEND_VALUES = HIERARCHY_TYPES

    figlegend = pylab.figure(figsize=(13, 0.5))
    idx = 0
    lines = [None] * (len(LEGEND_VALUES) + 1)
    data = [1]
    x_values = [1]

    TITLE = "STORAGE HIERARCHY TYPES:"
    LABELS = [TITLE, "DRAM-DISK", "NVM-DISK", "DRAM-NVM-DISK"]

    lines[idx], = ax1.plot(x_values, data, linewidth = 0)
    idx = 1

    for group in range(len(LEGEND_VALUES)):
        lines[idx], = ax1.plot(x_values, data,
                               color=COLOR_MAPS[color_mode][idx - 1],
                               linewidth=OPT_LINE_WIDTH,
                               marker=OPT_MARKERS[idx - 1],
                               markersize=OPT_MARKER_SIZE)
        idx = idx + 1

    # LEGEND
    figlegend.legend(lines, LABELS, prop=LEGEND_FP,
                     loc=1, ncol=4,
                     mode="expand", shadow=OPT_LEGEND_SHADOW,
                     frameon=False, borderaxespad=0.0,
                     handleheight=1, handlelength=3)

    figlegend.savefig(LEGEND_PLOT_DIR + 'legend_hierarchy_type_line_'
                      + str(color_mode) + '.pdf')


def create_legend_hierarchy_type_bar(color_mode):
    fig = pylab.figure()
    ax1 = fig.add_subplot(111)

    LOG.info("Creating hierarchy type bar");

    LEGEND_VALUES = HIERARCHY_TYPES

    figlegend = pylab.figure(figsize=(13, 0.5))

    num_items = len(LEGEND_VALUES) + 1;
    ind = np.arange(1)
    margin = 0.10
    width = (1.0 - 2 * margin) / num_items
    idx = 0
    data = [1]

    bars = [None] * len(LEGEND_VALUES) * num_items

    TITLE = "STORAGE HIERARCHY TYPES:"
    LABELS = [TITLE, "DRAM-DISK", "NVM-DISK", "DRAM-NVM-DISK"]

    bars[idx], = ax1.bar(0, data, 0, color = 'w', linewidth = 0)
    idx = 1

    for group in xrange(len(LABELS)):
        bars[idx], = ax1.bar(ind + margin + (idx * width),
                              data,
                              width,
                              color=COLOR_MAPS[color_mode][idx - 1],
                              hatch=PATTERNS[color_mode][idx - 1],
                              linewidth=BAR_LINEWIDTH)
        idx = idx + 1

    # LEGEND
    figlegend.legend(bars, LABELS, prop=LEGEND_FP,
                     loc=1, ncol=4,
                     mode="expand", shadow=OPT_LEGEND_SHADOW,
                     frameon=False, borderaxespad=0.0,
                     handleheight=1, handlelength=3)

    figlegend.savefig(LEGEND_PLOT_DIR + 'legend_hierarchy_type_bar_'
                      + str(color_mode) + '.pdf')

###################################################################################
# PLOT
###################################################################################

def get_label(label):
    bold_label = "\\textbf{" + label + "}"
    return bold_label

def create_latency_line_chart(datasets, color_mode):
    fig = plot.figure()
    ax1 = fig.add_subplot(111)

    # X-AXIS
    x_values = [LATENCY_TYPES_STRINGS[i] for i in LATENCY_EXP_LATENCY_TYPES]
    N = len(x_values)
    ind = np.arange(N)

    idx = 0
    for group in range(len(datasets)):
        # GROUP
        y_values = []
        for line in  range(len(datasets[group])):
            for col in  range(len(datasets[group][line])):
                if col == 1:
                    y_values.append(datasets[group][line][col])
        LOG.info("group_data = %s", str(y_values))
        ax1.plot(ind + 0.5, y_values,
                 color=COLOR_MAPS[color_mode][idx],
                 linewidth=OPT_LINE_WIDTH,
                 marker=OPT_MARKERS[idx],
                 markersize=OPT_MARKER_SIZE,
                 label=str(group))
        idx = idx + 1

    # GRID
    makeGrid(ax1)

    # Y-AXIS
    #YAXIS_MIN = 1
    #YAXIS_MAX = 10**7
    ax1.yaxis.set_major_locator(LinearLocator(YAXIS_TICKS))
    ax1.minorticks_off()
    ax1.set_ylabel(get_label('Throughput (ops)'), fontproperties=LABEL_FP)
    #ax1.set_ylim(bottom=YAXIS_MIN, top=YAXIS_MAX)
    #ax1.set_yscale('log', nonposy='clip')

    # X-AXIS
    XAXIS_MIN = 0.25
    XAXIS_MAX = 7.75
    ax1.set_xticks(ind + 0.5)
    ax1.set_xlabel(get_label('Latency Types'), fontproperties=LABEL_FP)
    ax1.set_xticklabels(x_values)
    ax1.set_xlim([XAXIS_MIN, XAXIS_MAX])

    for label in ax1.get_yticklabels() :
        label.set_fontproperties(TICK_FP)
    for label in ax1.get_xticklabels() :
        label.set_fontproperties(TICK_FP)

    return fig

def create_size_bar_chart(datasets, color_mode):
    fig = plot.figure()
    ax1 = fig.add_subplot(111)

    # X-AXIS
    x_values = [str(chr(ord('A') + i - 1)) for i in SIZE_EXP_SIZE_TYPES]
    N = len(x_values)
    M = 3
    ind = np.arange(N)
    margin = 0.1
    width = (1.-2.*margin)/M
    bars = [None] * N

    idx = 0
    for group in range(len(datasets)):
        # GROUP
        y_values = []
        for line in  range(len(datasets[group])):
            for col in  range(len(datasets[group][line])):
                if col == 1:
                    y_values.append(datasets[group][line][col])
        LOG.info("group_data = %s", str(y_values))
        bars[group] =  ax1.bar(ind + margin + (group * width),
                               y_values, width,
                               color=COLOR_MAPS[color_mode][group],
                               hatch=PATTERNS[color_mode][group],
                               linewidth=BAR_LINEWIDTH)
        idx = idx + 1

    # GRID
    makeGrid(ax1)

    # Y-AXIS
    YAXIS_MIN = 1
    YAXIS_MAX = 10**5
    ax1.yaxis.set_major_locator(LinearLocator(YAXIS_TICKS))
    ax1.minorticks_off()
    ax1.set_ylabel(get_label('Throughput (ops)'), fontproperties=LABEL_FP)
    ax1.set_ylim(bottom=YAXIS_MIN, top=YAXIS_MAX)
    ax1.set_yscale('log', nonposy='clip')

    # X-AXIS
    ax1.set_xticks(ind + 0.5)
    ax1.set_xlabel(get_label('Size Types'), fontproperties=LABEL_FP)
    ax1.set_xticklabels(x_values)
    #ax1.set_xlim([XAXIS_MIN, XAXIS_MAX])

    for label in ax1.get_yticklabels() :
        label.set_fontproperties(TICK_FP)
    for label in ax1.get_xticklabels() :
        label.set_fontproperties(TICK_FP)

    return fig

def create_cache_line_chart(datasets, color_mode):
    fig = plot.figure()
    ax1 = fig.add_subplot(111)

    # X-AXIS
    x_values = [str(CACHING_TYPES_STRINGS[i]).upper() for i in CACHE_EXP_CACHING_TYPES]
    N = len(x_values)
    ind = np.arange(N)

    idx = 0
    for group in range(len(datasets)):
        # GROUP
        y_values = []
        for line in  range(len(datasets[group])):
            for col in  range(len(datasets[group][line])):
                if col == 1:
                    y_values.append(datasets[group][line][col])
        LOG.info("group_data = %s", str(y_values))
        ax1.plot(ind + 0.5, y_values,
                 color=COLOR_MAPS[color_mode][idx],
                 linewidth=OPT_LINE_WIDTH,
                 marker=OPT_MARKERS[idx],
                 markersize=OPT_MARKER_SIZE,
                 label=str(group))
        idx = idx + 1

    # GRID
    makeGrid(ax1)

    # Y-AXIS
    #YAXIS_MIN = 1
    #YAXIS_MAX = 10**5
    ax1.yaxis.set_major_locator(LinearLocator(YAXIS_TICKS))
    ax1.minorticks_off()
    ax1.set_ylabel(get_label('Throughput (ops)'), fontproperties=LABEL_FP)
    #ax1.set_ylim(bottom=YAXIS_MIN, top=YAXIS_MAX)
    #ax1.set_yscale('log', nonposy='clip')

    # X-AXIS
    XAXIS_MIN = 0.25
    XAXIS_MAX = 3.75
    ax1.set_xticks(ind + 0.5)
    ax1.set_xlabel(get_label('Caching Algorithm Types'), fontproperties=LABEL_FP)
    ax1.set_xticklabels(x_values)
    ax1.set_xlim([XAXIS_MIN, XAXIS_MAX])

    for label in ax1.get_yticklabels() :
        label.set_fontproperties(TICK_FP)
    for label in ax1.get_xticklabels() :
        label.set_fontproperties(TICK_FP)

    return fig

def create_size_ratio_bar_chart(datasets, color_mode):
    fig = plot.figure()
    ax1 = fig.add_subplot(111)

    # X-AXIS
    x_values = [str(chr(ord('A') + i - 1)) for i in SIZE_RATIO_EXP_SIZE_RATIO_TYPES]
    N = len(x_values)
    M = 3
    ind = np.arange(N)
    margin = 0.1
    width = (1.-2.*margin)/M
    bars = [None] * N

    idx = 0
    for group in range(len(datasets)):
        # GROUP
        y_values = []
        for line in  range(len(datasets[group])):
            for col in  range(len(datasets[group][line])):
                if col == 1:
                    y_values.append(datasets[group][line][col])
        LOG.info("group_data = %s", str(y_values))
        bars[group] =  ax1.bar(ind + margin + (group * width),
                               y_values, width,
                               color=COLOR_MAPS[color_mode][group],
                               hatch=PATTERNS[color_mode][group],
                               linewidth=BAR_LINEWIDTH)
        idx = idx + 1

    # GRID
    makeGrid(ax1)

    # Y-AXIS
    YAXIS_MIN = 1
    YAXIS_MAX = 10**5
    ax1.yaxis.set_major_locator(LinearLocator(YAXIS_TICKS))
    ax1.minorticks_off()
    ax1.set_ylabel(get_label('Throughput (ops)'), fontproperties=LABEL_FP)
    ax1.set_ylim(bottom=YAXIS_MIN, top=YAXIS_MAX)
    ax1.set_yscale('log', nonposy='clip')

    # X-AXIS
    ax1.set_xticks(ind + 0.5)
    ax1.set_xlabel(get_label('Size Ratio Types'), fontproperties=LABEL_FP)
    ax1.set_xticklabels(x_values)
    #ax1.set_xlim([XAXIS_MIN, XAXIS_MAX])

    for label in ax1.get_yticklabels() :
        label.set_fontproperties(TICK_FP)
    for label in ax1.get_xticklabels() :
        label.set_fontproperties(TICK_FP)

    return fig

###################################################################################
# PLOT HELPERS
###################################################################################

# LATENCY -- PLOT
def latency_plot():

    # CLEAN UP RESULT DIR
    clean_up_dir(LATENCY_PLOT_DIR)

    for trace_type in LATENCY_EXP_BENCHMARK_TYPES:
        LOG.info(MAJOR_STRING)

        for caching_type in LATENCY_EXP_CACHING_TYPES:
            LOG.info(MINOR_STRING)

            for size_type in LATENCY_EXP_SIZE_TYPES:

                datasets = []
                for hierarchy_type in LATENCY_EXP_HIERARCHY_TYPES:

                    group_dataset = []
                    for latency_type in LATENCY_EXP_LATENCY_TYPES:

                        # Get result file
                        result_dir_list = [BENCHMARK_TYPES_STRINGS[trace_type],
                                           CACHING_TYPES_STRINGS[caching_type],
                                           str(size_type),
                                           HIERARCHY_TYPES_STRINGS[hierarchy_type],
                                           str(latency_type)]
                        result_file = get_result_file(LATENCY_DIR, result_dir_list, LATENCY_CSV)

                        local_dataset = loadDataFile(result_file)
                        group_dataset.append(local_dataset)

                    merged_group_dataset = list(itertools.chain(*group_dataset))
                    datasets.append(merged_group_dataset)

                fig = create_latency_line_chart(datasets, 0)

                file_name = LATENCY_PLOT_DIR + "latency" + "-" + \
                            BENCHMARK_TYPES_STRINGS[trace_type] + "-" + \
                            CACHING_TYPES_STRINGS[caching_type] + "-" + \
                            str(size_type) + ".pdf"

                saveGraph(fig, file_name, width=OPT_GRAPH_WIDTH, height=OPT_GRAPH_HEIGHT)

# SIZE -- PLOT
def size_plot():

    # CLEAN UP RESULT DIR
    clean_up_dir(SIZE_PLOT_DIR)

    for trace_type in SIZE_EXP_BENCHMARK_TYPES:
        LOG.info(MAJOR_STRING)

        for caching_type in SIZE_EXP_CACHING_TYPES:
            LOG.info(MINOR_STRING)

            for latency_type in SIZE_EXP_LATENCY_TYPES:

                datasets = []
                for hierarchy_type in SIZE_EXP_HIERARCHY_TYPES:

                    group_dataset = []
                    for size_type in SIZE_EXP_SIZE_TYPES:

                        # Get result file
                        result_dir_list = [BENCHMARK_TYPES_STRINGS[trace_type],
                                           CACHING_TYPES_STRINGS[caching_type],
                                           str(latency_type),
                                           HIERARCHY_TYPES_STRINGS[hierarchy_type],
                                           str(size_type)]
                        result_file = get_result_file(SIZE_DIR, result_dir_list, SIZE_CSV)

                        local_dataset = loadDataFile(result_file)
                        group_dataset.append(local_dataset)

                    merged_group_dataset = list(itertools.chain(*group_dataset))
                    datasets.append(merged_group_dataset)

                fig = create_size_bar_chart(datasets, 0)

                file_name = SIZE_PLOT_DIR + "size" + "-" + \
                            BENCHMARK_TYPES_STRINGS[trace_type] + "-" + \
                            CACHING_TYPES_STRINGS[caching_type] + "-" + \
                            str(latency_type) + ".pdf"

                saveGraph(fig, file_name, width=OPT_GRAPH_WIDTH, height=OPT_GRAPH_HEIGHT)

# CACHE -- PLOT
def cache_plot():

    # CLEAN UP RESULT DIR
    clean_up_dir(CACHE_PLOT_DIR)

    for trace_type in CACHE_EXP_BENCHMARK_TYPES:
        LOG.info(MAJOR_STRING)

        for latency_type in CACHE_EXP_LATENCY_TYPES:
            LOG.info(MINOR_STRING)

            for size_type in CACHE_EXP_SIZE_TYPES:

                datasets = []
                for hierarchy_type in CACHE_EXP_HIERARCHY_TYPES:

                    group_dataset = []
                    for caching_type in CACHE_EXP_CACHING_TYPES:

                        # Get result file
                        result_dir_list = [BENCHMARK_TYPES_STRINGS[trace_type],
                                           str(size_type),
                                           str(latency_type),
                                           HIERARCHY_TYPES_STRINGS[hierarchy_type],
                                           CACHING_TYPES_STRINGS[caching_type]]
                        result_file = get_result_file(CACHE_DIR, result_dir_list, CACHE_CSV)

                        local_dataset = loadDataFile(result_file)
                        group_dataset.append(local_dataset)

                    merged_group_dataset = list(itertools.chain(*group_dataset))
                    datasets.append(merged_group_dataset)

                fig = create_cache_line_chart(datasets, 0)

                file_name = CACHE_PLOT_DIR + "cache" + "-" + \
                            BENCHMARK_TYPES_STRINGS[trace_type] + "-" + \
                            str(latency_type) + "-" + \
                            str(size_type) + ".pdf"

                saveGraph(fig, file_name, width=OPT_GRAPH_WIDTH, height=OPT_GRAPH_HEIGHT)


# SIZE RATIO -- PLOT
def size_ratio_plot():

    # CLEAN UP RESULT DIR
    clean_up_dir(SIZE_RATIO_PLOT_DIR)

    for trace_type in SIZE_RATIO_EXP_BENCHMARK_TYPES:
        LOG.info(MAJOR_STRING)

        for caching_type in SIZE_RATIO_EXP_CACHING_TYPES:
            LOG.info(MINOR_STRING)

            for latency_type in SIZE_RATIO_EXP_LATENCY_TYPES:

                datasets = []
                for hierarchy_type in SIZE_RATIO_EXP_HIERARCHY_TYPES:

                    group_dataset = []
                    for size_ratio_type in SIZE_RATIO_EXP_SIZE_RATIO_TYPES:

                        # Get result file
                        result_dir_list = [BENCHMARK_TYPES_STRINGS[trace_type],
                                           CACHING_TYPES_STRINGS[caching_type],
                                           str(latency_type),
                                           HIERARCHY_TYPES_STRINGS[hierarchy_type],
                                           str(size_ratio_type)]
                        result_file = get_result_file(SIZE_RATIO_DIR, result_dir_list, SIZE_RATIO_CSV)

                        local_dataset = loadDataFile(result_file)
                        group_dataset.append(local_dataset)

                    merged_group_dataset = list(itertools.chain(*group_dataset))
                    datasets.append(merged_group_dataset)

                fig = create_size_ratio_bar_chart(datasets, 0)

                file_name = SIZE_RATIO_PLOT_DIR + "size-ratio" + "-" + \
                            BENCHMARK_TYPES_STRINGS[trace_type] + "-" + \
                            CACHING_TYPES_STRINGS[caching_type] + "-" + \
                            str(latency_type) + ".pdf"

                saveGraph(fig, file_name, width=OPT_GRAPH_WIDTH, height=OPT_GRAPH_HEIGHT)


# HARD DISK -- PLOT
def hard_disk_plot():

    # CLEAN UP RESULT DIR
    clean_up_dir(HARD_DISK_PLOT_DIR)

    for trace_type in HARD_DISK_EXP_BENCHMARK_TYPES:
        LOG.info(MAJOR_STRING)

        for caching_type in HARD_DISK_EXP_CACHING_TYPES:
            LOG.info(MINOR_STRING)

            for size_type in HARD_DISK_EXP_SIZE_TYPES:

                datasets = []
                for hierarchy_type in HARD_DISK_EXP_HIERARCHY_TYPES:

                    group_dataset = []
                    for latency_type in HARD_DISK_EXP_LATENCY_TYPES:

                        # Get result file
                        result_dir_list = [BENCHMARK_TYPES_STRINGS[trace_type],
                                           CACHING_TYPES_STRINGS[caching_type],
                                           str(size_type),
                                           HIERARCHY_TYPES_STRINGS[hierarchy_type],
                                           str(latency_type)]
                        result_file = get_result_file(HARD_DISK_DIR, result_dir_list, HARD_DISK_CSV)

                        local_dataset = loadDataFile(result_file)
                        group_dataset.append(local_dataset)

                    merged_group_dataset = list(itertools.chain(*group_dataset))
                    datasets.append(merged_group_dataset)

                fig = create_latency_line_chart(datasets, 0)

                file_name = HARD_DISK_PLOT_DIR + "hard-disk" + "-" + \
                            BENCHMARK_TYPES_STRINGS[trace_type] + "-" + \
                            CACHING_TYPES_STRINGS[caching_type] + "-" + \
                            str(size_type) + ".pdf"

                saveGraph(fig, file_name, width=OPT_GRAPH_WIDTH, height=OPT_GRAPH_HEIGHT)


###################################################################################
# UTILITIES
###################################################################################

# COLLECT STATS

# Collect stat
def collect_stat(summary_file, stat_offset):

    # Collect stats
    with open(summary_file) as fp:
        for line in fp:
            line = line.strip()
            line_data = line.split(" ")
            stat = float(line_data[stat_offset])

    LOG.info("stat: " + str(stat))
    return stat

# Write result to a given file that already exists
def write_stat(result_file_name,
               independent_variable,
               stat):

    # Open result file in append mode
    result_file = open(result_file_name, "a")

    # Write out stat
    result_file.write(str(independent_variable) + " , " + str(stat) + "\n")

    result_file.close()

# Print ETA
def print_eta(*args):
    eta_seconds = 1
    for arg in args:
        eta_seconds = eta_seconds * arg

    # Factor in repeat count and default duration
    eta_seconds = eta_seconds * DEFAULT_DURATION

    LOG.info("EXPECTED TIME TO COMPLETE (HH:MM::SS): " +
             str(datetime.timedelta(seconds=eta_seconds)))

###################################################################################
# EVAL
###################################################################################

# LATENCY -- EVAL
def latency_eval():

    # CLEAN UP RESULT DIR
    clean_up_dir(LATENCY_DIR)
    LOG.info("LATENCY EVAL")

    # ETA
    l1 = len(LATENCY_EXP_BENCHMARK_TYPES)
    l2 = len(LATENCY_EXP_CACHING_TYPES)
    l3 = len(LATENCY_EXP_SIZE_TYPES)
    l4 = len(LATENCY_EXP_LATENCY_TYPES)
    l5 = len(LATENCY_EXP_HIERARCHY_TYPES)
    print_eta(l1, l2, l3, l4, l5)

    for trace_type in LATENCY_EXP_BENCHMARK_TYPES:
        LOG.info(MAJOR_STRING)

        for caching_type in LATENCY_EXP_CACHING_TYPES:
            LOG.info(MINOR_STRING)

            for size_type in LATENCY_EXP_SIZE_TYPES:
                LOG.info(SUB_MINOR_STRING)

                for hierarchy_type in LATENCY_EXP_HIERARCHY_TYPES:

                    for latency_type in LATENCY_EXP_LATENCY_TYPES:
                        LOG.info(" > trace_type: " + BENCHMARK_TYPES_STRINGS[trace_type] +
                              " caching_type: " + CACHING_TYPES_STRINGS[caching_type] +
                              " size_type: " + str(size_type) +
                              " latency_type: " + str(latency_type) +
                              " hierarchy_type: " + HIERARCHY_TYPES_STRINGS[hierarchy_type] +
                              "\n"
                        )

                        # Get result file
                        result_dir_list = [BENCHMARK_TYPES_STRINGS[trace_type],
                                           CACHING_TYPES_STRINGS[caching_type],
                                           str(size_type),
                                           HIERARCHY_TYPES_STRINGS[hierarchy_type],
                                           str(latency_type)]
                        result_file = get_result_file(LATENCY_DIR, result_dir_list, LATENCY_CSV)

                        # Run experiment
                        stat = run_experiment(result_file,
                                              latency_type,
                                              stat_offset=THROUGHPUT_OFFSET,
                                              trace_type=trace_type,
                                              hierarchy_type=hierarchy_type,
                                              latency_type=latency_type,
                                              size_type=size_type,
                                              caching_type=caching_type,
                                              summary_file=LATENCY_EXPERIMENT_SUMMARY)

# SIZE -- EVAL
def size_eval():

    # CLEAN UP RESULT DIR
    clean_up_dir(SIZE_DIR)
    LOG.info("SIZE EVAL")

    # ETA
    l1 = len(SIZE_EXP_BENCHMARK_TYPES)
    l2 = len(SIZE_EXP_CACHING_TYPES)
    l3 = len(SIZE_EXP_SIZE_TYPES)
    l4 = len(SIZE_EXP_LATENCY_TYPES)
    l5 = len(SIZE_EXP_HIERARCHY_TYPES)
    print_eta(l1, l2, l3, l4, l5)

    for trace_type in SIZE_EXP_BENCHMARK_TYPES:
        LOG.info(MAJOR_STRING)

        for caching_type in SIZE_EXP_CACHING_TYPES:
            LOG.info(MINOR_STRING)

            for latency_type in SIZE_EXP_LATENCY_TYPES:
                LOG.info(SUB_MINOR_STRING)

                for hierarchy_type in SIZE_EXP_HIERARCHY_TYPES:

                    for size_type in SIZE_EXP_SIZE_TYPES:
                        LOG.info(" > trace_type: " + BENCHMARK_TYPES_STRINGS[trace_type] +
                              " caching_type: " + CACHING_TYPES_STRINGS[caching_type] +
                              " size_type: " + str(size_type) +
                              " latency_type: " + str(latency_type) +
                              " hierarchy_type: " + HIERARCHY_TYPES_STRINGS[hierarchy_type] +
                              "\n"
                        )

                        # Get result file
                        result_dir_list = [BENCHMARK_TYPES_STRINGS[trace_type],
                                           CACHING_TYPES_STRINGS[caching_type],
                                           str(latency_type),
                                           HIERARCHY_TYPES_STRINGS[hierarchy_type],
                                           str(size_type)]
                        result_file = get_result_file(SIZE_DIR, result_dir_list, SIZE_CSV)

                        # Run experiment
                        stat = run_experiment(result_file,
                                              size_type,
                                              stat_offset=THROUGHPUT_OFFSET,
                                              trace_type=trace_type,
                                              hierarchy_type=hierarchy_type,
                                              latency_type=latency_type,
                                              size_type=size_type,
                                              caching_type=caching_type,
                                              summary_file=SIZE_EXPERIMENT_SUMMARY)


# CACHE -- EVAL
def cache_eval():

    # CLEAN UP RESULT DIR
    clean_up_dir(CACHE_DIR)
    LOG.info("CACHE EVAL")

    # ETA
    l1 = len(CACHE_EXP_BENCHMARK_TYPES)
    l2 = len(CACHE_EXP_CACHING_TYPES)
    l3 = len(CACHE_EXP_SIZE_TYPES)
    l4 = len(CACHE_EXP_LATENCY_TYPES)
    l5 = len(CACHE_EXP_HIERARCHY_TYPES)
    print_eta(l1, l2, l3, l4, l5)

    for trace_type in CACHE_EXP_BENCHMARK_TYPES:
        LOG.info(MAJOR_STRING)

        for size_type in CACHE_EXP_SIZE_TYPES:
            LOG.info(MINOR_STRING)

            for latency_type in CACHE_EXP_LATENCY_TYPES:
                LOG.info(SUB_MINOR_STRING)

                for hierarchy_type in CACHE_EXP_HIERARCHY_TYPES:

                    for caching_type in CACHE_EXP_CACHING_TYPES:
                        LOG.info(" > trace_type: " + BENCHMARK_TYPES_STRINGS[trace_type] +
                              " caching_type: " + CACHING_TYPES_STRINGS[caching_type] +
                              " size_type: " + str(size_type) +
                              " latency_type: " + str(latency_type) +
                              " hierarchy_type: " + HIERARCHY_TYPES_STRINGS[hierarchy_type] +
                              "\n"
                        )

                        # Get result file
                        result_dir_list = [BENCHMARK_TYPES_STRINGS[trace_type],
                                           str(size_type),
                                           str(latency_type),
                                           HIERARCHY_TYPES_STRINGS[hierarchy_type],
                                           CACHING_TYPES_STRINGS[caching_type]]
                        result_file = get_result_file(CACHE_DIR, result_dir_list, CACHE_CSV)

                        # Run experiment
                        stat = run_experiment(result_file,
                                              caching_type,
                                              stat_offset=THROUGHPUT_OFFSET,
                                              trace_type=trace_type,
                                              hierarchy_type=hierarchy_type,
                                              latency_type=latency_type,
                                              size_type=size_type,
                                              caching_type=caching_type,
                                              summary_file=CACHE_EXPERIMENT_SUMMARY)


# SIZE RATIO -- EVAL
def size_ratio_eval():

    # CLEAN UP RESULT DIR
    clean_up_dir(SIZE_RATIO_DIR)
    LOG.info("SIZE RATIO EVAL")

    # ETA
    l1 = len(SIZE_RATIO_EXP_BENCHMARK_TYPES)
    l2 = len(SIZE_RATIO_EXP_CACHING_TYPES)
    l3 = len(SIZE_RATIO_EXP_SIZE_RATIO_TYPES)
    l4 = len(SIZE_RATIO_EXP_LATENCY_TYPES)
    l5 = len(SIZE_RATIO_EXP_HIERARCHY_TYPES)
    print_eta(l1, l2, l3, l4, l5)

    for trace_type in SIZE_RATIO_EXP_BENCHMARK_TYPES:
        LOG.info(MAJOR_STRING)

        for caching_type in SIZE_RATIO_EXP_CACHING_TYPES:
            LOG.info(MINOR_STRING)

            for latency_type in SIZE_RATIO_EXP_LATENCY_TYPES:
                LOG.info(SUB_MINOR_STRING)

                for hierarchy_type in SIZE_RATIO_EXP_HIERARCHY_TYPES:

                    for size_ratio_type in SIZE_RATIO_EXP_SIZE_RATIO_TYPES:
                        LOG.info(" > trace_type: " + BENCHMARK_TYPES_STRINGS[trace_type] +
                              " caching_type: " + CACHING_TYPES_STRINGS[caching_type] +
                              " size_ratio_type: " + str(size_ratio_type) +
                              " latency_type: " + str(latency_type) +
                              " hierarchy_type: " + HIERARCHY_TYPES_STRINGS[hierarchy_type] +
                              "\n"
                        )

                        # Get result file
                        result_dir_list = [BENCHMARK_TYPES_STRINGS[trace_type],
                                           CACHING_TYPES_STRINGS[caching_type],
                                           str(latency_type),
                                           HIERARCHY_TYPES_STRINGS[hierarchy_type],
                                           str(size_ratio_type)]
                        result_file = get_result_file(SIZE_RATIO_DIR, result_dir_list, SIZE_RATIO_CSV)

                        # Run experiment
                        stat = run_experiment(result_file,
                                              size_ratio_type,
                                              stat_offset=THROUGHPUT_OFFSET,
                                              trace_type=trace_type,
                                              hierarchy_type=hierarchy_type,
                                              latency_type=latency_type,
                                              size_ratio_type=size_ratio_type,
                                              caching_type=caching_type,
                                              summary_file=SIZE_RATIO_EXPERIMENT_SUMMARY)


# HARD DISK -- EVAL
def hard_disk_eval():

    # CLEAN UP RESULT DIR
    clean_up_dir(HARD_DISK_DIR)
    LOG.info("HARD DISK EVAL")

    # ETA
    l1 = len(HARD_DISK_EXP_BENCHMARK_TYPES)
    l2 = len(HARD_DISK_EXP_CACHING_TYPES)
    l3 = len(HARD_DISK_EXP_SIZE_TYPES)
    l4 = len(HARD_DISK_EXP_LATENCY_TYPES)
    l5 = len(HARD_DISK_EXP_HIERARCHY_TYPES)
    print_eta(l1, l2, l3, l4, l5)

    for trace_type in HARD_DISK_EXP_BENCHMARK_TYPES:
        LOG.info(MAJOR_STRING)

        for caching_type in HARD_DISK_EXP_CACHING_TYPES:
            LOG.info(MINOR_STRING)

            for size_type in HARD_DISK_EXP_SIZE_TYPES:
                LOG.info(SUB_MINOR_STRING)

                for hierarchy_type in HARD_DISK_EXP_HIERARCHY_TYPES:

                    for latency_type in HARD_DISK_EXP_LATENCY_TYPES:
                        LOG.info(" > trace_type: " + BENCHMARK_TYPES_STRINGS[trace_type] +
                              " caching_type: " + CACHING_TYPES_STRINGS[caching_type] +
                              " size_type: " + str(size_type) +
                              " latency_type: " + str(latency_type) +
                              " hierarchy_type: " + HIERARCHY_TYPES_STRINGS[hierarchy_type] +
                              "\n"
                        )

                        # Get result file
                        result_dir_list = [BENCHMARK_TYPES_STRINGS[trace_type],
                                           CACHING_TYPES_STRINGS[caching_type],
                                           str(size_type),
                                           HIERARCHY_TYPES_STRINGS[hierarchy_type],
                                           str(latency_type)]
                        result_file = get_result_file(HARD_DISK_DIR, result_dir_list, HARD_DISK_CSV)

                        # Run experiment
                        stat = run_experiment(result_file,
                                              latency_type,
                                              stat_offset=THROUGHPUT_OFFSET,
                                              trace_type=trace_type,
                                              hierarchy_type=hierarchy_type,
                                              latency_type=latency_type,
                                              size_type=size_type,
                                              caching_type=caching_type,
                                              disk_mode_type=HARD_DISK_EXPERIMENT_DISK_MODE_TYPE,
                                              summary_file=HARD_DISK_EXPERIMENT_SUMMARY)


###################################################################################
# TEST
###################################################################################

#  RUN PARTICULAR TEST
def run_test(test_id):
    LOG.info("Running test : " + str(test_id))

    if test_id == 0 or test_id not in TEST_STRINGS:
        LOG.error("INVALID TEST ID: " + str(test_id))
        return True

    # Run experiment
    return run_experiment(test_id=test_id,
                          log_to_stderr=1)

# RUN ALL TESTS
def run_all_tests():
    LOG.info("Running all tests")

    for test in TEST_STRINGS:
        run_test(test)

    return True

###################################################################################
# EVAL HELPERS
###################################################################################

# CLEAN UP RESULT DIR
def clean_up_dir(result_directory):

    subprocess.call(['rm', '-rf', result_directory])
    if not os.path.exists(result_directory):
        os.makedirs(result_directory)

# RUN EXPERIMENT
def run_experiment(
    result_file,
    independent_variable,
    program=PROGRAM_NAME,
    stat_offset=THROUGHPUT_OFFSET,
    hierarchy_type=DEFAULT_HIERARCHY_TYPE,
    disk_mode_type=DEFAULT_DISK_MODE_TYPE,
    latency_type=DEFAULT_LATENCY_TYPE,
    size_type=DEFAULT_SIZE_TYPE,
    size_ratio_type=DEFAULT_SIZE_RATIO_TYPE,
    caching_type=DEFAULT_CACHING_TYPE,
    trace_type=DEFAULT_BENCHMARK_TYPE,
    migration_frequency=DEFAULT_MIGRATION_FREQUENCY,
    summary_file=DEFAULT_SUMMARY_FILE):

    # subprocess.call(["rm -f " + OUTPUT_FILE], shell=True)
    arg_list = [program,
                    "-a", str(hierarchy_type),
                    "-d", str(disk_mode_type),
                    "-l", str(latency_type),
                    "-r", str(size_ratio_type),
                    "-s", str(size_type),
                    "-c", str(caching_type),
                    "-f", BENCHMARK_TYPES_DIRS[trace_type],
                    "-m", str(migration_frequency),
                    "-o", str(DEFAULT_OPERATION_COUNT),
                ]

    p = multiprocessing.Process(target=worker, args=(program,
                                                     arg_list,
                                                     result_file,
                                                     summary_file,
                                                     independent_variable,
                                                     stat_offset,
                                                     ))

    JOBS.append(p)
    p.start()


# WORKER PROCESS
def worker(program,
           arg_list,
           result_file,
           summary_file,
           independent_variable,
           stat_offset):

    RANDOM_FILE_NAME_STRING = uuid.uuid4().hex[:16].upper()
    SUMMARY_FILE_NAME = RANDOM_FILE_NAME_STRING + "_" + summary_file
    PROGRAM_OUTPUT_FILE_NAME = RANDOM_FILE_NAME_STRING + "_" + "output.txt"
    PROGRAM_OUTPUT_FILE = open(PROGRAM_OUTPUT_FILE_NAME, "w")

    # Set up arglist
    arg_list.append("-z");
    arg_list.append(str(SUMMARY_FILE_NAME))
    arg_string = ' '.join(arg_list[0:])
    LOG.info(arg_string)

    ## Run and check return status
    run_status = True
    try:
        subprocess.check_call(arg_list,
                              stdout=PROGRAM_OUTPUT_FILE)
    except subprocess.CalledProcessError as e:
        LOG.error("FAILED: " + PROGRAM_NAME)
        run_status = False

    ## Check output file
    PROGRAM_OUTPUT_FILE.close()
    PROGRAM_OUTPUT_FILE = open(PROGRAM_OUTPUT_FILE_NAME, "r")
    for line in PROGRAM_OUTPUT_FILE:
        if re.search("error", line):
              LOG.error("FAILED: " + PROGRAM_NAME + "\nERROR :: " + line)
              run_status = False
        if re.search("fail", line):
              LOG.error("FAILED: " + PROGRAM_NAME + "\nERROR :: " + line)
              run_status = False

    # Collect stat
    stat = collect_stat(SUMMARY_FILE_NAME, stat_offset)

    # Write stat
    write_stat(result_file, independent_variable, stat)

    # Clean up output file
    os.remove(PROGRAM_OUTPUT_FILE_NAME)
    os.remove(SUMMARY_FILE_NAME)

    return


## ==============================================
##  MAIN
## ==============================================
if __name__ == '__main__':

    parser = argparse.ArgumentParser(description='Run Machine Experiments')

    ## EVALUATION GROUP
    evaluation_group = parser.add_argument_group('evaluation_group')
    evaluation_group.add_argument("-a", "--latency_eval", help="eval latency", action='store_true')
    evaluation_group.add_argument("-b", "--size_eval", help="eval size", action='store_true')
    evaluation_group.add_argument("-c", "--cache_eval", help="eval cache", action='store_true')
    evaluation_group.add_argument("-d", "--size_ratio_eval", help="eval size ratio", action='store_true')
    evaluation_group.add_argument("-e", "--hard_disk_eval", help="eval hard disk", action='store_true')

    ## PLOTTING GROUP
    plotting_group = parser.add_argument_group('plotting_group')
    plotting_group.add_argument("-m", "--latency_plot", help="plot latency", action='store_true')
    plotting_group.add_argument("-n", "--size_plot", help="plot size", action='store_true')
    plotting_group.add_argument("-o", "--cache_plot", help="plot cache", action='store_true')
    plotting_group.add_argument("-p", "--size_ratio_plot", help="plot size ratio", action='store_true')
    plotting_group.add_argument("-q", "--hard_disk_plot", help="plot hard disk", action='store_true')

    args = parser.parse_args()

    ## EVALUATION GROUP

    if args.latency_eval:
        latency_eval()

    if args.size_eval:
        size_eval()

    if args.cache_eval:
        cache_eval()

    if args.size_ratio_eval:
        size_ratio_eval()

    if args.hard_disk_eval:
        hard_disk_eval()

    ## PLOTTING GROUP

    if args.latency_plot:
        latency_plot()

    if args.size_plot:
        size_plot()

    if args.cache_plot:
        cache_plot()

    if args.size_ratio_plot:
        size_ratio_plot()

    if args.hard_disk_plot:
        hard_disk_plot()

    ## LEGEND GROUP

    #create_legend_hierarchy_type_line(0)
    #create_legend_hierarchy_type_bar(0)
