/*
 * Copyright 2004-2022 the Pacemaker project contributors
 *
 * The version control history for this file may have further details.
 *
 * This source code is licensed under the GNU Lesser General Public License
 * version 2.1 or later (LGPLv2.1+) WITHOUT ANY WARRANTY.
 */

#ifndef PCMK__PCMKI_PCMKI_TRANSITION__H
#  define PCMK__PCMKI_PCMKI_TRANSITION__H

#  include <glib.h>
#  include <crm/crm.h>
#  include <crm/msg_xml.h>
#  include <crm/common/xml.h>

#ifdef __cplusplus
extern "C" {
#endif

enum pcmk__graph_action_type {
    pcmk__pseudo_graph_action,
    pcmk__rsc_graph_action,
    pcmk__cluster_graph_action,
};

typedef struct crm_graph_s crm_graph_t;

enum pcmk__synapse_flags {
    pcmk__synapse_ready       = (1 << 0),
    pcmk__synapse_failed      = (1 << 1),
    pcmk__synapse_executed    = (1 << 2),
    pcmk__synapse_confirmed   = (1 << 3),
};

typedef struct synapse_s {
    int id;
    int priority;

    uint32_t flags; // Group of pcmk__synapse_flags

    GList *actions;           /* crm_action_t* */
    GList *inputs;            /* crm_action_t* */
} synapse_t;

const char *synapse_state_str(synapse_t *synapse);

#define pcmk__set_synapse_flags(synapse, flags_to_set) do {             \
        (synapse)->flags = pcmk__set_flags_as(__func__, __LINE__,       \
            LOG_TRACE,                                                  \
            "Synapse", "synapse",                       \
            (synapse)->flags, (flags_to_set), #flags_to_set);           \
    } while (0)

#define pcmk__clear_synapse_flags(synapse, flags_to_clear) do {         \
        (synapse)->flags = pcmk__clear_flags_as(__func__, __LINE__,     \
            LOG_TRACE,                                                  \
            "Synapse", "synapse",                      \
            (synapse)->flags, (flags_to_clear), #flags_to_clear);       \
    } while (0)

enum pcmk__graph_action_flags {
    pcmk__graph_action_sent_update   = (1 << 0),     /* sent to the CIB */
    pcmk__graph_action_executed      = (1 << 1),     /* sent to the CRM */
    pcmk__graph_action_confirmed     = (1 << 2),
    pcmk__graph_action_failed        = (1 << 3),
    pcmk__graph_action_can_fail      = (1 << 4),     //! \deprecated Will be removed in a future release
};

typedef struct crm_action_s {
    int id;
    int timeout;
    int timer;
    guint interval_ms;
    GHashTable *params;
    enum pcmk__graph_action_type type;
    synapse_t *synapse;

    uint32_t flags; // Group of pcmk__graph_action_flags

    xmlNode *xml;

} crm_action_t;

#define crm__set_graph_action_flags(action, flags_to_set) do {             \
        (action)->flags = pcmk__set_flags_as(__func__, __LINE__,      \
            LOG_TRACE,                                                \
            "Action", "action",                                       \
            (action)->flags, (flags_to_set), #flags_to_set);          \
    } while (0)

#define crm__clear_graph_action_flags(action, flags_to_clear) do {         \
        (action)->flags = pcmk__clear_flags_as(__func__, __LINE__,    \
            LOG_TRACE,                                                \
            "Action", "action",                                       \
            (action)->flags, (flags_to_clear), #flags_to_clear);      \
    } while (0)

/* order matters here */
enum transition_action {
    tg_done,
    tg_stop,
    tg_restart,
    tg_shutdown,
};

struct crm_graph_s {
    int id;
    char *source;
    int abort_priority;

    gboolean complete;
    const char *abort_reason;
    enum transition_action completion_action;

    int num_actions;
    int num_synapses;

    int batch_limit;
    guint network_delay;
    guint stonith_timeout;

    int fired;
    int pending;
    int skipped;
    int completed;
    int incomplete;

    GList *synapses;          /* synapse_t* */

    int migration_limit;
};

typedef struct crm_graph_functions_s {
    gboolean(*pseudo) (crm_graph_t * graph, crm_action_t * action);
    gboolean(*rsc) (crm_graph_t * graph, crm_action_t * action);
    gboolean(*crmd) (crm_graph_t * graph, crm_action_t * action);
    gboolean(*stonith) (crm_graph_t * graph, crm_action_t * action);
    gboolean(*allowed) (crm_graph_t * graph, crm_action_t * action);
} crm_graph_functions_t;

enum transition_status {
    transition_active,
    transition_pending,         /* active but no actions performed this time */
    transition_complete,
    transition_terminated,
};

void pcmk__set_graph_functions(crm_graph_functions_t *fns);
crm_graph_t *pcmk__unpack_graph(xmlNode *xml_graph, const char *reference);
enum transition_status pcmk__execute_graph(crm_graph_t *graph);
void pcmk__update_graph(crm_graph_t *graph, crm_action_t *action);
void pcmk__free_graph(crm_graph_t *graph);
const char *pcmk__graph_status2text(enum transition_status state);
void pcmk__log_graph(unsigned int log_level, crm_graph_t *graph);
void pcmk__log_graph_action(int log_level, crm_action_t *action);
lrmd_event_data_t *pcmk__event_from_graph_action(xmlNode *resource,
                                                 crm_action_t *action,
                                                 int status, int rc,
                                                 const char *exit_reason);

#ifdef __cplusplus
}
#endif

#endif
