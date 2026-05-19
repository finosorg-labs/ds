#ifndef FC_RING_BUFFER_H
#define FC_RING_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Ring buffer structure for efficient sliding window operations
 *
 * A lock-free ring buffer implementation optimized for single-producer
 * single-consumer scenarios. Capacity is always a power of 2 for efficient
 * modulo operations using bitwise AND.
 */
typedef struct {
    double* data;    /**< Aligned data array */
    size_t capacity; /**< Capacity (power of 2) */
    size_t head;     /**< Write position */
    size_t count;    /**< Current element count */
} fc_ring_buffer_t;

/**
 * @brief Create a new ring buffer
 *
 * @param capacity Desired capacity (will be rounded up to next power of 2)
 * @return Pointer to newly created ring buffer, or NULL on failure
 *
 * Time complexity: O(1)
 * Space complexity: O(capacity)
 * Thread safety: Not thread-safe
 */
fc_ring_buffer_t* fc_ring_buffer_create(size_t capacity);

/**
 * @brief Destroy a ring buffer and free its memory
 *
 * @param rb Ring buffer to destroy
 *
 * Time complexity: O(1)
 * Thread safety: Not thread-safe
 */
void fc_ring_buffer_destroy(fc_ring_buffer_t* rb);

/**
 * @brief Push a single element to the ring buffer
 *
 * If buffer is full, the oldest element is overwritten.
 *
 * @param rb Ring buffer
 * @param value Value to push
 * @return true on success, false if rb is NULL
 *
 * Time complexity: O(1)
 * Thread safety: Single-producer safe
 */
bool fc_ring_buffer_push(fc_ring_buffer_t* rb, double value);

/**
 * @brief Push multiple elements to the ring buffer
 *
 * If buffer becomes full during the operation, oldest elements are overwritten.
 *
 * @param rb Ring buffer
 * @param values Array of values to push
 * @param n Number of values to push
 * @return Number of elements successfully pushed, 0 if rb or values is NULL
 *
 * Time complexity: O(n)
 * Thread safety: Single-producer safe
 */
size_t fc_ring_buffer_push_batch(fc_ring_buffer_t* rb, const double* values, size_t n);

/**
 * @brief Pop a single element from the ring buffer
 *
 * @param rb Ring buffer
 * @param out Pointer to store the popped value
 * @return true if element was popped, false if buffer is empty or rb/out is NULL
 *
 * Time complexity: O(1)
 * Thread safety: Single-consumer safe
 */
bool fc_ring_buffer_pop(fc_ring_buffer_t* rb, double* out);

/**
 * @brief Pop multiple elements from the ring buffer
 *
 * @param rb Ring buffer
 * @param out Array to store popped values
 * @param n Maximum number of values to pop
 * @return Number of elements actually popped, 0 if rb or out is NULL
 *
 * Time complexity: O(n)
 * Thread safety: Single-consumer safe
 */
size_t fc_ring_buffer_pop_batch(fc_ring_buffer_t* rb, double* out, size_t n);

/**
 * @brief Get element at specific index (0 = oldest, count-1 = newest)
 *
 * @param rb Ring buffer
 * @param index Index of element to retrieve
 * @param out Pointer to store the value
 * @return true if element was retrieved, false if index out of bounds or rb/out is NULL
 *
 * Time complexity: O(1)
 * Thread safety: Read-only, safe with single writer
 */
bool fc_ring_buffer_get(const fc_ring_buffer_t* rb, size_t index, double* out);

/**
 * @brief Get all elements in order (oldest to newest)
 *
 * @param rb Ring buffer
 * @param out Array to store elements (must have space for at least count elements)
 * @return Number of elements copied, 0 if rb or out is NULL
 *
 * Time complexity: O(count)
 * Thread safety: Read-only, safe with single writer
 */
size_t fc_ring_buffer_get_all(const fc_ring_buffer_t* rb, double* out);

/**
 * @brief Get the number of elements currently in the buffer
 *
 * @param rb Ring buffer
 * @return Number of elements, or 0 if rb is NULL
 *
 * Time complexity: O(1)
 * Thread safety: Read-only, safe with single writer
 */
size_t fc_ring_buffer_size(const fc_ring_buffer_t* rb);

/**
 * @brief Get the capacity of the buffer
 *
 * @param rb Ring buffer
 * @return Capacity, or 0 if rb is NULL
 *
 * Time complexity: O(1)
 * Thread safety: Read-only, safe
 */
size_t fc_ring_buffer_capacity(const fc_ring_buffer_t* rb);

/**
 * @brief Check if the buffer is empty
 *
 * @param rb Ring buffer
 * @return true if empty, false otherwise (or if rb is NULL)
 *
 * Time complexity: O(1)
 * Thread safety: Read-only, safe with single writer
 */
bool fc_ring_buffer_is_empty(const fc_ring_buffer_t* rb);

/**
 * @brief Check if the buffer is full
 *
 * @param rb Ring buffer
 * @return true if full, false otherwise (or if rb is NULL)
 *
 * Time complexity: O(1)
 * Thread safety: Read-only, safe with single writer
 */
bool fc_ring_buffer_is_full(const fc_ring_buffer_t* rb);

/**
 * @brief Clear all elements from the buffer
 *
 * @param rb Ring buffer
 *
 * Time complexity: O(1)
 * Thread safety: Not thread-safe
 */
void fc_ring_buffer_clear(fc_ring_buffer_t* rb);

#ifdef __cplusplus
}
#endif

#endif // FC_RING_BUFFER_H
