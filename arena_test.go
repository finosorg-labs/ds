package ds

import (
    "testing"
    "unsafe"
)

func TestArenaCreateDestroy(t *testing.T) {
    arena := NewArena(1024)
    if arena == nil {
        t.Fatal("Arena creation should succeed")
    }
    if arena.Capacity() != 1024 {
        t.Errorf("Expected capacity 1024, got %d", arena.Capacity())
    }
    if arena.Used() != 0 {
        t.Errorf("Expected initial usage 0, got %d", arena.Used())
    }
    if arena.Available() != 1024 {
        t.Errorf("Expected initial available 1024, got %d", arena.Available())
    }
    arena.Destroy()
}

func TestArenaCreateInvalid(t *testing.T) {
    arena := NewArena(0)
    if arena != nil {
        t.Error("Creating arena with 0 capacity should fail")
        arena.Destroy()
    }

    arena = NewArena(-1)
    if arena != nil {
        t.Error("Creating arena with negative capacity should fail")
        arena.Destroy()
    }
}

func TestArenaBasicAlloc(t *testing.T) {
    arena := NewArena(1024)
    defer arena.Destroy()

    ptr1 := arena.Alloc(64)
    if ptr1 == nil {
        t.Fatal("First allocation should succeed")
    }
    if arena.Used() == 0 {
        t.Error("Used space should increase after allocation")
    }

    ptr2 := arena.Alloc(128)
    if ptr2 == nil {
        t.Fatal("Second allocation should succeed")
    }
    if uintptr(ptr2) <= uintptr(ptr1) {
        t.Error("Second pointer should be after first")
    }

    used := arena.Used()
    if used > 1024 {
        t.Errorf("Used space %d should not exceed capacity 1024", used)
    }
}

func TestArenaAllocAligned(t *testing.T) {
    arena := NewArena(1024)
    defer arena.Destroy()

    ptr16 := arena.AllocAligned(10, 16)
    if ptr16 == nil {
        t.Fatal("16-byte aligned allocation should succeed")
    }
    if uintptr(ptr16)&15 != 0 {
        t.Error("Pointer should be 16-byte aligned")
    }

    ptr32 := arena.AllocAligned(10, 32)
    if ptr32 == nil {
        t.Fatal("32-byte aligned allocation should succeed")
    }
    if uintptr(ptr32)&31 != 0 {
        t.Error("Pointer should be 32-byte aligned")
    }

    ptr64 := arena.AllocAligned(10, 64)
    if ptr64 == nil {
        t.Fatal("64-byte aligned allocation should succeed")
    }
    if uintptr(ptr64)&63 != 0 {
        t.Error("Pointer should be 64-byte aligned")
    }
}

func TestArenaAllocInvalidAlignment(t *testing.T) {
    arena := NewArena(1024)
    defer arena.Destroy()

    ptr := arena.AllocAligned(10, 3)
    if ptr != nil {
        t.Error("Non-power-of-2 alignment should fail")
    }

    ptr = arena.AllocAligned(10, 7)
    if ptr != nil {
        t.Error("Non-power-of-2 alignment should fail")
    }
}

func TestArenaExhaustion(t *testing.T) {
    arena := NewArena(128)
    defer arena.Destroy()

    ptr1 := arena.Alloc(64)
    if ptr1 == nil {
        t.Fatal("First allocation should succeed")
    }

    ptr2 := arena.Alloc(64)
    if ptr2 == nil {
        t.Fatal("Second allocation should succeed")
    }

    ptr3 := arena.Alloc(64)
    if ptr3 != nil {
        t.Error("Third allocation should fail (exhausted)")
    }

    if arena.Available() >= 64 {
        t.Errorf("Available space %d should be less than 64", arena.Available())
    }
}

func TestArenaReset(t *testing.T) {
    arena := NewArena(1024)
    defer arena.Destroy()

    ptr1 := arena.Alloc(256)
    if ptr1 == nil {
        t.Fatal("Allocation should succeed")
    }
    usedBefore := arena.Used()
    if usedBefore == 0 {
        t.Error("Used space should be > 0")
    }

    arena.Reset()
    if arena.Used() != 0 {
        t.Errorf("Used space should be 0 after reset, got %d", arena.Used())
    }
    if arena.Available() != 1024 {
        t.Errorf("Available space should be 1024 after reset, got %d", arena.Available())
    }

    ptr2 := arena.Alloc(256)
    if ptr2 == nil {
        t.Error("Allocation after reset should succeed")
    }
}

func TestArenaAllocBytes(t *testing.T) {
    arena := NewArena(1024)
    defer arena.Destroy()

    slice := arena.AllocBytes(100)
    if slice == nil {
        t.Fatal("AllocBytes should succeed")
    }
    if len(slice) != 100 {
        t.Errorf("Expected slice length 100, got %d", len(slice))
    }

    for i := range slice {
        slice[i] = byte(i)
    }

    for i := range slice {
        if slice[i] != byte(i) {
            t.Errorf("Expected slice[%d] = %d, got %d", i, byte(i), slice[i])
        }
    }
}

func TestArenaWriteRead(t *testing.T) {
    arena := NewArena(1024)
    defer arena.Destroy()

    ptr := arena.Alloc(40)
    if ptr == nil {
        t.Fatal("Allocation should succeed")
    }

    numbers := unsafe.Slice((*int32)(ptr), 10)
    for i := range numbers {
        numbers[i] = int32(i * 10)
    }

    for i := range numbers {
        if numbers[i] != int32(i*10) {
            t.Errorf("Expected numbers[%d] = %d, got %d", i, i*10, numbers[i])
        }
    }
}

func TestArenaNullChecks(t *testing.T) {
    var arena *Arena

    if arena.Alloc(10) != nil {
        t.Error("Alloc on nil arena should return nil")
    }
    if arena.AllocAligned(10, 8) != nil {
        t.Error("AllocAligned on nil arena should return nil")
    }
    if arena.AllocBytes(10) != nil {
        t.Error("AllocBytes on nil arena should return nil")
    }

    arena.Reset()
    arena.Destroy()

    if arena.Used() != 0 {
        t.Error("Used on nil arena should return 0")
    }
    if arena.Capacity() != 0 {
        t.Error("Capacity on nil arena should return 0")
    }
    if arena.Available() != 0 {
        t.Error("Available on nil arena should return 0")
    }
}

func TestArenaLargeAllocation(t *testing.T) {
    arena := NewArena(1024 * 1024)
    defer arena.Destroy()

    large := arena.Alloc(512 * 1024)
    if large == nil {
        t.Fatal("Large allocation should succeed")
    }

    used := arena.Used()
    if used < 512*1024 {
        t.Errorf("Used space %d should be at least 512KB", used)
    }
}

func BenchmarkArenaAllocSmall(b *testing.B) {
    arena := NewArena(b.N * 32 * 2)
    defer arena.Destroy()

    b.ResetTimer()
    for i := 0; i < b.N; i++ {
        arena.Alloc(32)
    }
}

func BenchmarkArenaAllocMedium(b *testing.B) {
    arena := NewArena(b.N * 256 * 2)
    defer arena.Destroy()

    b.ResetTimer()
    for i := 0; i < b.N; i++ {
        arena.Alloc(256)
    }
}

func BenchmarkArenaAllocLarge(b *testing.B) {
    arena := NewArena(b.N * 4096 * 2)
    defer arena.Destroy()

    b.ResetTimer()
    for i := 0; i < b.N; i++ {
        arena.Alloc(4096)
    }
}

func BenchmarkArenaAllocAligned(b *testing.B) {
    arena := NewArena(b.N * 32 * 2)
    defer arena.Destroy()

    b.ResetTimer()
    for i := 0; i < b.N; i++ {
        arena.AllocAligned(32, 64)
    }
}

func BenchmarkArenaAllocBytes(b *testing.B) {
    arena := NewArena(b.N * 256 * 2)
    defer arena.Destroy()

    b.ResetTimer()
    for i := 0; i < b.N; i++ {
        arena.AllocBytes(256)
    }
}

func BenchmarkArenaReset(b *testing.B) {
    arena := NewArena(1024 * 1024)
    defer arena.Destroy()

    b.ResetTimer()
    for i := 0; i < b.N; i++ {
        arena.Alloc(256)
        arena.Alloc(512)
        arena.Alloc(128)
        arena.Reset()
    }
}

func BenchmarkArenaBatchAllocReset(b *testing.B) {
    arena := NewArena(1024 * 1024)
    defer arena.Destroy()

    b.ResetTimer()
    for i := 0; i < b.N; i++ {
        for j := 0; j < 100; j++ {
            arena.Alloc(256)
        }
        arena.Reset()
    }
}
