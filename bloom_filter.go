package ds

/*
#include <bloom_filter.h>
#include <stdlib.h>
*/
import "C"
import (
	"errors"
	"runtime"
	"unsafe"
)

// BloomFilter is a space-efficient probabilistic data structure for set membership testing.
// False positive matches are possible, but false negatives are not.
type BloomFilter struct {
	filter *C.fc_bloom_filter_t
}

// BloomConfig contains configuration parameters for creating a Bloom filter.
type BloomConfig struct {
	ExpectedElements  int     // Expected number of elements
	FalsePositiveRate float64 // Desired false positive rate (0.0 to 1.0)
}

// BloomStats contains statistics about a Bloom filter.
type BloomStats struct {
	BitArraySize     int     // Size of bit array in bits
	NumHashFunctions int     // Number of hash functions
	ElementsAdded    int     // Number of elements added
	EstimatedFPP     float64 // Estimated false positive probability
	MemoryBytes      int     // Total memory usage in bytes
}

// NewBloomFilter creates a new Bloom filter with optimal parameters.
// It automatically calculates the optimal bit array size and number of hash functions
// based on the expected number of elements and desired false positive rate.
func NewBloomFilter(config BloomConfig) (*BloomFilter, error) {
	if config.ExpectedElements <= 0 {
		return nil, errors.New("expected elements must be positive")
	}
	if config.FalsePositiveRate <= 0.0 || config.FalsePositiveRate >= 1.0 {
		return nil, errors.New("false positive rate must be between 0.0 and 1.0")
	}

	cConfig := C.fc_bloom_config_t{
		expected_elements:   C.size_t(config.ExpectedElements),
		false_positive_rate: C.double(config.FalsePositiveRate),
	}

	filter := C.fc_bloom_create(&cConfig)
	if filter == nil {
		return nil, errors.New("failed to create Bloom filter")
	}

	return &BloomFilter{filter: filter}, nil
}

// NewBloomFilterExplicit creates a Bloom filter with explicit parameters.
func NewBloomFilterExplicit(bitArraySize, numHashFunctions int) (*BloomFilter, error) {
	if bitArraySize <= 0 || numHashFunctions <= 0 {
		return nil, errors.New("bit array size and number of hash functions must be positive")
	}

	filter := C.fc_bloom_create_explicit(C.size_t(bitArraySize), C.size_t(numHashFunctions))
	if filter == nil {
		return nil, errors.New("failed to create Bloom filter")
	}

	return &BloomFilter{filter: filter}, nil
}

// Close destroys the Bloom filter and frees resources.
func (bf *BloomFilter) Close() error {
	if bf.filter != nil {
		C.fc_bloom_destroy(bf.filter)
		bf.filter = nil
	}
	return nil
}

// Add adds an element to the Bloom filter.
func (bf *BloomFilter) Add(data []byte) error {
	if bf.filter == nil {
		return errors.New("Bloom filter is closed")
	}

	var dataPtr unsafe.Pointer
	if len(data) > 0 {
		dataPtr = unsafe.Pointer(&data[0])
	}

	err := C.fc_bloom_add(bf.filter, dataPtr, C.size_t(len(data)))
	if err != C.FC_OK {
		return errors.New("failed to add element")
	}
	return nil
}

// AddString adds a string to the Bloom filter.
func (bf *BloomFilter) AddString(s string) error {
	return bf.Add([]byte(s))
}

// AddBatch adds multiple elements to the Bloom filter in a single operation.
func (bf *BloomFilter) AddBatch(data [][]byte) error {
	if bf.filter == nil {
		return errors.New("Bloom filter is closed")
	}
	if len(data) == 0 {
		return nil
	}

	// Allocate C arrays
	cPtrs := C.malloc(C.size_t(len(data)) * C.size_t(unsafe.Sizeof(uintptr(0))))
	defer C.free(cPtrs)

	cLengths := C.malloc(C.size_t(len(data)) * C.size_t(unsafe.Sizeof(C.size_t(0))))
	defer C.free(cLengths)

	// Fill arrays
	ptrSlice := (*[1 << 30]unsafe.Pointer)(cPtrs)[:len(data):len(data)]
	lenSlice := (*[1 << 30]C.size_t)(cLengths)[:len(data):len(data)]

	for i := range data {
		if len(data[i]) > 0 {
			ptrSlice[i] = unsafe.Pointer(&data[i][0])
		} else {
			ptrSlice[i] = nil
		}
		lenSlice[i] = C.size_t(len(data[i]))
	}

	err := C.fc_bloom_add_batch(
		bf.filter,
		(*unsafe.Pointer)(cPtrs),
		(*C.size_t)(cLengths),
		C.size_t(len(data)),
	)

	// Keep data alive until after C call completes
	runtime.KeepAlive(data)

	if err != C.FC_OK {
		return errors.New("failed to add batch")
	}
	return nil
}

// Contains checks if an element might be in the set.
// Returns true if the element might be present (possible false positive),
// or false if the element is definitely not present (no false negatives).
func (bf *BloomFilter) Contains(data []byte) (bool, error) {
	if bf.filter == nil {
		return false, errors.New("Bloom filter is closed")
	}

	var result C.bool
	var dataPtr unsafe.Pointer
	if len(data) > 0 {
		dataPtr = unsafe.Pointer(&data[0])
	}

	err := C.fc_bloom_contains(bf.filter, dataPtr, C.size_t(len(data)), &result)
	if err != C.FC_OK {
		return false, errors.New("failed to check element")
	}

	return bool(result), nil
}

// ContainsString checks if a string might be in the set.
func (bf *BloomFilter) ContainsString(s string) (bool, error) {
	return bf.Contains([]byte(s))
}

// ContainsBatch checks multiple elements in a single operation.
func (bf *BloomFilter) ContainsBatch(data [][]byte) ([]bool, error) {
	if bf.filter == nil {
		return nil, errors.New("Bloom filter is closed")
	}
	if len(data) == 0 {
		return []bool{}, nil
	}

	// Allocate C arrays
	cPtrs := C.malloc(C.size_t(len(data)) * C.size_t(unsafe.Sizeof(uintptr(0))))
	defer C.free(cPtrs)

	cLengths := C.malloc(C.size_t(len(data)) * C.size_t(unsafe.Sizeof(C.size_t(0))))
	defer C.free(cLengths)

	cResults := C.malloc(C.size_t(len(data)) * C.size_t(unsafe.Sizeof(C.bool(false))))
	defer C.free(cResults)

	// Fill arrays
	ptrSlice := (*[1 << 30]unsafe.Pointer)(cPtrs)[:len(data):len(data)]
	lenSlice := (*[1 << 30]C.size_t)(cLengths)[:len(data):len(data)]

	for i := range data {
		if len(data[i]) > 0 {
			ptrSlice[i] = unsafe.Pointer(&data[i][0])
		} else {
			ptrSlice[i] = nil
		}
		lenSlice[i] = C.size_t(len(data[i]))
	}

	err := C.fc_bloom_contains_batch(
		bf.filter,
		(*unsafe.Pointer)(cPtrs),
		(*C.size_t)(cLengths),
		C.size_t(len(data)),
		(*C.bool)(cResults),
	)

	// Keep data alive until after C call completes
	runtime.KeepAlive(data)

	if err != C.FC_OK {
		return nil, errors.New("failed to check batch")
	}

	// Copy results
	resultSlice := (*[1 << 30]C.bool)(cResults)[:len(data):len(data)]
	results := make([]bool, len(data))
	for i := range results {
		results[i] = bool(resultSlice[i])
	}

	return results, nil
}

// Clear removes all elements from the Bloom filter.
func (bf *BloomFilter) Clear() error {
	if bf.filter == nil {
		return errors.New("Bloom filter is closed")
	}

	err := C.fc_bloom_clear(bf.filter)
	if err != C.FC_OK {
		return errors.New("failed to clear Bloom filter")
	}
	return nil
}

// Stats returns statistics about the Bloom filter.
func (bf *BloomFilter) Stats() (BloomStats, error) {
	if bf.filter == nil {
		return BloomStats{}, errors.New("Bloom filter is closed")
	}

	var cStats C.fc_bloom_stats_t
	err := C.fc_bloom_get_stats(bf.filter, &cStats)
	if err != C.FC_OK {
		return BloomStats{}, errors.New("failed to get stats")
	}

	return BloomStats{
		BitArraySize:     int(cStats.bit_array_size),
		NumHashFunctions: int(cStats.num_hash_functions),
		ElementsAdded:    int(cStats.elements_added),
		EstimatedFPP:     float64(cStats.estimated_fpp),
		MemoryBytes:      int(cStats.memory_bytes),
	}, nil
}

// OptimalSize calculates the optimal bit array size for given parameters.
func OptimalSize(expectedElements int, falsePositiveRate float64) int {
	size := C.fc_bloom_optimal_size(C.size_t(expectedElements), C.double(falsePositiveRate))
	return int(size)
}

// OptimalHashCount calculates the optimal number of hash functions.
func OptimalHashCount(bitArraySize, expectedElements int) int {
	count := C.fc_bloom_optimal_hash_count(C.size_t(bitArraySize), C.size_t(expectedElements))
	return int(count)
}
