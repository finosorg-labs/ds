package ds

/*
#include "ring_buffer.h"
#include <stdlib.h>
*/
import "C"
import (
	"errors"
	"unsafe"
)

var (
	ErrNullBuffer   = errors.New("ring buffer is nil")
	ErrInvalidIndex = errors.New("index out of bounds")
	ErrEmptyBuffer  = errors.New("buffer is empty")
	ErrInvalidInput = errors.New("invalid input parameters")
)

// RingBuffer is a fixed-size circular buffer for efficient sliding window operations.
// It automatically overwrites the oldest elements when full.
type RingBuffer struct {
	ptr *C.fc_ring_buffer_t
}

// NewRingBuffer creates a new ring buffer with the specified capacity.
// The actual capacity will be rounded up to the next power of 2.
func NewRingBuffer(capacity int) (*RingBuffer, error) {
	if capacity <= 0 {
		return nil, ErrInvalidInput
	}

	ptr := C.fc_ring_buffer_create(C.size_t(capacity))
	if ptr == nil {
		return nil, errors.New("failed to create ring buffer")
	}

	return &RingBuffer{ptr: ptr}, nil
}

// Destroy frees the ring buffer's memory.
// The buffer should not be used after calling this method.
func (rb *RingBuffer) Destroy() {
	if rb.ptr != nil {
		C.fc_ring_buffer_destroy(rb.ptr)
		rb.ptr = nil
	}
}

// Push adds a single element to the buffer.
// If the buffer is full, the oldest element is overwritten.
func (rb *RingBuffer) Push(value float64) error {
	if rb.ptr == nil {
		return ErrNullBuffer
	}

	if !C.fc_ring_buffer_push(rb.ptr, C.double(value)) {
		return errors.New("push failed")
	}

	return nil
}

// PushBatch adds multiple elements to the buffer.
// If the buffer becomes full, oldest elements are overwritten.
func (rb *RingBuffer) PushBatch(values []float64) (int, error) {
	if rb.ptr == nil {
		return 0, ErrNullBuffer
	}

	if len(values) == 0 {
		return 0, nil
	}

	n := C.fc_ring_buffer_push_batch(
		rb.ptr,
		(*C.double)(unsafe.Pointer(&values[0])),
		C.size_t(len(values)),
	)

	return int(n), nil
}

// Pop removes and returns the oldest element from the buffer.
func (rb *RingBuffer) Pop() (float64, error) {
	if rb.ptr == nil {
		return 0, ErrNullBuffer
	}

	var val C.double
	if !C.fc_ring_buffer_pop(rb.ptr, &val) {
		return 0, ErrEmptyBuffer
	}

	return float64(val), nil
}

// PopBatch removes and returns up to n oldest elements from the buffer.
func (rb *RingBuffer) PopBatch(n int) ([]float64, error) {
	if rb.ptr == nil {
		return nil, ErrNullBuffer
	}

	if n <= 0 {
		return []float64{}, nil
	}

	out := make([]float64, n)
	popped := C.fc_ring_buffer_pop_batch(
		rb.ptr,
		(*C.double)(unsafe.Pointer(&out[0])),
		C.size_t(n),
	)

	return out[:int(popped)], nil
}

// Get returns the element at the specified index (0 = oldest, Size()-1 = newest).
func (rb *RingBuffer) Get(index int) (float64, error) {
	if rb.ptr == nil {
		return 0, ErrNullBuffer
	}

	var val C.double
	if !C.fc_ring_buffer_get(rb.ptr, C.size_t(index), &val) {
		return 0, ErrInvalidIndex
	}

	return float64(val), nil
}

// GetAll returns all elements in the buffer in order (oldest to newest).
func (rb *RingBuffer) GetAll() ([]float64, error) {
	if rb.ptr == nil {
		return nil, ErrNullBuffer
	}

	size := rb.Size()
	if size == 0 {
		return []float64{}, nil
	}

	out := make([]float64, size)
	count := C.fc_ring_buffer_get_all(
		rb.ptr,
		(*C.double)(unsafe.Pointer(&out[0])),
	)

	return out[:int(count)], nil
}

// Size returns the number of elements currently in the buffer.
func (rb *RingBuffer) Size() int {
	if rb.ptr == nil {
		return 0
	}
	return int(C.fc_ring_buffer_size(rb.ptr))
}

// Capacity returns the maximum capacity of the buffer.
func (rb *RingBuffer) Capacity() int {
	if rb.ptr == nil {
		return 0
	}
	return int(C.fc_ring_buffer_capacity(rb.ptr))
}

// IsEmpty returns true if the buffer contains no elements.
func (rb *RingBuffer) IsEmpty() bool {
	if rb.ptr == nil {
		return true
	}
	return bool(C.fc_ring_buffer_is_empty(rb.ptr))
}

// IsFull returns true if the buffer is at maximum capacity.
func (rb *RingBuffer) IsFull() bool {
	if rb.ptr == nil {
		return false
	}
	return bool(C.fc_ring_buffer_is_full(rb.ptr))
}

// Clear removes all elements from the buffer.
func (rb *RingBuffer) Clear() {
	if rb.ptr != nil {
		C.fc_ring_buffer_clear(rb.ptr)
	}
}
