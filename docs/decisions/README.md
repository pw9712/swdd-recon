# Architecture decisions

One file per decision: `NNNN-short-title.md` with *Context*, *Decision*, *Consequences*.
Superseded decisions stay, marked `Status: superseded by NNNN`.

| ADR | Decision |
|---|---|
| [0001](0001-json-model-single-source-of-truth.md) | JSON intermediate model is the single source of truth, with provenance |
| [0002](0002-llm-explains-never-invents-structure.md) | LLM names and explains; never creates structure |
| [0003](0003-mermaid-default-plantuml-optional.md) | Mermaid by default, PlantUML optional |
| [0004](0004-stack.md) | Python + tree-sitter + Jinja2 + lxml |
| [0005](0005-pluggable-llm-backend.md) | Pluggable LLM backend, local model supported |
| [0006](0006-arxml-crosscheck-in-scope.md) | Code vs. ARXML cross-check is in scope |
| [0007](0007-license-mit.md) | MIT license |
