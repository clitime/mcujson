#include "unity.h"
#include "mcujson.h"


void setUp(void) {}
void tearDown(void) {}
void suiteSetUp(void) {}
int suiteTearDown(int num_failures) {}


void test_get_string(void) {
    const char *end;
    TEST_ASSERT_EQUAL_STRING("str", parser_get_key("    \"str\"", &end));
    TEST_ASSERT_EQUAL_STRING("\0", end);

    TEST_ASSERT_EQUAL_STRING("str", parser_get_key(" \t\n\r \v  \"str\"", &end));
    TEST_ASSERT_EQUAL_STRING("\0", end);
    TEST_ASSERT_EQUAL_STRING("", parser_get_key(" \"\"", &end));
    TEST_ASSERT_EQUAL_STRING("\0", end);
    TEST_ASSERT_EQUAL_STRING("", parser_get_key(" \"\"    ", &end));
    TEST_ASSERT_EQUAL_STRING("\0", end);

    TEST_ASSERT_EQUAL_STRING("", parser_get_key(" \"\"    :", &end));
    TEST_ASSERT_EQUAL_STRING(":", end);

    TEST_ASSERT_EQUAL_STRING(NULL, parser_get_key("\"    ", &end));
    TEST_ASSERT_EQUAL_STRING(NULL, end);

    TEST_ASSERT_EQUAL_STRING(NULL, parser_get_key("    ", &end));
    TEST_ASSERT_EQUAL_STRING(NULL, end);
}


void test_get_value(void) {
    const char *end;
    TEST_ASSERT_EQUAL_STRING("string", parser_get_value("\"string\"", &end).str);
    TEST_ASSERT_EQUAL_INT(mcujson_string, parser_get_value("\"string\"", &end).type);
    TEST_ASSERT_EQUAL_STRING("\0", end);

    TEST_ASSERT_EQUAL_STRING("string", parser_get_value("       \"string\"     ", &end).str);
    TEST_ASSERT_EQUAL_INT(mcujson_string, parser_get_value("\"string\"", &end).type);
    TEST_ASSERT_EQUAL_STRING("\0", end);

    TEST_ASSERT_EQUAL_STRING("string", parser_get_value("       \"string\"     ,", &end).str);
    TEST_ASSERT_EQUAL_INT(mcujson_string, parser_get_value("\"string\",", &end).type);
    TEST_ASSERT_EQUAL_STRING(",", end);

    TEST_ASSERT_EQUAL_PTR(NULL,  parser_get_value("      true", &end).str);
    TEST_ASSERT_EQUAL_INT(mcujson_true, parser_get_value("   true    ", &end).type);
    TEST_ASSERT_EQUAL_STRING("\0", end);

    TEST_ASSERT_EQUAL_PTR(NULL,  parser_get_value("      false", &end).str);
    TEST_ASSERT_EQUAL_INT(mcujson_false, parser_get_value("   false    ", &end).type);
    TEST_ASSERT_EQUAL_STRING("\0", end);

    TEST_ASSERT_EQUAL_PTR(NULL,  parser_get_value("      null", &end).str);
    TEST_ASSERT_EQUAL_INT(mcujson_null, parser_get_value("   null    ", &end).type);
    TEST_ASSERT_EQUAL_STRING("\0", end);
}

void test_mcujson_base_object(void) {
    enum mcujson_error err;
    struct mcujson_root *root = mcujson_init_from_str("{}", &err);
    TEST_ASSERT(root != NULL);
    TEST_ASSERT(root->node != NULL);
    TEST_ASSERT_EQUAL_INT(mcujson_object, root->node->type);
    TEST_ASSERT(root->node->next == NULL);
    TEST_ASSERT(root->node->value.obj == NULL);
    TEST_ASSERT(root->node->parent == NULL);
}

void test_mcujson_init_from_str(void) {
    enum mcujson_error err;
    struct mcujson_root *root = mcujson_init_from_str("{\"key\":\"val\",\"key2\":\"val2\"}", &err);
    TEST_ASSERT(root != NULL);
    TEST_ASSERT(root->node != NULL);
    struct mcujson_node *node = root->node;
    TEST_ASSERT_EQUAL_INT(mcujson_object, node->type);
    struct mcujson_node *object_key_value = node->value.obj;
    TEST_ASSERT_EQUAL_INT(mcujson_string, object_key_value->type);
    TEST_ASSERT_EQUAL_PTR(node, object_key_value->parent);
    TEST_ASSERT_EQUAL_STRING("key", object_key_value->key);
    TEST_ASSERT_EQUAL_STRING("val", object_key_value->value.str);

    object_key_value = object_key_value->next;
    TEST_ASSERT_EQUAL_INT(mcujson_string, object_key_value->type);
    TEST_ASSERT_EQUAL_PTR(node, object_key_value->parent);
    TEST_ASSERT_EQUAL_STRING("key2", object_key_value->key);
    TEST_ASSERT_EQUAL_STRING("val2", object_key_value->value.str);
}

void test_mcujson_init_multi_from_str(void) {
    enum mcujson_error err;
    struct mcujson_root *root = mcujson_init_from_str("{\"key\":\"val\",\"obj\":{\"key2\":\"val2\"}, \"key3\":\"val3\"}", &err); //
    TEST_ASSERT(root != NULL);
    TEST_ASSERT(root->node != NULL);
    struct mcujson_node *node = root->node;
    TEST_ASSERT_EQUAL_INT(mcujson_object, node->type);
    struct mcujson_node *object_key_value = node->value.obj;
    TEST_ASSERT_EQUAL_INT(mcujson_string, object_key_value->type);
    TEST_ASSERT_EQUAL_PTR(node, object_key_value->parent);
    TEST_ASSERT_EQUAL_STRING("key", object_key_value->key);
    TEST_ASSERT_EQUAL_STRING("val", object_key_value->value.str);

    object_key_value = object_key_value->next;
    TEST_ASSERT_EQUAL_INT(mcujson_object, object_key_value->type);
    TEST_ASSERT_EQUAL_PTR(node, object_key_value->parent);
    TEST_ASSERT_EQUAL_STRING("obj", object_key_value->key);

    object_key_value = object_key_value->value.obj;

    TEST_ASSERT_EQUAL_INT(mcujson_string, object_key_value->type);
    TEST_ASSERT_EQUAL_STRING("key2", object_key_value->key);
    TEST_ASSERT_EQUAL_STRING("val2", object_key_value->value.str);

    object_key_value = object_key_value->parent->next;
    TEST_ASSERT_EQUAL_INT(mcujson_string, object_key_value->type);
    TEST_ASSERT_EQUAL_STRING("key3", object_key_value->key);
    TEST_ASSERT_EQUAL_STRING("val3", object_key_value->value.str);

}
