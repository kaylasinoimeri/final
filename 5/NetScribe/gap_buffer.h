#include <stdbool.h>

#ifndef _GAP_BUFFER
#define _GAP_BUFFER
#define MAX_USERS 8

struct gap_buffer {
    int limit;      // Maximum capacity of a single buffer segment
    unsigned char** buffer;
    int[MAX_USERS] gap_start;
    int[MAX_USERS] gap_end;
};
typedef struct gap_buffer* gapbuf;

bool is_gapbuf(gapbuf G);                       // Checks if G is a valid gap buffer
bool gapbuf_empty(gapbuf G);                    // Returns true if G is empty
bool gapbuf_full(gapbuf G);                     // Returns true if G is full
bool gap_at_left(gapbuf G, int user);           // Returns true if 'user' has a gap (cursor) at the left of G
bool gap_at_right(gapbuf G, int user);          // Returns true if 'user' has a gap (cursor) at the right of G
gapbuf new_gapbuf(int limit);                   // Returns a new gapbuf with size limit
void gapbuf_forward(gapbuf G, int user);        // Moves gapbuf for user forward
void gapbuf_backward(gapbuf G, int user);       // Moves gapbuf for user backward
void gapbuf_insert(gapbuf G, char c, int user); // Inserts char c before the gap for user
void gapbuf_delete(gapbuf G, int user);         // Deletes the character before the gap for user
#endif
