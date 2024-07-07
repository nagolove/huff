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

struct HuffBitRepr {
    unsigned char bits: 8;      // битовый код, от младшего бита к старшему
    unsigned char bits_num: 8;  // сколько битов используется в bits
} __attribute__((packed));

// Оригинальное название?
typedef struct Huff {
    struct HuffBitRepr map[256];
    HuffTreeNode *root, *nodes[1024];
    size_t        nodes_num;
} Huff;

static void huff_tree_free(HuffTreeNode *node);

void huff_init(Huff *h) {
    assert(h);
}

void huff_shutdown(Huff *h) {
    assert(h);
    if (h->root) 
        huff_tree_free(h->root);
}

static void quicksort(int64_t *A, size_t len, int64_t *indices) {
    if (len < 2) return;

    int64_t pivot = A[len / 2], temp, i, j;

    for (i = 0, j = len - 1; ; i++, j--) {
        while (A[i] > pivot) i++;
        while (A[j] < pivot) j--;

        if (i >= j) break;

        temp = A[i];
        A[i]     = A[j];
        A[j]     = temp;

        if (indices) {
            temp = indices[i];
            indices[i]     = indices[j];
            indices[j]     = temp;
        }
    }

    quicksort(A, i, indices);

    if (indices)
        quicksort(A + i, len - i, indices + i);
    else
        quicksort(A + i, len - i, indices);
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
    //printf("'huff_tree_free'\n");

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

/*
static int huff_cmp(const void *a, const void *b) {
    return *(uint64_t*)b - *(uint64_t*)a;
    //return *(uint64_t*)a - *(uint64_t*)b;
}
*/

FILE *dot_file = NULL;
const char *dot_preambule = 
"digraph binary_tree {\n"
"   rankdir=TB; // Top to Bottom, дерево будет вертикальным\n"
"   dpi=300\n" 
"   concentrate = true // исключение повторяющихся связей\n"
"   node [shape=circle, style=filled, color=\"lightblue\"]\n"  
"   edge [color=\"black\"]  // Определяем стиль ребер\n\n";

static void dot_init(const char *dot_fname) {
    // {{{
    assert(dot_file == NULL);
    dot_file = fopen(dot_fname, "w");
    assert(dot_file);

    fprintf(dot_file, "%s", dot_preambule);
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

static int huff_cmp_node(const void *a, const void *b) {
    const HuffTreeNode **_a = a, **_b = b;
    if (*_a && *_b)
        //return (*_a)->cnt - (*_b)->cnt;
        return (*_b)->cnt - (*_a)->cnt;
    else
        return 0;
}
//*/

static bool iter_dot_print_name(HuffTreeNode *node, void *udata) {
    FILE *file = udata;
    assert(file);
    const char *fmt = "%ld [label=\"cnt %d\n val '%c'\"]\n";
    if (node->val == -1)
        fmt = "%ld [label=\"cnt %d\n val %d\"]\n";

    //fprintf(dot_file, fmt, node->cnt, node->cnt, node->val);
    fprintf(file, fmt, (intptr_t)node, node->cnt, node->val);

    return true;
}

static bool iter_dot_print_link(HuffTreeNode *node, void *udata) {
    FILE *file = udata;
    assert(file);
    if (node->left) {
        //fprintf(dot_file, "%d -> %d\n", node->cnt, node->left->cnt);
        //printf("%d -> %d\n", node->cnt, node->left->cnt);
        fprintf(
            file, "%ld -> %ld\n", (intptr_t)node, (intptr_t)node->left
        );
        //printf("%d -> %d\n", node->cnt, node->left->cnt);
    }
    if (node->right) {
        //fprintf(dot_file, "%d -> %d\n", node->cnt, node->right->cnt);
        fprintf(
            file, "%ld -> %ld\n", (intptr_t)node, (intptr_t)node->right
        );
        //printf("%d -> %d\n", node->cnt, node->right->cnt);
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

static void huff_build_prefixes(
    Huff *h, HuffTreeNode *node, int *bits, int bits_num
) {
    if (!node)
        return;

    if (node->val != -1) {
        printf("'huff_build_prefixes', %d = { ", node->val);
        for (int i = 0; i < bits_num; i++) {
            printf("%d, ", bits[i]);
        }
        printf(" }, \n");
    }

    if (node->left) {
        bits[bits_num++] = 1;
        huff_build_prefixes(h, node->left, bits, bits_num);
        bits_num--;
    }
    if (node->right) {
        bits[bits_num++] = 0;
        huff_build_prefixes(h, node->right, bits, bits_num);
        bits_num--;
    }

}

static void dot_tmp_write(HuffTreeNode *preuse_root, int i) {
    char fname[128] = {};
    sprintf(fname, "tmp_%03d.dot", i);
    FILE *tmp_dot_file = fopen(fname, "w");
    fprintf(tmp_dot_file, "%s", dot_preambule);
    huff_tree_walk(preuse_root, iter_dot_print_name, tmp_dot_file);
    huff_tree_walk(preuse_root, iter_dot_print_link, tmp_dot_file);
    fprintf(tmp_dot_file, "\n}\n");
    fclose(tmp_dot_file);
}

// Построить дерево для данных определенной длины.
void huff_tree_build(Huff *h, const char *data, size_t data_sz) {
    assert(h);
    assert(data);

    
    huff_tree_free(h->root);
    h->root = NULL;

    if (!data_sz)
        return;

    int64_t freq[256] = {}, indices[256] = {};

    //memset(freq, 0, sizeof(freq));
    for (size_t i = 0; i < data_sz; i++) {
        freq[(int)data[i]]++;
    }

    const size_t freq_len = sizeof(freq) / sizeof(freq[0]);

    /*
    printf("'huff_tree_load', {\n");
    for (size_t i = 0; i < freq_len; i++) {
        if (freq[i])
            printf(
                "freq[%zu] = %lu,\n", 
                i, freq[i]
            );
    }
    printf("'huff_tree_load', }\n");
    */

    printf("'sorting',\n");

    for (size_t i = 0; i < freq_len; i++) {
        indices[i] = i;
    }

    // Здесь сортируются количества встреч, но теряются индексы
    //qsort(freq, freq_len, sizeof(freq[0]), huff_cmp);
    quicksort(freq, freq_len, indices);

    printf("'huff_tree_load', {\n");
    for (size_t i = 0; i < freq_len; i++) {
        if (freq[i])
            printf(
                "freq[%zu] = %lu,\n", 
                i, freq[i]
            );
    }
    printf("'huff_tree_load', }\n");

    memset(h->nodes, 0, sizeof(h->nodes));
    h->nodes_num = 0;

    for (size_t i = 0; i < freq_len; i++) {
        if (freq[i]) {
            HuffTreeNode *new_node = calloc(1, sizeof(*new_node));
            new_node->val = indices[i];
            new_node->cnt = freq[i];
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

        dot_tmp_write(new, i);

        h->nodes[h->nodes_num - 1] = NULL;
        h->nodes[h->nodes_num - 2] = NULL;

        h->nodes[h->nodes_num - 2] = new;
        h->nodes_num -= 1;

        // XXX: Есть необходимость пересортировать h->nodes?
        qsort(h->nodes, h->nodes_num, sizeof(h->nodes[0]), huff_cmp_node);
        
        i++;
    }
    //*/


    h->root = h->nodes[0];
    huff_tree_walk(h->root, iter_print, NULL);

    huff_tree_walk(h->root, iter_dot_print_name, dot_file);
    huff_tree_walk(h->root, iter_dot_print_link, dot_file);

    //huff_tree_walk(h->root, iter_build_prefixes, h);
    int bits[256] = {};
    huff_build_prefixes(h, h->root, bits, 0);
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

static void test_init_tree_build_short_str() {
    Huff h = {};
    huff_init(&h);

    dot_init("tree_short.dot");

    //                   1234567890
    //const char *input = "AABACDACAZ";
    const char *input = "so much words wow many compression";

    printf("'test_init_tree_build_short_str', input = '%s',\n", input);
    huff_tree_build(&h, input, strlen(input));

    dot_shutdown();

    huff_shutdown(&h);
}

static void test_init_tree_build_long_str() {
    Huff h = {};
    huff_init(&h);

    dot_init("tree_long.dot");

    //                   1234567890
    const char *input =
        "Instead of comparing elements explicitly, this solution puts the two "
        "elements-to-compare in a sum. After evaluating the sum its terms are "
        "sorted. Numbers are sorted numerically, strings alphabetically and "
        "compound expressions by comparing nodes and leafs in a left-to right "
        "order. Now there are three cases: either the terms stayed put, or "
        "they were swapped, or they were equal and were combined into one term "
        "with a factor 2 in front. To not let the evaluator add numbers "
        "together, each term is constructed as a dotted list.";

    printf("'test_init_tree_build_long_str', input = '%s',\n", input);
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
    //printf("sizeof(struct HuffBitRepr) = %zu\n", sizeof(struct HuffBitRepr));
    //exit(0);

    test_init_shutdown();
    //test_init_tree_build_short_str();
    test_init_tree_build_long_str();

    return EXIT_SUCCESS;
}
