import torch
import torch.nn as nn

class Evaluator(nn.Module):
    
    def __init__(self, io_channels, hidden_channels, out_channels):
        
        super().__init__()
        
        self.encode_table = nn.Embedding(io_channels, hidden_channels)
        self.net = nn.Sequential(
            nn.ReLU(),
            nn.Linear(hidden_channels, out_channels),
        )
        
    def forward(self, features):

        encodes = self.encode_table(features)
        encode  = torch.sum(encodes, 0)
        output  = self.net(encode)

        return output