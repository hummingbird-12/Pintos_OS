#ifndef VM_FRAME_H
#define VM_FRAME_H

typedef struct _FRAME_TABLE {
    struct hash frame_hash;     /* hash of frame table entries */
    struct lock frame_mutex;    /* mutex lock for frame table */
} FRAME_TABLE;

typedef struct _FRAME_ENTRY {
    struct hash_elem f_elem;    /* hash_elem for frame table */
    struct thread *holder;      /* thread holding the frame */
    void *uframe;               /* frame address (physical address) */
    void *upage;                /* user page address (virtual address) */
} FRAME_ENTRY;

void init_frame_table(void);

#endif /* vm/frame.h */