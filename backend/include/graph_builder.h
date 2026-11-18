/*
 * graph_builder.h — public API for M1: Session struct + conflict graph.
 */
#ifndef GRAPH_BUILDER_H
#define GRAPH_BUILDER_H

#include "constants.h"
#include "core_types.h"
#include <stddef.h>  /* size_t */

/* ── ConflictReason ──────────────────────────────────────────────────── *
 * Why two sessions conflict.  Stored on every graph edge so the UI can  *
 * show "Faculty conflict", "Room conflict", or "Division conflict".      */
typedef enum {
    CONFLICT_FACULTY   = 1 << 0,   /* same faculty member                */
    CONFLICT_ROOM      = 1 << 1,   /* same room                          */
    CONFLICT_DIVISION  = 1 << 2    /* same student division               */
} ConflictReason;

/* ── AdjNode ─────────────────────────────────────────────────────────── *
 * One node in the adjacency-list of the conflict graph.                  *
 * Each AdjNode represents one edge: (this session) --conflicts-with-->   *
 * (neighbour at index `neighbour_idx`).                                  */
typedef struct AdjNode {
    int            neighbour_idx;  /* index into ConflictGraph.sessions[] */
    ConflictReason reasons;        /* bitmask: may combine FACULTY|ROOM   */
    struct AdjNode *next;          /* singly-linked list                  */
} AdjNode;

/* ── ConflictGraph ───────────────────────────────────────────────────── *
 * Undirected graph: sessions[i] and sessions[j] share an edge if they   *
 * cannot be placed in the same TimeSlot.                                 */
typedef struct {
    Session  *sessions;            /* array of n sessions (caller-owned)  */
    int       n;                   /* number of sessions                  */
    AdjNode **adj;                 /* adj[i] = linked list of neighbours  */
    int      *degree;              /* degree[i] = number of conflicts     */
} ConflictGraph;

/* ─────────────────────────────────────────────────────────────────────── *
 *  Public functions                                                        *
 * ─────────────────────────────────────────────────────────────────────── */

void session_init(Session *s);
ConflictReason conflicts(const Session *a, const Session *b);
int graph_build(ConflictGraph *g, Session *sessions, int n);
void graph_free(ConflictGraph *g);
int graph_degree(const ConflictGraph *g, int i);
void conflict_reason_str(ConflictReason r, char *buf, size_t buf_len);

#endif
