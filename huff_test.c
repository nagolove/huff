#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>

/*
Простая реализация кодирования Хаффмана.
 */

// Оригинальное название?
typedef struct Huff {
    uint64_t freq[256];
} Huff;

void huff_init(Huff *h) {
    assert(h);
}

void huff_shutdown(Huff *h) {
    assert(h);
}

// Построить дерево для данных определенной длины.
void huff_tree_build(Huff *h, const char *data, size_t data_sz) {
    assert(h);
    assert(data);

    memset(h->freq, 0, sizeof(h->freq));
    for (size_t i = 0; i < data_sz; i++) {
        h->freq[(int)data[i]]++;
    }


    printf("'huff_tree_load', {\n");
    for (size_t i = 0; i < data_sz; i++) {
    }
    printf("'huff_tree_load', {\n");
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

    const char *input = "AABACDACA";
    huff_tree_build(&h, input, strlen(input));

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
