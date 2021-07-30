#ifndef mcujson_h_
#define mcujson_h_


enum mcujson_type {
    mcujson_unknown,
    mcujson_object,
    mcujson_array,
    mcujson_string,
    mcujson_number,
    mcujson_true,
    mcujson_false,
    mcujson_null,
};


enum mcujson_error {
    mcujson_error_ok,
    mcujson_error_invald,
    mcujson_error_memmory_overflow
};


/**
{
    "key1":"val1",
    "key2":"val2",
    "obj1":{
        "key3":"val3",
        "key4":"val4"
    },
    "arr1":[
        "val5",
        "val6",
        {
            "key7":"val7",
            "obj2":{
                "key8":"val8",
                "key9":"val9"
            }
        }
    ],
    "key10":"val10"
}

[
    "val1",
    "val2",
    {
        "key3":"val3",
        "key4":"val4"
    }
]
 */
struct mcujson_node {
    struct mcujson_node *next; // key1 => key2 => obj1 => arr1 => key10, key3 => key4, key8 => key9
    struct mcujson_node *parent; // key8 => obj2 =>
    const char *key; // for array value key = NULL, example ["val1", "val2", {"key1":"val1"}]

    union {
        const char *str;
        struct mcujson_node *obj; // type == object || type == array; obj1 => key3, arr1 => val5, obj2 => key8
    } value;
    enum mcujson_type type;
};


struct mcujson_root {
    struct mcujson_node *node;
};

const char *parser_get_key(const char *str, const char **end);

struct value {
    enum mcujson_type type;
    const char *str;
};
struct value parser_get_value(const char *str, const char **end);

struct mcujson_root *mcujson_init_from_str(const char *str, enum mcujson_error *const err);
#if 0
/**
 * @brief
 *
 * @return struct mcujson_root
 */
struct mcujson_root mcujson_init(void);


/**
 * @brief
 *
 * @param str
 * @return struct mcujson_root
 */
struct mcujson_root mcujson_init_from_str(const char *str);


void mcujson_destroy_tree(struct mcujson_root root);

/**
 * @brief
 *
 * @param node
 * @param type
 * @param key
 * @param value
 * @return struct mcujson_node*
 */
struct mcujson_node *mcujson_insert_object(
    struct mcujson_node *node,
    enum mcujson_type type,
    const char *key,
    const char *value
);


/**
 * @brief
 *
 * @param node
 * @param value
 * @return int 0 - OK, -1 - ERROR
 */
int mcujson_set_value(struct mcujson_node *node, const char *value);


/**
 * @brief lookup a node by level by key and value
 *
 * @param node begin of the level search
 * @param key search key, required
 * @param value search value, optional
 * @return struct mcujson_node* found item or NULL
 */
struct mcujson_node *mcujson_lookup_by_key_value(
    struct mcujson_node *node,
    const char *key,
    const char *value
);


void mcujson_visit_all(
    struct mcujson_root *root,
    int argc,
    void *argv,
    void (*action)(struct mcujson_node *node, int argc, void *argv)
);


static inline struct mcujson_node *mcujson_next_node(struct mcujson_node *node) {
    return node->next;
}

static inline struct mcujson_node *mcujson_value_obj(struct mcujson_node *node) {
    return node->type == mcujson_array || node->type == mcujson_object ? node->value.obj : 0;
}

static inline const char *mcujson_value_string(struct mcujson_node *node) {
    return node->type == mcujson_array || node->type == mcujson_object ? 0 : node->value.str;
}

#include <stdlib.h>
static inline double mcujson_value_number(struct mcujson_node *node) {
    return strtod(node->value.str, 0);
}

#include <string.h>
static inline bool mcujson_value_boolean(struct mcujson_node *node) {
    return strcmp(node->value.str, "true") == 0;
}

#endif
#endif
