#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_BOOKS   200
#define MAX_TITLE   64
#define MAX_AUTHOR  48

// Book structure definition
typedef struct {
    int  id;                       // unique identifier for each book
    char title[MAX_TITLE];         // book title
    char author[MAX_AUTHOR];       // author name
    int  year;                     // publication year
    int  available;                // 1=available, 0=borrowed
} Book;

// ---- function prototypes ----
void trim_newline(char *s);                      // remove trailing newline from input
int  read_line(char *buf, size_t n);             // safely read a line into buffer
int  read_int(const char *prompt);               // read an integer with validation

int  add_book(Book *arr, int *count, const char *title, const char *author, int year);
int  find_by_id(Book *arr, int count, int id);
int  find_first_title(Book *arr, int count, const char *query);
int  find_first_author(Book *arr, int count, const char *query);
int  borrow_book(Book *arr, int count, int id);
int  return_book(Book *arr, int count, int id);
void list_books(Book *arr, int count);

int main(void)
{
    Book books[MAX_BOOKS];   // array to hold all books
    int  count = 0;          // how many books are currently stored
    int  next_id = 1;        // auto-incrementing ID for new books

    for (;;)
    {
        // Print the menu
        printf("\n--- Mini Library ---\n");
        printf("1) Add book\n");
        printf("2) Search by title\n");
        printf("3) Search by author\n");
        printf("4) Borrow by ID\n");
        printf("5) Return by ID\n");
        printf("6) List all\n");
        printf("0) Exit\n");

        // Read user choice
        int choice = read_int("Choose: ");
        if (choice == 0) break;   // exit condition

        if (choice == 1) {
            // Add book
            if (count >= MAX_BOOKS) { puts("Library is full."); continue; }
            char title[MAX_TITLE], author[MAX_AUTHOR];
            printf("Title : ");  read_line(title,  sizeof(title));
            printf("Author: ");  read_line(author, sizeof(author));
            int year = read_int("Year  : ");

            // Add the book using add_book
            int idx = add_book(books, &count, title, author, year);
            if (idx >= 0) {
                books[idx].id = next_id++;   // assign unique ID
                puts("Added.");
            } else {
                puts("Add failed.");
            }
        }
        else if (choice == 2) {
            // Search by title (case-insensitive substring)
            char q[MAX_TITLE];
            printf("Title contains: "); read_line(q, sizeof(q));
            int i = find_first_title(books, count, q);
            if (i >= 0)
                printf("Found: [ID %d] \"%s\" by %s (%d) %s\n",
                    books[i].id, books[i].title, books[i].author, books[i].year,
                    books[i].available ? "available" : "borrowed");
            else
                puts("No match.");
        }
        else if (choice == 3) {
            // Search by author
            char q[MAX_AUTHOR];
            printf("Author contains: "); read_line(q, sizeof(q));
            int i = find_first_author(books, count, q);
            if (i >= 0)
                printf("Found: [ID %d] \"%s\" by %s (%d) %s\n",
                    books[i].id, books[i].title, books[i].author, books[i].year,
                    books[i].available ? "available" : "borrowed");
            else
                puts("No match.");
        }
        else if (choice == 4) {
            // Borrow a book by ID
            int id = read_int("ID to borrow: ");
            if (borrow_book(books, count, id)) puts("Borrowed.");
            else puts("Not available / not found.");
        }
        else if (choice == 5) {
            // Return a book by ID
            int id = read_int("ID to return: ");
            if (return_book(books, count, id)) puts("Returned.");
            else puts("Not found or already in shelf.");
        }
        else if (choice == 6) {
            // List all books
            list_books(books, count);
        }
        else {
            // Invalid menu choice
            puts("Unknown choice.");
        }
    }

    puts("Bye!");
    return 0;
}

// ---- helper functions ----

// Remove trailing newline from string (if present)
void trim_newline(char *s) {
    size_t n = strlen(s);
    if (n && s[n-1] == '\n') s[n-1] = '\0';
}

// Read a line of input safely into buffer
int read_line(char *buf, size_t n) {
    if (!fgets(buf, (int)n, stdin)) return 0;
    trim_newline(buf);
    return 1;
}

// Read and validate an integer from user input
int read_int(const char *prompt) {
    char line[64];
    for (;;) {
        printf("%s", prompt);
        if (!fgets(line, sizeof(line), stdin)) return 0;
        char *end;
        long v = strtol(line, &end, 10);  // convert string to long
        // valid if at least one digit and rest is newline or null
        if (end != line && (*end == '\n' || *end == '\0')) return (int)v;
        puts("Please enter a valid integer.");
    }
}

// Add a new book to the array
int add_book(Book *arr, int *count, const char *title, const char *author, int year) {
    if (*count >= MAX_BOOKS) return -1;   // library full
    Book *b = &arr[*count];
    strncpy(b->title,  title,  MAX_TITLE-1);  b->title[MAX_TITLE-1] = '\0';
    strncpy(b->author, author, MAX_AUTHOR-1); b->author[MAX_AUTHOR-1] = '\0';
    b->year = year;
    b->available = 1;                     // mark as available
    (*count)++;
    return *count - 1; // return index of new book
}

// Find a book index by its ID
int find_by_id(Book *arr, int count, int id) {
    for (int i = 0; i < count; i++)
        if (arr[i].id == id) return i;
    return -1;
}

// Case-insensitive substring check
static int contains_ci(const char *hay, const char *needle) {
    size_t H = strlen(hay), N = strlen(needle);
    if (N == 0) return 1;
    for (size_t i = 0; i + N <= H; i++) {
        size_t k = 0;
        while (k < N) {
            char a = (char)tolower((unsigned char)hay[i+k]);
            char b = (char)tolower((unsigned char)needle[k]);
            if (a != b) break;
            k++;
        }
        if (k == N) return 1;  // substring found
    }
    return 0;
}

// Find first book by title substring
int find_first_title(Book *arr, int count, const char *query) {
    for (int i = 0; i < count; i++)
        if (contains_ci(arr[i].title, query)) return i;
    return -1;
}

// Find first book by author substring
int find_first_author(Book *arr, int count, const char *query) {
    for (int i = 0; i < count; i++)
        if (contains_ci(arr[i].author, query)) return i;
    return -1;
}

// Borrow a book if available
int borrow_book(Book *arr, int count, int id) {
    int i = find_by_id(arr, count, id);
    if (i < 0) return 0;           // not found
    if (!arr[i].available) return 0; // already borrowed
    arr[i].available = 0;          // mark as borrowed
    return 1;
}

// Return a borrowed book
int return_book(Book *arr, int count, int id) {
    int i = find_by_id(arr, count, id);
    if (i < 0) return 0;          // not found
    if (arr[i].available) return 0; // already in shelf
    arr[i].available = 1;         // mark as available
    return 1;
}

// Print the entire library in table format
void list_books(Book *arr, int count) {
    if (count == 0) { puts("(no books)"); return; }
    printf("\n%-5s | %-32s | %-22s | %-4s | %s\n",
           "ID", "Title", "Author", "Year", "Status");
    puts("-------------------------------------------------------------------------------");
    for (int i = 0; i < count; i++) {
        printf("%-5d | %-32s | %-22s | %-4d | %s\n",
               arr[i].id, arr[i].title, arr[i].author, arr[i].year,
               arr[i].available ? "available" : "borrowed");
    }
}
