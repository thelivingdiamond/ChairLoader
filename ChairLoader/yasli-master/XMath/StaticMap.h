#pragma once

#include <vector>
#include <algorithm>
#include <functional>
#include "yasli/Assert.h"

namespace yasli { class Archive; }

template<class K, class T, class Cmp=std::less<K>, class A=std::allocator<std::pair<K, T> > > 
class StaticMap 
{
public:
	typedef K key_type;
	typedef T mapped_type;
	
	typedef std::pair<K, T> value_type;
	
	typedef Cmp key_compare;
    typedef A allocator_type;
	
	typedef typename A::reference reference;
    typedef typename A::const_reference const_reference;
	typedef typename A::pointer pointer;
	typedef typename A::const_pointer const_pointer;
	typedef typename A::difference_type difference_type;
	typedef typename A::size_type size_type;

	typedef std::vector<std::pair<K, T>, A> Vec;
	typedef typename Vec::iterator iterator;
	typedef typename Vec::const_iterator const_iterator;

	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	class value_compare : public std::binary_function<value_type, value_type, bool> {
		friend class StaticMap<K, T, Cmp, A>;
	protected:
		Cmp MComp;
		value_compare(Cmp c) : MComp(c) {}
	public:
		bool operator()(const value_type& x, const value_type& y) const {
			return MComp(x.first, y.first);
		}
	};

private:

	Vec	MapVector;
	
	iterator binary_search(iterator first, iterator last, const key_type& key) {
		if (MapVector.empty())
			return MapVector.end();
        iterator vc;
		iterator vb = first;
		iterator ve = last - 1;
		if (key_comp()(key, vb->first)) 
			return vb;
		if (key_comp()(ve->first, key)) 
			return MapVector.end();
		if (vb==ve) {
			if (!key_comp()(vb->first, key))
				return vb;
			return last;
		}
		while ((vb + 1)!=ve) {
			vc = vb + ((ve - vb + 1) >> 1); 
			if (key_comp()(vc->first, key)) 
				vb = vc;
			else 
				ve = vc;
		}
		if ((!key_comp()(vb->first, key))&&(!key_comp()(key, vb->first)))
			return vb;
		else 
			return ve;
	}

	const_iterator binary_search(const_iterator first, const_iterator last, const key_type& key) const {
		if (MapVector.empty()) 
			return MapVector.end();
        const_iterator vc;
		const_iterator vb = first;
		const_iterator ve = last - 1;
		if (key_comp()(key, vb->first)) 
			return vb;
		if (key_comp()(ve->first, key)) 
			return MapVector.end();
		if (vb==ve) {
			if (!key_comp()(vb->first, key))
				return vb;
			return last;
		}
		while ((vb + 1)!=ve) {
			vc = vb + ((ve - vb + 1) >> 1); 
			if (key_comp()(vc->first, key)) 
				vb = vc;
			else 
				ve = vc;
		}
		if ((!key_comp()(vb->first, key))&&(!key_comp()(key, vb->first)))
			return vb;
		else 
			return ve;
	}

	int lock_;
	
public:
	StaticMap() : lock_(0) {}
	explicit StaticMap(const key_compare& comp, const allocator_type& a) : MapVector(a), lock_(0) {}

	StaticMap(const_iterator first, const_iterator last) : MapVector(allocator_type()), lock_(0) {
		insert(first, last);
	}

    StaticMap(const_iterator first, const_iterator last, const key_compare& comp, const allocator_type& a) : MapVector(a), lock_(0) {
		insert(first, last); 
	}

	StaticMap(const StaticMap& x) : MapVector(x.MapVector), lock_(0) {}
    StaticMap& operator=(const StaticMap& x) {
		MapVector = x.MapVector;
		return *this; 
	}
	
	key_compare key_comp() const { return key_compare(); }
	value_compare value_comp() const { return value_compare(key_comp()); }

	iterator begin() { return MapVector.begin(); }
	const_iterator begin( ) const { return MapVector.begin(); }
	
	iterator end() { return MapVector.end(); }
	const_iterator end() const { return MapVector.end(); } 

	reverse_iterator rbegin() { return MapVector.rbegin(); }
	const_reverse_iterator rbegin() const { return MapVector.rbegin(); }

	reverse_iterator rend() { return MapVector.rend(); }
	const_reverse_iterator rend() const { return MapVector.rend(); }

	void clear() { MapVector.clear(); }
	bool empty() const { return MapVector.empty(); }
	size_type size() const { return MapVector.size(); }

	std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const {
		const_iterator vb = binary_search(MapVector.begin(), MapVector.end(), key);
		const_iterator ve = vb;
		while(ve != MapVector.end() && ((!key_comp()(ve->first, key))&&(!key_comp()(key, ve->first)))) { ve++; }
		return std::make_pair(vb, ve);
	}

	std::pair<iterator, iterator> equal_range (const key_type& key) {
		iterator vb = binary_search(MapVector.begin(), MapVector.end(), key);
		iterator ve = vb;
		while (ve != MapVector.end() && ( (!key_comp()(ve->first, key))&&(!key_comp()(key, ve->first))) ) { ve++; }
		return std::make_pair(vb, ve);
	}

	size_type count(const key_type& key) const {
		if (MapVector.empty()) 
			return 0;
		const_iterator vlb = binary_search(MapVector.begin(), MapVector.end(), key);
		size_type sz = 0;
		while ((!key_comp()(vlb->first, key))&&(!key_comp()(key, vlb->first))) { sz++; vlb++; }
		return sz;
	}

	std::pair<iterator, bool> insert(const value_type& x) {
		iterator vend = MapVector.end();
		iterator vi = binary_search(MapVector.begin(), vend, x.first);
		if (vi!=vend) {
			if ((value_comp()(*vi, x))||(value_comp()(x, *vi)))
				vi = MapVector.insert(vi,x);
			else 
				return std::make_pair(vi, false);		
		}
		else {
			MapVector.push_back(x);
			vi = MapVector.end()-1;
		}
		return std::make_pair(vi, true);
	}

	iterator insert(iterator where, const value_type& x) {
		iterator vend = MapVector.end();
		iterator vi = binary_search(where, vend, x.first);
		if (vi!=vend) {
			if ((value_comp()(*vi, x))||(value_comp()(x, *vi))) {
				if (vi!=where) 
					vi = MapVector.insert(vi,x);
				else 
					return insert(x).first;
			}
			else 
				return vi;		
		}
		else 
			return insert(x).first;
		return vi;
	}

	void insert(const_iterator first, const_iterator last) {
		for (const_iterator p = first;p!=last;++p) 
			{ insert(*p); } 
    }

	iterator erase(iterator where)	{ return MapVector.erase(where); }
	iterator erase(iterator first, iterator last) { return MapVector.erase(first, last); }
	size_type erase(const key_type& key){
		if(MapVector.empty())
			return 0;
		std::pair<iterator, iterator> rng = equal_range(key);
		if(rng.first == MapVector.end())
			return 0;
		if ((!key_compare()(rng.first->first, key))&&(!key_compare()(key, rng.first->first))) {
			if (rng.first==rng.second) {
				MapVector.erase(rng.first); 
				return 1;
			}
			else {
				size_type count = rng.second - rng.first;
				MapVector.erase(rng.first, rng.second);
				return count;
			}
		}
		else 
			return 0;
	}

	iterator find(const key_type& key) {
		iterator vend = MapVector.end();
		if (MapVector.empty()) 
			return vend;
		iterator vi = binary_search(MapVector.begin(), vend, key);
		if (vi!=vend) {
			if ((!key_comp()(vi->first, key))&&(!key_comp()(key, vi->first))) 
				return vi;		
		}
		else {
			if ((!key_comp()((vi - 1)->first, key))&&(!key_comp()(key, (vi - 1)->first))) 
				return (vi - 1);
		}
		return vend;
	}

	const_iterator find(const key_type& key) const {
		const_iterator vend = MapVector.end();
		if (MapVector.empty()) 
			return vend;
		const_iterator vi = binary_search(MapVector.begin(), vend, key);
		if (vi!=vend) {
			if ((!key_comp()(vi->first, key))&&(!key_comp()(key, vi->first))) 
				return vi;		
		}
		else {
			if ((!key_comp()((vi - 1)->first, key))&&(!key_comp()(key, (vi - 1)->first))) 
				return (vi - 1);
		}
		return vend;
	}

	allocator_type get_allocator( ) const { return MapVector.get_allocator(); }
	
	iterator lower_bound(const key_type& key) {
		return binary_search(MapVector.begin(), MapVector.end(), key);
	}

	const_iterator lower_bound(const key_type& key) const {
		return binary_search(MapVector.begin(), MapVector.end(), key);	
	}

	iterator upper_bound(const key_type& key) {
		iterator ve = MapVector.end();
		iterator vs = binary_search(MapVector.begin(), ve, key);
		if(vs != ve){
			while ((!key_comp()(vs->first, key))&&(!key_comp()(key, vs->first))) { vs++; }
			return vs;
		}      
		return ve;
	}

	const_iterator upper_bound(const key_type& key) const {
		const_iterator ve = MapVector.end();
		const_iterator vs = binary_search(MapVector.begin(), ve, key);
		if(vs != ve) {
			while ((!key_comp()(vs->first, key))&&(!key_comp()(key, vs->first))) { vs++; }
			return vs;
		}      
		return ve;
	}

	size_type max_size( ) const { return MapVector.max_size(); }
	
	void swap(StaticMap& right) { MapVector.swap(right.MapVector); }

	mapped_type& operator[](const key_type& key) {
		iterator vi = binary_search(MapVector.begin(), MapVector.end(), key);
		if (vi==MapVector.end()||(key_comp()(key, vi->first))){
			YASLI_ASSERT(!lock_ && "StaticMap is locked");
			vi = MapVector.insert(vi, value_type(key, mapped_type()));
		}
		return (*vi).second;
	}

	const mapped_type& operator[](const key_type& key) const {
		const_iterator vi = binary_search(MapVector.begin(), MapVector.end(), key);
		YASLI_ASSERT(!(vi==MapVector.end() || (key_comp()(key, vi->first))) && "Не найден элемент в const-operator[]");
		return (*vi).second;
	}

	bool exists(const key_type& key) const {
		const_iterator vi = binary_search(MapVector.begin(), MapVector.end(), key);
		if (vi==MapVector.end() || (key_comp()(key, vi->first)))
			return false;
		return true;
	}

	const mapped_type& get(const key_type& key) const {
		static mapped_type def;
		const_iterator it = find(key); 
		return it != end() ? it->second : def;
	}

	void sort() { std::sort(MapVector.begin(), MapVector.end(), value_comp()); }
	void reserve(size_type size) { MapVector.reserve(size);	}

	bool operator==(const StaticMap& map) const { return MapVector == map.MapVector; }

	template<class K1, class T1, class Cmp1, class A1> 
	friend bool serialize(yasli::Archive& ar, StaticMap<K1, T1, Cmp1, A1>& map, const char* name, const char* nameAlt);

	Vec& vector() { return MapVector; }
	const Vec& vector() const { return MapVector; }

	struct Lock {
		StaticMap& map;
		Lock(StaticMap& map) : map(map) { ++map.lock_; }
		Lock(const Lock& loc) : map(loc.map) { ++map.lock_; }
		~Lock() { --map.lock_; }
	};
	Lock lock() { return Lock(*this); }

	class Ref {
		T* t_;
		StaticMap* map_;
	public:
		Ref(StaticMap& map, const K& key) : map_(&map), t_(&map[key]) { ++map_->lock_; }
		explicit Ref(T* t) : map_(0), t_(t) {}
		Ref& operator=(const Ref& ref) { t_ = ref.t_; map_ = ref.map_; if(map_) ++map_->lock_; return *this; }
		Ref(const Ref& ref) { *this = ref; }
		~Ref() { if(map_) --map_->lock_; }
		operator T *() const  { return t_; }
		T *operator->() const { return t_; }
	};
};

template<class K, class T, class Cmp, class A> 
bool serialize(yasli::Archive& ar, StaticMap<K, T, Cmp, A>& map, const char* name, const char* nameAlt) {
	bool nodeExists = ar(map.MapVector, name, nameAlt);
	if(!ar.isEdit()) 
		map.sort();
	return nodeExists;
}

