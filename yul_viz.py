#!/usr/bin/env python3
"""
yul_viz.py — visualize a Yul or Solidity file's AST and SSA control-flow graph.

Accepts a .yul or a .sol file:
  * .yul — used directly.
  * .sol — compiled with `solc --ir` (or `--optimize --ir-optimized` with
           --optimize); the resulting Yul is dumped next to the input as
           <name>.yul (so it can be examined), and the deployed (runtime)
           object is fed to the pipeline (the SSA-CFG stage does not support
           the multi-object creation wrapper or data sections).

For the chosen Yul unit it:
  1. runs `solc --strict-assembly --ast-compact-json` and turns the AST JSON into DOT,
  2. runs the SSA-CFG pipeline via isoltest (which emits DOT for the SSA CFG),
  3. renders both to PNG and PDF with `dot`,
  4. displays them inline via the kitty graphics protocol (`wezterm imgcat`).

All source artifacts are also written into yul_viz_pics/ for examination:
  <name>_ast.dot, <name>_ast.json  (the AST DOT and the raw AST JSON), and
  <name>_cfg.dot                   (the SSA CFG DOT; no JSON form exists).

Usage:
    ./yul_viz.py stuff.yul
    ./yul_viz.py Contract.sol
    ./yul_viz.py --optimize Contract.sol

Env overrides (all optional):
    SOLC            path to the solc binary
    ISOLTEST        path to the isoltest binary
    EVMONE_LIB      dir containing libevmone (for LD_LIBRARY_PATH)
    IMGCAT          image display command (default: "wezterm imgcat")
"""

import argparse
import json
import os
import re
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path
from typing import NoReturn, Optional


# ----------------------------------------------------------------------------
# Locating the solidity build
# ----------------------------------------------------------------------------

def find_repo_root() -> Path:
    """Walk up from this script (then cwd) looking for build/solc/solc."""
    for start in [Path(__file__).resolve().parent, Path.cwd()]:
        for d in [start, *start.parents]:
            if (d / "build" / "solc" / "solc").exists():
                return d
    return Path("/home/matesoos/development/solidity")


REPO = find_repo_root()
SOLC = Path(os.environ.get("SOLC", REPO / "build" / "solc" / "solc"))
ISOLTEST = Path(os.environ.get("ISOLTEST", REPO / "build" / "test" / "tools" / "isoltest"))
EVMONE_LIB = os.environ.get("EVMONE_LIB", "/home/matesoos/development/evmone/build/lib")
IMGCAT = os.environ.get("IMGCAT", "wezterm imgcat")

# The SSA-CFG isoltest suite reads its cases from here.
CFG_SUITE_DIR = REPO / "test" / "libyul" / "ssa" / "controlFlowGraph"


def die(msg: str) -> NoReturn:
    print(f"error: {msg}", file=sys.stderr)
    sys.exit(1)


def child_env() -> dict:
    env = dict(os.environ)
    ld = env.get("LD_LIBRARY_PATH", "")
    env["LD_LIBRARY_PATH"] = EVMONE_LIB + (":" + ld if ld else "")
    return env


# ----------------------------------------------------------------------------
# Yul text utilities (comment stripping, brace matching, object/data spans)
# ----------------------------------------------------------------------------

def _strip_comments(text: str) -> str:
    """Drop // and /* */ comments while preserving string literals."""
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            while i < n and text[i] != "\n":
                i += 1
        elif c == "/" and i + 1 < n and text[i + 1] == "*":
            i += 2
            while i + 1 < n and not (text[i] == "*" and text[i + 1] == "/"):
                i += 1
            i += 2
        elif c == '"':
            out.append(c)
            i += 1
            while i < n:
                out.append(text[i])
                if text[i] == "\\" and i + 1 < n:
                    out.append(text[i + 1])
                    i += 2
                    continue
                if text[i] == '"':
                    i += 1
                    break
                i += 1
        else:
            out.append(c)
            i += 1
    return "".join(out)


def _match_brace(text: str, open_idx: int) -> int:
    """Index of the '}' matching text[open_idx]=='{' (string-aware), or -1."""
    depth, i, n = 0, open_idx, len(text)
    while i < n:
        c = text[i]
        if c == '"':
            i += 1
            while i < n:
                if text[i] == "\\":
                    i += 2
                    continue
                if text[i] == '"':
                    break
                i += 1
        elif c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    return -1


def _object_spans(text: str):
    """All `object "name" { ... }` spans as (name, start, end) over `text`."""
    spans = []
    for m in re.finditer(r'object\s+"([^"]*)"\s*\{', text):
        close = _match_brace(text, m.end() - 1)
        if close >= 0:
            spans.append((m.group(1), m.start(), close + 1))
    return spans


def _strip_data_sections(text: str) -> str:
    """Remove `data "name" hex"..."` / `data "name" "..."` entries."""
    return re.sub(
        r'\n[ \t]*data\s+"[^"]*"\s+(?:hex"[^"]*"|"(?:[^"\\]|\\.)*")',
        "", text,
    )


def _has_subobjects(unit: str) -> bool:
    """True if the unit still has nested objects or data sections (CFG-unsupported)."""
    cleaned = _strip_comments(unit)
    if len(_object_spans(cleaned)) > 1:
        return True
    return re.search(r'\n[ \t]*data\s+"', cleaned) is not None


# ----------------------------------------------------------------------------
# Solidity -> Yul (IR), and picking a sub-object-free unit to visualize
# ----------------------------------------------------------------------------

def compile_sol_to_ir(sol_file: Path, optimize: bool) -> tuple:
    """Return (raw_ir_text, top_object_name, has_multiple_contracts)."""
    if not SOLC.exists():
        die(f"solc not found at {SOLC} (set $SOLC)")
    args = ["--optimize", "--ir-optimized"] if optimize else ["--ir"]
    marker = "Optimized IR:" if optimize else "IR:"
    proc = subprocess.run(
        [str(SOLC), *args, str(sol_file)],
        capture_output=True, text=True, env=child_env(),
    )
    if proc.returncode != 0:
        die(f"solc {' '.join(args)} failed:\n{proc.stderr}")
    out = proc.stdout
    banners = re.findall(r"^=======", out, re.M)
    if marker + "\n" not in out:
        die(f"no '{marker}' section in solc output:\n{out}")
    body = out.split(marker + "\n", 1)[1]
    body = body.split("\n=======", 1)[0].strip("\n")  # keep only the first contract
    name_m = re.search(r'object\s+"([^"]*)"', body)
    if not name_m:
        die(f"no Yul object found in solc output:\n{out}")
    return body, name_m.group(1), len(banners) > 1


def pick_unit(raw_ir: str) -> str:
    """From full IR pick a unit to visualize (the deployed object if present),
    with data sections stripped so the SSA-CFG stage can consume it."""
    cleaned = _strip_comments(raw_ir)
    spans = _object_spans(cleaned)
    if not spans:
        return _strip_data_sections(cleaned)  # already a bare block
    tops = [s for s in spans
            if not any(a2 < s[1] and s[2] <= b2 for (_, a2, b2) in spans if (a2, b2) != (s[1], s[2]))]
    tname, ta, tb = tops[0]
    nested = [s for s in spans if s[1] > ta and s[2] <= tb and s != (tname, ta, tb)]
    deployed = [s for s in nested if s[0].endswith("_deployed")]
    if deployed:
        n, a, b = deployed[0]
        print(f"  visualizing deployed (runtime) object '{n}'")
        return _strip_data_sections(cleaned[a:b])
    return _strip_data_sections(cleaned[ta:tb])


def load_yul(input_path: Path, optimize: bool) -> str:
    """Return the Yul text to visualize, handling .yul and .sol inputs."""
    ext = input_path.suffix.lower()
    if ext == ".yul":
        if optimize:
            print("note: --optimize only affects .sol inputs; using .yul as-is")
        return input_path.read_text()
    if ext == ".sol":
        raw_ir, name, multi = compile_sol_to_ir(input_path, optimize)
        dump = input_path.with_suffix(".yul")
        dump.write_text(raw_ir + "\n")
        kind = "optimized " if optimize else ""
        print(f"{kind}Yul IR for '{name}' written to {dump}")
        if multi:
            print("  (source has multiple contracts; visualizing the first)")
        return pick_unit(raw_ir)
    die("unsupported input: only .sol and .yul files are accepted")


# ----------------------------------------------------------------------------
# Yul AST -> JSON -> DOT
# ----------------------------------------------------------------------------

# Scalar fields folded into a node's label rather than drawn as child nodes.
_LABEL_FIELDS = ("name", "value", "kind", "type", "hexValue")
_SKIP_FIELDS = {"nodeType", "src", "nativeSrc", "documentation"}


def _esc(s) -> str:
    return str(s).replace("\\", "\\\\").replace('"', '\\"').replace("\n", "\\n")


# Fill color per Yul AST node type (light fills, dark text stays readable).
_NODE_COLORS = {
    "YulObject": "#D5D8DC",
    "YulCode": "#D5D8DC",
    "YulBlock": "#AED6F1",              # blocks — blue
    "YulFunctionDefinition": "#F5B041",  # functions — orange
    "YulFunctionCall": "#A9DFBF",        # calls — green
    "YulBuiltinName": "#A9DFBF",
    "YulIdentifier": "#F9E79F",          # identifiers — yellow
    "YulLiteral": "#F5B7B1",             # literals — red
    "YulTypedName": "#D2B4DE",           # names — purple
    "YulNameWithDebugData": "#D2B4DE",
    "YulVariableDeclaration": "#AAB7B8",  # declarations — gray
    "YulAssignment": "#85C1E9",          # assignments — cyan-blue
    "YulIf": "#FAD7A0",                  # control flow — peach
    "YulSwitch": "#FAD7A0",
    "YulCase": "#FDEBD0",
    "YulForLoop": "#FAD7A0",
    "YulBreak": "#F0B27A",
    "YulContinue": "#F0B27A",
    "YulLeave": "#F0B27A",
    "YulExpressionStatement": "#E5E8E8",
}
_DEFAULT_COLOR = "#FFFFFF"


def _node_color(node_type: str) -> str:
    return _NODE_COLORS.get(node_type, _DEFAULT_COLOR)


def _node_label(node: dict) -> str:
    nt = node.get("nodeType", "?")
    if nt.startswith("Yul"):
        nt = nt[3:]
    lines = [nt]
    for f in _LABEL_FIELDS:
        v = node.get(f)
        if isinstance(v, (str, int, float)) and v != "" and v is not None:
            lines.append(f"{f}={v}")
    return _esc("\n".join(lines))


def ast_json_to_dot(root: dict) -> str:
    lines = [
        "digraph YulAST {",
        '  node[shape=box, style=filled, fontname="DejaVu Sans"];',
        '  edge[fontname="DejaVu Sans", fontsize=9];',
        "",
    ]
    counter = [0]

    def new_id() -> str:
        counter[0] += 1
        return f"n{counter[0]}"

    def walk(obj, parent_id, edge_label):
        if isinstance(obj, dict):
            if "nodeType" in obj:
                nid = new_id()
                color = _node_color(obj.get("nodeType", ""))
                lines.append(f'  {nid} [label="{_node_label(obj)}", fillcolor="{color}"];')
                if parent_id is not None:
                    lbl = f' [label="{_esc(edge_label)}"]' if edge_label else ""
                    lines.append(f"  {parent_id} -> {nid}{lbl};")
                for k, v in obj.items():
                    if k in _SKIP_FIELDS or k in _LABEL_FIELDS:
                        continue
                    walk(v, nid, k)
            else:
                for v in obj.values():
                    walk(v, parent_id, edge_label)
        elif isinstance(obj, list):
            for item in obj:
                walk(item, parent_id, edge_label)

    walk(root, None, None)
    lines.append("}")
    return "\n".join(lines)


def get_ast_dot(yul_text: str) -> tuple:
    """Return (ast_dot, ast_json_pretty) for the given Yul text."""
    if not SOLC.exists():
        die(f"solc not found at {SOLC} (set $SOLC)")
    with tempfile.NamedTemporaryFile("w", suffix=".yul", delete=False) as tf:
        tf.write(yul_text)
        tmp = tf.name
    try:
        proc = subprocess.run(
            [str(SOLC), "--strict-assembly", "--ast-compact-json", tmp],
            capture_output=True, text=True, env=child_env(),
        )
    finally:
        os.unlink(tmp)
    if proc.returncode != 0:
        die(f"solc failed:\n{proc.stderr}")
    brace = proc.stdout.find("{")
    if brace < 0:
        die(f"no JSON in solc output:\n{proc.stdout}\n{proc.stderr}")
    try:
        root, _ = json.JSONDecoder().raw_decode(proc.stdout[brace:])
    except json.JSONDecodeError as e:
        die(f"could not parse AST JSON: {e}")
    return ast_json_to_dot(root), json.dumps(root, indent=2)


# ----------------------------------------------------------------------------
# SSA CFG -> DOT  (via isoltest, the only thing that emits it)
# ----------------------------------------------------------------------------

def get_cfg_dot(yul_text: str) -> str:
    if not ISOLTEST.exists():
        die(f"isoltest not found at {ISOLTEST} (set $ISOLTEST); "
            f"build it with: cmake --build build --target isoltest")
    if not CFG_SUITE_DIR.is_dir():
        die(f"SSA CFG suite dir missing: {CFG_SUITE_DIR}")

    # Unique case name so we never clobber a real test and can filter to it.
    case_name = f"__yulviz_{os.getpid()}__"
    case_file = CFG_SUITE_DIR / f"{case_name}.yul"
    case_file.write_text(yul_text.rstrip("\n") + "\n// ----\n")
    try:
        # `input`/timeout guard: if isoltest ever drops to its interactive
        # (e)dit/(s)kip/(q)uit prompt, feed it quits and cap the wait.
        proc = subprocess.run(
            [str(ISOLTEST), "--accept-updates", "--no-color",
             "--testpath", str(REPO / "test"),
             "-t", f"*ssa/controlFlowGraph/{case_name}"],
            input="q\n" * 20, capture_output=True, text=True,
            env=child_env(), timeout=300,
        )
        text = case_file.read_text()
    except subprocess.TimeoutExpired:
        die("isoltest timed out generating the CFG")
    finally:
        case_file.unlink(missing_ok=True)

    marker = "// ----"
    idx = text.find(marker)
    if idx < 0:
        die(f"isoltest produced no expectation.\n{proc.stdout}\n{proc.stderr}")
    dot_lines = []
    for ln in text[idx + len(marker):].splitlines():
        if ln.startswith("// "):
            dot_lines.append(ln[3:])
        elif ln.startswith("//"):
            dot_lines.append(ln[2:])
        elif ln.strip() == "":
            dot_lines.append("")
    dot = "\n".join(dot_lines).strip()
    if not dot.startswith("digraph"):
        die("isoltest did not emit a CFG (Yul rejected?). Output:\n"
            f"{proc.stdout}\n{proc.stderr}")
    return dot


# ----------------------------------------------------------------------------
# DOT -> PNG -> inline display
# ----------------------------------------------------------------------------

def render(dot: str, base_path: Path) -> Path:
    """Render `dot` to <base>.png and <base>.pdf; return the PNG path."""
    for fmt in ("png", "pdf"):
        out = base_path.with_suffix(f".{fmt}")
        proc = subprocess.run(
            ["dot", f"-T{fmt}", "-o", str(out)],
            input=dot, text=True, capture_output=True,
        )
        if proc.returncode != 0:
            die(f"graphviz `dot` (-T{fmt}) failed:\n{proc.stderr}")
    return base_path.with_suffix(".png")


def display(png_path: Path, title: str) -> None:
    print(f"\n=== {title} ===")
    cmd = shlex.split(IMGCAT) + [str(png_path)]
    try:
        subprocess.run(cmd, check=True)
    except FileNotFoundError:
        print(f"(could not run {IMGCAT!r}; PNG written to {png_path})")
    except subprocess.CalledProcessError as e:
        print(f"(display command failed: {e}; PNG at {png_path})")


def main() -> None:
    parser = argparse.ArgumentParser(description="Visualize a Yul/Solidity file's AST and SSA CFG.")
    parser.add_argument("file", help="input .yul or .sol file")
    parser.add_argument("--optimize", action="store_true",
                        help="for .sol inputs, emit optimized IR (solc --optimize --ir-optimized)")
    ns = parser.parse_args()

    input_path = Path(ns.file).resolve()
    if not input_path.is_file():
        die(f"no such file: {input_path}")
    if input_path.suffix.lower() not in (".yul", ".sol"):
        die("unsupported input: only .sol and .yul files are accepted")

    yul_text = load_yul(input_path, ns.optimize)

    ast_dot, ast_json = get_ast_dot(yul_text)
    cfg_dot: Optional[str] = None
    if _has_subobjects(yul_text):
        print("  note: unit still has sub-objects/data; skipping the SSA-CFG stage "
              "(unsupported). Showing AST only.")
    else:
        cfg_dot = get_cfg_dot(yul_text)

    outdir = Path.cwd() / "yul_viz_pics"
    outdir.mkdir(exist_ok=True)
    stem = input_path.stem

    written = []  # source artifacts (dot/json) written for examination

    # Dump the DOT/JSON sources next to the renders so they can be examined.
    ast_dot_path = outdir / f"{stem}_ast.dot"
    ast_dot_path.write_text(ast_dot + "\n")
    written.append(ast_dot_path)
    ast_json_path = outdir / f"{stem}_ast.json"
    ast_json_path.write_text(ast_json + "\n")
    written.append(ast_json_path)

    ast_png = render(ast_dot, outdir / f"{stem}_ast")
    display(ast_png, f"Yul AST — {input_path.name}")
    written += [ast_png, ast_png.with_suffix(".pdf")]

    if cfg_dot is not None:
        cfg_dot_path = outdir / f"{stem}_cfg.dot"
        cfg_dot_path.write_text(cfg_dot + "\n")
        written.append(cfg_dot_path)
        # The SSA CFG has no JSON form (isoltest emits DOT only), so none is written.

        cfg_png = render(cfg_dot, outdir / f"{stem}_cfg")
        display(cfg_png, f"SSA CFG — {input_path.name}")
        written += [cfg_png, cfg_png.with_suffix(".pdf")]

    print("\nWrote:")
    for path in written:
        print(f"  {path.resolve()}")


if __name__ == "__main__":
    main()
