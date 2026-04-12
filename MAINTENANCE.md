# MAINTENANCE

## Extending

- To increase `SPELLS_MAX`, update the define in `grimoire.h`. The `Grimoire` struct grows linearly.
- To add new `PatternType` values, extend the enum. Code using `by_type` will naturally include new types.
- The `now_ms()` placeholder in `grimoire.c` should be replaced with a real clock source when timestamps matter.

## Testing

- Add tests to `test_grimoire.c` following the `TEST/CHECK/PASS/FAIL` pattern.
- Run `make check` after changes.

## Notes

- `grimoire_prune` uses backward iteration + `memmove` for safe in-place removal.
- `grimoire_by_confidence` copies eligible spells then bubble-sorts — fine for N≤128, replace with qsort if needed.
- `grimoire_find_name` returns a mutable pointer (not const) because callers may want to update stats.
