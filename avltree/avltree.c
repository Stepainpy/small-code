#include "avltree.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct avl_node {
    struct avl_node* lhs;
    struct avl_node* rhs;
    unsigned char height;
    const void* key;
    void* data;
} avl_node_t;

#define avl_height(node) (node ? node->height : 0)
#define avl_balance_factor(node) \
(avl_height(node->rhs) - avl_height(node->lhs))

static inline void avl_fix_height(avl_node_t* node) {
    unsigned char hl = avl_height(node->lhs);
    unsigned char hr = avl_height(node->rhs);
    node->height = (hl > hr ? hl : hr) + 1;
}

static inline avl_node_t* avl_rotr(avl_node_t* node) {
    avl_node_t* left = node->lhs;
    node->lhs = left->rhs;
    left->rhs = node;
    avl_fix_height(node);
    avl_fix_height(left);
    return left;
}

static inline avl_node_t* avl_rotl(avl_node_t* node) {
    avl_node_t* righ = node->rhs;
    node->rhs = righ->lhs;
    righ->lhs = node;
    avl_fix_height(node);
    avl_fix_height(righ);
    return righ;
}

static avl_node_t* avl_balance(avl_node_t* node) {
    avl_fix_height(node);
    if (avl_balance_factor(node) == 2) {
        if (avl_balance_factor(node->rhs) < 0)
            node->rhs = avl_rotr(node->rhs);
        return avl_rotl(node);
    }
    if (avl_balance_factor(node) == -2) {
        if (avl_balance_factor(node->lhs) > 0)
            node->lhs = avl_rotl(node->lhs);
        return avl_rotr(node);
    }
    return node;
}

static avl_node_t* avl_create_node(const void* key, void* data) {
    avl_node_t* node = malloc(sizeof *node);
    assert(node != NULL && "Cannot allocate memory for node.");
    node->lhs = node->rhs = NULL; node->height = 1;
    node->key = key; node->data = data;
    return node;
}

static avl_node_t* avl_insert_node(avl_tree_t* tree, avl_node_t* node, const void* key, void* data) {
    if (!node) return avl_create_node(key, data);
    int cmp = tree->comp(key, node->key);
    /**/ if (cmp < 0) node->lhs = avl_insert_node(tree, node->lhs, key, data);
    else if (cmp > 0) node->rhs = avl_insert_node(tree, node->rhs, key, data);
    else node->data = data;
    return avl_balance(node);
}

static inline avl_node_t* avl_get_min(avl_node_t* node) {
    return !node->lhs ? node : avl_get_min(node->lhs);
}

static avl_node_t* avl_remove_min(avl_node_t* node) {
    if (!node->lhs) return node->rhs;
    node->lhs = avl_remove_min(node->lhs);
    return avl_balance(node);
}

static avl_node_t* avl_remove_node(avl_tree_t* tree, avl_node_t* node, const void* key) {
    if (!node) return NULL;
    int cmp = tree->comp(key, node->key);
    /**/ if (cmp < 0) node->lhs = avl_remove_node(tree, node->lhs, key);
    else if (cmp > 0) node->rhs = avl_remove_node(tree, node->rhs, key);
    else {
        avl_node_t* left = node->lhs;
        avl_node_t* righ = node->rhs;
        free(node);
        if (!righ) return left;
        avl_node_t* min = avl_get_min(righ);
        min->rhs = avl_remove_min(righ);
        min->lhs = left;
        return avl_balance(min);
    }
    return avl_balance(node);
}

typedef struct {
    char* prefix; size_t prefix_len;
    void (*putk)(const void*);
    void (*putd)(/* */ void*);
} avl_outnode_ctx_t;

static void avl_output_node(avl_outnode_ctx_t* ctx, avl_node_t* node, bool is_left) {
    if (!node) return;

    printf("%s%s[", ctx->prefix, is_left ? "|---" : "'---");
    if (ctx->putk) { ctx->putk(node->key ); } printf("] ");
    if (ctx->putd) { ctx->putd(node->data); } putchar(10);

    size_t saved_count = ctx->prefix_len;
    memcpy(ctx->prefix + ctx->prefix_len, is_left ? "|   " : "    ", 5);
    ctx->prefix_len += 4; avl_output_node(ctx, node->lhs, true);

    ctx->prefix_len = saved_count;
    memcpy(ctx->prefix + ctx->prefix_len, is_left ? "|   " : "    ", 5);
    ctx->prefix_len += 4; avl_output_node(ctx, node->rhs, false);
}

/* API definitions */

bool avl_search(const avl_tree_t* tree, const void* key, void** out) {
    avl_node_t* node = tree->root;
    while (node) {
        int cmp = tree->comp(key, node->key);
        /**/ if (cmp < 0) node = node->lhs;
        else if (cmp > 0) node = node->rhs;
        else break;
    }
    if (!node) return false;
    if (out) *out = node->data;
    return true;
}

void avl_insert(avl_tree_t* tree, const void* key, void* data) {
    tree->root = avl_insert_node(tree, tree->root, key, data);
}

void avl_remove(avl_tree_t* tree, const void* key) {
    tree->root = avl_remove_node(tree, tree->root, key);
}

void avl_delete(avl_tree_t* tree) {
    avl_node_t* stack[avl_height(tree->root) + 1];
    size_t stack_size = 1; stack[0] = tree->root;
    while (stack_size) {
        avl_node_t* node = stack[--stack_size];
        if (!node) continue;
        stack[stack_size++] = node->rhs;
        stack[stack_size++] = node->lhs;
        free(node);
    }
    tree->root = NULL;
}

void avl_forall(const avl_tree_t* tree, void (*func)(const void*, void*)) {
    avl_node_t* stack[avl_height(tree->root)];
    avl_node_t* current = tree->root;
    size_t stack_size = 0;
    while (stack_size || current) {
        if (current) {
            stack[stack_size++] = current;
            current = current->lhs;
            continue;
        }
        avl_node_t* node = stack[--stack_size];
        func(node->key, node->data);
        current = node->rhs;
    }
}

void avl_output(const avl_tree_t* tree, void (*putkey)(const void*), void (*putdata)(void*)) {
    if (!tree->root) return;
    avl_outnode_ctx_t ctx = {NULL, 0, putkey, putdata};
    ctx.prefix = malloc(tree->root->height * 4 + 1);
    assert(ctx.prefix != NULL && "Cannot alloc memory for printing tree.");
    memset(ctx.prefix, 0, tree->root->height * 4 + 1);
    avl_output_node(&ctx, tree->root, false);
    free(ctx.prefix);
}