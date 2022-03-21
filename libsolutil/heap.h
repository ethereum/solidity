/******************************************************************************************[Heap.h]
Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
Copyright (c) 2007-2010, Niklas Sorensson

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**************************************************************************************************/

#ifndef Glucose_Heap_h
#define Glucose_Heap_h

#include <vector>
#include <cassert>
#include <iostream>

//=================================================================================================
// A heap implementation with support for decrease/increase key.


template<class Comp>
class Heap {
    Comp     lt;       // The heap is a minimum-heap with respect to this comparator
    std::vector<int> heap;     // Heap of integers
    std::vector<int> indices;  // Each integers position (index) in the Heap

    // Index "traversal" functions
    static inline int left  (int i)
    {
        return i * 2 + 1;
    }
    static inline int right (int i)
    {
        return (i + 1) * 2;
    }
    static inline int parent(int i)
    {
        return (i - 1) >> 1;
    }


    void percolateUp(int i)
    {
        int x  = heap[(size_t)i];
        int p  = parent(i);

        while (i != 0 && lt(x, heap[(size_t)p])) {
            heap[(size_t)i]          = heap[(size_t)p];
            indices[(size_t)heap[(size_t)p]] = i;
            i                = p;
            p                = parent(p);
        }
        heap   [(size_t)i] = x;
        indices[(size_t)x] = i;
    }


    void percolateDown(int i)
    {
        int x = heap[(size_t)i];
        while (left(i) < (int)heap.size()) {
            int child = right(i) < (int)heap.size() && lt(heap[(size_t)right(i)], heap[(size_t)left(i)]) ? right(i) : left(i);
            if (!lt(heap[(size_t)child], x)) {
                break;
            }
            heap[(size_t)i]          = heap[(size_t)child];
            indices[(size_t)heap[(size_t)i]] = i;
            i                = child;
        }
        heap   [(size_t)i] = x;
        indices[(size_t)x] = i;
    }


public:
    Heap(const Comp& c) : lt(c) { }

    void print_heap() {
        std::cout << "heap:";
        for(auto x: heap) {
            std::cout << x << " ";
        }
        std::cout << std::endl;

        std::cout << "ind:";
        for(auto x: indices) {
            std::cout << x << " ";
        }
        std::cout << std::endl;
    }

    uint32_t  size      ()          const
    {
        return heap.size();
    }
    bool empty     ()          const
    {
        return heap.size() == 0;
    }
    bool inHeap    (int n)     const
    {
        return n < (int)indices.size() && indices[(size_t)n] >= 0;
    }
    int  operator[](int index) const
    {
        assert(index < (int)heap.size());
        return heap[index];
    }

    void decrease  (int n)
    {
        assert(inHeap(n));
        percolateUp  (indices[(size_t)n]);
    }
    void increase  (int n)
    {
        assert(inHeap(n));
        percolateDown(indices[n]);
    }


    // Safe variant of insert/decrease/increase:
    void update(int n)
    {
        if (!inHeap(n)) {
            insert(n);
        } else {
            percolateUp(indices[n]);
            percolateDown(indices[n]);
        }
    }


    void insert(int n)
    {
        indices.resize((size_t)n + 1, -1);
        assert(!inHeap(n));

        indices[(size_t)n] = (int)heap.size();
        heap.push_back(n);
        percolateUp(indices[(size_t)n]);
    }


    int  removeMin()
    {
        int x            = heap[0];
        heap[0]          = (int)heap.back();
        indices[(size_t)heap[0]] = 0;
        indices[(size_t)x]       = -1;
        heap.pop_back();
        if (heap.size() > 1) {
            percolateDown(0);
        }
        return x;
    }


    // Rebuild the heap from scratch, using the elements in 'ns':
    template<typename T>
    void build(const T& ns)
    {
        for (int i = 0; i < (int)heap.size(); i++) {
            indices[heap[i]] = -1;
        }
        heap.clear();

        for (uint32_t i = 0; i < ns.size(); i++) {
            indices[ns[i]] = i;
            heap.push_back(ns[i]);
        }

        for (int i = (int)heap.size() / 2 - 1; i >= 0; i--) {
            percolateDown(i);
        }
    }

    void clear(bool dealloc = false)
    {
        for (int i = 0; i < (int)heap.size(); i++) {
            indices[heap[i]] = -1;
        }
        heap.clear();
    }

    size_t mem_used() const
    {
        size_t mem = 0;
        mem += heap.capacity()*sizeof(uint32_t);
        mem += indices.capacity()*sizeof(uint32_t);
        return mem;
    }

    bool heap_property (uint32_t i) const {
        return i >= heap.size()
            || ( (i == 0 || !lt(heap[i], heap[(size_t)parent((int)i)]))
                  && heap_property( (uint32_t)left((int)i)  )
                  && heap_property( (uint32_t)right((int)i) )
            );
    }

    bool heap_property() const {
        return heap_property(0);
    }

};


#endif
