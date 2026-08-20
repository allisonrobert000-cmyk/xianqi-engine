# xiangqi-engine

A Xiangqi (Chinese Chess) engine written in C++, with a UCCI interface and a
browser GUI.

## Build

```
g++ -std=c++17 main.cpp position.cpp pieces.cpp MoveGenerator.cpp \
    MoveValidator.cpp evaluator.cpp search.cpp UCCI.cpp -o xiangqi_engine
```

## Run

Web GUI:

```
python3 gui.py
```

Open `http://localhost:8080`.

UCCI (stdin/stdout):

```
$ ./xiangqi_engine
ucci
id name xiangqi-engine
ucciok
position startpos
go
bestmove b7b0
```

Tests:

```
g++ -std=c++17 test.cpp position.cpp pieces.cpp MoveGenerator.cpp \
    MoveValidator.cpp evaluator.cpp search.cpp -o test_engine
./test_engine
```

## Structure

```
Search -> Evaluator
Search -> MoveValidator -> MoveGenerator -> Position
```

| File | Contents |
|---|---|
| `pieces.*` | `Piece`, `Side` enums |
| `position.*` | Board representation, starting position, `makeMove` |
| `move.h` | `Move` struct |
| `MoveGenerator.*` | Pseudolegal move generation, all 7 piece types |
| `MoveValidator.*` | Check detection, legality filtering, mate/stalemate |
| `Evaluator.*` | Material evaluation |
| `search.*` | Negamax with alpha-beta pruning |
| `UCCI.*` | Protocol handling |
| `gui.py` | Local server + browser board |

## Perft

Move generation checked against known perft values from the starting
position.

| Depth | Nodes | Reference |
|---|---|---|
| 1 | 44 | 44 |
| 2 | 1,920 | 1,920 |
| 3 | 79,666 | 79,666 |

## Known issues

- No move ordering in the search.
- Material-only evaluation; no positional or endgame terms.
- No repetition detection.
- Fixed search depth, no time control.

## Credit

Engine and UCCI layer written by me. GUI generated with Claude.
