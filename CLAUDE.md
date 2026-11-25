# CLAUDE.md

Context for Claude Code sessions in this repo. Keep it short; details live in `docs/`.

## What this is

swdd-recon reconstructs a Software Detailed Design (ASPICE SWE.3) from legacy AUTOSAR C code.
Parser extracts facts → LLM adds meaning → validator checks every LLM claim against the facts →
Markdown + Mermaid that renders on GitHub (PlantUML optional). See `README.md`.

## Hard rules

- **The JSON model is the single source of truth.** Renderers read only the model, never the code.
- **Every model element has provenance** (`parser` | `arxml` | `crosscheck` | `llm`) and a source location.
- **The LLM never creates structure** (states, transitions, calls, ports). It only names and explains
  what the parser found. Anything beyond the facts is flagged `UNVERIFIED`.
- **The LLM backend is pluggable**; a local model must work, because company code must not leave the company.
- **Never feed `examples/*/ground_truth/` to the tool.** It is the answer key, used only by `eval/`.
- **Don't "fix" the planted bugs** in `examples/wiperctrl/` (F-01…F-17, the three traps). They are test input.

## Stack

Python + tree-sitter (C) + Jinja2, lxml for ARXML. Preprocessing via `gcc -E -C` with the stub headers.

## Layout

```
src/swdd_recon/
  preprocess/   gcc -E -C with AUTOSAR stubs, variant-aware
  parser/       tree-sitter: functions, call graph, globals R/W, RTE accesses, FSMs
  arxml/        ARXML reader
  model/        JSON model + schema
  crosscheck/   code vs. ARXML rules
  enrich/       LLM enrichment (backends/ = local, cloud)
  validate/     claim checking against facts
  render/       Markdown+Mermaid, PlantUML (templates/)
  eval/         scoring against ground_truth/
tests/          unit/, integration/, fixtures/
docs/decisions/ ADRs: why things are the way they are
docs/notes/     things learned during work (read parser-pitfalls.md before parser code)
examples/wiperctrl/  reference SWC (tool input) + ground_truth/ (answer key)
```

## Commands

```sh
make -C examples/wiperctrl test   # build SWC with stubs, run 9 scenarios; exit code = failed checks
```

The example must compile with `-std=c99 -Wall -Wextra -Wpedantic -Werror` (GCC and Clang).

## Current status

Done: example SWC, stubs, ARXML (98 refs resolve, not XSD-validated), host simulation, answer key.
Not started: the tool. Next is roadmap step 1 in `README.md` (facts without AI: preprocess,
tree-sitter, ARXML reader, JSON model, Markdown/Mermaid renderer, cross-check F-01…F-03, F-06).

## Conventions

- Repo content (code, docs, commit messages) in English.
- Add an ADR in `docs/decisions/` for any decision that changes the rules above.
