#ifndef GRIMOIRE_H
#define GRIMOIRE_H

#include <stdint.h>
#include <stddef.h>

typedef enum { PATTERN_BEHAVIORAL, PATTERN_COGNITIVE, PATTERN_SOCIAL, PATTERN_DEBUGGING, PATTERN_OPTIMIZATION } PatternType;

#define SPELL_NAME_MAX 32
#define SPELL_FIELD_MAX 128
#define SPELLS_MAX 128

typedef struct {
    uint32_t id;
    char name[SPELL_NAME_MAX];
    PatternType pattern_type;
    char trigger[SPELL_FIELD_MAX];   // when to use
    char action[SPELL_FIELD_MAX];    // what to do
    char context[SPELL_FIELD_MAX];   // when NOT to use
    uint16_t author_id;
    uint64_t created;
    uint64_t modified;
    uint32_t uses;
    uint32_t successes;
    uint32_t failures;
    uint8_t shared;                  // published to fleet
} Spell;

typedef struct {
    Spell spells[SPELLS_MAX];
    uint16_t count;
    uint32_t next_id;
} Grimoire;

// Curriculum
#define LEVELS_MAX 16
#define LEVEL_SPELLS_MAX 32

typedef struct {
    char name[32];
    uint32_t spell_ids[LEVEL_SPELLS_MAX];
    uint8_t spell_count;
    float min_confidence;
} CurriculumLevel;

typedef struct {
    char name[32];
    CurriculumLevel levels[LEVELS_MAX];
    uint8_t level_count;
} Curriculum;

// API
void grimoire_init(Grimoire *g);
int grimoire_inscribe(Grimoire *g, const Spell *spell);  // returns index, -1 if full
int grimoire_learn(Grimoire *g, const char *name, PatternType type, const char *trigger, const char *action, uint16_t author);  // returns id
Spell* grimoire_find(const Grimoire *g, uint32_t id);
Spell* grimoire_find_name(Grimoire *g, const char *name);
int grimoire_search_trigger(const Grimoire *g, const char *keyword, Spell *results, int max);
int grimoire_by_type(const Grimoire *g, PatternType type, Spell *results, int max);
float spell_success_rate(const Spell *s);
void spell_record_use(Spell *s, int success);
int spell_should_forget(const Spell *s, float min_rate, uint32_t min_uses);
int grimoire_prune(Grimoire *g, float min_rate, uint32_t min_uses);  // returns removed count
void grimoire_publish(Grimoire *g, uint32_t id);
int grimoire_shared_count(const Grimoire *g);
int grimoire_by_confidence(const Grimoire *g, int n, Spell *results);  // top N by success_rate, requires min 3 uses

#endif
