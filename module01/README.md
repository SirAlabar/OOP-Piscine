# The 4 OOP Relationships

## 1. COMPOSITION - Strong ownership, "dies together"

**Example:**
```
Worker HAS Position and Statistic
```

**Characteristics:**
- Position and Statistic are **inside** Worker (member variables)
- When Worker is destroyed → Position and Statistic are destroyed too
- Parts **cannot** exist without the whole
- Implementation: Direct member objects

**Code:**
```cpp
class Worker
{
private:
    Position coordonnee;  // Composition
    Statistic stat;       // Composition
};
```

---

## 2. AGGREGATION - Weak ownership, "survives independently"

**Example:**
```
Worker USES ATool (Shovel/Hammer)
```

**Characteristics:**
- Worker holds **pointer** to ATool
- When Worker is destroyed → ATool survives
- ATool can be transferred between Workers
- Implementation: Pointer member, tool lives outside Worker

**Code:**
```cpp
class Worker
{
private:
    std::vector<ATool*> tools;  // Aggregation - pointers to external objects
};
```

---

## 3. INHERITANCE - "Is-a" relationship

**Example:**
```
Shovel IS-A ATool
Hammer IS-A ATool
```

**Characteristics:**
- Shovel and Hammer **derive** from ATool
- They inherit numberOfUses and currentOwner
- Polymorphism: ATool* can point to Shovel or Hammer
- Implementation: `class Shovel : public ATool`

**Code:**
```cpp
class ATool
{
protected:
    int numberOfUses;
public:
    virtual void use() = 0;
};

class Shovel : public ATool  // Inheritance
{
public:
    void use();
};
```

---

## 4. ASSOCIATION - Mutual awareness, both independent

**Example:**
```
Worker KNOWS Workshop
Workshop KNOWS Worker
```

**Characteristics:**
- Both hold pointers to each other
- Both can exist independently
- Worker can join/leave Workshop freely
- Workshop can add/remove Workers freely
- Implementation: Both have `std::vector<OtherClass*>`

**Code:**
```cpp
class Worker
{
private:
    std::vector<Workshop*> workshops;  // Association
};

class Workshop
{
private:
    std::vector<Worker*> workers;  // Association
};
```

---

## Summary Table

| Relationship | Ownership | Lifetime | Example | Implementation |
|--------------|-----------|----------|---------|----------------|
| **Composition** | Strong | Dies together | Worker HAS Position | Direct member |
| **Aggregation** | Weak | Survives | Worker USES ATool | Pointer member |
| **Inheritance** | N/A | Type hierarchy | Shovel IS-A ATool | `: public` |
| **Association** | None | Independent | Worker ↔ Workshop | Mutual pointers |

---

## Key Differences

- **Composition** = ownership (dies together)
- **Aggregation** = usage (survives)
- **Inheritance** = type hierarchy
- **Association** = mutual reference