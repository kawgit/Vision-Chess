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

num_fbuckets = 8
num_kbuckets = 32
num_buckets  = num_fbuckets * num_kbuckets
phase_weights = [0, 3, 4, 8, 18, 0]
phase_start   = (phase_weights[0] * 16 
               + phase_weights[1] * 4 
               + phase_weights[2] * 4 
               + phase_weights[3] * 4 
               + phase_weights[4] * 2)
               
min_features = 2
max_features = 32

def feature_index(view, is_flipped, color, piece, square):

    pidx = (view == color) * 6 + piece
    square ^= (0b000111 * is_flipped | 0b111000 * (view == BLACK))

    assert(pidx   < 12)
    assert(square < 64)

    return pidx * 64 + square 

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
    white_flipped = (white_ksq % 8) >= 4
    black_flipped = (black_ksq % 8) >= 4
    
    phase = 0

    features_white = []
    features_black = []

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
            square = rank * 8 + file

            features_white.append(feature_index(WHITE, white_flipped, color, piece, square))
            features_black.append(feature_index(BLACK, black_flipped, color, piece, square))

            phase += phase_weights[piece]

            file += 1

    assert(len(features_white) <= max_features)
    assert(len(features_black) <= max_features)
    assert(len(features_black) >= min_features)
    assert(len(features_black) >= min_features)

    fbucket       = min(phase * num_fbuckets // phase_start, num_fbuckets - 1)

    white_ksq_adj = white_ksq ^ (0b000111 * white_flipped)
    black_ksq_adj = black_ksq ^ (0b000111 * black_flipped | 0b111000)

    kbucket_white = (white_ksq // 8) * 4 + (white_ksq_adj % 8)
    kbucket_black = (black_ksq // 8) * 4 + (black_ksq_adj % 8)

    assert(fbucket       < 8)
    assert(kbucket_white < 32)
    assert(kbucket_black < 32)

    bucket_white = (fbucket, kbucket_white)
    bucket_black = (fbucket, kbucket_black)
    
    stm = WHITE if fen.find("w") != -1 else BLACK

    features = np.array((features_black, features_white) if stm == WHITE else (features_white, features_black))
    buckets  = np.array((  bucket_black,   bucket_white) if stm == WHITE else (  bucket_white,   bucket_black))

    return features, buckets

def parse_datapoints(datapoints):
    
    result = []

    for datapoint in tqdm(datapoints):

        fen   = datapoint[0]
        score = datapoint[2]

        features, buckets = parse_fen(fen)

        features = np.array(features, dtype=np.int_  )
        buckets  = np.array(buckets,  dtype=np.int_  )
        outputs  = np.array([score],  dtype=np.float_)

        result.append((features, buckets, outputs))

    return result

def save_dataset(dataset, filename):

    x_rectangles = [ [] for i in range(max_features + 1) ]
    b_rectangles = [ [] for i in range(max_features + 1) ]
    y_rectangles = [ [] for i in range(max_features + 1) ]
    
    for x, b, y in tqdm(dataset):
        
        num_features = len(x[0])

        x_rectangles[num_features].append(x)
        b_rectangles[num_features].append(b)
        y_rectangles[num_features].append(y)

    x_rectangles = [ np.array(x_rectangle, dtype=np.int_  ) for x_rectangle in x_rectangles ]
    b_rectangles = [ np.array(b_rectangle, dtype=np.int_  ) for b_rectangle in b_rectangles ]
    y_rectangles = [ np.array(y_rectangle, dtype=np.float_) for y_rectangle in y_rectangles ]

    np.savez(filename, *x_rectangles, *b_rectangles, *y_rectangles)

def get_size(npz):
    count = 0
    for num_features in range(max_features):
        count += len(npz['arr_' + str(num_features)])
    return count

def load_dataset(filename, batch_size, device):
    
    npz = np.load(filename)

    batches = [[] for _ in range(max_features)]

    with tqdm(total = get_size(npz)) as pbar:

        for num_features in range(max_features):

            xs = npz['arr_' + str(num_features)]
            bs = npz['arr_' + str(max_features + num_features)]
            ys = npz['arr_' + str(max_features + max_features + num_features)]

            assert(len(xs) == len(ys))
            assert(len(xs) == len(bs))

            num_points  = len(xs)
            num_batches = num_points // batch_size

            xs = xs[:num_batches * batch_size]
            bs = bs[:num_batches * batch_size]
            ys = ys[:num_batches * batch_size]

            xs = xs.reshape(num_batches, batch_size, 2, num_features)
            bs = bs.reshape(num_batches, batch_size, 2, 2)
            ys = ys.reshape(num_batches, batch_size, 1)

            xs = torch.tensor(xs, dtype=torch.int32).to(device)
            bs = torch.tensor(bs, dtype=torch.int32).to(device)
            ys = torch.tensor(ys, dtype=torch.float).to(device)

            batches.extend(list(zip(xs, bs, ys)))

            pbar.update(num_points)

    return result