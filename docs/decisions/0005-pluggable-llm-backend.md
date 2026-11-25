# 0005 – Pluggable LLM backend

Status: accepted

## Context

The project is both a showcase and meant for real use at work, where production code must not leave the company.

## Decision

The enrichment stage talks to the LLM through a backend interface (`enrich/backends/`).
At least one local backend and one cloud backend.

## Consequences

- Prompts and validation must also work with weaker local models. The validator is the safety net.
