# AUTOBRIQ Documentation Suite

Filled documents for **AUTOBRIQ: Development of a Fully Automated Paper-Charcoal Briquette Machine** (PUP Computer Engineering Group 6). The source of truth is the thesis: [../chapters/AUTOBRIQ_fully_automated_thesis.md](../chapters/AUTOBRIQ_fully_automated_thesis.md). Agents working on these files should read [../agent.md](../agent.md) first.

Start with [PROJECT_CONTEXT.md](PROJECT_CONTEXT.md): what the machine is, what is built, and every place where the code and the docs differ from the thesis.

## Reading / writing order

```
PROJECT_CONTEXT → BRD → PRD → DSD → SDD → QAD → CLR → GTM
                                  Nextion_HMI_Design (alongside DSD)
```

| Document | Depends on | Covers |
|----------|------------|--------|
| [PROJECT_CONTEXT.md](PROJECT_CONTEXT.md) | Thesis | Summary, build status, thesis-vs-code gaps |
| [BRD_Template.md](BRD_Template.md) | Thesis | Business problem, scope, success metrics |
| [PRD_Template.md](PRD_Template.md) | BRD | Interface requirements, user stories, app flow |
| [DSD_Template.md](DSD_Template.md) | PRD | PWA design system (tokens, components) |
| [Nextion_HMI_Design.md](Nextion_HMI_Design.md) | PRD, DSD | On-machine 7" touchscreen |
| [SDD_Template.md](SDD_Template.md) | PRD | Architecture, relay map, data, API |
| [QAD_Template.md](QAD_Template.md) | PRD, SDD | Test plan and release criteria |
| [CLR_Template.md](CLR_Template.md) | PRD, SDD | Data privacy and legal readiness |
| [GTM_Template.md](GTM_Template.md) | PRD, CLR | Pilot and launch strategy |

The `*_Template.md` file names are kept so existing links keep working; the content is the filled AUTOBRIQ version.

Wiring and test guides (open in a browser) are in [guides/](guides/), listed in [PROJECT_CONTEXT.md](PROJECT_CONTEXT.md) section 5.

## Keeping them consistent

1. If code, docs and the thesis disagree, the thesis wins (agent.md Rule 4).
2. Shared parameters (cycle, batch size, force threshold, power, relay map) live in [../agent.md](../agent.md) section 2. Change them there first.
3. Do not invent numbers the thesis leaves to calibration or measurement; mark them "to be measured".
4. Update the Self-Check section of a document when you change it.
