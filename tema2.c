/* Stefan Ioan 311CA */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define ALPHABET_SIZE 26
#define MAX_COMMAND_SIZE 20

#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
			__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);					\
		}							\
	} while (0)

typedef struct file_node {
	char *id;
	int score;
	int nr_keys; //key sunt cuvintele cheie
	int key_capacity; //capacitatea vectorului de chei
	char **keys;
	struct file_node *prev;
	struct file_node *next;
} file_node;

typedef struct list {
	file_node *head;
	file_node *tail;
} list;

/* copiaza un sir de caractere in memoria nou alocata 
o folosim ca sa avem fiecare pointer spre o zona diferita
de memorie */
char *copy_string(char *string)
{
	int len = strlen(string);
	char *copy = malloc((len + 1) * sizeof(char));
	DIE(copy == NULL, "alocare esuata");
	strcpy(copy, string);
	return copy;
}

//initializam lista
void init_list(list *list_files)
{
	list_files->head = NULL;
	list_files->tail = NULL;
}

//cauta in lista un fisier dupa id
file_node *search_files(list *files, char *id)
{
	file_node *curr = files->head;
	while (curr != NULL) {
		if (strcmp(curr->id, id) == 0) {
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

// gaseste cheia in vectorul de cuvinte cheie
int find_key_pos(file_node *file, char *key)
{
	for (int i = 0; i < file->nr_keys; i++) {
		if (strcmp(file->keys[i], key) == 0) {
			return i;
		}
	}
	return -1; //daca nu are cheia
}

//folosim functia anterioara ca sa verificam daca
//un fisier are cuvantul cheie respectiv
//1 => exista deja cheia
int has_key(file_node *file, char *key)
{
	return find_key_pos(file, key) != -1;
}

/* mareste vectorul de chei cand e plin
cam ca un realloc, dar doar la nevoie*/
void realloc_key_capacity(file_node *file)
{
	if (file->nr_keys < file->key_capacity) return;
	int new_capacity;
	if (file->key_capacity == 0) {
		new_capacity = 2;
	} else {
		new_capacity = file->key_capacity * 2;
	}

	char **new_keys = realloc(file->keys, new_capacity * sizeof(char*));
	DIE(new_keys == NULL, "realocare esuata");
	file->keys = new_keys;
	file->key_capacity = new_capacity;
}

//adauga cheia intr-un file/fisier daca aceasta nu exista deja
int add_key(file_node *file, char *key)
{
	if (has_key(file, key)) {
		return 0;
	}

	realloc_key_capacity(file);
	file->keys[file->nr_keys] = copy_string(key);
	//punem cheia pe prima pozitie libera
	file->nr_keys++;
	return 1;
}

/*returneaza 1 daca fisierul exista si 0 daca 
el nu exista. Prin inserted returneaza 1 daca cheia e noua si a fost
adaugata sau 0 daca cheia exista deja, deci nu a fost adaugata*/
int add_key_final(list *files, char *id, char *key, int *inserted)
{
	if (inserted) {
		*inserted = 0;
	}

	file_node *file = search_files(files, id);
	if (file == NULL) {
		return 0;
	}
	int rez = add_key(file, key);
	if (inserted) {
		*inserted = rez;
	}

	return 1;
}

//creeza un fisier cu datele date
file_node* create_file(char *id, int score, int nr_keys, char **keys)
{
	file_node *file = malloc(sizeof(file_node));
	DIE(file == NULL, "alocare esuata");
	file->id = copy_string(id);
	file->score = score;
	file->nr_keys = 0;
	file->key_capacity = 0;
	file->keys = NULL;
	file->prev = NULL;
	file->next = NULL;

	for( int i = 0; i < nr_keys; i++) {
		add_key(file, keys[i]);
	}
	return file;
}

//da free la un fisier
void free_file(file_node *file)
{
	if (file == NULL) {
		return;
	}
	free(file->id);
	
	for (int i = 0; i < file->nr_keys; i++) {
		free(file->keys[i]);
	}
	free(file->keys);
	free(file);
}

//adauga un fisier la sfarsitul listei
void add_file(list *files, file_node *file)
{
	if (files->head == NULL) {
		files->head = file;
		files->tail = file;
		return;
	}
	file->next = NULL;
	file->prev = files->tail;
	files->tail->next = file;
	files->tail = file;
}

//scoate un fisier din lista si il returneaza
//cam ca un cut care da ctrl+V dupa
//Poate returna NULL
file_node* cut_file(list *files, file_node *file)
{
	if (file == NULL) {
		return NULL;
	}

	if (file->prev != NULL) {
		file->prev->next = file->next;
	} else {
		files->head = file->next;
	}

	if (file->next == NULL) {
		files->tail = file->prev;
	} else {
		file->next->prev = file->prev;
	}

	file->prev = NULL;
	file->next = NULL;
	return file;
}

//asta il scoate si ii da si free
void remove_file(list *files, file_node *file)
{
	file_node *file_to_free = cut_file(files, file);
	free_file(file_to_free);
}

//functia complexa care ia add_file
//si adauga un fisier nou verificand ca acesta sa nu existe deja
//tot ea creeaza si nodul pe care il returneaza
file_node* add_new_file_and_verif(list *files, char *id, int score, 
								  int nr_keys, char **keys)
{
	if (search_files(files, id) != NULL) {
		return NULL;
	}

	file_node *new_file = create_file(id, score, nr_keys, keys);
	if (new_file == NULL) {
		return NULL;
	}

	add_file(files, new_file);
	return new_file;
}

/* sterge un fisier dupa id
returneaza 1 pentru OK si 0 pentru NOT FOUND*/
int delete_file_by_id(list *files, char *id)
{
	file_node *file = search_files(files, id);

	if (!file) {
		return 0;
	}
	remove_file(files, file);
	return 1;
}

/*sterge o cheie dintr-un fisier existent si il sterge si pe el
daca nu mai are chei. returneaza 1 pt ok si 0 daca nu gaseste fisierul
In deleted_file punem 0 daca nu a fost sters fisierul din lista si 1 daca da
In removed spunem 1 daca cheia a fost stearsa din fisier si 0 altfel*/
int delete_key(list *files, char *id, char *key, int *removed,
			   int *deleted_file)
{
	if (removed) {
		*removed = 0; 
	}
	if (deleted_file) {
		*deleted_file = 0;
	}

	file_node *file = search_files(files, id);
	if (file == NULL) {
		return 0;
	}

	int pos = find_key_pos(file, key);
	if (pos == -1) {
		return 1;
	}

	free(file->keys[pos]);
	for (int i = pos; i < file->nr_keys - 1; i++) {
		file->keys[i] = file->keys[i + 1];
	}

	file->nr_keys--;
	file->keys[file->nr_keys] = NULL;

	if (removed) {
		*removed = 1;
	}

	if (file->nr_keys == 0) {
		remove_file(files, file);

		if (deleted_file != NULL) {
			*deleted_file = 1;
		}
	}

	return 1;
}

//da free la lista dubla de fisiere
void free_list(list *files)
{
	file_node *curr = files->head;

	while (curr != NULL) {
		file_node *next = curr->next;
		free_file(curr);
		curr = next;
	}

	files->head = NULL;
	files->tail = NULL;
}

/*----------TRIE----------*/

//aici tinem nodurile unei liste simpplle inlantuite de 
//referinte catre fisierele care au keywordul respectiv in trie
typedef struct ref_node {
	file_node *file;
	struct ref_node *next;
} ref_node;

//nod din trie
typedef struct trie_node {
	char letter;
	int is_terminal;
	ref_node *refferences;
	struct trie_node *children[ALPHABET_SIZE];
} trie_node;

//intoarce indexul unei anummite litere
int get_letter_index(char c)
{
	return c - 'a';
}

//creeaza un nod de trie
trie_node *create_trie_node(char letter)
{
	trie_node *node = malloc(sizeof(trie_node));
	DIE(node == NULL, "alocare esuata");

	node->letter = letter;
	node->is_terminal = 0;
	node->refferences = NULL;

	for (int i = 0; i < ALPHABET_SIZE; i++) {
		node->children[i] = NULL;
	}

	return node;
}

//verifica daca un fisier exista in lista de referinte
int is_file_in_refferences(ref_node *refs, file_node *file)
{
	ref_node *curr = refs;

	while (curr != NULL) {
		if (curr->file == file) {
			return 1;
		}
		curr = curr->next;
	}

	return 0; //nu E referinta
}

/*adauga o referinta catre un fisier in nodul terminal*/
void add_file_ref(trie_node *node, file_node *file)
{
	if (is_file_in_refferences(node->refferences, file)) {
		return;
	}
	ref_node* new_ref = malloc(sizeof(ref_node));
	DIE(new_ref == NULL, "alocare esuata");

	new_ref->file = file;
	new_ref->next = node->refferences;
	node->refferences = new_ref;
}

//sterge o referinta catre un fisier dintr-un nod terminal
//returneaza 1 daca a sters ceva si 0 daca nu a gasit referinta
int remove_file_refference(trie_node *node, file_node *file)
{
	ref_node *curr = node->refferences;
	ref_node *previous = NULL; //nodul anterior

	while (curr != NULL) {
		if (curr->file == file) {
			if (previous == NULL) {
				node->refferences = curr->next;
			} else {
				previous->next = curr->next;
			}

			free(curr);
			return 1;
		}

		previous = curr;
		curr = curr->next;
	}

	return 0;
}

//insereaza un keyword in trie si adauga referinta catre fisier
void insert_keyword_ref(trie_node *root, char *key, file_node *file)
{
	trie_node *curr = root;

	for (int i = 0; key[i] != '\0'; i++) {
		int index = get_letter_index(key[i]);

		if (curr->children[index] == NULL) {
			curr->children[index] = create_trie_node(key[i]);
		}
		curr = curr->children[index];
	}

	curr->is_terminal = 1;
	add_file_ref(curr, file);
}

/*cauta nodul terminal al unui cuvant cheie
returneaza NULL daca el nu exista in totalitate*/
trie_node *search_keyword_node(trie_node *root, char *key)
{
	trie_node *curr = root;
	for (int i = 0; key[i] != '\0'; i++) {
		int index = get_letter_index(key[i]);
		if (curr->children[index] == NULL) {
			return NULL;
		}

		curr = curr->children[index];
	}

	if (curr->is_terminal == 0) {
		return NULL;
	}

	return curr;
}

//verifica daca un nod din trie are copii ( 1 daca are  si 0 daca nu)
int trie_node_has_children(trie_node *node)
{
	for (int i = 0; i < ALPHABET_SIZE; i++) {
		if (node->children[i] != NULL) {
			return 1;
		}
	}
	
	return 0;
}

/*sterge recursiv o referinta din lista de referinte si elimina
 nodurile care devin apoi inutile
returneaza 1 daca nodul curent trebuie sters de parintele sau*/
int remove_ref_from_node(trie_node *node, char *key, int depth,
						 file_node *file)
{
	if (key[depth] == '\0') {
		remove_file_refference(node, file);

		if (node->refferences == NULL) {
			node->is_terminal = 0;
		}
		if (node->is_terminal == 0 && trie_node_has_children(node) == 0) {
			return 1;
		}

		return 0;
	}

	int index = get_letter_index(key[depth]);
	if (node->children[index] == NULL) {
		return 0; //nu stergem nimic
	}

	int delete_child = remove_ref_from_node(node->children[index],
											key, depth + 1, file);
	if (delete_child == 1) {
		free(node->children[index]);
		node->children[index] = NULL;
	}

	if (node->is_terminal == 0 && trie_node_has_children(node) == 0) {
		return 1;
	}

	return 0;
}

//da drumul la functia recursiva de mai sus, adica sterge o referinta din
//lista si nodurile care apoi nu mai au sens in trie
void remove_keyword_ref_start(trie_node *root, char *key, file_node *file) {
	remove_ref_from_node(root, key, 0, file);
	//pornim stergerea recursiva de la radacina
}

/*numara cate fisiere sunt asociate unui keyword*/
int count_refferences(ref_node *refferences)
{
	int cnt = 0;
	ref_node *curr = refferences;

	while (curr != NULL) {
		cnt++;
		curr = curr->next;
	}

	return cnt;
}

/*elibereaza lista de referinte*/
void free_refferences(ref_node *refferences)
{
	ref_node *curr = refferences;
	while (curr != NULL) {
		ref_node *next = curr->next;
		free(curr);
		curr = next;
	}
}

//elibereaza tot trie ul
void free_trie(trie_node *node)
{
	if (node == NULL) {
		return;
	}

	for (int i = 0; i < ALPHABET_SIZE; i++) {
		free_trie(node->children[i]);
	}

	free_refferences(node->refferences);
	free(node);
}

/*--------------HEAP-------------*/

typedef struct heap {
	file_node **data; //vector de pointeri catre fisiere
	int size; //nr de elemente din heap
	int capacity; //capacitatea vectorului
} heap;

//initializeaza un heap cu o capacitate data
void new_heap(heap *heap, int capacity)
{
	heap->size = 0;
	heap->capacity = capacity;
	heap->data = NULL;

	if (capacity > 0) {
		heap->data = malloc(capacity * sizeof(file_node*));
		DIE(heap->data == NULL, "alocare esuata");
	}
}

//functia de comparare pentru heap
int cmp_heap(file_node *a, file_node *b)
{
	if (a->score > b->score) {
		return 1;
	}
	if (a->score < b->score) {
		return 0;
	}

	if (strcmp(a->id, b->id) < 0) {
		return 1;
	} else {
		return 0;
	}
}

//interschimbam doua pozitii din heap
void swap_heap_nodes(file_node **a, file_node **b)
{
	file_node *aux = *a;
	*a = *b;
	*b = aux;
}

//repar heap-ul coborand elementul pana la locul lui adevarat
//ii zic elm pentru ca repara un singur element, nu tot heapul
void fix_heap_elm(heap *heap, int index)
{
	int best = index;
	int left = 2 * index + 1;
	int right = 2 * index + 2;

	if (left < heap->size && cmp_heap(heap->data[left], heap->data[best])) {
		best = left;
	}
	if (right < heap->size && cmp_heap(heap->data[right], heap->data[best])) {
		best = right;
	}

	if (best != index) {
		swap_heap_nodes(&(heap->data[index]), &(heap->data[best]));
		fix_heap_elm(heap, best);
	}
}

//construieste heapul din lista de referinte
void build_heap(heap *heap, ref_node *refferences)
{
	int nr_files = count_refferences(refferences);
	new_heap(heap, nr_files);

	ref_node *curr = refferences;
	while (curr) {
		heap->data[heap->size] = curr->file;
		heap->size++;
		curr = curr->next;
	}

	for (int i = heap->size / 2 - 1; i >= 0; i--) {
		//la frunze nu umblam
		fix_heap_elm(heap, i);
	}


}

/* scoate fisierul cel mai bun din heap (radacina)*/
file_node *heap_pop(heap *heap)
{
	if (heap->size == 0) {
		return NULL;
	}

	file_node *root = heap->data[0];
	heap->size--;;

	if (heap->size > 0) {
		heap->data[0] = heap->data[heap->size];
		heap->data[heap->size] = NULL;
		fix_heap_elm(heap, 0);
	}
	return root;
}

//elibereaza heapul
void free_heap(heap *heap)
{
	free(heap->data);
	heap->data = NULL;
	heap->size = 0;
	heap->capacity = 0;
}

/*------------------------INPUT/OUTPUT----------------------------*/
/*deschide fisierul pentru citire*/
FILE *open_input_file()
{
	FILE *in = fopen("indexare.in", "r");
	DIE(in == NULL, "fisierul nu a fost deschis");
	return in;
}

//deschide fisierul pentru afisare
FILE *open_output_file()
{
	FILE *out = fopen("indexare.out", "w");
	DIE(out == NULL, "fisierul nu a fost deschis");
	return out;
}

void close_files(FILE *in, FILE *out)
{
	if (in != NULL) {
		fclose(in);
	}
	if (out != NULL) {
		fclose(out);
	}
}

//citeste nr de operatii
int read_number_of_commands(FILE *in)
{
	int q = 0;
	int result = fscanf(in, "%d", &q);
	DIE(result != 1, "citire nr operatii esuata");
	return q;
}

/*-----------FUNCTIILE-EXECUTE-------------*/

//executa ADD
void execute_add(FILE *in, FILE *out, list *files, trie_node *root)
{
	char id[200];
	int score;
	int nr_keys = 0;

	int result = fscanf(in, "%199s %d %d", id, &score, &nr_keys);
	DIE(result != 3, "citire esuata");

	char **keys = NULL;
	if (nr_keys > 0) {
		keys = malloc(nr_keys * sizeof(char*));
		DIE(keys == NULL, "alocare esuata");
	}

	for (int i = 0; i < nr_keys; i++)  {
		char key[101];
		result = fscanf(in, "%100s", key);
		DIE(result != 1, "citire esuata");
		keys[i] = copy_string(key); //a intors pointerul la un sir nou creat
	}

	file_node *new_file = add_new_file_and_verif(files, id, score,
												 nr_keys, keys);
	if (new_file == NULL) {
		fprintf(out, "EXISTS\n");
	} else {
		for (int i = 0; i < new_file->nr_keys; i++) {
			insert_keyword_ref(root, new_file->keys[i], new_file);
		}
		fprintf(out, "OK\n");
	}

	for (int i = 0; i < nr_keys; i++) {
		free(keys[i]);
	}
	free(keys);
}

//executa DEL
void execute_del(FILE *in, FILE *out, list *files, trie_node *root)
{
	char id[201];
	int result = fscanf(in, "%200s", id);
	DIE(result != 1, "ccitire esuata");

	file_node *file = search_files(files, id);
	if (file == NULL) {
		fprintf(out, "NOT FOUND\n");
		return;
	}

	for (int i = 0; i < file->nr_keys; i++) {
		remove_keyword_ref_start(root, file->keys[i], file);
	}

	remove_file(files, file); //stergem fisierul din lista
	fprintf(out, "OK\n");
}

//executa ADDKW
void execute_addkw(FILE *in, FILE *out, list *files, trie_node *root)
{
	char id[201];
	char key[101];
	int result = fscanf(in, "%200s %100s", id, key);
	DIE(result != 2, "citire esuata");

	file_node *file = search_files(files, id);
	if (file == NULL) {
		fprintf(out, "NOT FOUND\n");
		return;
	}

	int inserted = add_key(file, key);
	if (inserted == 1) {
		insert_keyword_ref(root, key, file);
	}
	fprintf(out, "OK\n");
}

//executa DELKW
void execute_delkw(FILE *in, FILE *out, list *files, trie_node *root)
{
	char id[201];
	char key[101];
	int result = fscanf(in, "%200s %100s", id, key);
	DIE(result != 2, "citire esuata");

	file_node *file = search_files(files, id);
	if (file == NULL) {
		fprintf(out, "NOT FOUND\n");
		return;
	}

	int pos = find_key_pos(file, key);
	if (pos != -1) {
		int removed_sem = 0;
		int deleted_file_sem = 0;
		remove_keyword_ref_start(root, key, file);
		delete_key(files, id, key, &removed_sem, &deleted_file_sem);
	}
	fprintf(out, "OK\n");
}

/*------------functii pentru FIND--------------*/

/*afiseaza pe o singura linie toate fisierele dupa id*/
void print_files_id(FILE *out, file_node **files, int cnt)
{
	fprintf(out, "%d", cnt);
	for (int i = 0; i < cnt; i++) {
		fprintf(out, " %s", (files[i])->id);
	}
	fprintf(out, "\n");

}

//functia cmp pe care i-o dam lui qsort
int cmp_files_by_id(const void *a, const void *b)
{
	file_node *a1 = *(file_node **)a;
	file_node *b1 = *(file_node **)b;

	return strcmp(a1->id, b1->id);
}

/*transforma lista de fisiere intr-un vector de fisiere sortat dupa id*/
file_node **get_sorted_files_from_ref(ref_node *refferences, int *cnt)
{
	*cnt = count_refferences(refferences);
	if (*cnt == 0) {
		return NULL;
	}

	file_node **files = malloc((*cnt) * sizeof(file_node*));
	DIE(files == NULL, "alocare esuata");

	ref_node *curr = refferences;
	int place = 0;
	while (curr != NULL) {
		files[place] = curr->file;
		place++;
		curr = curr->next;
	}

	//e ca sortul din c++
	qsort(files, *cnt, sizeof(file_node*), cmp_files_by_id);
	return files;
}

//afiseaza referintele din nodul terminal dat in ordine lexicografica
//dupa id
void print_refferences_sorted(FILE *out, ref_node *refferences)
{
	int cnt = 0;
	file_node **files = get_sorted_files_from_ref(refferences, &cnt);
	print_files_id(out, files, cnt);
	free(files);
}

//executa FIND
void execute_find(FILE *in, FILE *out, trie_node *root)
{
	char key[101];
	int result = fscanf(in, "%100s", key);
	DIE(result != 1, "citire esuata");

	trie_node *node = search_keyword_node(root, key);
	if (node == NULL || node->refferences == NULL) {
		fprintf(out, "EMPTY\n");
		return;
	}
	print_refferences_sorted(out, node->refferences);
}

//executa TOPK
void execute_topk(FILE *in, FILE *out, trie_node *root)
{
	char key[101];
	int k;
	int result = fscanf(in, "%100s %d", key, &k);
	DIE(result != 2, "citire esuata");

	trie_node *node = search_keyword_node(root, key);
	if (node == NULL || node->refferences == NULL || k <= 0) {
		fprintf(out, "EMPTY\n");
		return;
	}

	heap heap;
	build_heap(&heap, node->refferences);
	if (k > heap.size) {
		 k = heap.size;
	}
	fprintf(out, "%d", k);
	for (int i = 0; i < k; i++) {
		file_node *file = heap_pop(&heap);
		fprintf(out, " %s", file->id);
	}
	fprintf(out, "\n");
	free_heap(&heap);
}

/*-----------aici pun functii pentru PRINT--------------------*/

//afiseaza un cuvant din trie si fisierele asociate lui
void print_word_with_refs(FILE *out, char *word, ref_node *refs)
{
	int cnt = 0;
	file_node **files = get_sorted_files_from_ref(refs, &cnt);
	fprintf(out, "%s %d", word, cnt);
	for (int i = 0; i < cnt; i++) {
		fprintf(out, " %s", files[i]->id);
	}
	fprintf(out, "\n");
	free(files);
}

//parcurge trieul in ordine alfabetica si afiseaza cuvintele terminale
void print_trie_words(trie_node *node, FILE *out, char *word, int depth,
					  int *gasit)
{
	if (node == NULL) {
		return;
	}

	if (node->is_terminal == 1 && node->refferences != NULL) {
		word[depth] = '\0';
		print_word_with_refs(out, word, node->refferences);
		*gasit = 1;
	}

	for (int i = 0; i < ALPHABET_SIZE; i++) {
		if (node->children[i] != NULL) {
			word[depth] = (char)('a' + i);
			print_trie_words(node->children[i], out, word, depth + 1, gasit);
		}
	}
}

//executa PRINT
void execute_print(FILE *out, trie_node *root)
{
	char word[101];
	int gasit = 0;
	print_trie_words(root, out, word, 0, &gasit);
	if (gasit == 0) {
		fprintf(out, "EMPTY\n");
	}
}

/*-----------------aici punem functiile pentru PREFIX-----------------*/

//e nevoie de aceasta structura pentru a nu afisa un fisier
//de doua ori
typedef struct file_vector {
	file_node **data;
	int size;
	int max_size;
} file_vector;

//gaseste nodul unde se termina un prefix
trie_node *search_prefix_end(trie_node *root, char *prefix)
{
	trie_node *curr = root;
	for (int i = 0; prefix[i] != '\0'; i++) {
		int index = get_letter_index(prefix[i]);
		if (curr->children[index] == NULL) {
			return NULL;
		}
		curr = curr->children[index];
	}
	return curr;
}

//initilizeaza vectorul auziliar pentru prefix
void init_file_vector(file_vector *vector)
{
	vector->data = NULL;
	vector->size = 0;
	vector->max_size = 0;
}

//elibereaza vectorul auxiliar
void free_file_vector(file_vector *vector)
{
	free(vector->data);
	vector->data = NULL;
	vector->size = 0;
	vector->max_size = 0;
}

/*verifica daca un fisier e deja in vectorul auxiliar,
intoarce 1 daca e acelasi pointer si 0 altfel*/
int is_file_in_file_vector(file_vector *vector, file_node *file)
{
	for (int i = 0; i < vector->size; i++) {
		if (vector->data[i] == file) {
			return 1;
		}
	}
	return 0;
}

/*adauga un fisier doar daca nu exista deja*/
void add_unique_file_to_vector(file_vector *vector, file_node *file)
{
	if (is_file_in_file_vector(vector, file) == 1) {
		return;
	}

	if (vector->size == vector->max_size) {
		int new_max_size;
		if (vector->max_size == 0) {
			new_max_size = 2;
		} else {
			new_max_size = vector->max_size * 2;
		}
		file_node **new_data = realloc(vector->data,
									   new_max_size * sizeof(file_node *));
		DIE(new_data == NULL, "alocare esuata");
		vector->data = new_data;
		vector->max_size = new_max_size;
	}

	vector->data[vector->size] = file;
	vector->size++;
}

//aduna toate prefixele din subarbore
void add_prefix_files(trie_node *node, file_vector *vector)
{
	if (node == NULL) {
		return;
	}

	if (node->is_terminal == 1 && node->refferences != NULL) {
		ref_node *curr = node->refferences;
		while (curr != NULL) {
			add_unique_file_to_vector(vector, curr->file);
			curr = curr->next;
		}
	}

	for (int i = 0; i < ALPHABET_SIZE; i++) {
		add_prefix_files(node->children[i], vector);
	}
}

//executa PREFIX
void execute_prefix(FILE *in, FILE *out, trie_node *root)
{
	char prefix[101];
	int result = fscanf(in, "%100s", prefix);
	DIE(result != 1, "citirre esuata");

	trie_node *node = search_prefix_end(root, prefix);
	if (node == NULL) {
		fprintf(out, "EMPTY\n");
		return;
	}

	file_vector file_vector;
	init_file_vector(&file_vector);
	add_prefix_files(node, &file_vector);

	if (file_vector.size == 0) {
		fprintf(out, "EMPTY\n");
	} else {
		qsort(file_vector.data, file_vector.size, sizeof(file_node *),
			  cmp_files_by_id);
		print_files_id(out, file_vector.data, file_vector.size);
	}

	free_file_vector(&file_vector);
}

//citeste numele comenzii
void read_command(FILE *in, FILE *out, list *files, trie_node *root)
{
	char command[MAX_COMMAND_SIZE];
	int result = fscanf(in, "%19s", command);
	DIE(result != 1, "citire esuata");

	if (strcmp(command, "ADD") == 0) {
		execute_add(in, out, files, root);
	} else if (strcmp(command, "DEL") == 0) {
		execute_del(in, out, files, root);
	} else if (strcmp(command, "ADDKW") == 0) {
		execute_addkw(in, out, files, root);	
	} else if (strcmp(command, "DELKW") == 0) {
		execute_delkw(in, out, files, root);
	} else if (strcmp(command, "FIND") == 0) {
		execute_find(in, out, root);
	} else if (strcmp(command, "TOPK") == 0) {
		execute_topk(in, out, root);
	} else if (strcmp(command, "PRINT") == 0) {
		execute_print(out, root);
	} else if (strcmp(command, "PREFIX") == 0) {
		execute_prefix(in, out, root);
	}
}

/*------------------MAIN------------------*/

int main(void)
{
	FILE *in = open_input_file();
	FILE *out = open_output_file();

	int nr_com = read_number_of_commands(in); //adica Q

	list files;
	init_list(&files); 

	trie_node *root = create_trie_node('\0');

	for (int i = 0; i < nr_com; i++) {
		read_command(in, out, &files, root);
	}

	free_trie(root);
	free_list(&files);
	close_files(in, out);

	return 0;
}