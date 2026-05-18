package ds

import (
	"testing"
)

func TestRoaringBitmapCreateDestroy(t *testing.T) {
	bitmap, err := NewRoaringBitmap()
	if err != nil {
		t.Fatalf("Failed to create Roaring Bitmap: %v", err)
	}
	defer bitmap.Close()

	if !bitmap.IsEmpty() {
		t.Error("New bitmap should be empty")
	}

	if bitmap.Cardinality() != 0 {
		t.Errorf("Expected cardinality 0, got %d", bitmap.Cardinality())
	}
}

func TestRoaringBitmapCreateFromArray(t *testing.T) {
	values := []uint32{1, 5, 10, 100, 1000, 10000}

	bitmap, err := NewRoaringBitmapFromArray(values)
	if err != nil {
		t.Fatalf("Failed to create bitmap from array: %v", err)
	}
	defer bitmap.Close()

	if bitmap.Cardinality() != uint64(len(values)) {
		t.Errorf("Expected cardinality %d, got %d", len(values), bitmap.Cardinality())
	}

	for _, v := range values {
		contains, err := bitmap.Contains(v)
		if err != nil {
			t.Errorf("Contains failed: %v", err)
		}
		if !contains {
			t.Errorf("Expected bitmap to contain %d", v)
		}
	}
}

func TestRoaringBitmapCreateFromRange(t *testing.T) {
	bitmap, err := NewRoaringBitmapFromRange(100, 200)
	if err != nil {
		t.Fatalf("Failed to create bitmap from range: %v", err)
	}
	defer bitmap.Close()

	if bitmap.Cardinality() != 100 {
		t.Errorf("Expected cardinality 100, got %d", bitmap.Cardinality())
	}

	// Check boundaries
	contains, _ := bitmap.Contains(99)
	if contains {
		t.Error("Bitmap should not contain 99")
	}

	contains, _ = bitmap.Contains(100)
	if !contains {
		t.Error("Bitmap should contain 100")
	}

	contains, _ = bitmap.Contains(199)
	if !contains {
		t.Error("Bitmap should contain 199")
	}

	contains, _ = bitmap.Contains(200)
	if contains {
		t.Error("Bitmap should not contain 200")
	}
}

func TestRoaringBitmapAddSingle(t *testing.T) {
	bitmap, err := NewRoaringBitmap()
	if err != nil {
		t.Fatalf("Failed to create bitmap: %v", err)
	}
	defer bitmap.Close()

	err = bitmap.Add(42)
	if err != nil {
		t.Fatalf("Failed to add value: %v", err)
	}

	if bitmap.Cardinality() != 1 {
		t.Errorf("Expected cardinality 1, got %d", bitmap.Cardinality())
	}

	contains, err := bitmap.Contains(42)
	if err != nil {
		t.Fatalf("Contains failed: %v", err)
	}
	if !contains {
		t.Error("Bitmap should contain 42")
	}

	contains, _ = bitmap.Contains(43)
	if contains {
		t.Error("Bitmap should not contain 43")
	}

	// Add duplicate
	err = bitmap.Add(42)
	if err != nil {
		t.Fatalf("Failed to add duplicate: %v", err)
	}

	if bitmap.Cardinality() != 1 {
		t.Errorf("Expected cardinality 1 after duplicate add, got %d", bitmap.Cardinality())
	}
}

func TestRoaringBitmapAddMultiple(t *testing.T) {
	bitmap, err := NewRoaringBitmap()
	if err != nil {
		t.Fatalf("Failed to create bitmap: %v", err)
	}
	defer bitmap.Close()

	for i := uint32(0); i < 1000; i++ {
		err = bitmap.Add(i)
		if err != nil {
			t.Fatalf("Failed to add value %d: %v", i, err)
		}
	}

	if bitmap.Cardinality() != 1000 {
		t.Errorf("Expected cardinality 1000, got %d", bitmap.Cardinality())
	}

	for i := uint32(0); i < 1000; i++ {
		contains, _ := bitmap.Contains(i)
		if !contains {
			t.Errorf("Bitmap should contain %d", i)
		}
	}

	contains, _ := bitmap.Contains(1000)
	if contains {
		t.Error("Bitmap should not contain 1000")
	}
}

func TestRoaringBitmapAddBatch(t *testing.T) {
	bitmap, err := NewRoaringBitmap()
	if err != nil {
		t.Fatalf("Failed to create bitmap: %v", err)
	}
	defer bitmap.Close()

	values := make([]uint32, 100)
	for i := range values {
		values[i] = uint32(i * 10)
	}

	err = bitmap.AddBatch(values)
	if err != nil {
		t.Fatalf("Failed to add batch: %v", err)
	}

	if bitmap.Cardinality() != uint64(len(values)) {
		t.Errorf("Expected cardinality %d, got %d", len(values), bitmap.Cardinality())
	}

	for _, v := range values {
		contains, _ := bitmap.Contains(v)
		if !contains {
			t.Errorf("Bitmap should contain %d", v)
		}
	}
}

func TestRoaringBitmapAddRange(t *testing.T) {
	bitmap, err := NewRoaringBitmap()
	if err != nil {
		t.Fatalf("Failed to create bitmap: %v", err)
	}
	defer bitmap.Close()

	err = bitmap.AddRange(1000, 2000)
	if err != nil {
		t.Fatalf("Failed to add range: %v", err)
	}

	if bitmap.Cardinality() != 1000 {
		t.Errorf("Expected cardinality 1000, got %d", bitmap.Cardinality())
	}

	contains, _ := bitmap.Contains(999)
	if contains {
		t.Error("Bitmap should not contain 999")
	}

	contains, _ = bitmap.Contains(1000)
	if !contains {
		t.Error("Bitmap should contain 1000")
	}

	contains, _ = bitmap.Contains(1999)
	if !contains {
		t.Error("Bitmap should contain 1999")
	}

	contains, _ = bitmap.Contains(2000)
	if contains {
		t.Error("Bitmap should not contain 2000")
	}
}

func TestRoaringBitmapContainsBatch(t *testing.T) {
	bitmap, err := NewRoaringBitmap()
	if err != nil {
		t.Fatalf("Failed to create bitmap: %v", err)
	}
	defer bitmap.Close()

	// Add even numbers
	for i := uint32(0); i < 100; i += 2 {
		bitmap.Add(i)
	}

	values := make([]uint32, 100)
	for i := range values {
		values[i] = uint32(i)
	}

	results, err := bitmap.ContainsBatch(values)
	if err != nil {
		t.Fatalf("ContainsBatch failed: %v", err)
	}

	if len(results) != len(values) {
		t.Fatalf("Expected %d results, got %d", len(values), len(results))
	}

	for i, result := range results {
		expected := (i % 2) == 0
		if result != expected {
			t.Errorf("Value %d: expected %v, got %v", i, expected, result)
		}
	}
}

func TestRoaringBitmapClear(t *testing.T) {
	bitmap, err := NewRoaringBitmap()
	if err != nil {
		t.Fatalf("Failed to create bitmap: %v", err)
	}
	defer bitmap.Close()

	for i := uint32(0); i < 100; i++ {
		bitmap.Add(i)
	}

	if bitmap.Cardinality() != 100 {
		t.Errorf("Expected cardinality 100, got %d", bitmap.Cardinality())
	}

	err = bitmap.Clear()
	if err != nil {
		t.Fatalf("Failed to clear bitmap: %v", err)
	}

	if bitmap.Cardinality() != 0 {
		t.Errorf("Expected cardinality 0 after clear, got %d", bitmap.Cardinality())
	}

	if !bitmap.IsEmpty() {
		t.Error("Bitmap should be empty after clear")
	}
}

func TestRoaringBitmapMinMax(t *testing.T) {
	bitmap, err := NewRoaringBitmap()
	if err != nil {
		t.Fatalf("Failed to create bitmap: %v", err)
	}
	defer bitmap.Close()

	bitmap.Add(10)
	bitmap.Add(50)
	bitmap.Add(100)

	min, err := bitmap.Min()
	if err != nil {
		t.Fatalf("Min failed: %v", err)
	}
	if min != 10 {
		t.Errorf("Expected min 10, got %d", min)
	}

	max, err := bitmap.Max()
	if err != nil {
		t.Fatalf("Max failed: %v", err)
	}
	if max != 100 {
		t.Errorf("Expected max 100, got %d", max)
	}
}

func TestRoaringBitmapToArray(t *testing.T) {
	bitmap, err := NewRoaringBitmap()
	if err != nil {
		t.Fatalf("Failed to create bitmap: %v", err)
	}
	defer bitmap.Close()

	input := []uint32{5, 10, 15, 20, 25}
	for _, v := range input {
		bitmap.Add(v)
	}

	output, err := bitmap.ToArray()
	if err != nil {
		t.Fatalf("ToArray failed: %v", err)
	}

	if len(output) != len(input) {
		t.Fatalf("Expected %d values, got %d", len(input), len(output))
	}

	for i, v := range output {
		if v != input[i] {
			t.Errorf("Index %d: expected %d, got %d", i, input[i], v)
		}
	}
}

func TestRoaringBitmapStats(t *testing.T) {
	bitmap, err := NewRoaringBitmap()
	if err != nil {
		t.Fatalf("Failed to create bitmap: %v", err)
	}
	defer bitmap.Close()

	for i := uint32(0); i < 1000; i++ {
		bitmap.Add(i)
	}

	stats, err := bitmap.Stats()
	if err != nil {
		t.Fatalf("Stats failed: %v", err)
	}

	if stats.Cardinality != 1000 {
		t.Errorf("Expected cardinality 1000, got %d", stats.Cardinality)
	}

	if stats.NumContainers == 0 {
		t.Error("Expected at least one container")
	}

	if stats.MemoryBytes == 0 {
		t.Error("Expected non-zero memory usage")
	}
}

func TestRoaringBitmapContainerConversion(t *testing.T) {
	bitmap, err := NewRoaringBitmap()
	if err != nil {
		t.Fatalf("Failed to create bitmap: %v", err)
	}
	defer bitmap.Close()

	// Add enough values to trigger array->bitmap conversion
	for i := uint32(0); i < 5000; i++ {
		bitmap.Add(i)
	}

	if bitmap.Cardinality() != 5000 {
		t.Errorf("Expected cardinality 5000, got %d", bitmap.Cardinality())
	}

	stats, _ := bitmap.Stats()
	if stats.NumBitmapContainers == 0 {
		t.Error("Expected at least one bitmap container after conversion")
	}
}

func TestRoaringBitmapMultipleContainers(t *testing.T) {
	bitmap, err := NewRoaringBitmap()
	if err != nil {
		t.Fatalf("Failed to create bitmap: %v", err)
	}
	defer bitmap.Close()

	// Add values in different containers
	bitmap.Add(100)           // Container 0
	bitmap.Add(65536 + 200)   // Container 1
	bitmap.Add(131072 + 300)  // Container 2

	if bitmap.Cardinality() != 3 {
		t.Errorf("Expected cardinality 3, got %d", bitmap.Cardinality())
	}

	stats, _ := bitmap.Stats()
	if stats.NumContainers != 3 {
		t.Errorf("Expected 3 containers, got %d", stats.NumContainers)
	}
}

func TestRoaringBitmapEquals(t *testing.T) {
	a, _ := NewRoaringBitmap()
	defer a.Close()

	b, _ := NewRoaringBitmap()
	defer b.Close()

	// Empty bitmaps are equal
	if !a.Equals(b) {
		t.Error("Empty bitmaps should be equal")
	}

	// Add same values
	for i := uint32(0); i < 100; i++ {
		a.Add(i)
		b.Add(i)
	}

	if !a.Equals(b) {
		t.Error("Bitmaps with same values should be equal")
	}

	// Add different value to one
	a.Add(1000)

	if a.Equals(b) {
		t.Error("Bitmaps with different values should not be equal")
	}
}

func TestRoaringBitmapOptimize(t *testing.T) {
	bitmap, err := NewRoaringBitmap()
	if err != nil {
		t.Fatalf("Failed to create bitmap: %v", err)
	}
	defer bitmap.Close()

	for i := uint32(0); i < 100; i++ {
		bitmap.Add(i)
	}

	err = bitmap.Optimize()
	if err != nil {
		t.Fatalf("Optimize failed: %v", err)
	}

	if bitmap.Cardinality() != 100 {
		t.Errorf("Expected cardinality 100 after optimize, got %d", bitmap.Cardinality())
	}
}

func TestRoaringBitmapIsEmpty(t *testing.T) {
	bitmap, err := NewRoaringBitmap()
	if err != nil {
		t.Fatalf("Failed to create bitmap: %v", err)
	}
	defer bitmap.Close()

	if !bitmap.IsEmpty() {
		t.Error("New bitmap should be empty")
	}

	bitmap.Add(1)
	if bitmap.IsEmpty() {
		t.Error("Bitmap with element should not be empty")
	}

	bitmap.Clear()
	if !bitmap.IsEmpty() {
		t.Error("Cleared bitmap should be empty")
	}
}

func TestRoaringBitmapClone(t *testing.T) {
	original, err := NewRoaringBitmap()
	if err != nil {
		t.Fatalf("Failed to create bitmap: %v", err)
	}
	defer original.Close()

	for i := uint32(0); i < 100; i++ {
		original.Add(i)
	}

	clone, err := original.Clone()
	if err != nil {
		t.Fatalf("Failed to clone bitmap: %v", err)
	}
	defer clone.Close()

	if !original.Equals(clone) {
		t.Error("Clone should be equal to original")
	}

	// Modify clone
	clone.Add(1000)

	if original.Equals(clone) {
		t.Error("Modified clone should not be equal to original")
	}
}

// Benchmarks

func BenchmarkRoaringBitmapAdd(b *testing.B) {
	bitmap, _ := NewRoaringBitmap()
	defer bitmap.Close()

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		bitmap.Add(uint32(i))
	}
}

func BenchmarkRoaringBitmapAddBatch(b *testing.B) {
	values := make([]uint32, 1000)
	for i := range values {
		values[i] = uint32(i)
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		bitmap, _ := NewRoaringBitmap()
		bitmap.AddBatch(values)
		bitmap.Close()
	}
}

func BenchmarkRoaringBitmapContains(b *testing.B) {
	bitmap, _ := NewRoaringBitmap()
	defer bitmap.Close()

	for i := uint32(0); i < 10000; i++ {
		bitmap.Add(i)
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		bitmap.Contains(uint32(i % 10000))
	}
}

func BenchmarkRoaringBitmapContainsBatch(b *testing.B) {
	bitmap, _ := NewRoaringBitmap()
	defer bitmap.Close()

	for i := uint32(0); i < 10000; i++ {
		bitmap.Add(i)
	}

	values := make([]uint32, 1000)
	for i := range values {
		values[i] = uint32(i)
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		bitmap.ContainsBatch(values)
	}
}

func BenchmarkRoaringBitmapCardinality(b *testing.B) {
	bitmap, _ := NewRoaringBitmap()
	defer bitmap.Close()

	for i := uint32(0); i < 10000; i++ {
		bitmap.Add(i)
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		_ = bitmap.Cardinality()
	}
}
