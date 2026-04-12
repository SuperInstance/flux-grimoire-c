# flux-grimoire-c

A pure C11 pattern library for agent spell accumulation. Zero dependencies, no heap allocation.

## Overview

Agents accumulate reusable "spells" (patterns) from experience. Each spell has a trigger, action, context, and tracks success/failure rates. Spells can be published to a fleet and pruned when they underperform.

## Build

```sh
make          # build libgrimoire.a + test binary
make check    # build and run tests
make clean    # remove artifacts
```

## API

| Function | Description |
|---|---|
| `grimoire_init` | Initialize a grimoire |
| `grimoire_learn` | Add a spell, returns id |
| `grimoire_inscribe` | Add a raw spell struct, returns index |
| `grimoire_find` | Find spell by id |
| `grimoire_find_name` | Find spell by name |
| `grimoire_search_trigger` | Search triggers for keyword |
| `grimoire_by_type` | Filter by pattern type |
| `spell_success_rate` | Get success ratio |
| `spell_record_use` | Record outcome |
| `spell_should_forget` | Check if spell is below threshold |
| `grimoire_prune` | Remove underperforming spells |
| `grimoire_publish` | Mark spell as shared |
| `grimoire_shared_count` | Count published spells |
| `grimoire_by_confidence` | Top N spells by success rate (min 3 uses) |

## Constraints

- Pure C11, no external dependencies
- No `malloc`/`free` — all storage is stack/static
- Max 128 spells per grimoire
- Single header + implementation, easy to embed

## License

MIT
