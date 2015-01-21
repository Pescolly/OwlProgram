/* Copyright 2014 Section6. All Rights Reserved. */

#ifndef _MESSAGE_QUEUE_H_
#define _MESSAGE_QUEUE_H_

#include "HvMessage.h"
#include "MessagePool.h"

struct HvBase;

typedef struct MessageNode {
  struct MessageNode *prev; // doubly linked list
  struct MessageNode *next;
  int let;
  HvMessage *m;
  void (*sendMessage)(struct HvBase *, int, const HvMessage *);
} MessageNode;

/** A doubly linked list containing scheduled messages. */
typedef struct MessageQueue {
  MessageNode *head; // the head of the queue
  MessageNode *tail; // the tail of the queue
  MessageNode *pool; // the head of the reserve pool
  MessagePool mp;
} MessageQueue;

hv_size_t mq_init(MessageQueue *q);

void mq_initWithPoolSize(MessageQueue *q, hv_size_t poolSizeKB);

void mq_free(MessageQueue *q);

int mq_size(MessageQueue *q);

static inline HvMessage *mq_node_getMessage(MessageNode *n) {
  return n->m;
}

static inline int mq_node_getLet(MessageNode *n) {
  return n->let;
}

static inline bool mq_hasMessage(MessageQueue *q) {
  return (q->head != NULL);
}

// true if there is a message and it occurs before (<) timestamp
static inline bool mq_hasMessageBefore(MessageQueue *const q, const hv_uint32_t timestamp) {
  return mq_hasMessage(q) && (msg_getTimestamp(mq_node_getMessage(q->head)) < timestamp);
}

static inline MessageNode *mq_peek(MessageQueue *q) {
  return q->head;
}

/** Appends the message to the end of the queue. */
HvMessage *mq_addMessage(MessageQueue *q, const HvMessage *m, int let,
    void (*sendMessage)(struct HvBase *, int, const HvMessage *));

/** Insert in ascending order the message acccording to its timestamp. */
HvMessage *mq_addMessageByTimestamp(MessageQueue *q, HvMessage *m, int let,
    void (*sendMessage)(struct HvBase *, int, const HvMessage *));

/** Pop the message at the head of the queue (and free its memory). */
void mq_pop(MessageQueue *q);

/** Remove a message from the queue (and free its memory) */
void mq_removeMessage(MessageQueue *q, HvMessage *m);

/** Clears (and frees) all messages in the queue. */
void mq_clear(MessageQueue *q);

/** Removes all messages occuring at or after the given timestamp. */
void mq_clearAfter(MessageQueue *q, const double timestamp);

#endif // _MESSAGE_QUEUE_H_
