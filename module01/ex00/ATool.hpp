#ifndef ATOOL_HPP
#define ATOOL_HPP

class Worker;

class ATool
{
protected:
	int numberOfUses;
	Worker* currentOwner;

private:
	ATool(const ATool& other);
	ATool& operator=(const ATool& other);

public:
	ATool();
	virtual ~ATool();
	
	virtual void use() = 0;
	
	Worker* getOwner() const;
	void setOwner(Worker* owner);
};

#endif