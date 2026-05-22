package ds

import (
	"encoding/binary"
	"math"
	"testing"
)

func TestGenericRingBufferCreateDestroy(t *testing.T) {
	rb := NewGenericRingBuffer(10, 4)
	if rb == nil {
		t.Fatal("Failed to create generic ring buffer")
	}
	defer rb.Destroy()

	if rb.Capacity() < 10 {
		t.Errorf("Capacity should be at least 10, got %d", rb.Capacity())
	}

	if rb.ElementSize() != 4 {
		t.Errorf("Element size should be 4, got %d", rb.ElementSize())
	}

	if !rb.IsEmpty() {
		t.Error("New buffer should be empty")
	}
}

func TestGenericRingBufferCreateInvalid(t *testing.T) {
	tests := []struct {
		name        string
		capacity    int
		elementSize int
	}{
		{"zero capacity", 0, 4},
		{"negative capacity", -1, 4},
		{"zero element size", 10, 0},
		{"negative element size", 10, -1},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			rb := NewGenericRingBuffer(tt.capacity, tt.elementSize)
			if rb != nil {
				rb.Destroy()
				t.Error("Should fail to create buffer with invalid parameters")
			}
		})
	}
}

func TestGenericRingBufferPushPopInt32(t *testing.T) {
	rb := NewGenericRingBuffer(4, 4)
	if rb == nil {
		t.Fatal("Failed to create generic ring buffer")
	}
	defer rb.Destroy()

	values := []int32{10, 20, 30, 40}
	for _, v := range values {
		data := make([]byte, 4)
		binary.LittleEndian.PutUint32(data, uint32(v))
		if !rb.Push(data) {
			t.Errorf("Failed to push value %d", v)
		}
	}

	if rb.Size() != 4 {
		t.Errorf("Size should be 4, got %d", rb.Size())
	}

	if !rb.IsFull() {
		t.Error("Buffer should be full")
	}

	for _, expected := range values {
		data, ok := rb.Pop()
		if !ok {
			t.Error("Failed to pop value")
		}
		actual := int32(binary.LittleEndian.Uint32(data))
		if actual != expected {
			t.Errorf("Expected %d, got %d", expected, actual)
		}
	}

	if !rb.IsEmpty() {
		t.Error("Buffer should be empty")
	}
}

func TestGenericRingBufferPushPopStruct(t *testing.T) {
	type TestStruct struct {
		ID    int32
		Value float64
	}

	elementSize := 12 // 4 bytes for int32 + 8 bytes for float64
	rb := NewGenericRingBuffer(4, elementSize)
	if rb == nil {
		t.Fatal("Failed to create generic ring buffer")
	}
	defer rb.Destroy()

	values := []TestStruct{
		{1, 1.1},
		{2, 2.2},
		{3, 3.3},
		{4, 4.4},
	}

	for _, v := range values {
		data := make([]byte, elementSize)
		binary.LittleEndian.PutUint32(data[0:4], uint32(v.ID))
		binary.LittleEndian.PutUint64(data[4:12], math.Float64bits(v.Value))
		if !rb.Push(data) {
			t.Errorf("Failed to push struct %+v", v)
		}
	}

	if rb.Size() != 4 {
		t.Errorf("Size should be 4, got %d", rb.Size())
	}

	for _, expected := range values {
		data, ok := rb.Pop()
		if !ok {
			t.Error("Failed to pop struct")
		}
		id := int32(binary.LittleEndian.Uint32(data[0:4]))
		value := math.Float64frombits(binary.LittleEndian.Uint64(data[4:12]))

		if id != expected.ID {
			t.Errorf("Expected ID %d, got %d", expected.ID, id)
		}
		if value != expected.Value {
			t.Errorf("Expected Value %f, got %f", expected.Value, value)
		}
	}
}

func TestGenericRingBufferOverwrite(t *testing.T) {
	rb := NewGenericRingBuffer(4, 4)
	if rb == nil {
		t.Fatal("Failed to create generic ring buffer")
	}
	defer rb.Destroy()

	// Push 8 values into a buffer with capacity 4
	for i := 0; i < 8; i++ {
		data := make([]byte, 4)
		binary.LittleEndian.PutUint32(data, uint32(i))
		rb.Push(data)
	}

	if rb.Size() != 4 {
		t.Errorf("Size should be 4, got %d", rb.Size())
	}

	// Should get the last 4 values (4, 5, 6, 7)
	for i := 4; i < 8; i++ {
		data, ok := rb.Pop()
		if !ok {
			t.Error("Failed to pop value")
		}
		actual := int32(binary.LittleEndian.Uint32(data))
		if actual != int32(i) {
			t.Errorf("Expected %d, got %d", i, actual)
		}
	}
}

func TestGenericRingBufferBatchOperations(t *testing.T) {
	rb := NewGenericRingBuffer(8, 4)
	if rb == nil {
		t.Fatal("Failed to create generic ring buffer")
	}
	defer rb.Destroy()

	values := []int32{1, 2, 3, 4, 5}
	data := make([]byte, len(values)*4)
	for i, v := range values {
		binary.LittleEndian.PutUint32(data[i*4:(i+1)*4], uint32(v))
	}

	pushed := rb.PushBatch(data)
	if pushed != 5 {
		t.Errorf("Should push 5 values, got %d", pushed)
	}

	if rb.Size() != 5 {
		t.Errorf("Size should be 5, got %d", rb.Size())
	}

	outData := rb.PopBatch(5)
	if len(outData) != 20 {
		t.Errorf("Should pop 20 bytes, got %d", len(outData))
	}

	for i, expected := range values {
		actual := int32(binary.LittleEndian.Uint32(outData[i*4 : (i+1)*4]))
		if actual != expected {
			t.Errorf("Expected %d, got %d", expected, actual)
		}
	}
}

func TestGenericRingBufferGet(t *testing.T) {
	rb := NewGenericRingBuffer(8, 4)
	if rb == nil {
		t.Fatal("Failed to create generic ring buffer")
	}
	defer rb.Destroy()

	values := []int32{10, 20, 30, 40, 50}
	data := make([]byte, len(values)*4)
	for i, v := range values {
		binary.LittleEndian.PutUint32(data[i*4:(i+1)*4], uint32(v))
	}

	rb.PushBatch(data)

	for i, expected := range values {
		outData, ok := rb.Get(i)
		if !ok {
			t.Errorf("Failed to get value at index %d", i)
		}
		actual := int32(binary.LittleEndian.Uint32(outData))
		if actual != expected {
			t.Errorf("Expected %d, got %d", expected, actual)
		}
	}

	// Out of bounds
	_, ok := rb.Get(5)
	if ok {
		t.Error("Should fail for out of bounds index")
	}
}

func TestGenericRingBufferGetAll(t *testing.T) {
	rb := NewGenericRingBuffer(8, 4)
	if rb == nil {
		t.Fatal("Failed to create generic ring buffer")
	}
	defer rb.Destroy()

	values := []int32{10, 20, 30, 40, 50}
	data := make([]byte, len(values)*4)
	for i, v := range values {
		binary.LittleEndian.PutUint32(data[i*4:(i+1)*4], uint32(v))
	}

	rb.PushBatch(data)

	allData := rb.GetAll()
	if len(allData) != 20 {
		t.Errorf("Should get 20 bytes, got %d", len(allData))
	}

	for i, expected := range values {
		actual := int32(binary.LittleEndian.Uint32(allData[i*4 : (i+1)*4]))
		if actual != expected {
			t.Errorf("Expected %d, got %d", expected, actual)
		}
	}
}

func TestGenericRingBufferClear(t *testing.T) {
	rb := NewGenericRingBuffer(8, 4)
	if rb == nil {
		t.Fatal("Failed to create generic ring buffer")
	}
	defer rb.Destroy()

	values := []int32{1, 2, 3, 4, 5}
	data := make([]byte, len(values)*4)
	for i, v := range values {
		binary.LittleEndian.PutUint32(data[i*4:(i+1)*4], uint32(v))
	}

	rb.PushBatch(data)

	if rb.Size() != 5 {
		t.Errorf("Size should be 5, got %d", rb.Size())
	}

	rb.Clear()

	if !rb.IsEmpty() {
		t.Error("Buffer should be empty after clear")
	}

	if rb.Size() != 0 {
		t.Errorf("Size should be 0 after clear, got %d", rb.Size())
	}
}

func TestGenericRingBufferInvalidInputs(t *testing.T) {
	rb := NewGenericRingBuffer(8, 4)
	if rb == nil {
		t.Fatal("Failed to create generic ring buffer")
	}
	defer rb.Destroy()

	// Push with wrong size
	wrongData := make([]byte, 3)
	if rb.Push(wrongData) {
		t.Error("Should fail to push data with wrong size")
	}

	// PushBatch with wrong size
	wrongBatch := make([]byte, 7)
	if rb.PushBatch(wrongBatch) != 0 {
		t.Error("Should fail to push batch with wrong size")
	}

	// Pop from empty buffer
	_, ok := rb.Pop()
	if ok {
		t.Error("Should fail to pop from empty buffer")
	}

	// PopBatch from empty buffer
	emptyBatch := rb.PopBatch(5)
	if emptyBatch != nil {
		t.Error("Should return nil when popping from empty buffer")
	}
}

func TestGenericRingBufferImplementation(t *testing.T) {
	impl := GenericRingBufferImplementation()
	if impl == "" {
		t.Error("Implementation string should not be empty")
	}
	t.Logf("Generic Ring Buffer Implementation: %s", impl)
}
