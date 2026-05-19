package ds

import (
	"testing"
)

func TestRingBufferCreateDestroy(t *testing.T) {
	rb, err := NewRingBuffer(10)
	if err != nil {
		t.Fatalf("Failed to create ring buffer: %v", err)
	}
	defer rb.Destroy()

	if rb.Capacity() != 16 {
		t.Errorf("Expected capacity 16 (rounded up), got %d", rb.Capacity())
	}

	if rb.Size() != 0 {
		t.Errorf("Expected initial size 0, got %d", rb.Size())
	}

	if !rb.IsEmpty() {
		t.Error("Expected buffer to be empty initially")
	}

	if rb.IsFull() {
		t.Error("Expected buffer not to be full initially")
	}
}

func TestRingBufferPushPop(t *testing.T) {
	rb, err := NewRingBuffer(4)
	if err != nil {
		t.Fatalf("Failed to create ring buffer: %v", err)
	}
	defer rb.Destroy()

	if err := rb.Push(1.0); err != nil {
		t.Errorf("Push failed: %v", err)
	}

	if rb.Size() != 1 {
		t.Errorf("Expected size 1, got %d", rb.Size())
	}

	if err := rb.Push(2.0); err != nil {
		t.Errorf("Push failed: %v", err)
	}

	val, err := rb.Pop()
	if err != nil {
		t.Errorf("Pop failed: %v", err)
	}
	if val != 1.0 {
		t.Errorf("Expected 1.0, got %f", val)
	}

	val, err = rb.Pop()
	if err != nil {
		t.Errorf("Pop failed: %v", err)
	}
	if val != 2.0 {
		t.Errorf("Expected 2.0, got %f", val)
	}

	if !rb.IsEmpty() {
		t.Error("Expected buffer to be empty after popping all elements")
	}
}

func TestRingBufferWraparound(t *testing.T) {
	rb, err := NewRingBuffer(4)
	if err != nil {
		t.Fatalf("Failed to create ring buffer: %v", err)
	}
	defer rb.Destroy()

	for i := 0; i < 10; i++ {
		rb.Push(float64(i))
	}

	if rb.Size() != 4 {
		t.Errorf("Expected size 4, got %d", rb.Size())
	}

	if !rb.IsFull() {
		t.Error("Expected buffer to be full")
	}

	val, err := rb.Pop()
	if err != nil {
		t.Errorf("Pop failed: %v", err)
	}
	if val != 6.0 {
		t.Errorf("Expected oldest value 6.0 (wraparound), got %f", val)
	}
}

func TestRingBufferBatchOperations(t *testing.T) {
	rb, err := NewRingBuffer(8)
	if err != nil {
		t.Fatalf("Failed to create ring buffer: %v", err)
	}
	defer rb.Destroy()

	values := []float64{1.0, 2.0, 3.0, 4.0, 5.0}
	n, err := rb.PushBatch(values)
	if err != nil {
		t.Errorf("PushBatch failed: %v", err)
	}
	if n != 5 {
		t.Errorf("Expected to push 5 values, pushed %d", n)
	}

	if rb.Size() != 5 {
		t.Errorf("Expected size 5, got %d", rb.Size())
	}

	popped, err := rb.PopBatch(3)
	if err != nil {
		t.Errorf("PopBatch failed: %v", err)
	}
	if len(popped) != 3 {
		t.Errorf("Expected to pop 3 values, popped %d", len(popped))
	}

	expected := []float64{1.0, 2.0, 3.0}
	for i, v := range expected {
		if popped[i] != v {
			t.Errorf("Expected popped[%d] = %f, got %f", i, v, popped[i])
		}
	}

	if rb.Size() != 2 {
		t.Errorf("Expected size 2 after pop, got %d", rb.Size())
	}
}

func TestRingBufferGet(t *testing.T) {
	rb, err := NewRingBuffer(8)
	if err != nil {
		t.Fatalf("Failed to create ring buffer: %v", err)
	}
	defer rb.Destroy()

	values := []float64{10.0, 20.0, 30.0, 40.0}
	rb.PushBatch(values)

	val, err := rb.Get(0)
	if err != nil {
		t.Errorf("Get(0) failed: %v", err)
	}
	if val != 10.0 {
		t.Errorf("Expected Get(0) = 10.0, got %f", val)
	}

	val, err = rb.Get(3)
	if err != nil {
		t.Errorf("Get(3) failed: %v", err)
	}
	if val != 40.0 {
		t.Errorf("Expected Get(3) = 40.0, got %f", val)
	}

	_, err = rb.Get(4)
	if err == nil {
		t.Error("Expected Get(4) to fail (out of bounds)")
	}
}

func TestRingBufferGetAll(t *testing.T) {
	rb, err := NewRingBuffer(8)
	if err != nil {
		t.Fatalf("Failed to create ring buffer: %v", err)
	}
	defer rb.Destroy()

	values := []float64{10.0, 20.0, 30.0, 40.0}
	rb.PushBatch(values)

	all, err := rb.GetAll()
	if err != nil {
		t.Errorf("GetAll failed: %v", err)
	}

	if len(all) != 4 {
		t.Errorf("Expected GetAll to return 4 elements, got %d", len(all))
	}

	for i, expected := range values {
		if all[i] != expected {
			t.Errorf("Expected all[%d] = %f, got %f", i, expected, all[i])
		}
	}
}

func TestRingBufferClear(t *testing.T) {
	rb, err := NewRingBuffer(8)
	if err != nil {
		t.Fatalf("Failed to create ring buffer: %v", err)
	}
	defer rb.Destroy()

	values := []float64{1.0, 2.0, 3.0}
	rb.PushBatch(values)

	if rb.Size() != 3 {
		t.Errorf("Expected size 3, got %d", rb.Size())
	}

	rb.Clear()

	if rb.Size() != 0 {
		t.Errorf("Expected size 0 after clear, got %d", rb.Size())
	}

	if !rb.IsEmpty() {
		t.Error("Expected buffer to be empty after clear")
	}
}

func TestRingBufferSlidingWindow(t *testing.T) {
	rb, err := NewRingBuffer(5)
	if err != nil {
		t.Fatalf("Failed to create ring buffer: %v", err)
	}
	defer rb.Destroy()

	// Capacity is rounded up to 8 (next power of 2)
	actualCapacity := rb.Capacity()

	for i := 0; i < 20; i++ {
		rb.Push(float64(i))

		// Once buffer is full, verify sliding window behavior
		if rb.IsFull() {
			all, _ := rb.GetAll()
			if len(all) != actualCapacity {
				t.Errorf("Expected window size %d, got %d", actualCapacity, len(all))
			}

			// Verify the values are the most recent ones
			expectedStart := i - actualCapacity + 1
			for j, v := range all {
				expected := float64(expectedStart + j)
				if v != expected {
					t.Errorf("Expected all[%d] = %f, got %f", j, expected, v)
				}
			}
		}
	}
}

func TestRingBufferEmptyPop(t *testing.T) {
	rb, err := NewRingBuffer(4)
	if err != nil {
		t.Fatalf("Failed to create ring buffer: %v", err)
	}
	defer rb.Destroy()

	_, err = rb.Pop()
	if err != ErrEmptyBuffer {
		t.Errorf("Expected ErrEmptyBuffer, got %v", err)
	}

	popped, err := rb.PopBatch(5)
	if err != nil {
		t.Errorf("PopBatch on empty buffer failed: %v", err)
	}
	if len(popped) != 0 {
		t.Errorf("Expected empty slice, got %d elements", len(popped))
	}
}

func TestRingBufferInvalidInputs(t *testing.T) {
	_, err := NewRingBuffer(0)
	if err == nil {
		t.Error("Expected error when creating buffer with 0 capacity")
	}

	_, err = NewRingBuffer(-1)
	if err == nil {
		t.Error("Expected error when creating buffer with negative capacity")
	}
}

func BenchmarkRingBufferPush(b *testing.B) {
	rb, _ := NewRingBuffer(1024)
	defer rb.Destroy()

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		rb.Push(float64(i))
	}
}

func BenchmarkRingBufferPushBatch(b *testing.B) {
	rb, _ := NewRingBuffer(1024)
	defer rb.Destroy()

	values := make([]float64, 100)
	for i := range values {
		values[i] = float64(i)
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		rb.PushBatch(values)
	}
}

func BenchmarkRingBufferPop(b *testing.B) {
	rb, _ := NewRingBuffer(1024)
	defer rb.Destroy()

	for i := 0; i < 1024; i++ {
		rb.Push(float64(i))
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		rb.Pop()
		if rb.IsEmpty() {
			rb.Push(float64(i))
		}
	}
}

func BenchmarkRingBufferGetAll(b *testing.B) {
	rb, _ := NewRingBuffer(1024)
	defer rb.Destroy()

	for i := 0; i < 1024; i++ {
		rb.Push(float64(i))
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		rb.GetAll()
	}
}

func BenchmarkRingBufferSlidingWindow(b *testing.B) {
	rb, _ := NewRingBuffer(100)
	defer rb.Destroy()

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		rb.Push(float64(i))

		if i%10 == 0 && rb.Size() == 100 {
			all, _ := rb.GetAll()
			sum := 0.0
			for _, v := range all {
				sum += v
			}
		}
	}
}
