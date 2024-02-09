import numpy as np
from parse import max_features
from tqdm import tqdm
import torch

def save_dataset(dataset, filename):

    x_rectangles = [ [] for i in range(max_features) ]
    y_rectangles = [ [] for i in range(max_features) ]
    
    for x, y in tqdm(dataset):
        
        num_features = len(x[0])

        x_rectangles[num_features].append(x)
        y_rectangles[num_features].append(y)

    x_rectangles = [ np.array(x_rectangle, dtype=np.float_) for x_rectangle in x_rectangles ]
    y_rectangles = [ np.array(y_rectangle, dtype=np.int_  ) for y_rectangle in y_rectangles ]

    np.savez(filename, *x_rectangles, *y_rectangles)

def get_size(npz):
    count = 0
    for num_features in range(max_features):
        count += len(npz['arr_' + str(num_features)])
    return count

def load_dataset(filename, batch_size, device):
    
    npz = np.load(filename)

    xys = []

    with tqdm(total = get_size(npz)) as pbar:

        for num_features in range(max_features):

            xs = npz['arr_' + str(num_features)]
            ys = npz['arr_' + str(max_features + num_features)]

            assert(len(xs) == len(ys))

            num_points  = len(xs)
            num_batches = num_points // batch_size

            xs = xs[:num_batches * batch_size]
            ys = ys[:num_batches * batch_size]

            xs = xs.reshape(num_batches, batch_size, 2, num_features)
            ys = ys.reshape(num_batches, batch_size, 1)

            xs = torch.tensor(xs, dtype=torch.int32).to(device)
            ys = torch.tensor(ys, dtype=torch.float).to(device)

            xys.extend(list(zip(xs, ys)))

            pbar.update(num_points)

    return xys