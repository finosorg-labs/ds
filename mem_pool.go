package ds

/*
#include "mem_pool.h"
#include <stdlib.h>
*/
import "C"
import (
	"errors"
	"runtime"
	"unsafe"
)

// MemPool represents a fixed-size memory pool for high-frequency allocation.
// It provides O(1) allocation and deallocation with lock-free thread-safe operations.
type MemPool struct {
	ptr *C.fc_mem_pool_t
}

// MemPoolStats contains statistics about memory pool usage.
type MemPoolStats struct {
	BlockSize       uint64 // Size of each block in bytes
	TotalBlocks     uint64 // Total number of blocks in pool
	UsedBlocks      uint64 // Number of currently allocated blocks
	AvailableBlocks uint64 // Number of available blocks
	PeakUsage       uint64 // Peak number of blocks used
	AllocCount      uint64 // Total allocation count
	FreeCount       uint64 // Total free count
}

// NewMemPool creates a new fixed-size memory pool.
//
// Parameters:
//   - blockSize: Size of each memory block in bytes (must be > 0)
//   - numBlocks: Number of blocks to pre-allocate (must be > 0)
//
// Returns:
//   - *MemPool: Pointer to created pool
//   - error: Error if creation fails
//
// The pool pre-allocates all memory at creation time. All blocks are
// cache-line aligned (64 bytes) to avoid false sharing in multi-threaded use.
//
// Example:
//
//	pool, err := ds.NewMemPool(1024, 1000)
//	if err != nil {
//	    log.Fatal(err)
//	}
//	defer pool.Destroy()
func NewMemPool(blockSize, numBlocks uint64) (*MemPool, error) {
	if blockSize == 0 || numBlocks == 0 {
		return nil, errors.New("blockSize and numBlocks must be greater than 0")
	}

	ptr := C.fc_ds_mem_pool_create(C.size_t(blockSize), C.size_t(numBlocks))
	if ptr == nil {
		return nil, errors.New("failed to create memory pool")
	}

	pool := &MemPool{ptr: ptr}
	runtime.SetFinalizer(pool, (*MemPool).Destroy)
	return pool, nil
}

// Destroy frees all resources associated with the memory pool.
// Any outstanding allocated blocks become invalid after this call.
//
// Warning: Ensure all allocated blocks are freed before destroying the pool.
func (p *MemPool) Destroy() {
	if p.ptr != nil {
		C.fc_ds_mem_pool_destroy(p.ptr)
		p.ptr = nil
		runtime.SetFinalizer(p, nil)
	}
}

// Alloc allocates a single block from the pool.
// The memory is uninitialized and must be initialized before use.
//
// Returns:
//   - unsafe.Pointer: Pointer to allocated block
//   - error: Error if pool is exhausted
//
// Time complexity: O(1)
// Thread-safe: Yes (lock-free CAS operations)
//
// Example:
//
//	ptr, err := pool.Alloc()
//	if err != nil {
//	    log.Printf("Pool exhausted: %v", err)
//	    return
//	}
//	defer pool.Free(ptr)
//	// Use ptr...
func (p *MemPool) Alloc() (unsafe.Pointer, error) {
	if p.ptr == nil {
		return nil, errors.New("pool is destroyed")
	}

	ptr := C.fc_ds_mem_pool_alloc(p.ptr)
	if ptr == nil {
		return nil, errors.New("pool exhausted")
	}

	return ptr, nil
}

// Free returns a previously allocated block to the pool for reuse.
//
// Parameters:
//   - ptr: Pointer to block allocated from this pool
//
// Warning: ptr must have been allocated from this pool.
// Warning: Do not free the same block twice.
//
// Time complexity: O(1)
// Thread-safe: Yes (lock-free CAS operations)
func (p *MemPool) Free(ptr unsafe.Pointer) {
	if p.ptr == nil || ptr == nil {
		return
	}

	C.fc_ds_mem_pool_free(p.ptr, ptr)
}

// AllocBatch allocates multiple blocks in batch.
// If fewer than count blocks are available, allocates as many as possible.
//
// Parameters:
//   - count: Number of blocks to allocate
//
// Returns:
//   - []unsafe.Pointer: Slice of allocated block pointers
//   - error: Error if pool is destroyed or count is 0
//
// Time complexity: O(count)
// Thread-safe: Yes (lock-free CAS operations)
//
// Example:
//
//	ptrs, err := pool.AllocBatch(100)
//	if err != nil {
//	    log.Fatal(err)
//	}
//	defer pool.FreeBatch(ptrs)
//	// Use ptrs...
func (p *MemPool) AllocBatch(count uint64) ([]unsafe.Pointer, error) {
	if p.ptr == nil {
		return nil, errors.New("pool is destroyed")
	}
	if count == 0 {
		return nil, errors.New("count must be greater than 0")
	}

	cPtrs := make([]unsafe.Pointer, count)

	allocated := C.fc_ds_mem_pool_alloc_batch(
		p.ptr,
		C.size_t(count),
		(*unsafe.Pointer)(unsafe.Pointer(&cPtrs[0])),
	)

	if allocated == 0 {
		return nil, errors.New("pool exhausted")
	}

	result := make([]unsafe.Pointer, allocated)
	copy(result, cPtrs[:allocated])

	return result, nil
}

// FreeBatch returns multiple blocks to the pool in batch.
//
// Parameters:
//   - ptrs: Slice of block pointers to free
//
// Warning: All pointers must have been allocated from this pool.
// Warning: Do not free the same block twice.
//
// Time complexity: O(len(ptrs))
// Thread-safe: Yes (lock-free CAS operations)
func (p *MemPool) FreeBatch(ptrs []unsafe.Pointer) {
	if p.ptr == nil || len(ptrs) == 0 {
		return
	}

	C.fc_ds_mem_pool_free_batch(
		p.ptr,
		(*unsafe.Pointer)(unsafe.Pointer(&ptrs[0])),
		C.size_t(len(ptrs)),
	)
}

// Available returns the current number of free blocks in the pool.
//
// Note: This is a snapshot value and may change immediately in multi-threaded use.
//
// Time complexity: O(1)
// Thread-safe: Yes (atomic load)
func (p *MemPool) Available() uint64 {
	if p.ptr == nil {
		return 0
	}

	return uint64(C.fc_ds_mem_pool_available(p.ptr))
}

// Used returns the current number of allocated blocks.
//
// Note: This is a snapshot value and may change immediately in multi-threaded use.
//
// Time complexity: O(1)
// Thread-safe: Yes (atomic load)
func (p *MemPool) Used() uint64 {
	if p.ptr == nil {
		return 0
	}

	return uint64(C.fc_ds_mem_pool_used(p.ptr))
}

// Capacity returns the total capacity of the pool (used + available).
//
// Note: This value is constant after pool creation.
//
// Time complexity: O(1)
// Thread-safe: Yes (read-only access)
func (p *MemPool) Capacity() uint64 {
	if p.ptr == nil {
		return 0
	}

	return uint64(C.fc_ds_mem_pool_capacity(p.ptr))
}

// BlockSize returns the size of each block in bytes.
//
// Note: This value is constant after pool creation.
//
// Time complexity: O(1)
// Thread-safe: Yes (read-only access)
func (p *MemPool) BlockSize() uint64 {
	if p.ptr == nil {
		return 0
	}

	return uint64(C.fc_ds_mem_pool_block_size(p.ptr))
}

// GetStats returns comprehensive statistics about pool usage.
//
// Note: Statistics are snapshots and may be inconsistent in multi-threaded use.
//
// Time complexity: O(1)
// Thread-safe: Yes (atomic loads)
//
// Example:
//
//	stats, err := pool.GetStats()
//	if err != nil {
//	    log.Fatal(err)
//	}
//	fmt.Printf("Peak usage: %d/%d blocks (%.1f%%)\n",
//	    stats.PeakUsage, stats.TotalBlocks,
//	    float64(stats.PeakUsage)/float64(stats.TotalBlocks)*100)
func (p *MemPool) GetStats() (*MemPoolStats, error) {
	if p.ptr == nil {
		return nil, errors.New("pool is destroyed")
	}

	var cStats C.fc_mem_pool_stats_t
	status := C.fc_ds_mem_pool_get_stats(p.ptr, &cStats)
	if status != C.FC_OK {
		return nil, errors.New("failed to get pool statistics")
	}

	stats := &MemPoolStats{
		BlockSize:       uint64(cStats.block_size),
		TotalBlocks:     uint64(cStats.total_blocks),
		UsedBlocks:      uint64(cStats.used_blocks),
		AvailableBlocks: uint64(cStats.available_blocks),
		PeakUsage:       uint64(cStats.peak_usage),
		AllocCount:      uint64(cStats.alloc_count),
		FreeCount:       uint64(cStats.free_count),
	}

	return stats, nil
}

// ResetStats resets peak usage and allocation/free counters to zero.
// Does not affect current allocation state.
//
// Time complexity: O(1)
// Thread-safe: Yes (atomic stores)
func (p *MemPool) ResetStats() {
	if p.ptr == nil {
		return
	}

	C.fc_ds_mem_pool_reset_stats(p.ptr)
}
