# 0002 – LLM explains, never invents structure

Status: accepted

## Context

Asked to "draw the state machine", an LLM produces plausible diagrams with invented transitions.

## Decision

States, transitions, calls, ports and accesses come only from deterministic analysis.
The LLM adds names, descriptions, intent and scenarios, each citing source lines.
The validator checks every claim against the facts and marks unverifiable claims `UNVERIFIED`.

## Consequences

- Some findings need semantic reasoning (units in comments, timing assumptions, F-04/F-05/F-07).
  They are reported as `llm` findings with evidence, never as structure.
