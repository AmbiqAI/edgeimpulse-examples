#include "screen_groups.h"
#include "gui_tree.h"
#include "ng_globals.h"
#include "ng_gitem.h"
#include "nema_transitions.h"

#define NUM_GROUPS 1
#define NUM_POPUPS 0

static int screens_per_group[NUM_GROUPS] = {4};

static tree_node_t *group0_tree_nodes[] = {
    &node_1MainScreen,
    &node_3AnalogWatchFaceScreen,
    &node_5DigitalWatchFaceScreen,
    &node_6ContactScreen,
};

static tree_node_t **group_tree_nodes[NUM_GROUPS] = {group0_tree_nodes};

static int cur_screen_group = 0;
static int cur_screen_node  = 0;


static tree_node_t *popup_tree_nodes[] = {
    NULL
};

static tree_node_t **cur_group_tree_nodes = group0_tree_nodes;

nema_transition_t group_effect[1] = {NEMA_TRANS_LINEAR_H};

uint8_t group_layout[1] = {NG_LAYOUT_HOR};

void init_screen_groups() {
    ng_globals_register_screen_groups(NUM_GROUPS, NUM_POPUPS, screens_per_group, group_tree_nodes, group_effect, group_layout, popup_tree_nodes, cur_screen_group, cur_screen_node, cur_group_tree_nodes);
}
