from tqdm import tqdm
import torch
import numpy as np
import math

def parse_file(file_name):
    
    with open(file_name, "r") as file:
        text = file.read()

    records = text.split("e\n")

    datapoints = []

    for record in tqdm(records):

        if len(record) < 10:
            continue
    
        try:

            lines = record.split("\n")

            fen    =     lines[0][4:]
            move   =     lines[1][5:]
            score  = int(lines[2][6:])
            ply    = int(lines[3][4:])
            result = int(lines[4][7:])

            datapoints.append((fen, move, score, ply, result))

        except Exception as e:
    
            print("Error:", e)

    return datapoints

max_features = 64

def feature_index(ksq, ours, piece, square):
    pidx = ours * 6 + piece

    ksq_rank = ksq // 8
    ksq_file = ksq %  8
    flip_const = 0b111 * int(ksq_file >= 4)
    ksq = ksq_rank * 4 + (ksq_file ^ flip_const)
    square = square ^ flip_const

    assert(pidx   <= 11)
    assert(ksq    <  32)
    assert(square <  64)

    return pidx * 64 * 32 + square * 32 + ksq 

SLASH = 47
ZERO = 48
NINE = 57

BLACK = 0
WHITE = 1

char_to_piece = {
    ord('p') : (BLACK, 0),
    ord('n') : (BLACK, 1),
    ord('b') : (BLACK, 2),
    ord('r') : (BLACK, 3),
    ord('q') : (BLACK, 4),
    ord('k') : (BLACK, 5),
    ord('P') : (WHITE, 0),
    ord('N') : (WHITE, 1),
    ord('B') : (WHITE, 2),
    ord('R') : (WHITE, 3),
    ord('Q') : (WHITE, 4),
    ord('K') : (WHITE, 5),
}

char_to_offset = {
    'p' : 1,
    'n' : 1,
    'b' : 1,
    'r' : 1,
    'q' : 1,
    'k' : 1,
    'P' : 1,
    'N' : 1,
    'B' : 1,
    'R' : 1,
    'Q' : 1,
    'K' : 1,
    '/' : 0,
    '1' : 1,
    '2' : 2,
    '3' : 3,
    '4' : 4,
    '5' : 5,
    '6' : 6,
    '7' : 7,
    '8' : 8,
}

def find_ksqs(fen):
    curr_sq = 0
    white_ksq = None
    black_ksq = None
    for char in fen:

        if char == 'K':
            white_ksq = curr_sq
            if black_ksq != None:
                break

        if char == 'k':
            black_ksq = curr_sq
            if white_ksq != None:
                break

        curr_sq += char_to_offset[char]


    assert(white_ksq != None)
    assert(black_ksq != None)

    return white_ksq, black_ksq

def parse_fen(fen):
    
    white_ksq, black_ksq = find_ksqs(fen)

    white_features = []
    black_features = []

    rank = 7
    file = 0
    fen_idx = 0
    
    while rank != 0 or file != 8:

        char = ord(fen[fen_idx])
        fen_idx += 1
        
        if char == SLASH:

            file = 0
            rank -= 1

        elif char >= ZERO and char <= NINE:
        
            file += char - ZERO
        
        else:
            
            color, piece = char_to_piece[char]

            if char != ord('K'):
                white_features.append(feature_index(white_ksq, int(color == WHITE), piece, file + 8 * rank      ))
            
            if char != ord('k'):
                black_features.append(feature_index(black_ksq, int(color == BLACK), piece, file + 8 * (7 - rank)))

            file += 1
    
    stm = WHITE if fen.find("w") != -1 else BLACK

    features = np.array([white_features, black_features] if stm == WHITE else [black_features, white_features])

    return features

def parse_features(datapoints):
    
    xys = []

    for datapoint in tqdm(datapoints):

        fen   = datapoint[0]
        score = datapoint[2]

        x = np.array(parse_fen(fen), dtype=np.int_  )
        y = np.array([score],        dtype=np.float_)

        xys.append((x, y))

    return xys
