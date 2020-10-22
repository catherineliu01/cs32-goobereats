// ExpandableHashMap.h

// Skeleton for the ExpandableHashMap class template.  You must implement the first six
// member functions.

template<typename KeyType, typename ValueType>
class ExpandableHashMap
{
public:
	ExpandableHashMap(double maximumLoadFactor = 0.5); // constructor
    ~ExpandableHashMap();// destructor; deletes all of the items in the hashmap
    void reset(); // resets the hashmap back to 8 buckets, deletes all items
    int size() const; // return the number of associations in the hashmap
    
    // The associate method associates one item (key) with another (value).
    // If no association currently exists with that key, this method inserts
    // a new association into the hashmap with that key/value pair. If there is
    // already an association with that key in the hashmap, then the item
    // associated with that key is replaced by the second parameter (value).
    // Thus, the hashmap must contain no duplicate keys.
	void associate(const KeyType& key, const ValueType& value);

    // If no association exists with the given key, return nullptr; otherwise,
    // return a pointer to the value associated with that key. This pointer can be
    // used to examine that value, and if the hashmap is allowed to be modified, to
    // modify that value directly within the map (the second overload enables
    // this). Using a little C++ magic, we have implemented it in terms of the
    // first overload, which you must implement.
    
	  // for a map that can't be modified, return a pointer to const ValueType
	const ValueType* find(const KeyType& key) const;

	  // for a modifiable map, return a pointer to modifiable ValueType
	ValueType* find(const KeyType& key)
	{
		return const_cast<ValueType*>(const_cast<const ExpandableHashMap*>(this)->find(key));
	}

	  // C++11 syntax for preventing copying and assignment
	ExpandableHashMap(const ExpandableHashMap&) = delete;
	ExpandableHashMap& operator=(const ExpandableHashMap&) = delete;

private:
    struct Node
    {
        KeyType m_key;
        ValueType m_value;
        Node *m_next;
        Node *m_prev;
    };
    unsigned int getBucketNumber(const KeyType& key, int size) const
    {
        unsigned int hasher(const KeyType& k); // prototype
        unsigned int h = hasher(key);
        return h % size;
    }
    std::vector<std::list<Node>*> m_buckets;
    double m_maxLoadFactor;
};

template<typename KeyType, typename ValueType>
inline
ExpandableHashMap<KeyType, ValueType>::ExpandableHashMap(double maximumLoadFactor)
{
    m_maxLoadFactor = maximumLoadFactor;
    m_buckets = std::vector<std::list<Node>*> (8);
    for (int i = 0; i < m_buckets.size(); i++)
    {
        m_buckets[i] = new std::list<Node>;
    }
}

template<typename KeyType, typename ValueType>
inline
ExpandableHashMap<KeyType, ValueType>::~ExpandableHashMap()
{
    for (int i = 0; i < m_buckets.size(); i++)
    {
        delete m_buckets[i];
    }
}

template<typename KeyType, typename ValueType>
inline
void ExpandableHashMap<KeyType, ValueType>::reset()
{
    for (int i = 0; i < m_buckets.size(); i++)
    {
        delete m_buckets[i];
    }
    m_buckets = std::vector<std::list<Node>*> (8);
    for (int i = 0; i < m_buckets.size(); i++)
    {
        m_buckets[i] = new std::list<Node>;
    }
}

template<typename KeyType, typename ValueType>
inline
int ExpandableHashMap<KeyType, ValueType>::size() const
{
    int total = 0;
    for (int i = 0; i < m_buckets.size(); i++)
    {
        total += m_buckets[i]->size();
    }
    return total;
}

// The associate method associates one item (key) with another (value).
// If no association currently exists with that key, this method inserts
// a new association into the hashmap with that key/value pair. If there is
// already an association with that key in the hashmap, then the item
// associated with that key is replaced by the second parameter (value).
// Thus, the hashmap must contain no duplicate keys.
template<typename KeyType, typename ValueType>
inline
void ExpandableHashMap<KeyType, ValueType>::associate(const KeyType& key, const ValueType& value)
{
    ValueType* findResult = find(key);
    if (findResult != nullptr)
    {
        *findResult = value;
    }
    else
    {
        unsigned int bucketNumber = getBucketNumber(key, m_buckets.size());
        Node keyValuePair;
        keyValuePair.m_key = key;
        keyValuePair.m_value = value;
        m_buckets[bucketNumber]->push_back(keyValuePair);
    }
    
    // rehash
//    cerr << "associations: " << size() << endl;
    double loadFactor = static_cast<double>(size()) / m_buckets.size();
//    cerr << "load factor " << loadFactor << endl;
    if (loadFactor > m_maxLoadFactor)
    {
//        cerr << "must rehash" << endl;
        std::vector<std::list<Node>*> newBuckets = std::vector<std::list<Node>*> (2 * m_buckets.size());
        for (int i = 0; i < newBuckets.size(); i++)
        {
            newBuckets[i] = new std::list<Node>;
        }
        
        for (int i = 0; i < m_buckets.size(); i++)
        {
            typename std::list<Node>::iterator it = m_buckets[i]->begin();
            while (it != m_buckets[i]->end())
            {
                list<Node>* newBucket = newBuckets[getBucketNumber((*it).m_key, newBuckets.size())];
                newBucket->splice(newBucket->begin(), *m_buckets[i], it++);
            }
        }
        m_buckets.resize(2 * m_buckets.size());
        m_buckets = newBuckets;
//        cerr << "rehashed" << endl;
//        cerr << "new size: " << m_buckets.size() << endl;
//        cerr << "new associations: " << size() << endl;
//        cerr << "new load factor: " << static_cast<double>(size()) / m_buckets.size() << endl;
    }
}

// for a modifiable map, return a pointer to modifiable ValueType
template<typename KeyType, typename ValueType>
inline
const ValueType* ExpandableHashMap<KeyType, ValueType>::find(const KeyType& key) const
{
    unsigned int bucketNumber = getBucketNumber(key, m_buckets.size());
    std::list<Node>* bucketList = m_buckets[bucketNumber];
    typename std::list<Node>::iterator it = bucketList->begin();
    while (it != bucketList->end())
    {
        if (it->m_key == key)
        {
//            cerr << "Found key!" << endl;
            return &(it->m_value);
        }
        it++;
    }
//    cerr << "Didn't find key!" << endl;
    return nullptr;
}

