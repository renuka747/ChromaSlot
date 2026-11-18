/*
 * graph_builder.c — M1 implementation.
 *
 * Implements:
 *   session_init, conflicts, graph_build, graph_free,
 *   graph_degree, conflict_reason_str
 */
#include "graph_builder.h"

#include <stdlib.h>   /* malloc, calloc, free */
#include <string.h>   /* strcmp, strcat, strncpy, memset */
#include <stdio.h>    /* snprintf  */

/* ── session_init ────────────────────────────────────────────────────── *
 * Zero-fill the struct and mark fixed_slot as "none".                    *
 * Time: O(1)                                                             */
void session_init(Session *s) {
    memset(s, 0, sizeof(Session));
    s->fixed_slot    = -1;   /* -1 = no fixed slot */
    s->unavail_count = 0;
    s->duration      = 1;
    s->student_count = 0;
    s->splittable    = 1;
    s->batch_index   = 0;
    s->allowed_rooms_count = 0;
    s->joint_divisions_count = 0;
    s->required_type = ROOM_CLASSROOM;
    s->parent_id[0]  = '\0';
}

/* ── conflicts ───────────────────────────────────────────────────────── *
 * Check if sessions a and b share a resource.                            *
 * Returns a bitmask of ConflictReason values (0 = no conflict).          *
 * KNOWN LIMITATION: faculty name matching is exact string comparison;    *
 *   "Dr. Sharma" and "dr. sharma" are treated as different people.       *
 * Time: O(1)  — three strcmp calls                                       */
ConflictReason conflicts(const Session *a, const Session *b) {
    ConflictReason r = 0;

    if (strcmp(a->faculty,  b->faculty)  == 0) r |= CONFLICT_FACULTY;
    if (strcmp(a->room,     b->room)     == 0) r |= CONFLICT_ROOM;
    if (strcmp(a->division, b->division) == 0) r |= CONFLICT_DIVISION;

    return r;
}

/* ── add_edge (internal helper) ────────────────────────────────────────── *
 * Prepend a new AdjNode to the adjacency list of vertex `from`.           *
 * Returns 0 on success, -1 if malloc fails.                               *
 * Time: O(1)                                                               */
static int add_edge(ConflictGraph *g, int from, int to, ConflictReason r) {
    AdjNode *node = (AdjNode *)malloc(sizeof(AdjNode));
    if (!node) return -1;

    node->neighbour_idx = to;
    node->reasons       = r;
    node->next          = g->adj[from];   /* prepend */
    g->adj[from]        = node;
    g->degree[from]++;

    return 0;
}

/* ── graph_build ─────────────────────────────────────────────────────── *
 * Build an undirected conflict graph for `n` sessions.                   *
 * For every pair (i, j) with i < j:                                      *
 *   if conflicts(sessions[i], sessions[j]) != 0, add edge i<->j.        *
 * The graph does NOT own the sessions array; caller must keep it alive.  *
 * Time: O(n²)                                                            */
int graph_build(ConflictGraph *g, Session *sessions, int n) {
    /* Zero-init the struct so graph_free is safe even on partial failure */
    memset(g, 0, sizeof(ConflictGraph));

    g->sessions = sessions;
    g->n        = n;

    /* Allocate adj[] and degree[] arrays */
    g->adj    = (AdjNode **)calloc(n, sizeof(AdjNode *));
    g->degree = (int *)     calloc(n, sizeof(int));
    if (!g->adj || !g->degree) {
        graph_free(g);
        return -1;
    }

    /* Examine every pair of sessions exactly once */
    for (int i = 0; i < n; i++) {
        for (int j = i + 1; j < n; j++) {
            ConflictReason r = conflicts(&sessions[i], &sessions[j]);
            if (r == 0) continue;   /* no shared resource → no edge */

            /* Add both directions (undirected graph) */
            if (add_edge(g, i, j, r) != 0 ||
                add_edge(g, j, i, r) != 0) {
                graph_free(g);
                return -1;
            }
        }
    }

    return 0;
}

/* ── graph_free ──────────────────────────────────────────────────────── *
 * Walk every adjacency list and free all AdjNodes, then free adj[]       *
 * and degree[].  Safe to call multiple times (pointers set to NULL).     *
 * Time: O(n + E) where E = number of edges                               */
void graph_free(ConflictGraph *g) {
    if (!g) return;

    if (g->adj) {
        for (int i = 0; i < g->n; i++) {
            AdjNode *cur = g->adj[i];
            while (cur) {
                AdjNode *nxt = cur->next;
                free(cur);
                cur = nxt;
            }
        }
        free(g->adj);
        g->adj = NULL;
    }

    if (g->degree) {
        free(g->degree);
        g->degree = NULL;
    }

    /* g->sessions is caller-owned — do NOT free it here */
    g->sessions = NULL;
    g->n        = 0;
}

/* ── graph_degree ────────────────────────────────────────────────────── *
 * Return the number of conflict edges attached to session i.             *
 * Time: O(1)                                                             */
int graph_degree(const ConflictGraph *g, int i) {
    if (!g || i < 0 || i >= g->n) return -1;
    return g->degree[i];
}

/* ── conflict_reason_str ─────────────────────────────────────────────── *
 * Produce a comma-separated label such as "Faculty, Room" from a bitmask.*
 * buf must be at least MAX_REASON_LEN bytes.                             *
 * Time: O(1)                                                             */
void conflict_reason_str(ConflictReason r, char *buf, size_t buf_len) {
    if (!buf || buf_len == 0) return;
    buf[0] = '\0';

    int first = 1;

    if (r & CONFLICT_FACULTY) {
        strncat(buf, "Faculty",  buf_len - strlen(buf) - 1);
        first = 0;
    }
    if (r & CONFLICT_ROOM) {
        if (!first) strncat(buf, ", ", buf_len - strlen(buf) - 1);
        strncat(buf, "Room", buf_len - strlen(buf) - 1);
        first = 0;
    }
    if (r & CONFLICT_DIVISION) {
        if (!first) strncat(buf, ", ", buf_len - strlen(buf) - 1);
        strncat(buf, "Division", buf_len - strlen(buf) - 1);
    }

    if (buf[0] == '\0') {
        strncpy(buf, "None", buf_len - 1);
        buf[buf_len - 1] = '\0';
    }
}
