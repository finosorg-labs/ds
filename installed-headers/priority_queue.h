#ifndef FC_PRIORITY_QUEUE_H
#define FC_PRIORITY_QUEUE_H

#include <stdbool.h>
#include <stddef.h>

/**
 * @file priority_queue.h
 * @brief Binary heap-based priority queue for efficient priority-based ordering
 *
 * A max-heap implementation where the highest priority element is always at the root.
 * Suitable for event scheduling, task ordering, and order matching price priority.
 *
 * Time Complexity:
 *   - Insert: O(log n)
 *   - Pop (extract max): O(log n)
 *   - Peek (get max): O(1)
 *   - Build heap: O(n)
 *
 * Space Complexity: O(n)
 *
 * Usage Scenarios:
 *   - Event-driven systems with ≥100 subscribers
 *   - Task scheduling with priorities
 *   - Order matching engine price priority
 *   - Time-based event queues
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Priority queue element structure
 */
typedef struct {
    double priority; /**< Priority value (higher = higher priority) */
    void* data;      /**< User data pointer */
} fc_pq_element_t;

/**
 * @brief Priority queue structure
 */
typedef struct {
    fc_pq_element_t* elements; /**< Heap array */
    size_t size;               /**< Current number of elements */
    size_t capacity;           /**< Maximum capacity */
} fc_priority_queue_t;

/**
 * @brief Create a new priority queue
 *
 * @param capacity Maximum number of elements
 * @return Pointer to newly created priority queue, or NULL on failure
 *
 * Time complexity: O(1)
 * Space complexity: O(capacity)
 * Thread safety: Not thread-safe
 */
fc_priority_queue_t* fc_priority_queue_create(size_t capacity);

/**
 * @brief Destroy a priority queue and free its memory
 *
 * @param pq Priority queue to destroy
 *
 * Time complexity: O(1)
 * Thread safety: Not thread-safe
 */
void fc_priority_queue_destroy(fc_priority_queue_t* pq);

/**
 * @brief Insert an element into the priority queue
 *
 * @param pq Priority queue
 * @param priority Priority value (higher = higher priority)
 * @param data User data pointer
 * @return true on success, false if pq is NULL or queue is full
 *
 * Time complexity: O(log n)
 * Thread safety: Requires external synchronization
 */
bool fc_priority_queue_insert(fc_priority_queue_t* pq, double priority, void* data);

/**
 * @brief Remove and return the highest priority element
 *
 * @param pq Priority queue
 * @param out_priority Pointer to store the priority value
 * @param out_data Pointer to store the user data
 * @return true if element was popped, false if queue is empty or pq is NULL
 *
 * Time complexity: O(log n)
 * Thread safety: Requires external synchronization
 */
bool fc_priority_queue_pop(fc_priority_queue_t* pq, double* out_priority, void** out_data);

/**
 * @brief Get the highest priority element without removing it
 *
 * @param pq Priority queue
 * @param out_priority Pointer to store the priority value
 * @param out_data Pointer to store the user data
 * @return true if element exists, false if queue is empty or pq is NULL
 *
 * Time complexity: O(1)
 * Thread safety: Requires external synchronization for writes
 */
bool fc_priority_queue_peek(const fc_priority_queue_t* pq, double* out_priority, void** out_data);

/**
 * @brief Get the number of elements in the queue
 *
 * @param pq Priority queue
 * @return Number of elements, or 0 if pq is NULL
 *
 * Time complexity: O(1)
 * Thread safety: Requires external synchronization for writes
 */
size_t fc_priority_queue_size(const fc_priority_queue_t* pq);

/**
 * @brief Get the capacity of the queue
 *
 * @param pq Priority queue
 * @return Capacity, or 0 if pq is NULL
 *
 * Time complexity: O(1)
 * Thread safety: Always safe (immutable after creation)
 */
size_t fc_priority_queue_capacity(const fc_priority_queue_t* pq);

/**
 * @brief Check if the queue is empty
 *
 * @param pq Priority queue
 * @return true if empty, false otherwise (or if pq is NULL)
 *
 * Time complexity: O(1)
 * Thread safety: Requires external synchronization for writes
 */
bool fc_priority_queue_is_empty(const fc_priority_queue_t* pq);

/**
 * @brief Check if the queue is full
 *
 * @param pq Priority queue
 * @return true if full, false otherwise (or if pq is NULL)
 *
 * Time complexity: O(1)
 * Thread safety: Requires external synchronization for writes
 */
bool fc_priority_queue_is_full(const fc_priority_queue_t* pq);

/**
 * @brief Clear all elements from the queue
 *
 * @param pq Priority queue
 *
 * Time complexity: O(1)
 * Thread safety: Not thread-safe
 */
void fc_priority_queue_clear(fc_priority_queue_t* pq);

/**
 * @brief Build a heap from an array of elements
 *
 * More efficient than inserting elements one by one.
 *
 * @param pq Priority queue
 * @param elements Array of elements
 * @param n Number of elements
 * @return true on success, false if pq/elements is NULL or n > capacity
 *
 * Time complexity: O(n)
 * Thread safety: Not thread-safe
 */
bool fc_priority_queue_heapify(fc_priority_queue_t* pq, const fc_pq_element_t* elements, size_t n);

#ifdef __cplusplus
}
#endif

#endif /* FC_PRIORITY_QUEUE_H */
