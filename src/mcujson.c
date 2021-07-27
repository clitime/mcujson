#include "mcujson.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#include <stdio.h>

enum parser_state {
    parser_state_wait_begin,
    parser_state_parse_array,
    parser_state_parse_object,
    parser_state_parse_finish,
    parser_state_invalid_json
};

static char *parser_new_string(int len) {
    return (char *)malloc(len);
}

static struct mcujson_node *mcujson_new_node(void) {
    struct mcujson_node *node = (struct mcujson_node *)malloc(sizeof(struct mcujson_node));
    memset(node, 0, sizeof(struct mcujson_node));
    return node;
}

static const char *parser_get_string(const char *begin, const char *end);
static const char *strip_whipespace(const char *str);


struct mcujson_root *mcujson_init(void) {
    struct mcujson_root *root = malloc(sizeof(struct mcujson_root));
    memset(root, 0, sizeof(struct mcujson_root));
    return root;
}



static enum parser_state parser_wait_begin(char c) {
    if (c == '{') {
        return parser_state_parse_object;
    }
    if (c == '[') {
        return parser_state_parse_array;
    }
    return parser_state_wait_begin;
}

/**
'

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
'

[
    "val1",
    "val2",
    {
        "key3":"val3",
        "key4":"val4"
    }
]
 */

static void mcujson_destroy_tree(struct mcujson_root **root);
static enum parser_state parser_parse_object(const char *str, const char **end, struct mcujson_node **root);
// TODO: description whithout layer, a simple object or array that does not
// contain object or array {"key":"val","key":"val"}
struct mcujson_root *mcujson_init_from_str(const char *str, enum mcujson_error *const err) {
    struct mcujson_root *root = mcujson_init();
    if (str == NULL || *str == '\0') {
        return root;
    }
    struct mcujson_node *node = mcujson_new_node();
    root->node = node;

    *err = mcujson_error_ok;

    enum parser_state parser_state = parser_state_wait_begin;

    while (*str != '\0') {
        switch(parser_state) {
        case parser_state_wait_begin:
            parser_state = parser_wait_begin(*str);
            break;
        case parser_state_parse_object:
            node->type = mcujson_object;
            parser_state = parser_parse_object(str, &str, &node);
            break;
        case parser_state_parse_array:
            node->type = mcujson_array;
            break;
        case parser_state_parse_finish:
            break;
        case parser_state_invalid_json:
            // free root
            return NULL;
            // mcujson_destroy_tree(&root);
            /* break; */
        default:
            break;
        }

        str++;
    }
    return root;
}

static void mcujson_destroy_tree(struct mcujson_root **root) {
    // TODO: implement recursive resource release
    *root = NULL;
}

static enum parser_state parser_parse_object(const char *str, const char **end, struct mcujson_node **node) {
/**
 * "{                          node1->type object      node1->parent = NULL        node1->next = NULL      node1->value.obj = node2
 *     "key":"val",            node2->type string      node2->parent = node1       node2->next = node3     node2->value.str = 'val'
 *     "obj":{                 node3->type object      node3->parent = node1       node3->next = NULL      node3->value.obj = node4
 *          "key2":"val2"      node4->type string      node4->parent = node3       node4->next = NULL      node4->value.str = 'val2'
 *      }
 *  }"
 */
    *end = str;
    struct value value;
    enum parser_state parser_state = parser_state_invalid_json;
    if (*node == NULL) {
        return parser_state;
    }
    struct mcujson_node *current = (*node)->value.obj;
    while(current && current->next) {
        current = current->next;
    }

    while (*end && **end != '\0' && **end != '}') {
        struct mcujson_node *new_node = mcujson_new_node();

        if (current == NULL) {
            (*node)->value.obj = new_node;
            current = (*node)->value.obj;
        } else {
            current->next = new_node;
            current = current->next;
        }
        current->parent = *node;

        current->key = parser_get_key(*end, end);
        if (current->key == NULL || *current->key == '\0' || *end == NULL) {
            break;
        }
        if (**end != ':') {
            break;
        }
        (*end)++;

        value = parser_get_value(*end, end);
        current->type = value.type;

        if (current->type != mcujson_array && current->type != mcujson_object) {
            current->value.str = value.str;
            if (**end != ',') {
                break;
            }

            (*end)++;
        } else if (current->type == mcujson_array) {
            parser_state = parser_state_parse_array;
            break;
        } else {
            parser_state = parser_state_parse_object;
            *node = current;
            break;
        }
    }

    if (**end == '}') {
        *node = (*node)->parent;
        if (*node == NULL) {
            parser_state = parser_state_parse_finish;
        } else if ((*node)->type == mcujson_array) {
            parser_state = parser_state_parse_array;
        } else if ((*node)->type == mcujson_object) {
            parser_state = parser_state_parse_object;
            if (*(*end + 1) == ',') {
                *end += 2;
            }
        }
    }

    return parser_state;
}



const char *parser_get_key(const char *str, const char **end) {
    str = strip_whipespace(str);

    if (*str != '"') {
        *end = NULL;
        return NULL;
    }
    *end = ++str;

    while(*end && **end != '\0' && **end != '"') {
        (*end)++;
    }

    if (**end != '"') {
        *end = NULL;
        return NULL;
    }

    const char *key = parser_get_string(str, *end);
    (*end)++;

    *end = strip_whipespace(*end);

    return key;
}



struct value parser_get_value(const char *str, const char **end) {
    str = strip_whipespace(str);
    *end = str;

    struct value value;

    if (*str == '"') {
        value.str = parser_get_key(str, end);
        value.type = mcujson_string;
    } else if (*str == '{') {
        value.str = NULL;
        value.type = mcujson_object;
    } else if (*str == '[') {
        value.str = NULL;
        value.type = mcujson_array;
    } else if (*str == 't' && *(str+1) == 'r' && *(str+2) == 'u' && *(str+3) == 'e') {
        *end += 4;
        value.str = NULL;
        value.type = mcujson_true;
    } else if (*str == 'f' && *(str+1) == 'a' && *(str+2) == 'l' && *(str+3) == 's' && *(str+4) == 'e') {
        *end += 5;
        value.str = NULL;
        value.type = mcujson_false;
    } else if (*str == 'n' && *(str+1) == 'u' && *(str+2) == 'l' && *(str+3) == 'l') {
        *end += 4;
        value.str = NULL;
        value.type = mcujson_null;
    } else if (*str == '-' || *str >= '0' && *str <= '9') {
        // value.str = parser_get_number(str, end);
        value.type = mcujson_number;
    }

    *end = strip_whipespace(*end);

    return value;
}



static const char *parser_get_string(const char *begin, const char *end) {
    char *new_str = parser_new_string(end - begin + 1);
    char *token = new_str;
    while (begin != end) {
        *new_str++ = *begin++;
    }
    *new_str = '\0';

    return token;
}

static const char *strip_whipespace(const char *str) {
    while (str && isspace(*str)) {
        str++;
    }
    return str;
}
