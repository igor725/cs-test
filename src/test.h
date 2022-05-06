#ifndef TEST_H
#define TEST_H
#include <core.h>

/**
 * Указанные тут ID будут использованы у клиентов, не
 * поддерживающих создание блоков через CPE.
 * Соответственно блоки должны быть из стандартного
 * набора ID.
*/
#define FALLBACK_BLOCK_ID 1
#define FALLBACK_BLOCK_ID_EXT 2

/* ID новых блоков */
#define BLOCK_ID 47
#define BLOCK_ID_EXT 48
#define BLOCK_ID_DYN 49
#endif // TEST_H
