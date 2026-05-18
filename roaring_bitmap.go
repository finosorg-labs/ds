package ds

/*
#include <roaring_bitmap.h>
#include <stdlib.h>
*/
import "C"
import (
	"errors"
	"runtime"
	"unsafe"
)

// RoaringBitmap is a compressed bitmap data structure for efficient set operations.
// It provides fast union, intersection, and difference operations while using
// significantly less memory than traditional bitmaps.
type RoaringBitmap struct {
	bitmap *C.fc_roaring_bitmap_t
}

// RoaringStats contains statistics about a Roaring Bitmap.
type RoaringStats struct {
	Cardinality         uint64 // Number of elements in the bitmap
	NumContainers       int    // Number of containers
	NumArrayContainers  int    // Number of array containers
	NumBitmapContainers int    // Number of bitmap containers
	NumRunContainers    int    // Number of run containers
	MemoryBytes         int    // Total memory usage in bytes
}

// NewRoaringBitmap creates a new empty Roaring Bitmap.
func NewRoaringBitmap() (*RoaringBitmap, error) {
	bitmap := C.fc_roaring_create()
	if bitmap == nil {
		return nil, errors.New("failed to create Roaring Bitmap")
	}

	rb := &RoaringBitmap{bitmap: bitmap}
	runtime.SetFinalizer(rb, (*RoaringBitmap).Close)
	return rb, nil
}

// NewRoaringBitmapFromArray creates a Roaring Bitmap from an array of values.
func NewRoaringBitmapFromArray(values []uint32) (*RoaringBitmap, error) {
	if len(values) == 0 {
		return nil, errors.New("values array is empty")
	}

	var valuesPtr *C.uint32_t
	if len(values) > 0 {
		valuesPtr = (*C.uint32_t)(unsafe.Pointer(&values[0]))
	}

	bitmap := C.fc_roaring_create_from_array(valuesPtr, C.size_t(len(values)))
	if bitmap == nil {
		return nil, errors.New("failed to create Roaring Bitmap from array")
	}

	rb := &RoaringBitmap{bitmap: bitmap}
	runtime.SetFinalizer(rb, (*RoaringBitmap).Close)
	return rb, nil
}

// NewRoaringBitmapFromRange creates a Roaring Bitmap from a range [min, max).
func NewRoaringBitmapFromRange(min, max uint32) (*RoaringBitmap, error) {
	if min >= max {
		return nil, errors.New("invalid range: min must be less than max")
	}

	bitmap := C.fc_roaring_create_from_range(C.uint32_t(min), C.uint32_t(max))
	if bitmap == nil {
		return nil, errors.New("failed to create Roaring Bitmap from range")
	}

	rb := &RoaringBitmap{bitmap: bitmap}
	runtime.SetFinalizer(rb, (*RoaringBitmap).Close)
	return rb, nil
}

// Clone creates a copy of the Roaring Bitmap.
func (rb *RoaringBitmap) Clone() (*RoaringBitmap, error) {
	if rb.bitmap == nil {
		return nil, errors.New("Roaring Bitmap is closed")
	}

	cloned := C.fc_roaring_clone(rb.bitmap)
	if cloned == nil {
		return nil, errors.New("failed to clone Roaring Bitmap")
	}

	clone := &RoaringBitmap{bitmap: cloned}
	runtime.SetFinalizer(clone, (*RoaringBitmap).Close)
	return clone, nil
}

// Close destroys the Roaring Bitmap and frees resources.
func (rb *RoaringBitmap) Close() error {
	if rb.bitmap != nil {
		C.fc_roaring_destroy(rb.bitmap)
		rb.bitmap = nil
		runtime.SetFinalizer(rb, nil)
	}
	return nil
}

// Add adds a value to the bitmap.
func (rb *RoaringBitmap) Add(value uint32) error {
	if rb.bitmap == nil {
		return errors.New("Roaring Bitmap is closed")
	}

	err := C.fc_roaring_add(rb.bitmap, C.uint32_t(value))
	if err != C.FC_OK {
		return errors.New("failed to add value")
	}
	return nil
}

// AddBatch adds multiple values to the bitmap in a single operation.
func (rb *RoaringBitmap) AddBatch(values []uint32) error {
	if rb.bitmap == nil {
		return errors.New("Roaring Bitmap is closed")
	}
	if len(values) == 0 {
		return nil
	}

	var valuesPtr *C.uint32_t
	if len(values) > 0 {
		valuesPtr = (*C.uint32_t)(unsafe.Pointer(&values[0]))
	}

	err := C.fc_roaring_add_batch(rb.bitmap, valuesPtr, C.size_t(len(values)))

	// Keep values alive until after C call completes
	runtime.KeepAlive(values)

	if err != C.FC_OK {
		return errors.New("failed to add batch")
	}
	return nil
}

// AddRange adds a range of values [min, max) to the bitmap.
func (rb *RoaringBitmap) AddRange(min, max uint32) error {
	if rb.bitmap == nil {
		return errors.New("Roaring Bitmap is closed")
	}
	if min >= max {
		return errors.New("invalid range: min must be less than max")
	}

	err := C.fc_roaring_add_range(rb.bitmap, C.uint32_t(min), C.uint32_t(max))
	if err != C.FC_OK {
		return errors.New("failed to add range")
	}
	return nil
}

// Remove removes a value from the bitmap.
func (rb *RoaringBitmap) Remove(value uint32) error {
	if rb.bitmap == nil {
		return errors.New("Roaring Bitmap is closed")
	}

	err := C.fc_roaring_remove(rb.bitmap, C.uint32_t(value))
	if err != C.FC_OK {
		return errors.New("failed to remove value")
	}
	return nil
}

// RemoveRange removes a range of values [min, max) from the bitmap.
func (rb *RoaringBitmap) RemoveRange(min, max uint32) error {
	if rb.bitmap == nil {
		return errors.New("Roaring Bitmap is closed")
	}
	if min >= max {
		return errors.New("invalid range: min must be less than max")
	}

	err := C.fc_roaring_remove_range(rb.bitmap, C.uint32_t(min), C.uint32_t(max))
	if err != C.FC_OK {
		return errors.New("failed to remove range")
	}
	return nil
}

// Contains checks if a value is in the bitmap.
func (rb *RoaringBitmap) Contains(value uint32) (bool, error) {
	if rb.bitmap == nil {
		return false, errors.New("Roaring Bitmap is closed")
	}

	var result C.bool
	err := C.fc_roaring_contains(rb.bitmap, C.uint32_t(value), &result)
	if err != C.FC_OK {
		return false, errors.New("failed to check value")
	}

	return bool(result), nil
}

// ContainsBatch checks multiple values in a single operation.
func (rb *RoaringBitmap) ContainsBatch(values []uint32) ([]bool, error) {
	if rb.bitmap == nil {
		return nil, errors.New("Roaring Bitmap is closed")
	}
	if len(values) == 0 {
		return []bool{}, nil
	}

	var valuesPtr *C.uint32_t
	if len(values) > 0 {
		valuesPtr = (*C.uint32_t)(unsafe.Pointer(&values[0]))
	}

	cResults := C.malloc(C.size_t(len(values)) * C.size_t(unsafe.Sizeof(C.bool(false))))
	defer C.free(cResults)

	err := C.fc_roaring_contains_batch(
		rb.bitmap,
		valuesPtr,
		C.size_t(len(values)),
		(*C.bool)(cResults),
	)

	// Keep values alive until after C call completes
	runtime.KeepAlive(values)

	if err != C.FC_OK {
		return nil, errors.New("failed to check batch")
	}

	// Copy results
	resultSlice := (*[1 << 30]C.bool)(cResults)[:len(values):len(values)]
	results := make([]bool, len(values))
	for i := range results {
		results[i] = bool(resultSlice[i])
	}

	return results, nil
}

// Cardinality returns the number of elements in the bitmap.
func (rb *RoaringBitmap) Cardinality() uint64 {
	if rb.bitmap == nil {
		return 0
	}
	return uint64(C.fc_roaring_cardinality(rb.bitmap))
}

// IsEmpty returns true if the bitmap is empty.
func (rb *RoaringBitmap) IsEmpty() bool {
	if rb.bitmap == nil {
		return true
	}
	return bool(C.fc_roaring_is_empty(rb.bitmap))
}

// Clear removes all elements from the bitmap.
func (rb *RoaringBitmap) Clear() error {
	if rb.bitmap == nil {
		return errors.New("Roaring Bitmap is closed")
	}

	err := C.fc_roaring_clear(rb.bitmap)
	if err != C.FC_OK {
		return errors.New("failed to clear bitmap")
	}
	return nil
}

// Union computes the union of two bitmaps (A ∪ B).
func (rb *RoaringBitmap) Union(other *RoaringBitmap) (*RoaringBitmap, error) {
	if rb.bitmap == nil || other.bitmap == nil {
		return nil, errors.New("Roaring Bitmap is closed")
	}

	result := C.fc_roaring_union(rb.bitmap, other.bitmap)
	if result == nil {
		return nil, errors.New("failed to compute union")
	}

	union := &RoaringBitmap{bitmap: result}
	runtime.SetFinalizer(union, (*RoaringBitmap).Close)
	return union, nil
}

// Intersection computes the intersection of two bitmaps (A ∩ B).
func (rb *RoaringBitmap) Intersection(other *RoaringBitmap) (*RoaringBitmap, error) {
	if rb.bitmap == nil || other.bitmap == nil {
		return nil, errors.New("Roaring Bitmap is closed")
	}

	result := C.fc_roaring_intersection(rb.bitmap, other.bitmap)
	if result == nil {
		return nil, errors.New("failed to compute intersection")
	}

	intersection := &RoaringBitmap{bitmap: result}
	runtime.SetFinalizer(intersection, (*RoaringBitmap).Close)
	return intersection, nil
}

// Difference computes the difference of two bitmaps (A - B).
func (rb *RoaringBitmap) Difference(other *RoaringBitmap) (*RoaringBitmap, error) {
	if rb.bitmap == nil || other.bitmap == nil {
		return nil, errors.New("Roaring Bitmap is closed")
	}

	result := C.fc_roaring_difference(rb.bitmap, other.bitmap)
	if result == nil {
		return nil, errors.New("failed to compute difference")
	}

	difference := &RoaringBitmap{bitmap: result}
	runtime.SetFinalizer(difference, (*RoaringBitmap).Close)
	return difference, nil
}

// Xor computes the symmetric difference of two bitmaps (A ⊕ B).
func (rb *RoaringBitmap) Xor(other *RoaringBitmap) (*RoaringBitmap, error) {
	if rb.bitmap == nil || other.bitmap == nil {
		return nil, errors.New("Roaring Bitmap is closed")
	}

	result := C.fc_roaring_xor(rb.bitmap, other.bitmap)
	if result == nil {
		return nil, errors.New("failed to compute xor")
	}

	xor := &RoaringBitmap{bitmap: result}
	runtime.SetFinalizer(xor, (*RoaringBitmap).Close)
	return xor, nil
}

// UnionInPlace computes union in-place (A = A ∪ B).
func (rb *RoaringBitmap) UnionInPlace(other *RoaringBitmap) error {
	if rb.bitmap == nil || other.bitmap == nil {
		return errors.New("Roaring Bitmap is closed")
	}

	err := C.fc_roaring_union_inplace(rb.bitmap, other.bitmap)
	if err != C.FC_OK {
		return errors.New("failed to compute union in-place")
	}
	return nil
}

// IntersectionInPlace computes intersection in-place (A = A ∩ B).
func (rb *RoaringBitmap) IntersectionInPlace(other *RoaringBitmap) error {
	if rb.bitmap == nil || other.bitmap == nil {
		return errors.New("Roaring Bitmap is closed")
	}

	err := C.fc_roaring_intersection_inplace(rb.bitmap, other.bitmap)
	if err != C.FC_OK {
		return errors.New("failed to compute intersection in-place")
	}
	return nil
}

// Equals checks if two bitmaps are equal.
func (rb *RoaringBitmap) Equals(other *RoaringBitmap) bool {
	if rb.bitmap == nil || other.bitmap == nil {
		return false
	}
	return bool(C.fc_roaring_equals(rb.bitmap, other.bitmap))
}

// IsSubset checks if this bitmap is a subset of another bitmap (A ⊆ B).
func (rb *RoaringBitmap) IsSubset(other *RoaringBitmap) bool {
	if rb.bitmap == nil || other.bitmap == nil {
		return false
	}
	return bool(C.fc_roaring_is_subset(rb.bitmap, other.bitmap))
}

// Intersects checks if two bitmaps have common elements.
func (rb *RoaringBitmap) Intersects(other *RoaringBitmap) bool {
	if rb.bitmap == nil || other.bitmap == nil {
		return false
	}
	return bool(C.fc_roaring_intersects(rb.bitmap, other.bitmap))
}

// Min returns the minimum value in the bitmap.
func (rb *RoaringBitmap) Min() (uint32, error) {
	if rb.bitmap == nil {
		return 0, errors.New("Roaring Bitmap is closed")
	}

	var result C.uint32_t
	err := C.fc_roaring_min(rb.bitmap, &result)
	if err != C.FC_OK {
		return 0, errors.New("bitmap is empty or error occurred")
	}

	return uint32(result), nil
}

// Max returns the maximum value in the bitmap.
func (rb *RoaringBitmap) Max() (uint32, error) {
	if rb.bitmap == nil {
		return 0, errors.New("Roaring Bitmap is closed")
	}

	var result C.uint32_t
	err := C.fc_roaring_max(rb.bitmap, &result)
	if err != C.FC_OK {
		return 0, errors.New("bitmap is empty or error occurred")
	}

	return uint32(result), nil
}

// ToArray converts the bitmap to an array of values.
func (rb *RoaringBitmap) ToArray() ([]uint32, error) {
	if rb.bitmap == nil {
		return nil, errors.New("Roaring Bitmap is closed")
	}

	cardinality := rb.Cardinality()
	if cardinality == 0 {
		return []uint32{}, nil
	}

	values := make([]uint32, cardinality)
	err := C.fc_roaring_to_array(rb.bitmap, (*C.uint32_t)(unsafe.Pointer(&values[0])))
	if err != C.FC_OK {
		return nil, errors.New("failed to convert to array")
	}

	return values, nil
}

// Optimize optimizes the bitmap by converting containers to most efficient type.
func (rb *RoaringBitmap) Optimize() error {
	if rb.bitmap == nil {
		return errors.New("Roaring Bitmap is closed")
	}

	err := C.fc_roaring_optimize(rb.bitmap)
	if err != C.FC_OK {
		return errors.New("failed to optimize bitmap")
	}
	return nil
}

// Stats returns statistics about the bitmap.
func (rb *RoaringBitmap) Stats() (RoaringStats, error) {
	if rb.bitmap == nil {
		return RoaringStats{}, errors.New("Roaring Bitmap is closed")
	}

	var cStats C.fc_roaring_stats_t
	err := C.fc_roaring_get_stats(rb.bitmap, &cStats)
	if err != C.FC_OK {
		return RoaringStats{}, errors.New("failed to get stats")
	}

	return RoaringStats{
		Cardinality:         uint64(cStats.cardinality),
		NumContainers:       int(cStats.num_containers),
		NumArrayContainers:  int(cStats.num_array_containers),
		NumBitmapContainers: int(cStats.num_bitmap_containers),
		NumRunContainers:    int(cStats.num_run_containers),
		MemoryBytes:         int(cStats.memory_bytes),
	}, nil
}
