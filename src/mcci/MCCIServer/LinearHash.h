
#pragma once

#include <string>
#include <map>

using namespace std;


/* A table of prime numbers, each less than a power of 2 */
static unsigned int LINEAR_HASH_TABLE_PRIMES[] = {
    1,
    2,
    4 - 1,
    8 - 1,
    16 - 3,
    32 - 1,
    64 - 3,
    128 - 1,
    256 - 5,
    512 - 3,
    1024 - 3,
    2048 - 9,
    4096 - 3,
    8192 - 1,
    16384 - 3,
    32768 - 19,
    65536 - 15,
};


static unsigned int find_good_linear_hash_table_size(unsigned int desired_size)
{
    unsigned int i;
    if (desired_size < 2) return 1;
    for (i = 1; i < 16 && LINEAR_HASH_TABLE_PRIMES[i] <= desired_size; ++i);

    return LINEAR_HASH_TABLE_PRIMES[i-1];
}




/**
   A simple hash table that uses f(i) = i for the hash function (so does Sun's hash table implementation)

   This hash table is designed for speed and for integer keys (key should be some variant of int/long/etc)

   It is assumed that contiguous blocks of integer keys will be hashed.
   
 */
template <typename Key, typename Data> class LinearHash
{

  protected:
    // to deal with collisions, we use a map (implemented as a tree usually). initialize an array of trees
    map <Key, Data>* m_container;
    
    unsigned int m_size;
    
  public:

    LinearHash()
    {
        this->resize(1);
    };
    

    LinearHash(unsigned int size)
    {
        this->m_size = 0;
        this->resize(size);
    };

    
    ~LinearHash()
    {
        delete[] this->m_container; // TODO: check proper delete syntax
    };


    //resize, destructively
    void resize(unsigned int size)
    {
        if (!size)
        {
            throw string("Tried to set hash size to 0");
        }
        
        if (this->m_size) delete[] this->m_container;

        this->m_size = size;
        this->m_container = new map<Key, Data>[this->m_size]();
        
    }

    
    // return the size of the hash table
    void get_size() const
    {
        return this->m_size;
    }

    
    // return the number of elements in the hash table
    unsigned int count() const
    {
        unsigned int sum = 0;

        for (unsigned int i = 0; i < this->m_size; ++i)
            sum += this->m_container[i].size();

        return sum;
    }

    
    // get the maximum key collisions on any given bucket
    unsigned int max_collisions() const
    {
        unsigned int max = 0;

        for (unsigned int i = 0; i < this->m_size; ++i)
            if (max < this->m_container[i].size())
                max = this->m_container[i].size();

        return max;
    }

    
    // explicitly insert an element into the hash
    void insert(Key k, Data d)
    {
        this->m_container[k % this->m_size][k] = d;
    }

    
    // explicitly remove a key from the hash
    void remove(Key k)
    {
        this->m_container[k % this->m_size].erase(k);
    }

    
    // check existence of a hashed value
    bool has_key(Key k)
    {
        unsigned int idx = k % this->m_size;
        return this->m_container[idx].end() != this->m_container[idx].find(k);
    }

    
    // array-style access to the hash
    Data& operator[] (Key k)
    {
        return this->m_container[k % this->m_size][k];
    }


    // remove all elements from the hash
    void clear()
    {
        for (int i = 0; i < this->m_size; ++i)
            this->m_container[i].clear();
    }
    
};
