#include "grimoire.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  %-40s ", #name); } while(0)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)
#define CHECK(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)

void test_init_empty(void) {
    TEST(init_empty);
    Grimoire g;
    grimoire_init(&g);
    CHECK(g.count == 0, "count should be 0");
    CHECK(g.next_id == 1, "next_id should be 1");
    PASS();
}

void test_learn_and_find(void) {
    TEST(learn_and_find);
    Grimoire g;
    grimoire_init(&g);
    int id = grimoire_learn(&g, "debug-print", PATTERN_DEBUGGING,
                            "stderr output", "add logging", 1);
    CHECK(id == 1, "first id should be 1");
    CHECK(g.count == 1, "count should be 1");
    Spell *s = grimoire_find(&g, 1);
    CHECK(s != NULL, "should find id 1");
    CHECK(strcmp(s->name, "debug-print") == 0, "name mismatch");
    PASS();
}

void test_find_name(void) {
    TEST(find_name);
    Grimoire g;
    grimoire_init(&g);
    grimoire_learn(&g, "refactor", PATTERN_OPTIMIZATION, "slow code", "cache", 1);
    Spell *s = grimoire_find_name(&g, "refactor");
    CHECK(s != NULL, "should find by name");
    CHECK(s->pattern_type == PATTERN_OPTIMIZATION, "type mismatch");
    s = grimoire_find_name(&g, "nonexistent");
    CHECK(s == NULL, "should return NULL for missing");
    PASS();
}

void test_search_trigger(void) {
    TEST(search_trigger);
    Grimoire g;
    grimoire_init(&g);
    grimoire_learn(&g, "a", PATTERN_BEHAVIORAL, "user angry", "calm", 1);
    grimoire_learn(&g, "b", PATTERN_COGNITIVE, "complex problem", "decompose", 2);
    grimoire_learn(&g, "c", PATTERN_SOCIAL, "user angry", "empathize", 3);
    Spell results[4];
    int n = grimoire_search_trigger(&g, "angry", results, 4);
    CHECK(n == 2, "should find 2");
    PASS();
}

void test_by_type(void) {
    TEST(by_type);
    Grimoire g;
    grimoire_init(&g);
    grimoire_learn(&g, "a", PATTERN_DEBUGGING, "t1", "a1", 1);
    grimoire_learn(&g, "b", PATTERN_DEBUGGING, "t2", "a2", 1);
    grimoire_learn(&g, "c", PATTERN_SOCIAL, "t3", "a3", 2);
    Spell results[4];
    int n = grimoire_by_type(&g, PATTERN_DEBUGGING, results, 4);
    CHECK(n == 2, "should find 2 debugging");
    PASS();
}

void test_success_rate(void) {
    TEST(success_rate);
    Grimoire g;
    grimoire_init(&g);
    int id = grimoire_learn(&g, "x", PATTERN_BEHAVIORAL, "t", "a", 1);
    Spell *s = grimoire_find(&g, id);
    CHECK(fabsf(spell_success_rate(s)) < 0.001f, "new spell rate should be 0");
    spell_record_use(s, 1);
    spell_record_use(s, 1);
    spell_record_use(s, 0);
    CHECK(fabsf(spell_success_rate(s) - 0.6667f) < 0.01f, "rate should be ~0.667");
    PASS();
}

void test_record_use_success(void) {
    TEST(record_use_success);
    Grimoire g;
    grimoire_init(&g);
    int id = grimoire_learn(&g, "y", PATTERN_COGNITIVE, "t", "a", 1);
    Spell *s = grimoire_find(&g, id);
    spell_record_use(s, 1);
    CHECK(s->uses == 1 && s->successes == 1 && s->failures == 0, "counts wrong");
    PASS();
}

void test_record_use_failure(void) {
    TEST(record_use_failure);
    Grimoire g;
    grimoire_init(&g);
    int id = grimoire_learn(&g, "z", PATTERN_COGNITIVE, "t", "a", 1);
    Spell *s = grimoire_find(&g, id);
    spell_record_use(s, 0);
    CHECK(s->uses == 1 && s->successes == 0 && s->failures == 1, "counts wrong");
    PASS();
}

void test_should_forget(void) {
    TEST(should_forget);
    Grimoire g;
    grimoire_init(&g);
    int id = grimoire_learn(&g, "bad", PATTERN_BEHAVIORAL, "t", "a", 1);
    Spell *s = grimoire_find(&g, id);
    CHECK(!spell_should_forget(s, 0.5f, 3), "too few uses");
    for (int i = 0; i < 5; i++) spell_record_use(s, 0);
    CHECK(spell_should_forget(s, 0.5f, 3), "all failures should forget");
    PASS();
}

void test_prune_removes_bad(void) {
    TEST(prune_removes_bad);
    Grimoire g;
    grimoire_init(&g);
    int id = grimoire_learn(&g, "bad", PATTERN_BEHAVIORAL, "t", "a", 1);
    Spell *s = grimoire_find(&g, id);
    for (int i = 0; i < 5; i++) spell_record_use(s, 0);
    int removed = grimoire_prune(&g, 0.5f, 3);
    CHECK(removed == 1, "should remove 1");
    CHECK(g.count == 0, "grimoire should be empty");
    PASS();
}

void test_prune_keeps_good(void) {
    TEST(prune_keeps_good);
    Grimoire g;
    grimoire_init(&g);
    int id = grimoire_learn(&g, "good", PATTERN_BEHAVIORAL, "t", "a", 1);
    Spell *s = grimoire_find(&g, id);
    for (int i = 0; i < 5; i++) spell_record_use(s, 1);
    int removed = grimoire_prune(&g, 0.5f, 3);
    CHECK(removed == 0, "should remove 0");
    CHECK(g.count == 1, "should keep good spell");
    PASS();
}

void test_publish_sets_shared(void) {
    TEST(publish_sets_shared);
    Grimoire g;
    grimoire_init(&g);
    int id = grimoire_learn(&g, "pub", PATTERN_SOCIAL, "t", "a", 1);
    grimoire_publish(&g, id);
    Spell *s = grimoire_find(&g, id);
    CHECK(s->shared == 1, "should be shared");
    PASS();
}

void test_shared_count(void) {
    TEST(shared_count);
    Grimoire g;
    grimoire_init(&g);
    int a = grimoire_learn(&g, "a", PATTERN_SOCIAL, "t", "a", 1);
    int b = grimoire_learn(&g, "b", PATTERN_SOCIAL, "t", "a", 1);
    grimoire_publish(&g, a);
    CHECK(grimoire_shared_count(&g) == 1, "count should be 1");
    grimoire_publish(&g, b);
    CHECK(grimoire_shared_count(&g) == 2, "count should be 2");
    PASS();
}

void test_by_confidence_top_n(void) {
    TEST(by_confidence_top_n);
    Grimoire g;
    grimoire_init(&g);
    int id1 = grimoire_learn(&g, "med", PATTERN_BEHAVIORAL, "t", "a", 1);
    int id2 = grimoire_learn(&g, "high", PATTERN_BEHAVIORAL, "t", "a", 1);
    int id3 = grimoire_learn(&g, "low", PATTERN_BEHAVIORAL, "t", "a", 1);
    Spell *s;
    // low: 3 uses, 0 success
    s = grimoire_find(&g, id3);
    for (int i = 0; i < 3; i++) spell_record_use(s, 0);
    // med: 3 uses, 2 success
    s = grimoire_find(&g, id1);
    spell_record_use(s, 1); spell_record_use(s, 1); spell_record_use(s, 0);
    // high: 3 uses, 3 success
    s = grimoire_find(&g, id2);
    for (int i = 0; i < 3; i++) spell_record_use(s, 1);

    Spell results[4];
    int n = grimoire_by_confidence(&g, 2, results);
    CHECK(n == 2, "should return 2");
    CHECK(strcmp(results[0].name, "high") == 0, "first should be high");
    CHECK(strcmp(results[1].name, "med") == 0, "second should be med");
    PASS();
}

void test_inscribe_full_fails(void) {
    TEST(inscribe_full_fails);
    Grimoire g;
    grimoire_init(&g);
    for (int i = 0; i < SPELLS_MAX; i++)
        grimoire_learn(&g, "s", PATTERN_BEHAVIORAL, "t", "a", 1);
    CHECK(g.count == SPELLS_MAX, "should be full");
    int id = grimoire_learn(&g, "overflow", PATTERN_BEHAVIORAL, "t", "a", 1);
    CHECK(id == -1, "should return -1 when full");
    PASS();
}

int main(void) {
    printf("=== flux-grimoire-c tests ===\n");
    test_init_empty();
    test_learn_and_find();
    test_find_name();
    test_search_trigger();
    test_by_type();
    test_success_rate();
    test_record_use_success();
    test_record_use_failure();
    test_should_forget();
    test_prune_removes_bad();
    test_prune_keeps_good();
    test_publish_sets_shared();
    test_shared_count();
    test_by_confidence_top_n();
    test_inscribe_full_fails();
    printf("\n%d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
