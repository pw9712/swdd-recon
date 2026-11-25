# 0003 – Mermaid by default, PlantUML optional

Status: accepted

## Context

The design should be readable in the repo and in PR diffs. Automotive tool chains often use PlantUML.

## Decision

Default output is Markdown + Mermaid, which GitHub renders natively. PlantUML is an optional renderer,
even though GitHub does not preview it.

## Consequences

- PlantUML is rendered to SVG locally or in CI.
- Never use public PlantUML servers for proprietary code: the diagram source is sent to them.
