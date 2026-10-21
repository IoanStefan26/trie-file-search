# Trie-Based File Search Engine

An in-memory file indexing application written in C. The program associates file identifiers with keywords and relevance scores, supporting exact keyword lookup, prefix search, and ranked top-k queries. Developed as a university data structures coursework project.

## Features

- Add and remove indexed file records.
- Add and remove keywords associated with a file.
- Retrieve files by exact keyword or keyword prefix.
- Rank matching files using heap-based top-k selection.
- Display the keyword index and produce lexicographically ordered search results where applicable.

The implementation combines a trie for keyword indexing, linked lists for file records and keyword references, and dynamic arrays for collecting query results. Allocated data structures are released at program termination.

## Build and Run

Requires GCC and Make. Run from the project root on Linux or WSL:

```bash
make build
cp examples/demo.in indexare.in
./search_index
cat indexare.out
```

The program reads commands from `indexare.in` and writes results to `indexare.out` in the current working directory. It indexes the file metadata supplied in the input; it does not scan directories or read document contents.

To remove the compiled executable:

```bash
make clean
```

## Input Format

The first line specifies the number of commands. Each following command uses one of these forms:

```text
ADD <file_id> <score> <keyword_count> <keyword_1> ... <keyword_n>
DEL <file_id>
ADDKW <file_id> <keyword>
DELKW <file_id> <keyword>
FIND <keyword>
PREFIX <prefix>
TOPK <keyword> <k>
PRINT
```

Keywords use lowercase English letters (`a`–`z`), matching the trie's 26-character alphabet. See `examples/demo.in` and `examples/demo.out` for a complete example.

## Project Structure

```text
tema2.c       Data structures, command handling, and application entry point
Makefile      Build and clean targets
examples/     Sample commands and expected output
```

## Author

Ioan Ștefan
