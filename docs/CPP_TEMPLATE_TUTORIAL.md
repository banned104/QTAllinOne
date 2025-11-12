# C++ 模板编程完全指南

## 目录
1. [什么是模板](#什么是模板)
2. [函数模板](#函数模板)
3. [类模板](#类模板)
4. [模板特化](#模板特化)
5. [模板参数](#模板参数)
6. [模板与继承](#模板与继承)
7. [SFINAE 和类型萃取](#sfinae-和类型萃取)
8. [实战示例](#实战示例)
9. [最佳实践](#最佳实践)
10. [常见陷阱](#常见陷阱)

---

## 什么是模板

模板是 C++ 的一种**泛型编程**机制，允许你编写与类型无关的代码。编译器会根据使用的类型自动生成具体的代码。

### 为什么需要模板？

**问题**：假设你要写一个函数，返回两个数中的最大值

```cpp
// 没有模板，需要为每种类型写一个函数
int max(int a, int b) {
    return (a > b) ? a : b;
}

double max(double a, double b) {
    return (a > b) ? a : b;
}

std::string max(std::string a, std::string b) {
    return (a > b) ? a : b;
}
// ... 还有很多类型
```

**解决方案**：使用模板，一次编写，适用所有类型

```cpp
template<typename T>
T max(T a, T b) {
    return (a > b) ? a : b;
}

// 使用
int i = max(10, 20);           // 编译器生成 max<int>
double d = max(3.14, 2.71);    // 编译器生成 max<double>
std::string s = max("hello", "world"); // 编译器生成 max<std::string>
```

---

## 函数模板

### 基本语法

```cpp
template<typename T>  // 或 template<class T>，两者等价
T functionName(T param1, T param2) {
    // 函数体
}
```

### 示例 1：简单的函数模板

```cpp
#include <iostream>

// 交换两个值
template<typename T>
void swap(T& a, T& b) {
    T temp = a;
    a = b;
    b = temp;
}

int main() {
    int x = 10, y = 20;
    swap(x, y);  // 自动推导为 swap<int>
    std::cout << "x=" << x << ", y=" << y << std::endl;  // x=20, y=10
    
    std::string s1 = "hello", s2 = "world";
    swap(s1, s2);  // 自动推导为 swap<std::string>
    std::cout << "s1=" << s1 << ", s2=" << s2 << std::endl;  // s1=world, s2=hello
    
    return 0;
}
```

### 示例 2：多个模板参数

```cpp
template<typename T1, typename T2>
void print(T1 first, T2 second) {
    std::cout << first << " " << second << std::endl;
}

// 使用
print(10, "hello");      // T1=int, T2=const char*
print(3.14, true);       // T1=double, T2=bool
```

### 示例 3：返回类型推导

```cpp
// C++11 之前
template<typename T1, typename T2>
auto add(T1 a, T2 b) -> decltype(a + b) {
    return a + b;
}

// C++14 及以后
template<typename T1, typename T2>
auto add(T1 a, T2 b) {
    return a + b;  // 自动推导返回类型
}

// 使用
auto result1 = add(10, 20);      // int
auto result2 = add(3.14, 2);     // double
auto result3 = add(1.5f, 2.5);   // double
```

---

## 类模板

### 基本语法

```cpp
template<typename T>
class ClassName {
public:
    T member;
    
    ClassName(T value) : member(value) {}
    
    T getValue() const { return member; }
    void setValue(T value) { member = value; }
};
```

### 示例 1：简单的容器类

```cpp
template<typename T>
class Box {
private:
    T value;
    
public:
    Box(T val) : value(val) {}
    
    T get() const { return value; }
    void set(T val) { value = val; }
    
    void print() const {
        std::cout << "Box contains: " << value << std::endl;
    }
};

// 使用
Box<int> intBox(42);
intBox.print();  // Box contains: 42

Box<std::string> strBox("Hello");
strBox.print();  // Box contains: Hello

Box<double> doubleBox(3.14);
doubleBox.print();  // Box contains: 3.14
```

### 示例 2：动态数组类

```cpp
template<typename T>
class DynamicArray {
private:
    T* data;
    size_t size;
    size_t capacity;
    
public:
    DynamicArray() : data(nullptr), size(0), capacity(0) {}
    
    ~DynamicArray() {
        delete[] data;
    }
    
    void push_back(const T& value) {
        if (size >= capacity) {
            capacity = (capacity == 0) ? 1 : capacity * 2;
            T* newData = new T[capacity];
            for (size_t i = 0; i < size; ++i) {
                newData[i] = data[i];
            }
            delete[] data;
            data = newData;
        }
        data[size++] = value;
    }
    
    T& operator[](size_t index) {
        return data[index];
    }
    
    size_t getSize() const { return size; }
};

// 使用
DynamicArray<int> numbers;
numbers.push_back(10);
numbers.push_back(20);
numbers.push_back(30);

for (size_t i = 0; i < numbers.getSize(); ++i) {
    std::cout << numbers[i] << " ";  // 10 20 30
}
```

### 示例 3：类模板的成员函数定义

```cpp
// 在类外定义成员函数
template<typename T>
class Calculator {
public:
    T add(T a, T b);
    T multiply(T a, T b);
};

// 必须在每个函数前加 template<typename T>
template<typename T>
T Calculator<T>::add(T a, T b) {
    return a + b;
}

template<typename T>
T Calculator<T>::multiply(T a, T b) {
    return a * b;
}

// 使用
Calculator<int> intCalc;
std::cout << intCalc.add(5, 3) << std::endl;  // 8

Calculator<double> doubleCalc;
std::cout << doubleCalc.multiply(2.5, 4.0) << std::endl;  // 10.0
```

---

## 模板特化

### 全特化（Full Specialization）

为特定类型提供完全不同的实现

```cpp
// 通用模板
template<typename T>
class Printer {
public:
    void print(T value) {
        std::cout << "Value: " << value << std::endl;
    }
};

// 为 bool 类型特化
template<>
class Printer<bool> {
public:
    void print(bool value) {
        std::cout << "Boolean: " << (value ? "true" : "false") << std::endl;
    }
};

// 使用
Printer<int> intPrinter;
intPrinter.print(42);  // Value: 42

Printer<bool> boolPrinter;
boolPrinter.print(true);  // Boolean: true
```

### 偏特化（Partial Specialization）

只对部分模板参数特化

```cpp
// 通用模板
template<typename T1, typename T2>
class Pair {
public:
    T1 first;
    T2 second;
    
    void print() {
        std::cout << "Generic Pair: " << first << ", " << second << std::endl;
    }
};

// 偏特化：当两个类型相同时
template<typename T>
class Pair<T, T> {
public:
    T first;
    T second;
    
    void print() {
        std::cout << "Same Type Pair: " << first << ", " << second << std::endl;
    }
};

// 偏特化：当第二个类型是指针时
template<typename T1, typename T2>
class Pair<T1, T2*> {
public:
    T1 first;
    T2* second;
    
    void print() {
        std::cout << "Pointer Pair: " << first << ", " << *second << std::endl;
    }
};

// 使用
Pair<int, double> p1;
p1.first = 1;
p1.second = 2.5;
p1.print();  // Generic Pair: 1, 2.5

Pair<int, int> p2;
p2.first = 10;
p2.second = 20;
p2.print();  // Same Type Pair: 10, 20

int value = 100;
Pair<std::string, int> p3;
p3.first = "Value";
p3.second = &value;
p3.print();  // Pointer Pair: Value, 100
```

---

## 模板参数

### 类型参数（Type Parameters）

```cpp
template<typename T>
class Container {
    T data;
};
```

### 非类型参数（Non-Type Parameters）

可以是整数、枚举、指针等

```cpp
template<typename T, int Size>
class FixedArray {
private:
    T data[Size];
    
public:
    int size() const { return Size; }
    
    T& operator[](int index) {
        return data[index];
    }
};

// 使用
FixedArray<int, 5> arr1;  // 大小为 5 的 int 数组
FixedArray<double, 10> arr2;  // 大小为 10 的 double 数组

std::cout << arr1.size() << std::endl;  // 5
std::cout << arr2.size() << std::endl;  // 10
```

### 模板模板参数（Template Template Parameters）

```cpp
template<typename T, template<typename> class Container>
class Stack {
private:
    Container<T> data;
    
public:
    void push(const T& value) {
        data.push_back(value);
    }
    
    T pop() {
        T value = data.back();
        data.pop_back();
        return value;
    }
};

// 使用
Stack<int, std::vector> intStack;
Stack<std::string, std::list> strStack;
```

### 默认模板参数

```cpp
template<typename T, typename Container = std::vector<T>>
class MyStack {
private:
    Container data;
    
public:
    void push(const T& value) {
        data.push_back(value);
    }
};

// 使用
MyStack<int> stack1;  // 使用默认的 std::vector<int>
MyStack<int, std::deque<int>> stack2;  // 使用 std::deque<int>
```

---

## 模板与继承

### 模板类继承普通类

```cpp
class Base {
public:
    void baseMethod() {
        std::cout << "Base method" << std::endl;
    }
};

template<typename T>
class Derived : public Base {
private:
    T value;
    
public:
    Derived(T val) : value(val) {}
    
    void derivedMethod() {
        std::cout << "Derived method with value: " << value << std::endl;
    }
};

// 使用
Derived<int> obj(42);
obj.baseMethod();     // Base method
obj.derivedMethod();  // Derived method with value: 42
```

### 模板类继承模板类

```cpp
template<typename T>
class Base {
protected:
    T value;
    
public:
    Base(T val) : value(val) {}
    
    T getValue() const { return value; }
};

template<typename T>
class Derived : public Base<T> {
public:
    Derived(T val) : Base<T>(val) {}
    
    void print() {
        // 注意：需要使用 this-> 或 Base<T>::
        std::cout << "Value: " << this->value << std::endl;
    }
};

// 使用
Derived<int> obj(42);
obj.print();  // Value: 42
```

---

## SFINAE 和类型萃取

SFINAE = Substitution Failure Is Not An Error（替换失败不是错误）

### 基本 SFINAE

```cpp
#include <type_traits>

// 只对整数类型启用
template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
processValue(T value) {
    return value * 2;
}

// 只对浮点类型启用
template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
processValue(T value) {
    return value * 3.14;
}

// 使用
int i = processValue(10);      // 调用整数版本，返回 20
double d = processValue(2.0);  // 调用浮点版本，返回 6.28
```

### C++17 的 if constexpr

```cpp
template<typename T>
T processValue(T value) {
    if constexpr (std::is_integral<T>::value) {
        return value * 2;
    } else if constexpr (std::is_floating_point<T>::value) {
        return value * 3.14;
    } else {
        return value;
    }
}
```

### 类型萃取（Type Traits）

```cpp
#include <type_traits>

template<typename T>
void checkType() {
    if (std::is_integral<T>::value) {
        std::cout << "T is an integral type" << std::endl;
    }
    
    if (std::is_pointer<T>::value) {
        std::cout << "T is a pointer type" << std::endl;
    }
    
    if (std::is_class<T>::value) {
        std::cout << "T is a class type" << std::endl;
    }
}

// 使用
checkType<int>();        // T is an integral type
checkType<int*>();       // T is a pointer type
checkType<std::string>();  // T is a class type
```

---

## 实战示例

### 示例 1：智能指针（简化版）

```cpp
template<typename T>
class UniquePtr {
private:
    T* ptr;
    
public:
    // 构造函数
    explicit UniquePtr(T* p = nullptr) : ptr(p) {}
    
    // 禁止拷贝
    UniquePtr(const UniquePtr&) = delete;
    UniquePtr& operator=(const UniquePtr&) = delete;
    
    // 允许移动
    UniquePtr(UniquePtr&& other) noexcept : ptr(other.ptr) {
        other.ptr = nullptr;
    }
    
    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this != &other) {
            delete ptr;
            ptr = other.ptr;
            other.ptr = nullptr;
        }
        return *this;
    }
    
    // 析构函数
    ~UniquePtr() {
        delete ptr;
    }
    
    // 解引用
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    
    // 获取原始指针
    T* get() const { return ptr; }
    
    // 释放所有权
    T* release() {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }
    
    // 重置
    void reset(T* p = nullptr) {
        delete ptr;
        ptr = p;
    }
};

// 使用
UniquePtr<int> ptr1(new int(42));
std::cout << *ptr1 << std::endl;  // 42

UniquePtr<std::string> ptr2(new std::string("Hello"));
std::cout << ptr2->length() << std::endl;  // 5
```

### 示例 2：观察者模式（模板版）

```cpp
template<typename EventType>
class Observer {
public:
    virtual ~Observer() = default;
    virtual void onEvent(const EventType& event) = 0;
};

template<typename EventType>
class Subject {
private:
    std::vector<Observer<EventType>*> observers;
    
public:
    void attach(Observer<EventType>* observer) {
        observers.push_back(observer);
    }
    
    void detach(Observer<EventType>* observer) {
        observers.erase(
            std::remove(observers.begin(), observers.end(), observer),
            observers.end()
        );
    }
    
    void notify(const EventType& event) {
        for (auto* observer : observers) {
            observer->onEvent(event);
        }
    }
};

// 使用
struct MouseEvent {
    int x, y;
    std::string button;
};

class MouseObserver : public Observer<MouseEvent> {
public:
    void onEvent(const MouseEvent& event) override {
        std::cout << "Mouse " << event.button << " at (" 
                  << event.x << ", " << event.y << ")" << std::endl;
    }
};

// 测试
Subject<MouseEvent> mouseSubject;
MouseObserver observer;
mouseSubject.attach(&observer);

MouseEvent event{100, 200, "left"};
mouseSubject.notify(event);  // Mouse left at (100, 200)
```

### 示例 3：类型安全的单位系统

```cpp
template<typename Unit>
class Quantity {
private:
    double value;
    
public:
    explicit Quantity(double v) : value(v) {}
    
    double getValue() const { return value; }
    
    // 同类型可以相加
    Quantity operator+(const Quantity& other) const {
        return Quantity(value + other.value);
    }
    
    // 同类型可以相减
    Quantity operator-(const Quantity& other) const {
        return Quantity(value - other.value);
    }
    
    // 可以乘以标量
    Quantity operator*(double scalar) const {
        return Quantity(value * scalar);
    }
};

// 定义单位类型
struct Meter {};
struct Second {};
struct Kilogram {};

// 类型别名
using Length = Quantity<Meter>;
using Time = Quantity<Second>;
using Mass = Quantity<Kilogram>;

// 使用
Length distance(100.0);  // 100 米
Time duration(10.0);     // 10 秒

Length total = distance + Length(50.0);  // 150 米
// Length wrong = distance + duration;  // 编译错误！不能将米和秒相加

std::cout << "Total distance: " << total.getValue() << " meters" << std::endl;
```

---

## 最佳实践

### 1. 在头文件中实现模板

模板必须在使用点可见，通常在头文件中实现：

```cpp
// MyTemplate.h
template<typename T>
class MyTemplate {
public:
    void method() {
        // 实现
    }
};
```

### 2. 使用 typename 关键字

访问依赖类型时使用 `typename`：

```cpp
template<typename T>
class MyClass {
public:
    typename T::value_type getValue();  // 告诉编译器这是一个类型
};
```

### 3. 使用概念（C++20）

```cpp
#include <concepts>

template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

template<Numeric T>
T add(T a, T b) {
    return a + b;
}

// 只能用于数值类型
int result = add(10, 20);  // OK
// std::string s = add("hello", "world");  // 编译错误
```

### 4. 避免过度使用模板

不要为了使用模板而使用模板。如果普通类或函数就能解决问题，就不要用模板。

### 5. 提供清晰的错误信息

```cpp
template<typename T>
class Container {
    static_assert(std::is_copy_constructible<T>::value,
                  "T must be copy constructible");
    // ...
};
```

---

## 常见陷阱

### 1. 模板代码膨胀

每个模板实例化都会生成新代码，可能导致二进制文件变大。

**解决方案**：使用基类提取公共代码

```cpp
// 非模板基类
class VectorBase {
protected:
    void* data;
    size_t size;
    
    void resize(size_t newSize, size_t elementSize);
};

// 模板派生类
template<typename T>
class Vector : private VectorBase {
public:
    void push_back(const T& value) {
        // 使用基类的 resize
    }
};
```

### 2. 依赖名称查找

```cpp
template<typename T>
class Derived : public Base<T> {
public:
    void method() {
        // 错误：找不到 baseMethod
        // baseMethod();
        
        // 正确：明确指定
        this->baseMethod();
        // 或
        Base<T>::baseMethod();
    }
};
```

### 3. 模板和虚函数

模板成员函数不能是虚函数：

```cpp
class Base {
public:
    // 错误！模板成员函数不能是虚函数
    // template<typename T>
    // virtual void method(T value);
};
```

### 4. 友元函数声明

```cpp
template<typename T>
class MyClass {
    T value;
    
    // 正确的友元函数声明
    friend std::ostream& operator<<(std::ostream& os, const MyClass<T>& obj) {
        os << obj.value;
        return os;
    }
};
```

---

## 总结

### 何时使用模板

✅ **应该使用**：
- 编写容器类（如 vector, list）
- 编写通用算法（如 sort, find）
- 需要类型安全的泛型代码
- 需要编译期优化

❌ **不应该使用**：
- 需要运行时多态（用虚函数）
- 代码逻辑因类型而完全不同（用继承或重载）
- 会导致代码过度复杂
- 编译时间是瓶颈

### 模板 vs 继承/多态

| 特性 | 模板 | 继承/多态 |
|------|------|-----------|
| 绑定时机 | 编译期 | 运行期 |
| 性能 | 更快（内联优化） | 稍慢（虚函数调用） |
| 灵活性 | 编译期确定 | 运行期可变 |
| 代码大小 | 可能更大 | 更小 |
| 类型安全 | 编译期检查 | 运行期检查 |
| 适用场景 | 类型无关的算法 | 行为多态 |

### 学习路径

1. **基础**：函数模板、类模板
2. **进阶**：模板特化、非类型参数
3. **高级**：SFINAE、类型萃取、变参模板
4. **专家**：模板元编程、概念（C++20）

---

## 推荐资源

- **书籍**：
  - 《C++ Templates: The Complete Guide》
  - 《Effective Modern C++》
  
- **在线资源**：
  - cppreference.com
  - learncpp.com

- **实践**：
  - 阅读 STL 源码
  - 实现自己的容器类
  - 参与开源项目
