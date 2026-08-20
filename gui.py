#!/usr/bin/env python3
"""
A browser-based GUI for the Xiangqi engine.

Run it:      python3 gui.py
Then open:   http://localhost:8080

It launches ./xiangqi_engine as a subprocess and talks to it over the UCCI
protocol on stdin/stdout -- the same protocol you already tested by hand.
The browser talks to this script over HTTP; this script talks to the engine.

Requires the 'legalmoves' and 'status' commands added to UCCI.cpp.
"""

import http.server
import json
import os
import subprocess
import sys
import threading

ENGINE_PATH = "./xiangqi_engine"
PORT = 8080

# Board layout mirrors position.cpp exactly: row 0 is Black's home rank,
# row 9 is Red's home rank, columns run 0-8 left to right.
STARTING_BOARD = [
    ["bR", "bN", "bB", "bA", "bK", "bA", "bB", "bN", "bR"],
    ["",   "",   "",   "",   "",   "",   "",   "",   ""  ],
    ["",   "bC", "",   "",   "",   "",   "",   "bC", ""  ],
    ["bP", "",   "bP", "",   "bP", "",   "bP", "",   "bP"],
    ["",   "",   "",   "",   "",   "",   "",   "",   ""  ],
    ["",   "",   "",   "",   "",   "",   "",   "",   ""  ],
    ["rP", "",   "rP", "",   "rP", "",   "rP", "",   "rP"],
    ["",   "rC", "",   "",   "",   "",   "",   "rC", ""  ],
    ["",   "",   "",   "",   "",   "",   "",   "",   ""  ],
    ["rR", "rN", "rB", "rA", "rK", "rA", "rB", "rN", "rR"],
]


class Engine:
    """Wraps the engine subprocess and the UCCI conversation with it."""

    def __init__(self, path):
        if not os.path.exists(path):
            sys.exit(
                f"Could not find the engine at {path}\n"
                f"Compile it first, and run this script from the same folder."
            )

        self.process = subprocess.Popen(
            [path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            text=True,
            bufsize=1,  # line buffered
        )
        self.lock = threading.Lock()

        self.send("ucci")
        self.read_until("ucciok")

    def send(self, command):
        self.process.stdin.write(command + "\n")
        self.process.stdin.flush()

    def read_until(self, prefix):
        """Reads engine output lines until one starts with prefix; returns it."""
        while True:
            line = self.process.stdout.readline()
            if not line:
                raise RuntimeError("The engine exited unexpectedly.")
            line = line.strip()
            if line.startswith(prefix):
                return line

    def set_position(self, moves):
        if moves:
            self.send("position startpos moves " + " ".join(moves))
        else:
            self.send("position startpos")

    def legal_moves(self, moves):
        with self.lock:
            self.set_position(moves)
            self.send("legalmoves")
            reply = self.read_until("legalmoves")
        return reply.split()[1:]

    def status(self, moves):
        with self.lock:
            self.set_position(moves)
            self.send("status")
            reply = self.read_until("status")
        return reply.split()[1]
    
    def in_check(self, moves):
        with self.lock:
            self.set_position(moves)
            self.send("incheck")
            reply = self.read_until("incheck")
        return reply.split()[1] == "yes"

    def best_move(self, moves):
        with self.lock:
            self.set_position(moves)
            self.send("go time 10000 increment 0")
            reply = self.read_until("bestmove")
        parts = reply.split()
        return parts[1] if len(parts) > 1 else None


def move_to_coords(move):
    """'h2e2' -> (from_row, from_col, to_row, to_col), matching UCCI/ICCS."""
    return (
        ord(move[1]) - ord("0"),
        ord(move[0]) - ord("a"),
        ord(move[3]) - ord("0"),
        ord(move[2]) - ord("a"),
    )


def board_after(moves):
    """Replays the move list from the starting position."""
    board = [row[:] for row in STARTING_BOARD]
    for move in moves:
        fr, fc, tr, tc = move_to_coords(move)
        board[tr][tc] = board[fr][fc]
        board[fr][fc] = ""
    return board


class Game:
    def __init__(self, engine):
        self.engine = engine
        self.moves = []

    def reset(self):
        self.moves = []

    def state(self):
        return {
            "board": board_after(self.moves),
            "legal": self.engine.legal_moves(self.moves),
            "status": self.engine.status(self.moves),
            "sideToMove": "red" if len(self.moves) % 2 == 0 else "black",
            "lastMove": self.moves[-1] if self.moves else None,
            "moveCount": len(self.moves),
            "inCheck": self.engine.in_check(self.moves),
        }

    def play(self, move):
        self.moves.append(move)

    def engine_reply(self):
        if self.engine.status(self.moves) != "continue":
            return None
        move = self.engine.best_move(self.moves)
        if move:
            self.moves.append(move)
        return move


HTML = r"""<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>Xiangqi Engine</title>
<style>
  body {
    margin: 0;
    padding: 24px;
    background: #23201c;
    color: #e8e0d4;
    font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif;
    display: flex;
    flex-direction: column;
    align-items: center;
    gap: 16px;
  }
  h1 { font-size: 18px; font-weight: 600; margin: 0; letter-spacing: 0.02em; }
  #status { font-size: 14px; min-height: 20px; color: #b8ac99; }
  canvas { background: #f0d9a8; border-radius: 4px; cursor: pointer; }
  .controls { display: flex; gap: 10px; }
  button {
    background: #3a352e; color: #e8e0d4; border: 1px solid #554d42;
    padding: 7px 16px; border-radius: 4px; font-size: 13px; cursor: pointer;
    font-family: inherit;
  }
  button:hover { background: #47413a; }
  button:disabled { opacity: 0.4; cursor: default; }
</style>
</head>
<body>

<h1>Xiangqi Engine</h1>
<canvas id="board" width="560" height="620"></canvas>
<div id="status">Loading…</div>
<div class="controls">
  <button id="newGame">New game</button>
  <button id="engineMove">Let engine move</button>
</div>

<script>
const CELL = 60, MARGIN = 40;
const canvas = document.getElementById("board");
const ctx = canvas.getContext("2d");

let state = null;
let selected = null;   // {row, col}
let busy = false;

const GLYPHS = {
  rK: "帥", rA: "仕", rB: "相", rN: "傌", rR: "俥", rC: "炮", rP: "兵",
  bK: "將", bA: "士", bB: "象", bN: "馬", bR: "車", bC: "砲", bP: "卒",
};

function x(col) { return MARGIN + col * CELL; }
function y(row) { return MARGIN + row * CELL; }

function drawBoard() {
  ctx.fillStyle = "#f0d9a8";
  ctx.fillRect(0, 0, canvas.width, canvas.height);

  ctx.strokeStyle = "#6b4f2a";
  ctx.lineWidth = 1;

  // Horizontal lines, one per rank.
  for (let r = 0; r < 10; r++) {
    ctx.beginPath();
    ctx.moveTo(x(0), y(r));
    ctx.lineTo(x(8), y(r));
    ctx.stroke();
  }

  // Vertical lines. The middle ones stop at the river, except at the edges.
  for (let c = 0; c < 9; c++) {
    ctx.beginPath();
    if (c === 0 || c === 8) {
      ctx.moveTo(x(c), y(0));
      ctx.lineTo(x(c), y(9));
    } else {
      ctx.moveTo(x(c), y(0));
      ctx.lineTo(x(c), y(4));
      ctx.moveTo(x(c), y(5));
      ctx.lineTo(x(c), y(9));
    }
    ctx.stroke();
  }

  // Palace diagonals, both ends.
  ctx.beginPath();
  ctx.moveTo(x(3), y(0)); ctx.lineTo(x(5), y(2));
  ctx.moveTo(x(5), y(0)); ctx.lineTo(x(3), y(2));
  ctx.moveTo(x(3), y(7)); ctx.lineTo(x(5), y(9));
  ctx.moveTo(x(5), y(7)); ctx.lineTo(x(3), y(9));
  ctx.stroke();

  // River.
  ctx.fillStyle = "#9b7d4f";
  ctx.font = "18px serif";
  ctx.textAlign = "center";
  ctx.textBaseline = "middle";
  ctx.fillText("楚 河", x(2), (y(4) + y(5)) / 2);
  ctx.fillText("漢 界", x(6), (y(4) + y(5)) / 2);
}

function drawLastMove() {
  if (!state || !state.lastMove) return;
  const m = state.lastMove;
  const fc = m.charCodeAt(0) - 97, fr = m.charCodeAt(1) - 48;
  const tc = m.charCodeAt(2) - 97, tr = m.charCodeAt(3) - 48;
  ctx.strokeStyle = "rgba(70,130,190,0.85)";
  ctx.lineWidth = 3;
  for (const [r, c] of [[fr, fc], [tr, tc]]) {
    ctx.beginPath();
    ctx.arc(x(c), y(r), 25, 0, Math.PI * 2);
    ctx.stroke();
  }
}

function drawSelection() {
  if (!selected) return;
  ctx.strokeStyle = "rgba(40,140,70,0.95)";
  ctx.lineWidth = 3;
  ctx.beginPath();
  ctx.arc(x(selected.col), y(selected.row), 25, 0, Math.PI * 2);
  ctx.stroke();

  // Dots on every square this piece can legally reach.
  ctx.fillStyle = "rgba(40,140,70,0.55)";
  for (const m of destinationsFrom(selected.row, selected.col)) {
    ctx.beginPath();
    ctx.arc(x(m.col), y(m.row), 8, 0, Math.PI * 2);
    ctx.fill();
  }
}

function destinationsFrom(row, col) {
  if (!state) return [];
  const out = [];
  for (const m of state.legal) {
    if (m.charCodeAt(1) - 48 === row && m.charCodeAt(0) - 97 === col) {
      out.push({ row: m.charCodeAt(3) - 48, col: m.charCodeAt(2) - 97 });
    }
  }
  return out;
}

function drawPieces() {
  if (!state) return;
  for (let r = 0; r < 10; r++) {
    for (let c = 0; c < 9; c++) {
      const piece = state.board[r][c];
      if (!piece) continue;

      const isRed = piece[0] === "r";
      ctx.beginPath();
      ctx.arc(x(c), y(r), 24, 0, Math.PI * 2);
      ctx.fillStyle = "#f7ecd5";
      ctx.fill();
      ctx.lineWidth = 2;
      ctx.strokeStyle = isRed ? "#b03a2e" : "#2c2c2c";
      ctx.stroke();

      ctx.fillStyle = isRed ? "#b03a2e" : "#2c2c2c";
      ctx.font = "26px 'Songti SC', 'STSong', serif";
      ctx.textAlign = "center";
      ctx.textBaseline = "middle";
      ctx.fillText(GLYPHS[piece] || "?", x(c), y(r) + 1);
    }
  }
}

function render() {
  drawBoard();
  drawLastMove();
  drawSelection();
  drawPieces();

  const el = document.getElementById("status");
  if (!state) { el.textContent = "Loading…"; return; }
  if (state.status === "checkmate") {
    el.textContent = (state.sideToMove === "red" ? "Black" : "Red") + " wins by checkmate.";
  } else if (state.status === "stalemate") {
    el.textContent = "Stalemate.";
  } else if (busy) {
    el.textContent = "Engine is thinking…";
  } else {
      el.textContent = (state.sideToMove === "red" ? "Red" : "Black") + " to move"
                    + (state.inCheck ? " — IN CHECK" : "")
                    + " — " + state.legal.length + " legal moves — move " + state.moveCount;
    }
}

function toMoveString(fr, fc, tr, tc) {
  return String.fromCharCode(97 + fc) + String.fromCharCode(48 + fr)
       + String.fromCharCode(97 + tc) + String.fromCharCode(48 + tr);
}

canvas.addEventListener("click", async (e) => {
  if (busy || !state || state.status !== "continue") return;

  const rect = canvas.getBoundingClientRect();
  const px = e.clientX - rect.left, py = e.clientY - rect.top;
  const col = Math.round((px - MARGIN) / CELL);
  const row = Math.round((py - MARGIN) / CELL);
  if (col < 0 || col > 8 || row < 0 || row > 9) return;
  if (Math.hypot(px - x(col), py - y(row)) > 28) return;

  if (selected) {
    const move = toMoveString(selected.row, selected.col, row, col);
    if (state.legal.includes(move)) {
      selected = null;
      await playMove(move);
      return;
    }
  }

  // Select a piece only if it actually has legal moves available.
  if (destinationsFrom(row, col).length > 0) {
    selected = { row, col };
  } else {
    selected = null;
  }
  render();
});

async function refresh() {
  const res = await fetch("/api/state");
  state = await res.json();
  render();
}

async function playMove(move) {
  busy = true; render();

  const res = await fetch("/api/move", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ move }),
  });
  state = await res.json();
  render();                       // your move appears now, engine hasn't run yet

  if (state.status === "continue") {
    const reply = await fetch("/api/engine", { method: "POST" });
    state = await reply.json();
  }

  busy = false; render();
}

document.getElementById("newGame").addEventListener("click", async () => {
  busy = true; selected = null; render();
  const res = await fetch("/api/new", { method: "POST" });
  state = await res.json();
  busy = false; render();
});

document.getElementById("engineMove").addEventListener("click", async () => {
  if (busy || !state || state.status !== "continue") return;
  busy = true; selected = null; render();
  const res = await fetch("/api/engine", { method: "POST" });
  state = await res.json();
  busy = false; render();
});

refresh();
</script>
</body>
</html>
"""


class Handler(http.server.BaseHTTPRequestHandler):
    game = None

    def log_message(self, *args):
        pass  # keep the terminal readable

    def _json(self, payload):
        body = json.dumps(payload).encode()
        self.send_response(200)
        self.send_header("Content-Type", "application/json")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)

    def do_GET(self):
        if self.path == "/":
            body = HTML.encode()
            self.send_response(200)
            self.send_header("Content-Type", "text/html; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
        elif self.path == "/api/state":
            self._json(Handler.game.state())
        else:
            self.send_error(404)

    def do_POST(self):
        length = int(self.headers.get("Content-Length", 0))
        raw = self.rfile.read(length) if length else b"{}"

        if self.path == "/api/move":
            move = json.loads(raw).get("move")
            Handler.game.play(move)
            self._json(Handler.game.state())  # engine answers immediately
        elif self.path == "/api/engine":
            Handler.game.engine_reply()
            self._json(Handler.game.state())
        elif self.path == "/api/new":
            Handler.game.reset()
            self._json(Handler.game.state())
        else:
            self.send_error(404)


def main():
    print("Starting engine…")
    engine = Engine(ENGINE_PATH)
    Handler.game = Game(engine)
    print(f"Ready.  Open  http://localhost:{PORT}  in your browser.")
    print("Ctrl+C here to stop.\n")

    server = http.server.HTTPServer(("localhost", PORT), Handler)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down.")
        engine.process.terminate()


if __name__ == "__main__":
    main()
