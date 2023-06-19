##Description
Vision-Chess is a UCI compatible chess engine written in C++. It is capable of 200m+ leaf nodes per second during perft search, and around 2m nodes per second during a real search. In the starting position, it searches to depth 16 in about 5 seconds. Building target 'Vision' gives you the UCI engine, while building 'test' runs a benchmark on several perft positions and puzzles.

##Optimizations used include:
 - bitboard board representation
 - magic bitboard move generation
 - transposition hash table
 - alpha-beta pruning
 - move ordering hueristics
  - counter-move hueristic
  - history hueristic
 - late move depth reduction
 - quiescence search
 - search extension for checks in quiescence search

Work is currently being done to implement an NNUE structure to improve position evaluation