#ifndef SINGLETON_HPP
#define SINGLETON_HPP

#include <vector>
#include <string>

template <typename T>
class Singleton
{
private:
	std::vector<T*> _items;

	Singleton() {}
	Singleton(const Singleton&);
	Singleton& operator=(const Singleton&);

public:
	static Singleton& getInstance()
	{
		static Singleton instance;
		return instance;
	}

	void add(T* item)
	{
		if (item)
			_items.push_back(item);
	}

	T* find(const std::string& name)
	{
		for (typename std::vector<T*>::iterator it = _items.begin(); it != _items.end(); ++it)
		{
			if ((*it)->getName() == name)
				return *it;
		}
		return NULL;
	}

	std::vector<T*>& getAll()
	{
		return _items;
	}

	size_t size() const
	{
		return _items.size();
	}
};

#endif