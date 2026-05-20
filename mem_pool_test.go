package ds

import (
	"runtime"
	"sync"
	"testing"
	"unsafe"
)

func TestMemPoolCreateDestroy(t *testing.T) {
	pool, err := NewMemPool(1024, 100)
	if err != nil {
		t.Fatalf("Failed to create pool: %v", err)
	}
	defer pool.Destroy()

	if pool.Capacity() != 100 {
		t.Errorf("Expected capacity 100, got %d", pool.Capacity())
	}
	if pool.Available() != 100 {
		t.Errorf("Expected available 100, got %d", pool.Available())
	}
	if pool.Used() != 0 {
		t.Errorf("Expected used 0, got %d", pool.Used())
	}
}

func TestMemPoolCreateInvalidArgs(t *testing.T) {
	_, err := NewMemPool(0, 100)
	if err == nil {
		t.Error("Expected error for blockSize=0")
	}

	_, err = NewMemPool(1024, 0)
	if err == nil {
		t.Error("Expected error for numBlocks=0")
	}
}

func TestMemPoolAllocFree(t *testing.T) {
	pool, err := NewMemPool(512, 10)
	if err != nil {
		t.Fatalf("Failed to create pool: %v", err)
	}
	defer pool.Destroy()

	ptr, err := pool.Alloc()
	if err != nil {
		t.Fatalf("Failed to allocate: %v", err)
	}
	if ptr == nil {
		t.Fatal("Allocated pointer is nil")
	}

	if pool.Used() != 1 {
		t.Errorf("Expected used 1, got %d", pool.Used())
	}
	if pool.Available() != 9 {
		t.Errorf("Expected available 9, got %d", pool.Available())
	}

	pool.Free(ptr)

	if pool.Used() != 0 {
		t.Errorf("Expected used 0 after free, got %d", pool.Used())
	}
	if pool.Available() != 10 {
		t.Errorf("Expected available 10 after free, got %d", pool.Available())
	}
}

func TestMemPoolAllocAllBlocks(t *testing.T) {
	const numBlocks = 50
	pool, err := NewMemPool(256, numBlocks)
	if err != nil {
		t.Fatalf("Failed to create pool: %v", err)
	}
	defer pool.Destroy()

	ptrs := make([]unsafe.Pointer, numBlocks)
	for i := 0; i < numBlocks; i++ {
		ptr, err := pool.Alloc()
		if err != nil {
			t.Fatalf("Failed to allocate block %d: %v", i, err)
		}
		ptrs[i] = ptr
	}

	if pool.Used() != numBlocks {
		t.Errorf("Expected used %d, got %d", numBlocks, pool.Used())
	}
	if pool.Available() != 0 {
		t.Errorf("Expected available 0, got %d", pool.Available())
	}

	_, err = pool.Alloc()
	if err == nil {
		t.Error("Expected error when pool is exhausted")
	}

	for _, ptr := range ptrs {
		pool.Free(ptr)
	}

	if pool.Used() != 0 {
		t.Errorf("Expected used 0 after freeing all, got %d", pool.Used())
	}
}

func TestMemPoolAllocFreePattern(t *testing.T) {
	pool, err := NewMemPool(128, 20)
	if err != nil {
		t.Fatalf("Failed to create pool: %v", err)
	}
	defer pool.Destroy()

	ptr1, _ := pool.Alloc()
	ptr2, _ := pool.Alloc()
	ptr3, _ := pool.Alloc()

	if pool.Used() != 3 {
		t.Errorf("Expected used 3, got %d", pool.Used())
	}

	pool.Free(ptr2)

	if pool.Used() != 2 {
		t.Errorf("Expected used 2 after freeing one, got %d", pool.Used())
	}

	ptr4, err := pool.Alloc()
	if err != nil {
		t.Fatalf("Failed to allocate after free: %v", err)
	}

	if pool.Used() != 3 {
		t.Errorf("Expected used 3 after realloc, got %d", pool.Used())
	}

	pool.Free(ptr1)
	pool.Free(ptr3)
	pool.Free(ptr4)

	if pool.Used() != 0 {
		t.Errorf("Expected used 0 at end, got %d", pool.Used())
	}
}

func TestMemPoolBatchAllocFree(t *testing.T) {
	pool, err := NewMemPool(256, 100)
	if err != nil {
		t.Fatalf("Failed to create pool: %v", err)
	}
	defer pool.Destroy()

	ptrs, err := pool.AllocBatch(50)
	if err != nil {
		t.Fatalf("Failed to batch allocate: %v", err)
	}

	if len(ptrs) != 50 {
		t.Errorf("Expected 50 pointers, got %d", len(ptrs))
	}
	if pool.Used() != 50 {
		t.Errorf("Expected used 50, got %d", pool.Used())
	}

	for _, ptr := range ptrs {
		if ptr == nil {
			t.Error("Batch allocated pointer is nil")
		}
	}

	pool.FreeBatch(ptrs)

	if pool.Used() != 0 {
		t.Errorf("Expected used 0 after batch free, got %d", pool.Used())
	}
}

func TestMemPoolBatchPartialAlloc(t *testing.T) {
	pool, err := NewMemPool(128, 30)
	if err != nil {
		t.Fatalf("Failed to create pool: %v", err)
	}
	defer pool.Destroy()

	ptrs, err := pool.AllocBatch(50)
	if err != nil && len(ptrs) == 0 {
		t.Fatalf("Failed to batch allocate: %v", err)
	}

	if len(ptrs) != 30 {
		t.Errorf("Expected 30 pointers (partial), got %d", len(ptrs))
	}
	if pool.Used() != 30 {
		t.Errorf("Expected used 30, got %d", pool.Used())
	}
	if pool.Available() != 0 {
		t.Errorf("Expected available 0, got %d", pool.Available())
	}

	pool.FreeBatch(ptrs)

	if pool.Used() != 0 {
		t.Errorf("Expected used 0 after batch free, got %d", pool.Used())
	}
}

func TestMemPoolStats(t *testing.T) {
	pool, err := NewMemPool(512, 100)
	if err != nil {
		t.Fatalf("Failed to create pool: %v", err)
	}
	defer pool.Destroy()

	stats, err := pool.GetStats()
	if err != nil {
		t.Fatalf("Failed to get stats: %v", err)
	}

	if stats.TotalBlocks != 100 {
		t.Errorf("Expected total blocks 100, got %d", stats.TotalBlocks)
	}
	if stats.UsedBlocks != 0 {
		t.Errorf("Expected used blocks 0, got %d", stats.UsedBlocks)
	}
	if stats.AvailableBlocks != 100 {
		t.Errorf("Expected available blocks 100, got %d", stats.AvailableBlocks)
	}
	if stats.PeakUsage != 0 {
		t.Errorf("Expected peak usage 0, got %d", stats.PeakUsage)
	}

	ptrs := make([]unsafe.Pointer, 10)
	for i := 0; i < 10; i++ {
		ptrs[i], _ = pool.Alloc()
	}

	stats, _ = pool.GetStats()
	if stats.UsedBlocks != 10 {
		t.Errorf("Expected used blocks 10, got %d", stats.UsedBlocks)
	}
	if stats.PeakUsage != 10 {
		t.Errorf("Expected peak usage 10, got %d", stats.PeakUsage)
	}
	if stats.AllocCount != 10 {
		t.Errorf("Expected alloc count 10, got %d", stats.AllocCount)
	}

	for i := 0; i < 5; i++ {
		pool.Free(ptrs[i])
	}

	stats, _ = pool.GetStats()
	if stats.UsedBlocks != 5 {
		t.Errorf("Expected used blocks 5, got %d", stats.UsedBlocks)
	}
	if stats.PeakUsage != 10 {
		t.Errorf("Expected peak usage still 10, got %d", stats.PeakUsage)
	}
	if stats.FreeCount != 5 {
		t.Errorf("Expected free count 5, got %d", stats.FreeCount)
	}

	for i := 5; i < 10; i++ {
		pool.Free(ptrs[i])
	}
}

func TestMemPoolResetStats(t *testing.T) {
	pool, err := NewMemPool(256, 50)
	if err != nil {
		t.Fatalf("Failed to create pool: %v", err)
	}
	defer pool.Destroy()

	ptrs := make([]unsafe.Pointer, 20)
	for i := 0; i < 20; i++ {
		ptrs[i], _ = pool.Alloc()
	}

	stats, _ := pool.GetStats()
	if stats.PeakUsage != 20 {
		t.Errorf("Expected peak usage 20, got %d", stats.PeakUsage)
	}

	pool.ResetStats()

	stats, _ = pool.GetStats()
	if stats.PeakUsage != 0 {
		t.Errorf("Expected peak usage 0 after reset, got %d", stats.PeakUsage)
	}
	if stats.AllocCount != 0 {
		t.Errorf("Expected alloc count 0 after reset, got %d", stats.AllocCount)
	}
	if stats.UsedBlocks != 20 {
		t.Errorf("Expected used blocks still 20, got %d", stats.UsedBlocks)
	}

	for _, ptr := range ptrs {
		pool.Free(ptr)
	}
}

func TestMemPoolConcurrent(t *testing.T) {
	pool, err := NewMemPool(512, 1000)
	if err != nil {
		t.Fatalf("Failed to create pool: %v", err)
	}
	defer pool.Destroy()

	const numGoroutines = 10
	const iterations = 100

	var wg sync.WaitGroup
	wg.Add(numGoroutines)

	for i := 0; i < numGoroutines; i++ {
		go func() {
			defer wg.Done()
			for j := 0; j < iterations; j++ {
				ptr, err := pool.Alloc()
				if err != nil {
					continue
				}
				runtime.Gosched()
				pool.Free(ptr)
			}
		}()
	}

	wg.Wait()

	if pool.Used() != 0 {
		t.Errorf("Expected used 0 after concurrent test, got %d", pool.Used())
	}

	stats, _ := pool.GetStats()
	if stats.AllocCount != stats.FreeCount {
		t.Errorf("Alloc/free count mismatch: %d allocs, %d frees",
			stats.AllocCount, stats.FreeCount)
	}
}

func TestMemPoolConcurrentBatch(t *testing.T) {
	pool, err := NewMemPool(256, 2000)
	if err != nil {
		t.Fatalf("Failed to create pool: %v", err)
	}
	defer pool.Destroy()

	const numGoroutines = 8
	const iterations = 50

	var wg sync.WaitGroup
	wg.Add(numGoroutines)

	for i := 0; i < numGoroutines; i++ {
		go func() {
			defer wg.Done()
			for j := 0; j < iterations; j++ {
				ptrs, err := pool.AllocBatch(10)
				if err != nil || len(ptrs) == 0 {
					continue
				}
				runtime.Gosched()
				pool.FreeBatch(ptrs)
			}
		}()
	}

	wg.Wait()

	if pool.Used() != 0 {
		t.Errorf("Expected used 0 after concurrent batch test, got %d", pool.Used())
	}
}

func TestMemPoolBlockSize(t *testing.T) {
	pool, err := NewMemPool(100, 10)
	if err != nil {
		t.Fatalf("Failed to create pool: %v", err)
	}
	defer pool.Destroy()

	blockSize := pool.BlockSize()
	if blockSize < 100 {
		t.Errorf("Block size %d is less than requested 100", blockSize)
	}
	if blockSize%64 != 0 {
		t.Errorf("Block size %d is not cache-line aligned", blockSize)
	}
}

func TestMemPoolDestroyedAccess(t *testing.T) {
	pool, err := NewMemPool(256, 10)
	if err != nil {
		t.Fatalf("Failed to create pool: %v", err)
	}

	pool.Destroy()

	_, err = pool.Alloc()
	if err == nil {
		t.Error("Expected error when allocating from destroyed pool")
	}

	if pool.Used() != 0 {
		t.Errorf("Expected used 0 for destroyed pool, got %d", pool.Used())
	}

	_, err = pool.GetStats()
	if err == nil {
		t.Error("Expected error when getting stats from destroyed pool")
	}
}

func BenchmarkMemPoolAllocFree(b *testing.B) {
	pool, err := NewMemPool(1024, 10000)
	if err != nil {
		b.Fatalf("Failed to create pool: %v", err)
	}
	defer pool.Destroy()

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		ptr, err := pool.Alloc()
		if err != nil {
			b.Fatal(err)
		}
		pool.Free(ptr)
	}
}

func BenchmarkMemPoolAllocFreeBatch(b *testing.B) {
	pool, err := NewMemPool(512, 100000)
	if err != nil {
		b.Fatalf("Failed to create pool: %v", err)
	}
	defer pool.Destroy()

	const batchSize = 100

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		ptrs, err := pool.AllocBatch(batchSize)
		if err != nil {
			b.Fatal(err)
		}
		pool.FreeBatch(ptrs)
	}
}

func BenchmarkMemPoolVsMalloc(b *testing.B) {
	b.Run("MemPool", func(b *testing.B) {
		pool, err := NewMemPool(1024, 10000)
		if err != nil {
			b.Fatalf("Failed to create pool: %v", err)
		}
		defer pool.Destroy()

		b.ResetTimer()
		for i := 0; i < b.N; i++ {
			ptr, _ := pool.Alloc()
			pool.Free(ptr)
		}
	})

	b.Run("Make", func(b *testing.B) {
		for i := 0; i < b.N; i++ {
			buf := make([]byte, 1024)
			_ = buf
		}
	})
}

func BenchmarkMemPoolConcurrent(b *testing.B) {
	pool, err := NewMemPool(512, 100000)
	if err != nil {
		b.Fatalf("Failed to create pool: %v", err)
	}
	defer pool.Destroy()

	b.RunParallel(func(pb *testing.PB) {
		for pb.Next() {
			ptr, err := pool.Alloc()
			if err != nil {
				continue
			}
			pool.Free(ptr)
		}
	})
}
