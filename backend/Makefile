CC = gcc
CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -g -I include -I vendor
BUILDDIR = build

.PHONY: all test clean asan test_phase1 test_phase2 test_phase3

all: test

test: $(BUILDDIR)/test_graph_builder $(BUILDDIR)/test_avail $(BUILDDIR)/test_graph_coloring $(BUILDDIR)/test_backtracking_repair $(BUILDDIR)/test_scorer $(BUILDDIR)/test_faculty_unavail_repair $(BUILDDIR)/test_complaints $(BUILDDIR)/test_health_check test_phase1 test_phase2 test_phase3
	./$(BUILDDIR)/test_graph_builder
	./$(BUILDDIR)/test_avail
	./$(BUILDDIR)/test_graph_coloring
	./$(BUILDDIR)/test_backtracking_repair
	./$(BUILDDIR)/test_scorer
	./$(BUILDDIR)/test_faculty_unavail_repair
	./$(BUILDDIR)/test_complaints
	./$(BUILDDIR)/test_health_check

test_phase1: $(BUILDDIR)/test_phase1 $(BUILDDIR)/test_json_loader
	./$(BUILDDIR)/test_phase1 test_phase2 test_phase3
	./$(BUILDDIR)/test_json_loader

test_phase2: $(BUILDDIR)/test_phase2
	./$(BUILDDIR)/test_phase2

test_phase3: $(BUILDDIR)/test_phase3
	./$(BUILDDIR)/test_phase3

$(BUILDDIR)/test_graph_builder: tests/test_graph_builder.c src/graph_builder.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILDDIR)/test_avail: tests/test_avail.c src/avail.c src/graph_builder.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILDDIR)/test_graph_coloring: tests/test_graph_coloring.c src/graph_coloring.c src/avail.c src/graph_builder.c src/slot_helpers.c src/preflight.c src/hash_map.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILDDIR)/test_backtracking_repair: tests/test_backtracking_repair.c src/backtracking_repair.c src/graph_coloring.c src/avail.c src/graph_builder.c src/slot_helpers.c src/preflight.c src/hash_map.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILDDIR)/test_scorer: tests/test_scorer.c src/scorer.c src/avail.c src/graph_builder.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILDDIR)/test_faculty_unavail_repair: tests/test_faculty_unavail_repair.c src/faculty_unavail_repair.c src/backtracking_repair.c src/graph_coloring.c src/avail.c src/graph_builder.c src/slot_helpers.c src/preflight.c src/hash_map.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILDDIR)/test_complaints: tests/test_complaints.c src/complaints.c src/backtracking_repair.c src/graph_coloring.c src/avail.c src/graph_builder.c src/slot_helpers.c src/preflight.c src/hash_map.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILDDIR)/test_health_check: tests/test_health_check.c src/health_check.c src/avail.c src/graph_builder.c src/hash_map.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILDDIR)/test_phase1: tests/test_phase1.c src/hash_map.c src/db.c src/graph_builder.c src/json_loader.c vendor/cJSON/cJSON.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -lsqlite3 $^ -o $@

$(BUILDDIR)/test_json_loader: tests/test_json_loader.c src/hash_map.c src/db.c src/graph_builder.c src/json_loader.c vendor/cJSON/cJSON.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -lsqlite3 $^ -o $@

asan: CFLAGS += -fsanitize=address,undefined -fno-omit-frame-pointer
asan: clean test test_phase1 test_phase2 test_phase3

clean:
	rm -rf $(BUILDDIR)

$(BUILDDIR)/test_phase2: tests/test_phase2.c src/slot_helpers.c src/avail.c src/hash_map.c src/graph_builder.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILDDIR)/test_phase3: tests/test_phase3.c src/preflight.c src/slot_helpers.c src/avail.c src/hash_map.c src/graph_builder.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILDDIR)/test_regression: tests/test_regression.c src/hash_map.c src/db.c src/graph_builder.c src/json_loader.c src/graph_coloring.c src/backtracking_repair.c src/health_check.c src/avail.c src/slot_helpers.c src/preflight.c src/faculty_unavail_repair.c src/complaints.c vendor/cJSON/cJSON.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -lsqlite3 $^ -o $@

test_regression: $(BUILDDIR)/test_regression
	./$(BUILDDIR)/test_regression


$(BUILDDIR)/server: src/main.c src/server.c src/hash_map.c src/db.c src/graph_builder.c src/json_loader.c src/graph_coloring.c src/backtracking_repair.c src/health_check.c src/avail.c src/slot_helpers.c src/preflight.c src/faculty_unavail_repair.c src/complaints.c src/scorer.c vendor/cJSON/cJSON.c vendor/mongoose/mongoose.c
	mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -lsqlite3 $^ -o $@

server: $(BUILDDIR)/server
	./$(BUILDDIR)/server 8000
