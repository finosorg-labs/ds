package ds

/*
#include "generic_ring_buffer.h"
#include <stdlib.h>
*/
import "C"
import (
	"unsafe"
)

// GenericRingBuffer is a generic ring buffer that can store any type of data.
// It uses void* internally and requires the element size to be specified at creation.
type GenericRingBuffer struct {
	ptr         *C.fc_generic_ring_buffer_t
	elementSize int
}

// NewGenericRingBuffer creates a new generic ring buffer with the specified capacity and element size.
// The capacity will be rounded up to the next power of 2.
func NewGenericRingBuffer(capacity int, elementSize int) *GenericRingBuffer {
	if capacity <= 0 || elementSize <= 0 {
		return nil
	}

	ptr := C.fc_generic_ring_buffer_create(C.size_t(capacity), C.size_t(elementSize))
	if ptr == nil {
		return nil
	}

	return &GenericRingBuffer{
		ptr:         ptr,
		elementSize: elementSize,
	}
}

// Destroy frees the ring buffer and its associated memory.
func (rb *GenericRingBuffer) Destroy() {
	if rb.ptr != nil {
		C.fc_generic_ring_buffer_destroy(rb.ptr)
		rb.ptr = nil
	}
}

// Push adds a single element to the ring buffer.
// If the buffer is full, the oldest element is overwritten.
func (rb *GenericRingBuffer) Push(data []byte) bool {
	if rb.ptr == nil || len(data) != rb.elementSize {
		return false
	}

	return bool(C.fc_generic_ring_buffer_push(rb.ptr, unsafe.Pointer(&data[0])))
}

// PushBatch adds multiple elements to the ring buffer.
// If the buffer becomes full during the operation, oldest elements are overwritten.
func (rb *GenericRingBuffer) PushBatch(data []byte) int {
	if rb.ptr == nil || len(data) == 0 || len(data)%rb.elementSize != 0 {
		return 0
	}

	n := len(data) / rb.elementSize
	pushed := C.fc_generic_ring_buffer_push_batch(
		rb.ptr,
		unsafe.Pointer(&data[0]),
		C.size_t(n),
	)

	return int(pushed)
}

// Pop removes and returns a single element from the ring buffer.
func (rb *GenericRingBuffer) Pop() ([]byte, bool) {
	if rb.ptr == nil {
		return nil, false
	}

	out := make([]byte, rb.elementSize)
	success := C.fc_generic_ring_buffer_pop(rb.ptr, unsafe.Pointer(&out[0]))

	if !success {
		return nil, false
	}

	return out, true
}

// PopBatch removes and returns multiple elements from the ring buffer.
func (rb *GenericRingBuffer) PopBatch(n int) []byte {
	if rb.ptr == nil || n <= 0 {
		return nil
	}

	out := make([]byte, n*rb.elementSize)
	popped := C.fc_generic_ring_buffer_pop_batch(
		rb.ptr,
		unsafe.Pointer(&out[0]),
		C.size_t(n),
	)

	if popped == 0 {
		return nil
	}

	return out[:int(popped)*rb.elementSize]
}

// Get retrieves the element at the specified index without removing it.
// Index 0 is the oldest element, Size()-1 is the newest.
func (rb *GenericRingBuffer) Get(index int) ([]byte, bool) {
	if rb.ptr == nil || index < 0 {
		return nil, false
	}

	out := make([]byte, rb.elementSize)
	success := C.fc_generic_ring_buffer_get(rb.ptr, C.size_t(index), unsafe.Pointer(&out[0]))

	if !success {
		return nil, false
	}

	return out, true
}

// GetAll retrieves all elements in order (oldest to newest) without removing them.
func (rb *GenericRingBuffer) GetAll() []byte {
	if rb.ptr == nil {
		return nil
	}

	size := rb.Size()
	if size == 0 {
		return nil
	}

	out := make([]byte, size*rb.elementSize)
	count := C.fc_generic_ring_buffer_get_all(rb.ptr, unsafe.Pointer(&out[0]))

	if count == 0 {
		return nil
	}

	return out[:int(count)*rb.elementSize]
}

// Size returns the number of elements currently in the buffer.
func (rb *GenericRingBuffer) Size() int {
	if rb.ptr == nil {
		return 0
	}
	return int(C.fc_generic_ring_buffer_size(rb.ptr))
}

// Capacity returns the maximum number of elements the buffer can hold.
func (rb *GenericRingBuffer) Capacity() int {
	if rb.ptr == nil {
		return 0
	}
	return int(C.fc_generic_ring_buffer_capacity(rb.ptr))
}

// ElementSize returns the size of each element in bytes.
func (rb *GenericRingBuffer) ElementSize() int {
	if rb.ptr == nil {
		return 0
	}
	return int(C.fc_generic_ring_buffer_element_size(rb.ptr))
}

// IsEmpty returns true if the buffer contains no elements.
func (rb *GenericRingBuffer) IsEmpty() bool {
	if rb.ptr == nil {
		return true
	}
	return bool(C.fc_generic_ring_buffer_is_empty(rb.ptr))
}

// IsFull returns true if the buffer is at capacity.
func (rb *GenericRingBuffer) IsFull() bool {
	if rb.ptr == nil {
		return false
	}
	return bool(C.fc_generic_ring_buffer_is_full(rb.ptr))
}

// Clear removes all elements from the buffer.
func (rb *GenericRingBuffer) Clear() {
	if rb.ptr != nil {
		C.fc_generic_ring_buffer_clear(rb.ptr)
	}
}

// Implementation returns a string describing the implementation type.
func GenericRingBufferImplementation() string {
	return C.GoString(C.fc_generic_ring_buffer_implementation())
}
