package ds

import (
	"fmt"
	"testing"
)

func TestBloomFilterCreate(t *testing.T) {
	config := BloomConfig{
		ExpectedElements:  1000,
		FalsePositiveRate: 0.01,
	}

	bf, err := NewBloomFilter(config)
	if err != nil {
		t.Fatalf("Failed to create Bloom filter: %v", err)
	}
	defer bf.Close()

	stats, err := bf.Stats()
	if err != nil {
		t.Fatalf("Failed to get stats: %v", err)
	}

	if stats.BitArraySize == 0 {
		t.Error("Bit array size should be non-zero")
	}
	if stats.NumHashFunctions == 0 {
		t.Error("Number of hash functions should be non-zero")
	}
	if stats.ElementsAdded != 0 {
		t.Errorf("Expected 0 elements added, got %d", stats.ElementsAdded)
	}
}

func TestBloomFilterCreateExplicit(t *testing.T) {
	bf, err := NewBloomFilterExplicit(1024, 7)
	if err != nil {
		t.Fatalf("Failed to create Bloom filter: %v", err)
	}
	defer bf.Close()

	stats, err := bf.Stats()
	if err != nil {
		t.Fatalf("Failed to get stats: %v", err)
	}

	if stats.BitArraySize != 1024 {
		t.Errorf("Expected bit array size 1024, got %d", stats.BitArraySize)
	}
	if stats.NumHashFunctions != 7 {
		t.Errorf("Expected 7 hash functions, got %d", stats.NumHashFunctions)
	}
}

func TestBloomFilterCreateInvalid(t *testing.T) {
	tests := []struct {
		name   string
		config BloomConfig
	}{
		{"zero elements", BloomConfig{ExpectedElements: 0, FalsePositiveRate: 0.01}},
		{"negative elements", BloomConfig{ExpectedElements: -1, FalsePositiveRate: 0.01}},
		{"zero fpp", BloomConfig{ExpectedElements: 1000, FalsePositiveRate: 0.0}},
		{"negative fpp", BloomConfig{ExpectedElements: 1000, FalsePositiveRate: -0.01}},
		{"fpp >= 1.0", BloomConfig{ExpectedElements: 1000, FalsePositiveRate: 1.0}},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			bf, err := NewBloomFilter(tt.config)
			if err == nil {
				bf.Close()
				t.Error("Expected error for invalid config")
			}
		})
	}
}

func TestBloomFilterAddContains(t *testing.T) {
	config := BloomConfig{
		ExpectedElements:  100,
		FalsePositiveRate: 0.01,
	}

	bf, err := NewBloomFilter(config)
	if err != nil {
		t.Fatalf("Failed to create Bloom filter: %v", err)
	}
	defer bf.Close()

	// Add some items
	items := []string{"apple", "banana", "cherry", "date"}
	for _, item := range items {
		if err := bf.AddString(item); err != nil {
			t.Fatalf("Failed to add item %s: %v", item, err)
		}
	}

	// Check added items
	for _, item := range items {
		contains, err := bf.ContainsString(item)
		if err != nil {
			t.Fatalf("Failed to check item %s: %v", item, err)
		}
		if !contains {
			t.Errorf("Item %s should be in the filter", item)
		}
	}

	// Check items not added
	notAdded := []string{"elderberry", "fig", "grape"}
	for _, item := range notAdded {
		contains, err := bf.ContainsString(item)
		if err != nil {
			t.Fatalf("Failed to check item %s: %v", item, err)
		}
		if contains {
			t.Logf("False positive for item %s (expected)", item)
		}
	}

	// Verify stats
	stats, err := bf.Stats()
	if err != nil {
		t.Fatalf("Failed to get stats: %v", err)
	}
	if stats.ElementsAdded != len(items) {
		t.Errorf("Expected %d elements added, got %d", len(items), stats.ElementsAdded)
	}
}

func TestBloomFilterAddContainsBytes(t *testing.T) {
	config := BloomConfig{
		ExpectedElements:  100,
		FalsePositiveRate: 0.01,
	}

	bf, err := NewBloomFilter(config)
	if err != nil {
		t.Fatalf("Failed to create Bloom filter: %v", err)
	}
	defer bf.Close()

	// Add byte slices
	items := [][]byte{
		{1, 2, 3, 4},
		{5, 6, 7, 8},
		{9, 10, 11, 12},
	}

	for _, item := range items {
		if err := bf.Add(item); err != nil {
			t.Fatalf("Failed to add item: %v", err)
		}
	}

	// Check added items
	for _, item := range items {
		contains, err := bf.Contains(item)
		if err != nil {
			t.Fatalf("Failed to check item: %v", err)
		}
		if !contains {
			t.Error("Item should be in the filter")
		}
	}

	// Check item not added
	notAdded := []byte{13, 14, 15, 16}
	contains, err := bf.Contains(notAdded)
	if err != nil {
		t.Fatalf("Failed to check item: %v", err)
	}
	if contains {
		t.Log("False positive (expected)")
	}
}

func TestBloomFilterBatchOperations(t *testing.T) {
	config := BloomConfig{
		ExpectedElements:  100,
		FalsePositiveRate: 0.01,
	}

	bf, err := NewBloomFilter(config)
	if err != nil {
		t.Fatalf("Failed to create Bloom filter: %v", err)
	}
	defer bf.Close()

	// Add batch
	items := [][]byte{
		[]byte("apple"),
		[]byte("banana"),
		[]byte("cherry"),
		[]byte("date"),
		[]byte("elderberry"),
	}

	if err := bf.AddBatch(items); err != nil {
		t.Fatalf("Failed to add batch: %v", err)
	}

	// Check batch
	results, err := bf.ContainsBatch(items)
	if err != nil {
		t.Fatalf("Failed to check batch: %v", err)
	}

	if len(results) != len(items) {
		t.Fatalf("Expected %d results, got %d", len(items), len(results))
	}

	for i, result := range results {
		if !result {
			t.Errorf("Item %d should be in the filter", i)
		}
	}

	// Check batch of items not added
	notAdded := [][]byte{
		[]byte("fig"),
		[]byte("grape"),
		[]byte("honeydew"),
	}

	results, err = bf.ContainsBatch(notAdded)
	if err != nil {
		t.Fatalf("Failed to check batch: %v", err)
	}

	falsePositives := 0
	for _, result := range results {
		if result {
			falsePositives++
		}
	}
	t.Logf("False positives: %d/%d", falsePositives, len(notAdded))
}

func TestBloomFilterClear(t *testing.T) {
	config := BloomConfig{
		ExpectedElements:  100,
		FalsePositiveRate: 0.01,
	}

	bf, err := NewBloomFilter(config)
	if err != nil {
		t.Fatalf("Failed to create Bloom filter: %v", err)
	}
	defer bf.Close()

	// Add item
	item := "test"
	if err := bf.AddString(item); err != nil {
		t.Fatalf("Failed to add item: %v", err)
	}

	// Verify it's there
	contains, err := bf.ContainsString(item)
	if err != nil {
		t.Fatalf("Failed to check item: %v", err)
	}
	if !contains {
		t.Error("Item should be in the filter")
	}

	// Clear
	if err := bf.Clear(); err != nil {
		t.Fatalf("Failed to clear filter: %v", err)
	}

	// Verify it's gone
	contains, err = bf.ContainsString(item)
	if err != nil {
		t.Fatalf("Failed to check item: %v", err)
	}
	if contains {
		t.Error("Item should not be in the filter after clear")
	}

	// Verify stats
	stats, err := bf.Stats()
	if err != nil {
		t.Fatalf("Failed to get stats: %v", err)
	}
	if stats.ElementsAdded != 0 {
		t.Errorf("Expected 0 elements after clear, got %d", stats.ElementsAdded)
	}
}

func TestBloomFilterFalsePositiveRate(t *testing.T) {
	config := BloomConfig{
		ExpectedElements:  1000,
		FalsePositiveRate: 0.01,
	}

	bf, err := NewBloomFilter(config)
	if err != nil {
		t.Fatalf("Failed to create Bloom filter: %v", err)
	}
	defer bf.Close()

	// Add 1000 items
	for i := 0; i < 1000; i++ {
		item := []byte(fmt.Sprintf("item-%d", i))
		if err := bf.Add(item); err != nil {
			t.Fatalf("Failed to add item: %v", err)
		}
	}

	// Check all added items
	for i := 0; i < 1000; i++ {
		item := []byte(fmt.Sprintf("item-%d", i))
		contains, err := bf.Contains(item)
		if err != nil {
			t.Fatalf("Failed to check item: %v", err)
		}
		if !contains {
			t.Errorf("Item %d should be in the filter", i)
		}
	}

	// Check false positive rate
	falsePositives := 0
	testCount := 10000
	for i := 1000; i < 1000+testCount; i++ {
		item := []byte(fmt.Sprintf("item-%d", i))
		contains, err := bf.Contains(item)
		if err != nil {
			t.Fatalf("Failed to check item: %v", err)
		}
		if contains {
			falsePositives++
		}
	}

	actualFPP := float64(falsePositives) / float64(testCount)
	t.Logf("Actual false positive rate: %.4f (expected < 0.02)", actualFPP)

	if actualFPP > 0.02 {
		t.Errorf("False positive rate too high: %.4f > 0.02", actualFPP)
	}

	// Check estimated FPP
	stats, err := bf.Stats()
	if err != nil {
		t.Fatalf("Failed to get stats: %v", err)
	}
	t.Logf("Estimated FPP: %.4f", stats.EstimatedFPP)
}

func TestBloomFilterEmptyData(t *testing.T) {
	config := BloomConfig{
		ExpectedElements:  100,
		FalsePositiveRate: 0.01,
	}

	bf, err := NewBloomFilter(config)
	if err != nil {
		t.Fatalf("Failed to create Bloom filter: %v", err)
	}
	defer bf.Close()

	// Add empty data
	empty := []byte{}
	if err := bf.Add(empty); err != nil {
		t.Fatalf("Failed to add empty data: %v", err)
	}

	// Check empty data
	contains, err := bf.Contains(empty)
	if err != nil {
		t.Fatalf("Failed to check empty data: %v", err)
	}
	if !contains {
		t.Error("Empty data should be in the filter")
	}
}

func TestOptimalCalculations(t *testing.T) {
	size := OptimalSize(1000, 0.01)
	if size == 0 {
		t.Error("Optimal size should be non-zero")
	}
	if size%64 != 0 {
		t.Error("Optimal size should be aligned to 64 bits")
	}

	hashCount := OptimalHashCount(size, 1000)
	if hashCount == 0 {
		t.Error("Optimal hash count should be non-zero")
	}
	if hashCount > 32 {
		t.Error("Optimal hash count should not exceed 32")
	}

	t.Logf("Optimal size for 1000 elements at 1%% FPP: %d bits", size)
	t.Logf("Optimal hash count: %d", hashCount)
}

func BenchmarkBloomFilterAdd(b *testing.B) {
	config := BloomConfig{
		ExpectedElements:  100000,
		FalsePositiveRate: 0.01,
	}

	bf, err := NewBloomFilter(config)
	if err != nil {
		b.Fatalf("Failed to create Bloom filter: %v", err)
	}
	defer bf.Close()

	data := make([][]byte, b.N)
	for i := 0; i < b.N; i++ {
		data[i] = []byte(fmt.Sprintf("item-%d", i))
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		bf.Add(data[i])
	}
}

func BenchmarkBloomFilterContains(b *testing.B) {
	config := BloomConfig{
		ExpectedElements:  100000,
		FalsePositiveRate: 0.01,
	}

	bf, err := NewBloomFilter(config)
	if err != nil {
		b.Fatalf("Failed to create Bloom filter: %v", err)
	}
	defer bf.Close()

	// Pre-populate
	for i := 0; i < 10000; i++ {
		item := []byte(fmt.Sprintf("item-%d", i))
		bf.Add(item)
	}

	data := make([][]byte, b.N)
	for i := 0; i < b.N; i++ {
		data[i] = []byte(fmt.Sprintf("item-%d", i%10000))
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		bf.Contains(data[i])
	}
}

func BenchmarkBloomFilterAddBatch(b *testing.B) {
	config := BloomConfig{
		ExpectedElements:  100000,
		FalsePositiveRate: 0.01,
	}

	bf, err := NewBloomFilter(config)
	if err != nil {
		b.Fatalf("Failed to create Bloom filter: %v", err)
	}
	defer bf.Close()

	batchSize := 1000
	batches := b.N / batchSize
	if batches == 0 {
		batches = 1
	}

	data := make([][]byte, batchSize)
	for i := 0; i < batchSize; i++ {
		data[i] = []byte(fmt.Sprintf("item-%d", i))
	}

	b.ResetTimer()
	for i := 0; i < batches; i++ {
		bf.AddBatch(data)
	}
}

func BenchmarkBloomFilterContainsBatch(b *testing.B) {
	config := BloomConfig{
		ExpectedElements:  100000,
		FalsePositiveRate: 0.01,
	}

	bf, err := NewBloomFilter(config)
	if err != nil {
		b.Fatalf("Failed to create Bloom filter: %v", err)
	}
	defer bf.Close()

	batchSize := 1000
	data := make([][]byte, batchSize)
	for i := 0; i < batchSize; i++ {
		data[i] = []byte(fmt.Sprintf("item-%d", i))
		bf.Add(data[i])
	}

	batches := b.N / batchSize
	if batches == 0 {
		batches = 1
	}

	b.ResetTimer()
	for i := 0; i < batches; i++ {
		bf.ContainsBatch(data)
	}
}
