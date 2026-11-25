# 0001 – JSON model is the single source of truth

Status: accepted

## Context

Parser, ARXML reader, LLM and cross-check all produce information. Renderers need one consistent view.

## Decision

All stages write into one JSON intermediate model. Each element carries `provenance`
(`parser` | `arxml` | `crosscheck` | `llm`) and a source location (file, line).
Renderers read only the model, never the code.

## Consequences

- Stages can be tested in isolation, and the model can be diffed between runs.
- The schema is versioned (`schema: swdd-recon/model/x.y`).
