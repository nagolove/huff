// vim: set colorcolumn=85
// vim: fdm=marker
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

/*
Простая реализация кодирования Хаффмана.
Данные разбиваются на байты.

TODO: Сделать разбивку на слова, сравнить эффективность сжатия на случайных 
данных.

 */

typedef struct HuffTreeNode {
    struct HuffTreeNode *left,  // 1
                        *right; // 0
    int val, // -1 если не лист, а соединительный узел
        cnt; // сколько раз встречается
} HuffTreeNode;

// Оригинальное название?
typedef struct Huff {
    uint64_t     freq[256];
    HuffTreeNode *root, *nodes[1024];
    size_t        nodes_num;
} Huff;

void huff_init(Huff *h) {
    assert(h);
}

void huff_shutdown(Huff *h) {
    assert(h);
}

// Добавляет узел и возвращает новый корень дерева.
/*
static HuffTreeNode *huff_tree_add(HuffTreeNode *root, int cnt, int value) {
    HuffTreeNode *cur = root, *new = calloc(1, sizeof(*new));
    assert(new);
    new->cnt = cnt;
    new->val = value;

    if (!root)
        return new;

    while (true) {
        if (cnt >= cur->cnt) {
            if (!cur->right) {
                cur->right = new;
                return root;
            } else {
                cur = cur->right;
            }
        } else {
            if (!cur->left) {
                cur->left = new;
                return root;
            } else
                cur = cur->left;
        }
    }

    return root;
}
*/

/*
Каждая запись журнала представляет собой правильное Луа выражение.
Все выражения разделены запятыми. Вложенная группировка производится таблицами.
 */

static void huff_tree_free(HuffTreeNode *node) {
    printf("'huff_tree_free'\n");

    if (!node)
        return;

    if (node->left) {
        //printf("'huff_tree_free', 'left'\n");
        huff_tree_free(node->left);
    }
    if (node->right) {
        //printf("'huff_tree_free', 'right'\n");
        huff_tree_free(node->right);
    }

    free(node);
}

static int huff_cmp(const void *a, const void *b) {
    return *(uint64_t*)b - *(uint64_t*)a;
    //return *(uint64_t*)a - *(uint64_t*)b;
}

FILE *dot_file = NULL;

static void dot_init(const char *dot_fname) {
    // {{{
    assert(dot_file == NULL);
    dot_file = fopen(dot_fname, "w");
    assert(dot_file);

    const char *preambule = 
        "digraph binary_tree {\n"
        "node [shape=circle, style=filled, color=\"lightblue\"]\n"  
        "edge [color=\"black\"]  // Определяем стиль ребер\n";

    fprintf(dot_file, "%s", preambule);
    // }}}
}

static void dot_shutdown() {
    // {{{
    assert(dot_file);
    fprintf(dot_file, "%s", "}");
    fclose(dot_file);
    dot_file = NULL;
    // }}}
}

static void huff_tree_walk(
    HuffTreeNode *node, 
    bool (*func)(HuffTreeNode *node, void *udata),
    void *udata
) {
    assert(func);

    if (!node)
        return;

    func(node, udata);

    if (node->left) {
        //fprintf(dot_file, "%d -> %d\n", node->cnt, node->left->cnt);
        huff_tree_walk(node->left, func, udata);
    }
    if (node->right) {
        //fprintf(dot_file, "%d -> %d\n", node->cnt, node->right->cnt);
        huff_tree_walk(node->right, func, udata);
    }

    //printf("'huff_tree_walk', val = %d,\n", node->val);
}

/*
static void huff_tree_ins(
    HuffTreeNode *node, uint64_t *freq, size_t freq_len, size_t free_i
) {
    printf("'huff_tree_ins',\n");

    if (free_i == freq_len) {
        printf("'huff_tree_ins', 'finished'\n");
        return;
    }

    int val = freq[free_i];

    node->left = calloc(1, sizeof(*node->left));
    node->left->val = val;

    node->right = calloc(1, sizeof(*node->left));
    huff_tree_ins(node->right, freq, freq_len, free_i + 1);
}
*/

/*
static int huff_cmp_node(const void *a, const void *b) {
    const HuffTreeNode **_a = a, **_b = b;
    if (*_a && *_b)
        return (*_a)->cnt - (*_b)->cnt;
    else
        return 0;
}
*/

static bool iter_print_dot(HuffTreeNode *node, void *udata) {
    if (node->left) {
        fprintf(dot_file, "%d -> %d\n", node->cnt, node->left->cnt);
        printf("%d -> %d\n", node->cnt, node->left->cnt);
    }
    if (node->right) {
        fprintf(dot_file, "%d -> %d\n", node->cnt, node->right->cnt);
        printf("%d -> %d\n", node->cnt, node->right->cnt);
    }
    return true;
}

static bool iter_print(HuffTreeNode *node, void *udata) {
    printf(
        "'iter_print', node.val = %d, node.cnt = %d\n",
        node->val, node->cnt
    );
    return true;
}

// Построить дерево для данных определенной длины.
void huff_tree_build(Huff *h, const char *data, size_t data_sz) {
    assert(h);
    assert(data);

    
    huff_tree_free(h->root);
    h->root = NULL;

    if (!data_sz)
        return;

    memset(h->freq, 0, sizeof(h->freq));
    for (size_t i = 0; i < data_sz; i++) {
        h->freq[(int)data[i]]++;
    }

    printf("'huff_tree_load', {\n");
    size_t freq_len = sizeof(h->freq) / sizeof(h->freq[0]);
    for (size_t i = 0; i < freq_len; i++) {
        if (h->freq[i])
            printf(
                "freq[%zu] = %lu,\n", 
                i, h->freq[i]
            );
    }
    printf("'huff_tree_load', }\n");

    printf("'sorting',\n");
    qsort(h->freq, freq_len, sizeof(h->freq[0]), huff_cmp);

    printf("'huff_tree_load', {\n");
    for (size_t i = 0; i < freq_len; i++) {
        if (h->freq[i])
            printf(
                "freq[%zu] = %lu,\n", 
                i, h->freq[i]
            );
    }
    printf("'huff_tree_load', }\n");

    memset(h->nodes, 0, sizeof(h->nodes));
    h->nodes_num = 0;

    for (size_t i = 0; i < freq_len; i++) {
        if (h->freq[i]) {
            HuffTreeNode *new_node = calloc(1, sizeof(*new_node));
            new_node->val = i;
            new_node->cnt = h->freq[i];
            h->nodes[h->nodes_num++] = new_node;
        }
    }

    int i = 0;
    while (h->nodes_num != 1) {
        //h->nodes_num--;

        HuffTreeNode *left = h->nodes[h->nodes_num - 1],
                     *right = h->nodes[h->nodes_num - 2],
                     *new = calloc(1, sizeof(*new));

        assert(left);
        assert(right);

        // Правая ветвь всегда больше или равна по количеству встреч
        if (left->cnt > right->cnt) {
            HuffTreeNode *tmp = left;
            left = right;
            right = tmp;
        }

        new->cnt = left->cnt + right->cnt;
        new->left = left;
        new->right = right;
        new->val = -1;

        h->nodes[h->nodes_num - 1] = NULL;
        h->nodes[h->nodes_num - 2] = NULL;

        h->nodes[h->nodes_num - 2] = new;
        //h->nodes_num -= 2;
        h->nodes_num -= 1;

        //h->nodes[h->nodes_num] = new;

        // XXX: Есть необходимость пересортировать h->nodes?
    }
    //*/


    h->root = h->nodes[0];
    huff_tree_walk(h->root, iter_print, NULL);

    huff_tree_walk(h->root, iter_print_dot, NULL);

}

// Загружает дерево(словарь) из данных по указателю.
void huff_tree_load(Huff *h, const char *tree, size_t tree_sz) {

}

// Закодировать дерево для данных определенной длины. Предварительно дерево 
// должно быть построено. Выделяет и возвращет указатель на блок памяти.
void *huff_code(Huff *h, const char *data, size_t data_sz, size_t *out_sz) {
    return NULL;
}

// Записывает дерево в двоичный формат. Выделяет память, возвращает указатель
// на блок.
void *huff_tree_write(Huff *h, size_t *tree_sz) {
    return NULL;
}

static void test_init_tree_build() {
    Huff h = {};
    huff_init(&h);

    dot_init("tree1.dot");

    const char *input = "AABACDACA";
    printf("'test_init_tree_build', input = '%s',\n", input);
    huff_tree_build(&h, input, strlen(input));

    dot_shutdown();

    huff_shutdown(&h);
}


static void test_init_shutdown() {
    Huff h = {};
    huff_init(&h);
    huff_shutdown(&h);
}

int main(int argc, char **argv) {
    test_init_shutdown();
    test_init_tree_build();

    return EXIT_SUCCESS;
}
