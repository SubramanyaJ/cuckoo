#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define MAX_TIMES 4

typedef struct regular_table {
	uint32_t table_size;
	char **table_data;
	bool *table_occupied;
} Regular_Table;

typedef struct hash_table {
	Regular_Table *T1; /* djb2 hashes */
	Regular_Table *T2; /* sdbm hashes */
} Hash_Table;

uint32_t hash_sdbm(const char *data, uint32_t table_size) {
	uint32_t hval = 0;
	int c;
	while ((c = *data++)) {
		hval = (hval << 16) + (hval << 6) - hval + c;
	}
	return hval % table_size;
}

uint32_t hash_djb2(const char *data, uint32_t table_size) {
	uint32_t hval = 5381u;
	int c;
	while ((c = *data++)) {
		hval = ((hval << 5) + hval) + c;
	}
	return hval % table_size;
}

void regular_table_init(Regular_Table *regular_table, size_t initial_size) {
	regular_table->table_size = initial_size;
	regular_table->table_data = calloc(initial_size, sizeof(char *));
	regular_table->table_occupied = calloc(initial_size, sizeof(bool));
}

void hash_table_init(Hash_Table *hash_table, size_t initial_size) {
	hash_table->T1 = calloc(1, sizeof(Regular_Table));
	hash_table->T2 = calloc(1, sizeof(Regular_Table));
	regular_table_init(hash_table->T1, initial_size);
	regular_table_init(hash_table->T2, initial_size);
}

void hash_table_free(Hash_Table *hash_table) {
	free(hash_table->T1->table_data);
	free(hash_table->T1->table_occupied);
	free(hash_table->T1);
	free(hash_table->T2->table_data);
	free(hash_table->T2->table_occupied);
	free(hash_table->T2);
	printf("Table freed.\n");

	return;
}

uint32_t hash_table_lookup(Hash_Table *hash_table, char *data, uint8_t *found) {
	uint32_t idx = hash_djb2(data, hash_table->T1->table_size);
	if ( hash_table->T1->table_occupied[idx] &&
		 !strcmp(data, hash_table->T1->table_data[idx])) {
	 	// printf("\" %s \" present at T1[%u]\n", hash_table->T1->table_data[idx], idx);
		*found = 1;
	 	return idx;
	}

	idx = hash_sdbm(data, hash_table->T2->table_size);
	if ( hash_table->T2->table_occupied[idx] &&
		 !strcmp(data, hash_table->T2->table_data[idx])) {
	 	// printf("\" %s \" present at T2[%u]\n", hash_table->T2->table_data[idx], idx);
		*found = 2;
	 	return idx;
	}

	// printf("\"%s\" is not present in the hash table.\n", data);
	*found = 0;
	return 0;
}

void hash_table_search(Hash_Table *hash_table, char *data) {
	uint8_t found_buffer;
	uint32_t idx = hash_table_lookup(hash_table, data, &found_buffer);
	if (found_buffer == 0) {
		// printf("\"%s\" not in the table.\n", data);
		return;
	}
	// printf("\"%s\" found at T%u[%u].\n", data, found_buffer, idx);
	return;
}

void hash_table_insert(Hash_Table *hash_table, char *input) {
	uint8_t found;
	char *data = strdup(input);

	uint32_t idx = hash_table_lookup(hash_table, data, &found);
	if(found != 0) { free(data); return; } /* Data already exists */

	char *swap_buffer;
	for(int i = 0; i < MAX_TIMES; i++) {

		idx = hash_djb2(data, hash_table->T1->table_size);
		if (hash_table->T1->table_occupied[idx] == false) {

			hash_table->T1->table_occupied[idx] = true;
			hash_table->T1->table_data[idx] = calloc(sizeof(char), 1 + strlen(data));

			strcpy(hash_table->T1->table_data[idx], data);
			// printf("\"%s\" inserted at T1[%u].\n", data, idx);
			free(data);
			return;
		}

		swap_buffer = hash_table->T1->table_data[idx];
		hash_table->T1->table_data[idx] = data;
		data = swap_buffer;
		// printf("\t\"%s\" inserted at T1[%u].\n", hash_table->T1->table_data[idx], idx);

		idx = hash_sdbm(data, hash_table->T2->table_size);
		if (hash_table->T2->table_occupied[idx] == false) {

			hash_table->T2->table_occupied[idx] = true;
			hash_table->T2->table_data[idx] = calloc(sizeof(char), 1 + strlen(data));

			strcpy(hash_table->T2->table_data[idx], data);
			// printf("\"%s\" inserted at T2[%u].\n", data, idx);
			free(data);
			return;
		}

		swap_buffer = hash_table->T2->table_data[idx];
		hash_table->T2->table_data[idx] = data;
		data = swap_buffer;
		// printf("\t\"%s\" inserted at T1[%u].\n", hash_table->T2->table_data[idx], idx);

	}

	// rehash();
	size_t new_size = ((hash_table->T1->table_size - 1) << 1) + 1;
	Hash_Table *new_table = calloc(1, sizeof(Hash_Table));
	hash_table_init(new_table, new_size);
	printf("Reallocation to table with size %u.\n", new_size);

	for(size_t idx = 0; idx < hash_table->T1->table_size; idx++) {
		if (hash_table->T1->table_occupied[idx]) {
			hash_table_insert(new_table, hash_table->T1->table_data[idx]);
		}
	}
	for(size_t idx = 0; idx < hash_table->T2->table_size; idx++) {
		if (hash_table->T2->table_occupied[idx]) {
			hash_table_insert(new_table, hash_table->T2->table_data[idx]);
		}
	}

	hash_table_free(hash_table);
	*hash_table = *new_table;
	hash_table_insert(hash_table, data);
}

void hash_table_remove(Hash_Table *hash_table, char *data) {
	uint8_t found;
	uint32_t idx = hash_table_lookup(hash_table, data, &found);
	if (found == 0) {
		return;
	}

	if (found == 1) {
		hash_table->T1->table_occupied[idx] = false;
	 	// printf("\" %s \" deleted from T1[%u]\n", hash_table->T1->table_data[idx], idx);
		free(hash_table->T1->table_data[idx]);
	}
	else if (found == 2) {
		hash_table->T2->table_occupied[idx] = false;
	 	// printf("\" %s \" deleted from T2[%u]\n", hash_table->T2->table_data[idx], idx);
		free(hash_table->T2->table_data[idx]);
	}
	return;
}

void hash_table_display(Hash_Table *hash_table) {
	for(size_t idx = 0; idx < hash_table->T1->table_size; idx++) {
		if(hash_table->T1->table_occupied[idx] == true) {
			printf("T1[%u] : \"%s\"\n", idx, hash_table->T1->table_data[idx]);
		}
	}
	printf("\n");
	for(size_t idx = 0; idx < hash_table->T2->table_size; idx++) {
		if(hash_table->T2->table_occupied[idx] == true) {
			printf("T2[%u] : \"%s\"\n", idx, hash_table->T2->table_data[idx]);
		}
	}
}

int main() {

	uint8_t fb;
	Hash_Table htable = {0};
	hash_table_init(&htable, 3);

	hash_table_display(&htable);

	hash_table_free(&htable);

	return 0;
}
