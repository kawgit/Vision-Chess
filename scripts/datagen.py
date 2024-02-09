
import argparse
import random

from parse import parse_features
from parse import parse_file
from serializer import save_dataset


parser = argparse.ArgumentParser(description='Process the source and destination file paths.')
parser.add_argument('src_file', type=str, help='The path of the source file')
parser.add_argument('dst_file', type=str, help='The path of the destination file')
args = parser.parse_args()


print("Parsing file...")
datapoints = parse_file(args.src_file)

print("Parsing features...")
dataset = parse_features(datapoints)

print("Saving results...")
save_dataset(dataset, args.dst_file)