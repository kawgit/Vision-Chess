from argparse import ArgumentParser
from random import shuffle

from parse import parse_datapoints
from parse import parse_file
from parse import save_dataset


parser = ArgumentParser(description='Process the source and destination file paths.')
parser.add_argument('src_file', type=str, help='The path of the source file. The destination file is assumed to be the src file\'s path minus the file extension plus \'.npz\'')
args = parser.parse_args()


src_file = args.src_file
dst_file = args.src_file.rsplit(".", 1)[0]

print("Parsing file...")
datapoints = parse_file(src_file)

print("Shuffling datapoints...")
shuffle(datapoints)

print("Parsing features...")
dataset = parse_datapoints(datapoints)

print("Saving results...")
save_dataset(dataset, dst_file)