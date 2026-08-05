package ds

/*
#include "priority_queue.h"
#include <stdlib.h>
*/
import "C"
import (
	"errors"
	"sync"
	"unsafe"
)

var (
	ErrNullPriorityQueue = errors.New("priority queue is nil")
	ErrQueueFull         = errors.New("queue is full")
	ErrQueueEmpty        = errors.New("queue is empty")
)

// PriorityQueue is a binary heap-based priority queue where higher values have higher priority.
// The highest priority element is always at the root and can be accessed in O(1) time.
type PriorityQueue struct {
	ptr      *C.fc_priority_queue_t
	dataSlice []interface{}  // Use slice instead of map for better performance
	freeIDs   []uintptr      // Recycle freed IDs
	nextID    uintptr
	mu        sync.Mutex
}

// Element represents a priority queue element with a priority and associated data.
type Element struct {
	Priority float64
	Data     interface{}
}

// NewPriorityQueue creates a new priority queue with the specified capacity.
func NewPriorityQueue(capacity int) (*PriorityQueue, error) {
	if capacity <= 0 {
		return nil, ErrInvalidInput
	}

	ptr := C.fc_priority_queue_create(C.size_t(capacity))
	if ptr == nil {
		return nil, errors.New("failed to create priority queue")
	}

	return &PriorityQueue{
		ptr:       ptr,
		dataSlice: make([]interface{}, 0, capacity),
		freeIDs:   make([]uintptr, 0, 32),
		nextID:    0,
	}, nil
}

// Destroy frees the priority queue's memory.
// The queue should not be used after calling this method.
func (pq *PriorityQueue) Destroy() {
	if pq.ptr != nil {
		C.fc_priority_queue_destroy(pq.ptr)
		pq.ptr = nil
		pq.dataSlice = nil
		pq.freeIDs = nil
	}
}

// allocID allocates a new ID, reusing freed IDs when possible
func (pq *PriorityQueue) allocID(data interface{}) uintptr {
	var id uintptr

	// Try to reuse a freed ID first
	if len(pq.freeIDs) > 0 {
		id = pq.freeIDs[len(pq.freeIDs)-1]
		pq.freeIDs = pq.freeIDs[:len(pq.freeIDs)-1]
		pq.dataSlice[id] = data
	} else {
		// Allocate new ID
		id = pq.nextID
		pq.nextID++
		pq.dataSlice = append(pq.dataSlice, data)
	}

	return id
}

// freeID marks an ID as available for reuse
func (pq *PriorityQueue) freeID(id uintptr) {
	if id < uintptr(len(pq.dataSlice)) {
		pq.dataSlice[id] = nil
		pq.freeIDs = append(pq.freeIDs, id)
	}
}

// Insert adds an element with the specified priority to the queue.
// Higher priority values are returned first.
// Returns an error if the queue is full or nil.
func (pq *PriorityQueue) Insert(priority float64, data interface{}) error {
	if pq.ptr == nil {
		return ErrNullPriorityQueue
	}

	pq.mu.Lock()
	id := pq.allocID(data)
	pq.mu.Unlock()

	// Store the ID as a pointer value (not dereferencing, just as an opaque handle)
	// We need to convert through uintptr to avoid go vet warnings
	var idPtr unsafe.Pointer
	*(*uintptr)(unsafe.Pointer(&idPtr)) = id

	if !C.fc_priority_queue_insert(pq.ptr, C.double(priority), idPtr) {
		pq.mu.Lock()
		pq.freeID(id)
		pq.mu.Unlock()
		return ErrQueueFull
	}

	return nil
}

// Pop removes and returns the highest priority element from the queue.
func (pq *PriorityQueue) Pop() (*Element, error) {
	if pq.ptr == nil {
		return nil, ErrNullPriorityQueue
	}

	var priority C.double
	var data unsafe.Pointer

	if !C.fc_priority_queue_pop(pq.ptr, &priority, &data) {
		return nil, ErrQueueEmpty
	}

	id := uintptr(data)
	pq.mu.Lock()
	var goData interface{}
	if id < uintptr(len(pq.dataSlice)) {
		goData = pq.dataSlice[id]
	}
	pq.freeID(id)
	pq.mu.Unlock()

	return &Element{
		Priority: float64(priority),
		Data:     goData,
	}, nil
}

// Peek returns the highest priority element without removing it.
func (pq *PriorityQueue) Peek() (*Element, error) {
	if pq.ptr == nil {
		return nil, ErrNullPriorityQueue
	}

	var priority C.double
	var data unsafe.Pointer

	if !C.fc_priority_queue_peek(pq.ptr, &priority, &data) {
		return nil, ErrQueueEmpty
	}

	id := uintptr(data)
	pq.mu.Lock()
	var goData interface{}
	if id < uintptr(len(pq.dataSlice)) {
		goData = pq.dataSlice[id]
	}
	pq.mu.Unlock()

	return &Element{
		Priority: float64(priority),
		Data:     goData,
	}, nil
}

// Size returns the number of elements currently in the queue.
func (pq *PriorityQueue) Size() int {
	if pq.ptr == nil {
		return 0
	}
	return int(C.fc_priority_queue_size(pq.ptr))
}

// Capacity returns the maximum capacity of the queue.
func (pq *PriorityQueue) Capacity() int {
	if pq.ptr == nil {
		return 0
	}
	return int(C.fc_priority_queue_capacity(pq.ptr))
}

// IsEmpty returns true if the queue has no elements.
func (pq *PriorityQueue) IsEmpty() bool {
	if pq.ptr == nil {
		return true
	}
	return bool(C.fc_priority_queue_is_empty(pq.ptr))
}

// IsFull returns true if the queue is at maximum capacity.
func (pq *PriorityQueue) IsFull() bool {
	if pq.ptr == nil {
		return false
	}
	return bool(C.fc_priority_queue_is_full(pq.ptr))
}

// Clear removes all elements from the queue.
func (pq *PriorityQueue) Clear() {
	if pq.ptr != nil {
		C.fc_priority_queue_clear(pq.ptr)
		pq.mu.Lock()
		// Reset data tracking
		for i := range pq.dataSlice {
			pq.dataSlice[i] = nil
		}
		pq.dataSlice = pq.dataSlice[:0]
		pq.freeIDs = pq.freeIDs[:0]
		pq.nextID = 0
		pq.mu.Unlock()
	}
}
