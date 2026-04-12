#include "grimoire.h"
#include <string.h>

static uint64_t now_ms(void) {
    // Placeholder — real impl would use clock_gettime
    return 0;
}

void grimoire_init(Grimoire *g) {
    memset(g, 0, sizeof(*g));
    g->next_id = 1;
}

int grimoire_inscribe(Grimoire *g, const Spell *spell) {
    if (g->count >= SPELLS_MAX) return -1;
    memcpy(&g->spells[g->count], spell, sizeof(Spell));
    return (int)g->count++;
}

int grimoire_learn(Grimoire *g, const char *name, PatternType type,
                   const char *trigger, const char *action, uint16_t author) {
    if (g->count >= SPELLS_MAX) return -1;
    Spell *s = &g->spells[g->count];
    memset(s, 0, sizeof(*s));
    s->id = g->next_id++;
    strncpy(s->name, name, SPELL_NAME_MAX - 1);
    s->pattern_type = type;
    strncpy(s->trigger, trigger, SPELL_FIELD_MAX - 1);
    strncpy(s->action, action, SPELL_FIELD_MAX - 1);
    s->author_id = author;
    s->created = now_ms();
    s->modified = s->created;
    g->count++;
    return (int)s->id;
}

Spell* grimoire_find(const Grimoire *g, uint32_t id) {
    for (uint16_t i = 0; i < g->count; i++) {
        if (g->spells[i].id == id) return (Spell*)&g->spells[i];
    }
    return NULL;
}

Spell* grimoire_find_name(Grimoire *g, const char *name) {
    for (uint16_t i = 0; i < g->count; i++) {
        if (strncmp(g->spells[i].name, name, SPELL_NAME_MAX) == 0)
            return &g->spells[i];
    }
    return NULL;
}

int grimoire_search_trigger(const Grimoire *g, const char *keyword,
                            Spell *results, int max) {
    int n = 0;
    for (uint16_t i = 0; i < g->count && n < max; i++) {
        if (strstr(g->spells[i].trigger, keyword))
            memcpy(&results[n++], &g->spells[i], sizeof(Spell));
    }
    return n;
}

int grimoire_by_type(const Grimoire *g, PatternType type,
                     Spell *results, int max) {
    int n = 0;
    for (uint16_t i = 0; i < g->count && n < max; i++) {
        if (g->spells[i].pattern_type == type)
            memcpy(&results[n++], &g->spells[i], sizeof(Spell));
    }
    return n;
}

float spell_success_rate(const Spell *s) {
    if (s->uses == 0) return 0.0f;
    return (float)s->successes / (float)s->uses;
}

void spell_record_use(Spell *s, int success) {
    s->uses++;
    if (success) s->successes++;
    else s->failures++;
    s->modified = now_ms();
}

int spell_should_forget(const Spell *s, float min_rate, uint32_t min_uses) {
    if (s->uses < min_uses) return 0;
    return spell_success_rate(s) < min_rate;
}

int grimoire_prune(Grimoire *g, float min_rate, uint32_t min_uses) {
    int removed = 0;
    for (int i = (int)g->count - 1; i >= 0; i--) {
        if (spell_should_forget(&g->spells[i], min_rate, min_uses)) {
            g->count--;
            if ((uint16_t)i < g->count)
                memmove(&g->spells[i], &g->spells[i + 1],
                        sizeof(Spell) * (g->count - i));
            removed++;
        }
    }
    return removed;
}

void grimoire_publish(Grimoire *g, uint32_t id) {
    Spell *s = grimoire_find(g, id);
    if (s) s->shared = 1;
}

int grimoire_shared_count(const Grimoire *g) {
    int n = 0;
    for (uint16_t i = 0; i < g->count; i++)
        if (g->spells[i].shared) n++;
    return n;
}

static void swap_spells(Spell *a, Spell *b) {
    Spell tmp;
    memcpy(&tmp, a, sizeof(Spell));
    memcpy(a, b, sizeof(Spell));
    memcpy(b, &tmp, sizeof(Spell));
}

int grimoire_by_confidence(const Grimoire *g, int n, Spell *results) {
    if (n <= 0 || g->count == 0) return 0;
    // Copy eligible spells (min 3 uses)
    int total = 0;
    for (uint16_t i = 0; i < g->count; i++) {
        if (g->spells[i].uses >= 3)
            memcpy(&results[total++], &g->spells[i], sizeof(Spell));
    }
    // Bubble sort top N by success_rate descending
    int sort_n = total < n ? total : n;
    for (int i = 0; i < sort_n; i++) {
        for (int j = i + 1; j < total; j++) {
            if (spell_success_rate(&results[j]) > spell_success_rate(&results[i]))
                swap_spells(&results[i], &results[j]);
        }
    }
    return sort_n;
}
