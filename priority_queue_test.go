package ds

import (
	"testing"
)

func TestPriorityQueueCreateDestroy(t *testing.T) {
	pq, err := NewPriorityQueue(10)
	if err != nil {
		t.Fatalf("Failed to create priority queue: %v", err)
	}
	defer pq.Destroy()

	if pq.Capacity() != 10 {
		t.Errorf("Expected capacity 10, got %d", pq.Capacity())
	}

	if pq.Size() != 0 {
		t.Errorf("Expected initial size 0, got %d", pq.Size())
	}

	if !pq.IsEmpty() {
		t.Error("Expected queue to be empty initially")
	}

	if pq.IsFull() {
		t.Error("Expected queue not to be full initially")
	}
}

func TestPriorityQueueInsertPop(t *testing.T) {
	pq, err := NewPriorityQueue(10)
	if err != nil {
		t.Fatalf("Failed to create priority queue: %v", err)
	}
	defer pq.Destroy()

	if err := pq.Insert(5.0, "five"); err != nil {
		t.Errorf("Insert failed: %v", err)
	}

	if pq.Size() != 1 {
		t.Errorf("Expected size 1, got %d", pq.Size())
	}

	if err := pq.Insert(3.0, "three"); err != nil {
		t.Errorf("Insert failed: %v", err)
	}

	if err := pq.Insert(8.0, "eight"); err != nil {
		t.Errorf("Insert failed: %v", err)
	}

	elem, err := pq.Pop()
	if err != nil {
		t.Errorf("Pop failed: %v", err)
	}
	if elem.Priority != 8.0 {
		t.Errorf("Expected priority 8.0, got %f", elem.Priority)
	}

	elem, err = pq.Pop()
	if err != nil {
		t.Errorf("Pop failed: %v", err)
	}
	if elem.Priority != 5.0 {
		t.Errorf("Expected priority 5.0, got %f", elem.Priority)
	}

	elem, err = pq.Pop()
	if err != nil {
		t.Errorf("Pop failed: %v", err)
	}
	if elem.Priority != 3.0 {
		t.Errorf("Expected priority 3.0, got %f", elem.Priority)
	}

	if !pq.IsEmpty() {
		t.Error("Expected queue to be empty after popping all elements")
	}
}

func TestPriorityQueuePeek(t *testing.T) {
	pq, err := NewPriorityQueue(5)
	if err != nil {
		t.Fatalf("Failed to create priority queue: %v", err)
	}
	defer pq.Destroy()

	pq.Insert(10.0, "ten")
	pq.Insert(20.0, "twenty")
	pq.Insert(15.0, "fifteen")

	elem, err := pq.Peek()
	if err != nil {
		t.Errorf("Peek failed: %v", err)
	}
	if elem.Priority != 20.0 {
		t.Errorf("Expected priority 20.0, got %f", elem.Priority)
	}

	if pq.Size() != 3 {
		t.Errorf("Expected size 3 after peek, got %d", pq.Size())
	}

	elem, err = pq.Peek()
	if err != nil {
		t.Errorf("Peek failed: %v", err)
	}
	if elem.Priority != 20.0 {
		t.Errorf("Expected priority 20.0 on second peek, got %f", elem.Priority)
	}
}

func TestPriorityQueueOrdering(t *testing.T) {
	pq, err := NewPriorityQueue(100)
	if err != nil {
		t.Fatalf("Failed to create priority queue: %v", err)
	}
	defer pq.Destroy()

	priorities := []float64{45.2, 12.3, 78.9, 23.4, 67.8, 34.5, 89.1, 56.7, 90.0, 11.1}
	for i, p := range priorities {
		if err := pq.Insert(p, i); err != nil {
			t.Errorf("Insert failed: %v", err)
		}
	}

	prevPriority := 1000.0
	for i := 0; i < len(priorities); i++ {
		elem, err := pq.Pop()
		if err != nil {
			t.Errorf("Pop failed: %v", err)
		}
		if elem.Priority > prevPriority {
			t.Errorf("Priority ordering violated: %f > %f", elem.Priority, prevPriority)
		}
		prevPriority = elem.Priority
	}

	if !pq.IsEmpty() {
		t.Error("Expected queue to be empty after popping all elements")
	}
}

func TestPriorityQueueFull(t *testing.T) {
	pq, err := NewPriorityQueue(3)
	if err != nil {
		t.Fatalf("Failed to create priority queue: %v", err)
	}
	defer pq.Destroy()

	if err := pq.Insert(1.0, nil); err != nil {
		t.Errorf("Insert failed: %v", err)
	}
	if err := pq.Insert(2.0, nil); err != nil {
		t.Errorf("Insert failed: %v", err)
	}
	if err := pq.Insert(3.0, nil); err != nil {
		t.Errorf("Insert failed: %v", err)
	}

	if !pq.IsFull() {
		t.Error("Expected queue to be full")
	}

	if err := pq.Insert(4.0, nil); err == nil {
		t.Error("Expected insert to fail when queue is full")
	}
}

func TestPriorityQueueClear(t *testing.T) {
	pq, err := NewPriorityQueue(10)
	if err != nil {
		t.Fatalf("Failed to create priority queue: %v", err)
	}
	defer pq.Destroy()

	for i := 0; i < 5; i++ {
		pq.Insert(float64(i), i)
	}

	if pq.Size() != 5 {
		t.Errorf("Expected size 5, got %d", pq.Size())
	}

	pq.Clear()

	if pq.Size() != 0 {
		t.Errorf("Expected size 0 after clear, got %d", pq.Size())
	}

	if !pq.IsEmpty() {
		t.Error("Expected queue to be empty after clear")
	}
}

func TestPriorityQueueNegativePriorities(t *testing.T) {
	pq, err := NewPriorityQueue(10)
	if err != nil {
		t.Fatalf("Failed to create priority queue: %v", err)
	}
	defer pq.Destroy()

	pq.Insert(-5.0, "neg5")
	pq.Insert(0.0, "zero")
	pq.Insert(-10.0, "neg10")
	pq.Insert(5.0, "pos5")

	elem, _ := pq.Pop()
	if elem.Priority != 5.0 {
		t.Errorf("Expected priority 5.0, got %f", elem.Priority)
	}

	elem, _ = pq.Pop()
	if elem.Priority != 0.0 {
		t.Errorf("Expected priority 0.0, got %f", elem.Priority)
	}

	elem, _ = pq.Pop()
	if elem.Priority != -5.0 {
		t.Errorf("Expected priority -5.0, got %f", elem.Priority)
	}

	elem, _ = pq.Pop()
	if elem.Priority != -10.0 {
		t.Errorf("Expected priority -10.0, got %f", elem.Priority)
	}
}

func TestPriorityQueueEmptyOperations(t *testing.T) {
	pq, err := NewPriorityQueue(5)
	if err != nil {
		t.Fatalf("Failed to create priority queue: %v", err)
	}
	defer pq.Destroy()

	_, err = pq.Pop()
	if err == nil {
		t.Error("Expected pop on empty queue to fail")
	}

	_, err = pq.Peek()
	if err == nil {
		t.Error("Expected peek on empty queue to fail")
	}
}

func TestPriorityQueueInvalidCapacity(t *testing.T) {
	_, err := NewPriorityQueue(0)
	if err == nil {
		t.Error("Expected creation with capacity 0 to fail")
	}

	_, err = NewPriorityQueue(-1)
	if err == nil {
		t.Error("Expected creation with negative capacity to fail")
	}
}

func TestPriorityQueueLargeScale(t *testing.T) {
	const n = 1000
	pq, err := NewPriorityQueue(n)
	if err != nil {
		t.Fatalf("Failed to create priority queue: %v", err)
	}
	defer pq.Destroy()

	for i := 0; i < n; i++ {
		if err := pq.Insert(float64(n-i), i); err != nil {
			t.Errorf("Insert failed at %d: %v", i, err)
		}
	}

	if pq.Size() != n {
		t.Errorf("Expected size %d, got %d", n, pq.Size())
	}

	prevPriority := float64(n + 1)
	for i := 0; i < n; i++ {
		elem, err := pq.Pop()
		if err != nil {
			t.Errorf("Pop failed at %d: %v", i, err)
		}
		if elem.Priority > prevPriority {
			t.Errorf("Priority ordering violated at %d: %f > %f", i, elem.Priority, prevPriority)
		}
		prevPriority = elem.Priority
	}

	if !pq.IsEmpty() {
		t.Error("Expected queue to be empty after popping all elements")
	}
}

func BenchmarkPriorityQueueInsert(b *testing.B) {
	pq, _ := NewPriorityQueue(b.N)
	defer pq.Destroy()

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		pq.Insert(float64(i), i)
	}
}

func BenchmarkPriorityQueuePop(b *testing.B) {
	pq, _ := NewPriorityQueue(b.N)
	defer pq.Destroy()

	for i := 0; i < b.N; i++ {
		pq.Insert(float64(i), i)
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		pq.Pop()
	}
}

func BenchmarkPriorityQueuePeek(b *testing.B) {
	pq, _ := NewPriorityQueue(1000)
	defer pq.Destroy()

	for i := 0; i < 1000; i++ {
		pq.Insert(float64(i), i)
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		pq.Peek()
	}
}

func BenchmarkPriorityQueueMixedOps(b *testing.B) {
	pq, _ := NewPriorityQueue(1000)
	defer pq.Destroy()

	for i := 0; i < 500; i++ {
		pq.Insert(float64(i), i)
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		pq.Insert(float64(i), i)
		pq.Peek()
		pq.Pop()
	}
}
