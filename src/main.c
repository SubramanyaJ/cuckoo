#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define TABLE_DEFAULT_SIZE 71

uint32_t hash_djb2(const char *data, uint32_t table_size) {
	int hval = 5381u;
	int c;
	while ((c = *data++)) {
		hval = ((hval << 5) + hval) + c;
	}
	return hval % table_size;
}

typedef struct hash_table {
	uint32_t table_size;
	char **table_data;
	bool *table_occupied;
} Hash_Table;

void hash_table_init(Hash_Table *ptr) {
	ptr->table_size = TABLE_DEFAULT_SIZE;
	ptr->table_data = calloc(TABLE_DEFAULT_SIZE, sizeof(char *));
	ptr->table_occupied = calloc(TABLE_DEFAULT_SIZE, sizeof(bool));
}

void hash_table_insert(Hash_Table *hash_table, char *data) {
	uint32_t idx = hash_djb2(data, hash_table->table_size);

	if (hash_table->table_occupied[idx]) { // collision
		printf("Slot %u occupied by \"%s\", cannot insert \"%s\".\n", idx, hash_table->table_data[idx], data);

		uint32_t original_idx = idx;

		while (true) {
			idx = (idx + 1) % hash_table->table_size;
			if (!(hash_table->table_occupied[idx])) {
				break;
			}
			if (idx == original_idx) {
				printf("Linear probing failed, table is full.\n");
				return;
			}
		}
		printf("Linear probing finds slot %d instead.\n", idx);
	}

	hash_table->table_occupied[idx] = true;
	hash_table->table_data[idx] = calloc(sizeof(char), 1 + strlen(data));
	strcpy(hash_table->table_data[idx], data);


	printf("\"%s\" inserted at %u.\n", data, idx);
	return;

}

void hash_table_get(Hash_Table *hash_table, char *data) {
	uint32_t idx = hash_djb2(data, hash_table->table_size);

	if (hash_table->table_occupied[idx] && !strcmp(data, hash_table->table_data[idx])) {
		printf("\"%s\" present at %u\n", hash_table->table_data[idx], idx);
		return;
	}

	printf("\"%s\" is not present in the hash table.\n", data);
}

int main() {

	Hash_Table ptr = {0};
	hash_table_init(&ptr);

	for(int i = 0; i < 18; i++) {
		hash_table_insert(&ptr, "Hello, mom!");
	}

	return 0;
}
