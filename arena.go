package ds

/*
#include "arena.h"
#include <stdlib.h>
*/
import "C"
import (
    "runtime"
    "unsafe"
)

// Arena is a linear memory allocator (bump allocator) that provides
// extremely fast allocation by simply moving a pointer forward.
// Individual allocations cannot be freed; instead, the entire arena
// is reset or destroyed at once.
//
// Arena is ideal for:
//   - Temporary calculations
//   - Batch processing
//   - Per-request memory pools
//   - Parser/compiler temporary data
//
// Thread Safety: Each Arena instance is NOT thread-safe.
// Use separate arenas per goroutine or external synchronization.
type Arena struct {
    ptr *C.fc_arena_t
}

// NewArena creates a new arena allocator with the specified capacity.
// Returns nil if capacity is 0 or allocation fails.
//
// Time Complexity: O(1)
func NewArena(capacity int) *Arena {
    if capacity <= 0 {
        return nil
    }

    ptr := C.fc_arena_create(C.size_t(capacity))
    if ptr == nil {
        return nil
    }

    arena := &Arena{ptr: ptr}
    runtime.SetFinalizer(arena, (*Arena).Destroy)
    return arena
}

// Alloc allocates memory from the arena with default alignment.
// Returns nil if size is 0 or insufficient space.
//
// Time Complexity: O(1)
//
// Note: The returned memory is valid until the arena is reset or destroyed.
// Do not use after Reset() or Destroy().
func (a *Arena) Alloc(size int) unsafe.Pointer {
    if a == nil || a.ptr == nil || size <= 0 {
        return nil
    }

    return unsafe.Pointer(C.fc_arena_alloc(a.ptr, C.size_t(size)))
}

// AllocAligned allocates memory from the arena with specified alignment.
// Alignment must be a power of 2 (1, 2, 4, 8, 16, 32, 64, ...).
// Returns nil if size is 0, alignment is invalid, or insufficient space.
//
// Time Complexity: O(1)
//
// Note: The returned memory is valid until the arena is reset or destroyed.
func (a *Arena) AllocAligned(size int, alignment int) unsafe.Pointer {
    if a == nil || a.ptr == nil || size <= 0 || alignment <= 0 {
        return nil
    }

    return unsafe.Pointer(C.fc_arena_alloc_aligned(a.ptr, C.size_t(size), C.size_t(alignment)))
}

// AllocBytes allocates a byte slice from the arena.
// Returns nil if size is 0 or insufficient space.
//
// Time Complexity: O(1)
//
// Note: The returned slice is valid until the arena is reset or destroyed.
// Do not use after Reset() or Destroy().
func (a *Arena) AllocBytes(size int) []byte {
    if size <= 0 {
        return nil
    }

    ptr := a.Alloc(size)
    if ptr == nil {
        return nil
    }

    return unsafe.Slice((*byte)(ptr), size)
}

// Reset resets the arena to its initial state, invalidating all allocations.
// All pointers returned by previous allocations become invalid.
//
// Time Complexity: O(1)
func (a *Arena) Reset() {
    if a != nil && a.ptr != nil {
        C.fc_arena_reset(a.ptr)
    }
}

// Used returns the number of bytes currently allocated.
//
// Time Complexity: O(1)
func (a *Arena) Used() int {
    if a == nil || a.ptr == nil {
        return 0
    }
    return int(C.fc_arena_used(a.ptr))
}

// Capacity returns the total capacity in bytes.
//
// Time Complexity: O(1)
func (a *Arena) Capacity() int {
    if a == nil || a.ptr == nil {
        return 0
    }
    return int(C.fc_arena_capacity(a.ptr))
}

// Available returns the number of bytes available for allocation.
//
// Time Complexity: O(1)
func (a *Arena) Available() int {
    if a == nil || a.ptr == nil {
        return 0
    }
    return int(C.fc_arena_available(a.ptr))
}

// Destroy destroys the arena and frees all memory.
// After destruction, all pointers returned by allocations are invalid.
//
// Time Complexity: O(1)
func (a *Arena) Destroy() {
    if a != nil && a.ptr != nil {
        C.fc_arena_destroy(a.ptr)
        a.ptr = nil
        runtime.SetFinalizer(a, nil)
    }
}
