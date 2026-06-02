# Documentation Template Suite

Reusable templates derived from the Mate document suite (`docs/*-mate.md`). Use these when starting a new project or regenerating a document type from scratch.

## Reading / writing order

```
BRD → PRD → DSD → SDD → RFC (feature-specific) → QAD → CLR → GTM
```

| Template | Output path | Depends on |
|----------|-------------|------------|
| [BRD_Template.md](BRD_Template.md) | `docs/brd-{{slug}}.md` | — |
| [PRD_Template.md](PRD_Template.md) | `docs/prd-{{slug}}.md` | BRD |
| [DSD_Template.md](DSD_Template.md) | `docs/dsd-{{slug}}.md` | PRD |
| [SDD_Template.md](SDD_Template.md) | `docs/sdd-{{slug}}.md` | PRD |
| RFC (not in this folder) | `docs/rfc-{{slug}}-{{feature}}.md` | PRD, SDD |
| [QAD_Template.md](QAD_Template.md) | `docs/qad-{{slug}}.md` | PRD, RFC(s) |
| [CLR_Template.md](CLR_Template.md) | `docs/clr-{{slug}}.md` | PRD, SDD |
| [GTM_Template.md](GTM_Template.md) | `docs/gtm-{{slug}}.md` | PRD, CLR |

## How to use

1. Read [AGENT_ROUTING.md](AGENT_ROUTING.md) to pick the right template.
2. Copy the template to the output path (or ask an agent to fill it in place).
3. Replace every `{{PLACEHOLDER}}` with project-specific content.
4. Cross-link sibling docs with relative paths (e.g. `[PRD](prd-mate.md)`).
5. Complete the **Self-Check** section before marking the doc done.

## Placeholder convention

| Placeholder | Meaning |
|-------------|---------|
| `{{PROJECT_NAME}}` | Full product/project name |
| `{{PROJECT_SLUG}}` | Lowercase slug for filenames (e.g. `mate`) |
| `{{DATE}}` | Document date (`YYYY-MM-DD`) |
| `{{VERSION}}` | Semantic version (e.g. `0.1`) |
| `{{OWNER}}` | Owning entity or team |
| `{{STATUS}}` | Draft / In review / Final |
| `{{PRIMARY_MARKET}}` | Launch jurisdiction (e.g. Philippines) |
| `{{LAUNCH_DATE}}` | Target launch or milestone date |

## Reference implementation

The Mate suite in `docs/` is the canonical filled example for every template in this folder.
