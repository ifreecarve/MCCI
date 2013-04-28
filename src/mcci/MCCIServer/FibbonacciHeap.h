/**
 * template Fibonacci Heap 
 *
 * @ref http://en.wikipedia.org/wiki/Fibonacci_heap
 * @ref http://www.cse.yorku.ca/~aaw/Jason/FibonacciHeapAlgorithm.html
 * @author Erel Segal http://tora.us.fm/rentabrain
 * @date 2010-11-11
 */

#include <iostream>
#include <vector>
using namespace std;

typedef unsigned int uint;


/**
 * The heap is a min-heap sorted by Key.
 */
template <typename Data, typename Key> class FibonacciHeapNode
{
    Key m_key;
    Data m_data;
    
    uint m_degree; // number of childern. used in the removeMinimum algorithm.
    bool m_mark;   // mark used in the decreaseKey algorithm.
    
    //uint m_count; // total number of elements in tree, including this. For debug only
    
    
    FibonacciHeapNode<Data, Key>* m_previous;  // pointers in a circular doubly linked list
    FibonacciHeapNode<Data, Key>* m_next;
    FibonacciHeapNode<Data, Key>* m_child; // pointer to the first child in the list of children
    FibonacciHeapNode<Data, Key>* m_parent;
    
    FibonacciHeapNode() {};
    
    FibonacciHeapNode(Data d, Key k);
	
    // whether this is the only node in the ring
    bool is_single() const;
        
    // inserts a new node after this node
    void insert(FibonacciHeapNode<Data, Key>* other);

    // remove a node from the ring
    void remove();

    void add_child(FibonacciHeapNode<Data, Key>* other);
    void remove_child(FibonacciHeapNode<Data, Key>* other);

    
    friend ostream& operator<< (ostream& out, const FibonacciHeapNode<Data, Key>& n);
    void print_tree(ostream& out) const;
    void print_all(ostream& out) const;
	
public:
    Key key() const { return m_key; }
    Data data() const { return m_data; }
	
    template <typename D, typename K> friend class FibonacciHeap;
}; // FibonacciHeapNode



template <typename Data, typename Key> class FibonacciHeap 
{
    typedef FibonacciHeapNode<Data, Key>* PNode;
    PNode m_root_with_min_key; // a circular d-list of nodes
    uint m_count;      // total number of elements in heap
    uint m_max_degree;  // maximum degree (=child count) of a root in the  circular d-list
    
protected:
    PNode insert_node(PNode new_node) 
    {
        //if (m_debug) cout << "insert " << (*new_node) << endl;
        if (!m_root_with_min_key) 
        { 
            // insert the first m_key to the heap:
            m_root_with_min_key = new_node;
        } 
        else 
        {
            m_root_with_min_key->insert(new_node);  // insert the root of new tree to the list of roots
            if (new_node->key() < m_root_with_min_key->key())
                m_root_with_min_key = new_node;
        }
        return new_node;
    }

public:
    bool m_debug, m_debug_remove_min, m_debug_decrease_key;
    

    FibonacciHeap()
    {
        m_root_with_min_key  = NULL;
        m_count              = 0;
        m_max_degree         = 0;
        m_debug              = false;
        m_debug_remove_min   = false;
        m_debug_decrease_key = false;
    }
    
    ~FibonacciHeap() { /* TODO: remove all nodes */ }

    bool empty() const {return m_count == 0;}

    PNode minimum() const 
    { 
        if (!m_root_with_min_key)
            throw string("no minimum element");
        return m_root_with_min_key;
    }

    void print_roots(ostream& out) const 
    {
        out << "m_max_degree=" << m_max_degree << "  m_count=" << m_count << "  roots=";
        if (m_root_with_min_key)
            m_root_with_min_key->print_all(out);
        else
            out << endl;
    }

    void merge(const FibonacciHeap& other) 
    {  // Fibonacci-Heap-Union
        m_root_with_min_key->insert(other.m_root_with_min_key);
        if (!m_root_with_min_key || 
            (other.m_root_with_min_key &&
             other.m_root_with_min_key->key() < m_root_with_min_key->key()))
            this->m_root_with_min_key = other.m_root_with_min_key;
        m_count += other.m_count;
    }
	
    PNode insert (Data d, Key k) 
    {
        if (m_debug) cout << "insert " << d << ":" << k << endl;
        m_count++;
        // create a new tree with a single m_key:
        return insert_node(new FibonacciHeapNode<Data, Key>(d,k));
    }


    void remove_minimum() 
    {  // Fibonacci-Heap-Extract-Min, CONSOLIDATE
        if (!m_root_with_min_key)
            throw string("trying to remove from an empty heap");

        if (m_debug) cout << "remove_minimum" << endl;
        m_count--;

        /// Phase 1: Make all the removed root's children new roots:
        // Make all children of root new roots:
        if (m_root_with_min_key->m_child) 
        {
            if (m_debug_remove_min) 
            {
                cout << "  root's children: "; 
                m_root_with_min_key->m_child->print_all(cout);
            }
            PNode c = m_root_with_min_key->m_child;
            do
            {
                c->m_parent = NULL;
                c = c->m_next;
            }
            while (c != m_root_with_min_key->m_child);
            
            m_root_with_min_key->m_child = NULL; // removed all children
            m_root_with_min_key->insert(c);
        }

        if (m_debug_remove_min) 
        {
            cout << "  roots after inserting children: "; 
            print_roots(cout);
        }
		

        /// Phase 2-a: handle the case where we delete the last m_key:
        if (m_root_with_min_key->m_next == m_root_with_min_key) 
        {
            if (m_debug_remove_min) cout << "  removed the last" << endl;
            if (m_count != 0)
                throw string ("Internal error: should have 0 keys");
            m_root_with_min_key = NULL;
            return;
        }

        /// Phase 2: merge roots with the same degree:
        vector<PNode> degree_roots (m_max_degree + 1); // make room for a new degree
        fill (degree_roots.begin(), degree_roots.end(), (PNode)NULL);
        m_max_degree = 0;
        PNode current_pointer = m_root_with_min_key->m_next;
        uint current_degree;
        do 
        {
            current_degree = current_pointer->m_degree;
            if (m_debug_remove_min) 
            {
                cout << "  roots starting from current_pointer: "; 
                current_pointer->print_all(cout);
                cout << "  checking root " 
                     << *current_pointer 
                     << " with degree " 
                     << current_degree << endl;
            }

            PNode current = current_pointer;
            current_pointer = current_pointer->m_next;
            while (degree_roots[current_degree]) 
            { // merge the two roots with the same degree:
                PNode other = degree_roots[current_degree]; // another root with the same degree
                if (current->key() > other->key())
                    swap(other,current); 
                // now current->key() <= other->key() - make other a child of current:
                other->remove(); // remove from list of roots
                current->add_child(other);
                if (m_debug_remove_min) cout << "  added " << *other << " as child of " << *current << endl;
                degree_roots[current_degree] = NULL;
                current_degree++;
                if (current_degree >= degree_roots.size())
                    degree_roots.push_back((PNode)NULL);
            }
            // keep the current root as the first of its degree in the degrees array:
            degree_roots[current_degree] = current;

        } 
        while (current_pointer != m_root_with_min_key);

        /// Phase 3: remove the current root, and calcualte the new m_root_with_min_key:
        delete m_root_with_min_key;
        m_root_with_min_key = NULL;

        uint new_max_degree = 0;
        for (uint d = 0; d < degree_roots.size(); ++d) 
        {
            if (m_debug_remove_min) cout << "  degree " << d << ": ";
            if (degree_roots[d]) 
            {
                if (m_debug_remove_min) cout << " " << *degree_roots[d] << endl;
                degree_roots[d]->m_next = degree_roots[d]->m_previous = degree_roots[d];
                insert_node(degree_roots[d]);
                if (d>new_max_degree)
                    new_max_degree = d;		
            } 
            else 
            {
                if (m_debug_remove_min) cout << "  no node" << endl;
            }
        }
        m_max_degree = new_max_degree;
    }
	
    void decrease_key(PNode node, Key new_key) 
    {
        if (new_key >= node->m_key)
            throw string("Trying to decrease key to a greater key");

        if (m_debug) cout << "decrease key of " << *node << " to " << new_key << endl;
        // Update the key and possibly the min key:
        node->m_key = new_key;

        // Check if the new key violates the heap invariant:
        PNode parent = node->m_parent;
        if (!parent) 
        { // root node - just make sure the minimum is correct
            if (new_key < m_root_with_min_key->key())
                m_root_with_min_key = node;
            return; // heap invariant not violated - nothing more to do
        } 
        else if (parent->key() <= new_key) 
        {
            return; // heap invariant not violated - nothing more to do
        }

        for(;;) 
        {
            parent->remove_child(node);
            insert_node(node);
            if (m_debug_decrease_key) 
            {
                cout << "  removed " << *node << " as child of " << *parent << endl;
                cout << "  roots after remove: "; 
                m_root_with_min_key->print_all(cout);
            }

            if (!parent->m_parent) 
            { // parent is a root - nothing more to do
                break;
            } 
            else if (!parent->m_mark) 
            {  // parent is not a root and is not marked - just mark it
                parent->m_mark = true;
                break;
            } 
            else 
            {
                node = parent;
                parent = parent->m_parent;
                continue;
            }
        };
    }

    void remove(PNode node, Key minus_infinity) 
    {
        if (minus_infinity >= minimum()->key())
            throw string("2nd argument to remove must be a key that is smaller than all other keys");
        decrease_key(node, minus_infinity);
        remove_minimum();
    }

};  // FibonacciHeap


