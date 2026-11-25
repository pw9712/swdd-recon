# 0004 – Stack: Python, tree-sitter, Jinja2, lxml

Status: accepted

## Decision

- Python for the tool.
- tree-sitter (C grammar) on preprocessed sources.
- Jinja2 templates for the renderers.
- lxml for ARXML.

## Consequences

- Preprocessing (`gcc -E -C`) needs a C compiler on the host. See `docs/notes/parser-pitfalls.md`.
