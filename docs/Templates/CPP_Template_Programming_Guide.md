# C++ 模板编程：从入门到精通

## 目录

1. [模板基础](#1-模板基础)
2. [函数模板](#2-函数模板)
3. [类模板](#3-类模板)
4. [模板特化](#4-模板特化)
5. [模板参数](#5-模板参数)
6. [模板元编程](#6-模板元编程)
7. [SFINAE 与类型萃取](#7-sfinae-与类型萃取)
8. [变参模板](#8-变参模板)
9. [现代 C++ 模板特性](#9-现代-c-模板特性)
10. [实战案例](#10-实战案例)

---

## 1. 模板基础

### 1.1 什么是模板？

模板是 C++ 的**泛型编程**机制，允许编写**与类型无关**的代码。编译器根据实际使用的类型生成具体的代码。

```cpp
// 不使用模板：需要为每种类型写一个函数
int max_int(int a, int b) { return a > b ? a : b; }
double max_double(double a, double b) { return a > b ? a : b; }

// 使用模板：一个函数处理所有类型
template<typename T>
T max_value(T a, T b) {
    return a > b ? a : b;
}

int main() {
    max_value(10, 20);        // T = int
    max_value(3.14, 2.71);    // T = double
    max_value('a', 'z');      // T = char
}
```

### 1.2 模板的编译过程

```
源代码阶段           实例化阶段            编译阶段
────────────        ────────────        ────────────
template<T>   →    max_value<int>   →   二进制代码
T max_value   →    max_value<double> →  二进制代码
              →    max_value<char>   →  二进制代码
```

**关键特性：**
- **编译期展开**：模板在编译时生成代码，不是运行时
- **类型安全**：编译器检查类型匹配
- **零开销抽象**：生成的代码与手写的专用代码性能相同

---

## 2. 函数模板

### 2.1 基本语法

```cpp
// 语法格式
template<typename T>        // 模板声明
返回类型 函数名(参数列表) {   // 函数定义
    // 函数体
}

// 示例
template<typename T>
T add(T a, T b) {
    return a + b;
}
```

**使用方式：**

```cpp
// 1. 显式指定类型
add<int>(10, 20);           // T = int

// 2. 类型推导（推荐）
add(10, 20);                // 编译器自动推导 T = int
add(1.5, 2.5);              // T = double
```

### 2.2 多参数模板

```cpp
// 两个不同类型的模板参数
template<typename T1, typename T2>
auto multiply(T1 a, T2 b) -> decltype(a * b) {
    return a * b;
}

// 使用
multiply(10, 3.5);          // T1=int, T2=double, 返回double
multiply(2.5, 4);           // T1=double, T2=int, 返回double
```

### 2.3 函数模板重载

```cpp
// 通用版本
template<typename T>
void print(T value) {
    std::cout << "Generic: " << value << std::endl;
}

// 特定类型的重载
template<typename T>
void print(T* ptr) {
    std::cout << "Pointer: " << *ptr << std::endl;
}

// 非模板重载
void print(const char* str) {
    std::cout << "C-String: " << str << std::endl;
}

int main() {
    int x = 42;
    print(x);           // 调用通用版本
    print(&x);          // 调用指针版本
    print("Hello");     // 调用非模板版本
}
```

### 2.4 默认模板参数

```cpp
template<typename T = int, typename Allocator = std::allocator<T>>
class Container {
    // ...
};

// 使用
Container<> c1;              // T=int, Allocator=std::allocator<int>
Container<double> c2;        // T=double, Allocator=std::allocator<double>
```

---

## 3. 类模板

### 3.1 基本类模板

```cpp
template<typename T>
class Stack {
private:
    std::vector<T> elements;
    
public:
    void push(const T& elem) {
        elements.push_back(elem);
    }
    
    T pop() {
        if (elements.empty()) {
            throw std::out_of_range("Stack<>::pop(): empty stack");
        }
        T elem = elements.back();
        elements.pop_back();
        return elem;
    }
    
    bool empty() const {
        return elements.empty();
    }
};

// 使用
Stack<int> intStack;
intStack.push(42);
intStack.push(100);
std::cout << intStack.pop() << std::endl;  // 100

Stack<std::string> stringStack;
stringStack.push("Hello");
stringStack.push("World");
```

### 3.2 成员函数的实现

```cpp
// 方式 1：类内定义（推荐用于简单函数）
template<typename T>
class MyClass {
public:
    void simpleFunc() {
        // 直接实现
    }
};

// 方式 2：类外定义
template<typename T>
class MyClass {
public:
    void complexFunc();  // 声明
};

// 定义必须也是模板
template<typename T>
void MyClass<T>::complexFunc() {
    // 实现
}
```

### 3.3 模板类的静态成员

```cpp
template<typename T>
class Counter {
private:
    static int count;  // 声明
    
public:
    Counter() { ++count; }
    ~Counter() { --count; }
    
    static int getCount() { return count; }
};

// 必须在类外定义静态成员
template<typename T>
int Counter<T>::count = 0;

int main() {
    Counter<int> c1, c2, c3;
    std::cout << Counter<int>::getCount() << std::endl;    // 3
    
    Counter<double> d1, d2;
    std::cout << Counter<double>::getCount() << std::endl; // 2
    
    // 注意：Counter<int> 和 Counter<double> 的 count 是独立的！
}
```

### 3.4 友元函数模板

```cpp
template<typename T>
class Array;  // 前向声明

// 友元函数模板
template<typename T>
std::ostream& operator<<(std::ostream& os, const Array<T>& arr);

template<typename T>
class Array {
private:
    T* data;
    size_t size;
    
public:
    // 声明友元
    friend std::ostream& operator<< <T>(std::ostream& os, const Array<T>& arr);
};

// 实现友元函数
template<typename T>
std::ostream& operator<<(std::ostream& os, const Array<T>& arr) {
    os << "[";
    for (size_t i = 0; i < arr.size; ++i) {
        os << arr.data[i];
        if (i < arr.size - 1) os << ", ";
    }
    os << "]";
    return os;
}
```

---

## 4. 模板特化

### 4.1 全特化（Full Specialization）

为**特定类型**提供**完全不同**的实现。

```cpp
// 通用模板
template<typename T>
class Storage {
public:
    void store(T value) {
        std::cout << "Storing generic type: " << value << std::endl;
    }
};

// 针对 bool 的全特化
template<>
class Storage<bool> {
public:
    void store(bool value) {
        std::cout << "Storing bool: " << (value ? "true" : "false") << std::endl;
    }
};

// 使用
Storage<int> intStorage;
intStorage.store(42);           // 使用通用版本

Storage<bool> boolStorage;
boolStorage.store(true);        // 使用特化版本
```

### 4.2 偏特化（Partial Specialization）

为**一类类型**提供特化（仅适用于类模板，不适用于函数模板）。

```cpp
// 通用模板
template<typename T1, typename T2>
class Pair {
public:
    void print() {
        std::cout << "Generic Pair" << std::endl;
    }
};

// 偏特化：两个参数类型相同
template<typename T>
class Pair<T, T> {
public:
    void print() {
        std::cout << "Same Type Pair" << std::endl;
    }
};

// 偏特化：第二个参数是指针
template<typename T1, typename T2>
class Pair<T1, T2*> {
public:
    void print() {
        std::cout << "Second is Pointer" << std::endl;
    }
};

int main() {
    Pair<int, double> p1;
    p1.print();  // Generic Pair
    
    Pair<int, int> p2;
    p2.print();  // Same Type Pair
    
    Pair<int, double*> p3;
    p3.print();  // Second is Pointer
}
```

### 4.3 函数模板特化

函数模板只支持全特化，不支持偏特化。

```cpp
// 通用模板
template<typename T>
bool compare(T a, T b) {
    return a < b;
}

// 针对 const char* 的全特化
template<>
bool compare<const char*>(const char* a, const char* b) {
    return strcmp(a, b) < 0;
}

int main() {
    compare(10, 20);              // 使用通用版本
    compare("abc", "xyz");        // 使用特化版本
}
```

**重要提示：** 优先使用函数重载而不是函数模板特化，因为重载更清晰。

---

## 5. 模板参数

### 5.1 类型参数（Type Parameters）

```cpp
template<typename T>        // T 是类型参数
class Container { };

template<class T>           // class 和 typename 等价（推荐用 typename）
class Container2 { };
```

### 5.2 非类型参数（Non-Type Parameters）

可以使用**整数、枚举、指针、引用**作为模板参数。

```cpp
// 固定大小的数组
template<typename T, size_t N>
class Array {
private:
    T data[N];
    
public:
    size_t size() const { return N; }
    
    T& operator[](size_t index) {
        return data[index];
    }
};

// 使用
Array<int, 10> arr1;        // 10 个 int
Array<double, 5> arr2;      // 5 个 double

std::cout << arr1.size() << std::endl;  // 10
```

**编译期计算：**

```cpp
template<int N>
struct Factorial {
    static constexpr int value = N * Factorial<N - 1>::value;
};

template<>
struct Factorial<0> {
    static constexpr int value = 1;
};

int main() {
    std::cout << Factorial<5>::value << std::endl;  // 120（编译期计算）
}
```

### 5.3 模板模板参数（Template Template Parameters）

模板本身作为参数。

```cpp
// Container 是一个模板模板参数
template<typename T, template<typename> class Container>
class Stack {
private:
    Container<T> elements;
    
public:
    void push(const T& elem) {
        elements.push_back(elem);
    }
};

// 使用
Stack<int, std::vector> intStack;       // Container = std::vector
Stack<double, std::deque> doubleStack;  // Container = std::deque
```

---

## 6. 模板元编程

模板元编程（Template Metaprogramming, TMP）是利用模板在**编译期**进行计算。

### 6.1 编译期条件判断

```cpp
// 使用 std::conditional
template<bool Condition, typename TrueType, typename FalseType>
struct Conditional {
    using type = TrueType;
};

template<typename TrueType, typename FalseType>
struct Conditional<false, TrueType, FalseType> {
    using type = FalseType;
};

// 使用
using Type1 = Conditional<true, int, double>::type;   // int
using Type2 = Conditional<false, int, double>::type;  // double

// C++11 标准库提供 std::conditional
using Type3 = std::conditional<sizeof(long) == 8, long, long long>::type;
```

### 6.2 编译期循环

```cpp
// 递归计算斐波那契数列
template<int N>
struct Fibonacci {
    static constexpr int value = Fibonacci<N - 1>::value + Fibonacci<N - 2>::value;
};

template<>
struct Fibonacci<0> {
    static constexpr int value = 0;
};

template<>
struct Fibonacci<1> {
    static constexpr int value = 1;
};

int main() {
    std::cout << Fibonacci<10>::value << std::endl;  // 55（编译期计算）
}
```

### 6.3 类型列表（Type List）

```cpp
// 简单的类型列表
template<typename... Types>
struct TypeList {};

// 获取第一个类型
template<typename Head, typename... Tail>
struct Front<TypeList<Head, Tail...>> {
    using type = Head;
};

// 获取列表长度
template<typename List>
struct Length;

template<typename... Types>
struct Length<TypeList<Types...>> {
    static constexpr size_t value = sizeof...(Types);
};

// 使用
using MyList = TypeList<int, double, char, std::string>;
using FirstType = Front<MyList>::type;  // int
constexpr size_t len = Length<MyList>::value;  // 4
```

---

## 7. SFINAE 与类型萃取

### 7.1 SFINAE（Substitution Failure Is Not An Error）

"替换失败不是错误"：模板参数替换失败时，编译器不报错，只是从候选集中移除该模板。

```cpp
// 检查类型是否有 size() 成员函数
template<typename T>
class HasSize {
private:
    // 如果 T 有 size()，这个函数会被选择
    template<typename U>
    static auto test(U* p) -> decltype(p->size(), std::true_type());
    
    // 否则选择这个
    template<typename>
    static std::false_type test(...);
    
public:
    static constexpr bool value = decltype(test<T>(nullptr))::value;
};

int main() {
    std::cout << HasSize<std::vector<int>>::value << std::endl;  // true
    std::cout << HasSize<int>::value << std::endl;               // false
}
```

### 7.2 std::enable_if

根据条件启用或禁用模板。

```cpp
// 只接受整数类型
template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
double_value(T value) {
    return value * 2;
}

// 只接受浮点类型
template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
double_value(T value) {
    return value * 2.0;
}

int main() {
    double_value(10);       // OK: 整数版本
    double_value(3.14);     // OK: 浮点版本
    // double_value("str"); // 编译错误：没有匹配的版本
}
```

**C++17 简化语法：**

```cpp
template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
T double_value(T value) {
    return value * 2;
}
```

### 7.3 类型萃取（Type Traits）

```cpp
#include <type_traits>

template<typename T>
void process(T value) {
    if constexpr (std::is_pointer_v<T>) {
        std::cout << "Pointer: " << *value << std::endl;
    } 
    else if constexpr (std::is_integral_v<T>) {
        std::cout << "Integer: " << value << std::endl;
    }
    else if constexpr (std::is_floating_point_v<T>) {
        std::cout << "Float: " << value << std::endl;
    }
    else {
        std::cout << "Other type" << std::endl;
    }
}

int main() {
    int x = 42;
    process(x);       // Integer: 42
    process(&x);      // Pointer: 42
    process(3.14);    // Float: 3.14
}
```

**常用类型萃取：**

```cpp
std::is_integral<T>::value         // 是否为整数
std::is_floating_point<T>::value   // 是否为浮点数
std::is_pointer<T>::value          // 是否为指针
std::is_array<T>::value            // 是否为数组
std::is_class<T>::value            // 是否为类
std::is_same<T1, T2>::value        // 两个类型是否相同

std::remove_reference<T>::type     // 移除引用
std::remove_const<T>::type         // 移除 const
std::remove_pointer<T>::type       // 移除指针
std::decay<T>::type                // 类型退化
```

---

## 8. 变参模板

C++11 引入的变参模板（Variadic Templates）可以接受**任意数量**的模板参数。

### 8.1 基本语法

```cpp
// ... 表示参数包
template<typename... Args>
void print(Args... args) {
    // sizeof...(Args) 获取参数个数
    std::cout << "Number of arguments: " << sizeof...(Args) << std::endl;
}

print();                  // 0 个参数
print(1);                 // 1 个参数
print(1, 2.5, "hello");   // 3 个参数
```

### 8.2 递归展开

```cpp
// 递归终止条件
void print() {
    std::cout << std::endl;
}

// 递归展开
template<typename T, typename... Args>
void print(T first, Args... rest) {
    std::cout << first << " ";
    print(rest...);  // 递归调用
}

int main() {
    print(1, 2.5, "hello", 'x');  // 输出: 1 2.5 hello x
}
```

### 8.3 折叠表达式（C++17）

更简洁的参数包展开方式。

```cpp
// 求和
template<typename... Args>
auto sum(Args... args) {
    return (args + ...);  // 右折叠: (a1 + (a2 + (a3 + a4)))
}

// 打印（使用逗号运算符）
template<typename... Args>
void print(Args... args) {
    ((std::cout << args << " "), ...);  // 左折叠
    std::cout << std::endl;
}

// 逻辑与
template<typename... Args>
bool all(Args... args) {
    return (args && ...);
}

int main() {
    std::cout << sum(1, 2, 3, 4, 5) << std::endl;  // 15
    print(1, 2.5, "hello", 'x');
    std::cout << all(true, true, false) << std::endl;  // false
}
```

**折叠表达式的四种形式：**

```cpp
(args + ...)       // 右折叠: (a1 + (a2 + (a3 + a4)))
(... + args)       // 左折叠: (((a1 + a2) + a3) + a4)
(init + ... + args)  // 带初始值的左折叠
(args + ... + init)  // 带初始值的右折叠
```

### 8.4 完美转发

```cpp
template<typename... Args>
void wrapper(Args&&... args) {
    // std::forward 保持参数的值类别（左值/右值）
    actual_function(std::forward<Args>(args)...);
}

// 示例：实现 make_unique
template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

class MyClass {
public:
    MyClass(int a, double b, std::string c) { }
};

int main() {
    auto ptr = make_unique<MyClass>(10, 3.14, "hello");
}
```

---

## 9. 现代 C++ 模板特性

### 9.1 模板别名（C++11）

```cpp
// using 定义模板别名
template<typename T>
using Vec = std::vector<T>;

template<typename T>
using Ptr = std::shared_ptr<T>;

// 使用
Vec<int> numbers = {1, 2, 3};
Ptr<MyClass> ptr = std::make_shared<MyClass>();

// 对比 typedef（不能用于模板）
typedef std::vector<int> IntVec;  // OK，但不是模板
```

### 9.2 变量模板（C++14）

```cpp
template<typename T>
constexpr T pi = T(3.1415926535897932385);

template<typename T>
constexpr T e = T(2.7182818284590452354);

int main() {
    std::cout << pi<double> << std::endl;  // 3.14159...
    std::cout << pi<float> << std::endl;   // 3.14159...f
    std::cout << e<double> << std::endl;   // 2.71828...
}
```

### 9.3 if constexpr（C++17）

编译期条件判断，未选中的分支**不会被实例化**。

```cpp
template<typename T>
auto get_value(T t) {
    if constexpr (std::is_pointer_v<T>) {
        return *t;  // 如果 T 不是指针，这行代码不会被编译
    } else {
        return t;
    }
}

int main() {
    int x = 42;
    std::cout << get_value(x) << std::endl;   // 42
    std::cout << get_value(&x) << std::endl;  // 42
}
```

### 9.4 概念（Concepts，C++20）

约束模板参数，提供更清晰的错误信息。

```cpp
#include <concepts>

// 定义概念
template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

template<typename T>
concept Printable = requires(T t) {
    { std::cout << t } -> std::convertible_to<std::ostream&>;
};

// 使用概念约束模板
template<Numeric T>
T add(T a, T b) {
    return a + b;
}

template<Printable T>
void print(T value) {
    std::cout << value << std::endl;
}

int main() {
    add(10, 20);          // OK
    add(3.14, 2.71);      // OK
    // add("a", "b");     // 编译错误：std::string 不满足 Numeric
    
    print(42);            // OK
    print("hello");       // OK
}
```

**requires 子句：**

```cpp
template<typename T>
requires std::is_integral_v<T>
T double_value(T value) {
    return value * 2;
}

// 或者使用简化语法
template<typename T>
T double_value(T value) requires std::is_integral_v<T> {
    return value * 2;
}
```

---

## 10. 实战案例

### 10.1 实现智能指针

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
    
    // 操作符重载
    T& operator*() const { return *ptr; }
    T* operator->() const { return ptr; }
    explicit operator bool() const { return ptr != nullptr; }
    
    // 获取原始指针
    T* get() const { return ptr; }
    
    // 释放所有权
    T* release() {
        T* temp = ptr;
        ptr = nullptr;
        return temp;
    }
    
    // 重置指针
    void reset(T* p = nullptr) {
        delete ptr;
        ptr = p;
    }
};

// 使用
UniquePtr<int> ptr1(new int(42));
std::cout << *ptr1 << std::endl;

UniquePtr<std::string> ptr2(new std::string("Hello"));
std::cout << ptr2->length() << std::endl;
```

### 10.2 实现类型安全的元组

```cpp
// 递归定义元组
template<typename... Types>
class Tuple;

// 空元组
template<>
class Tuple<> {};

// 递归定义
template<typename Head, typename... Tail>
class Tuple<Head, Tail...> : private Tuple<Tail...> {
private:
    Head value;
    
public:
    Tuple(Head h, Tail... t) : Tuple<Tail...>(t...), value(h) {}
    
    Head& getHead() { return value; }
    const Head& getHead() const { return value; }
    
    Tuple<Tail...>& getTail() { return *this; }
    const Tuple<Tail...>& getTail() const { return *this; }
};

// 辅助函数：获取第 N 个元素
template<size_t N, typename... Types>
struct TupleElement;

template<typename Head, typename... Tail>
struct TupleElement<0, Tuple<Head, Tail...>> {
    using type = Head;
    
    static Head& get(Tuple<Head, Tail...>& t) {
        return t.getHead();
    }
};

template<size_t N, typename Head, typename... Tail>
struct TupleElement<N, Tuple<Head, Tail...>> {
    using type = typename TupleElement<N - 1, Tuple<Tail...>>::type;
    
    static type& get(Tuple<Head, Tail...>& t) {
        return TupleElement<N - 1, Tuple<Tail...>>::get(t.getTail());
    }
};

// get 函数
template<size_t N, typename... Types>
auto& get(Tuple<Types...>& t) {
    return TupleElement<N, Tuple<Types...>>::get(t);
}

// 使用
Tuple<int, double, std::string> t(42, 3.14, "hello");
std::cout << get<0>(t) << std::endl;  // 42
std::cout << get<1>(t) << std::endl;  // 3.14
std::cout << get<2>(t) << std::endl;  // hello
```

### 10.3 实现编译期单位系统

```cpp
// 物理单位
template<int M, int L, int T>  // M=质量, L=长度, T=时间
struct Unit {
    double value;
    
    explicit Unit(double v) : value(v) {}
    
    double getValue() const { return value; }
};

// 类型别名
using Meter = Unit<0, 1, 0>;       // 长度
using Second = Unit<0, 0, 1>;      // 时间
using Velocity = Unit<0, 1, -1>;   // 速度 = 长度/时间
using Kilogram = Unit<1, 0, 0>;    // 质量
using Force = Unit<1, 1, -2>;      // 力 = 质量*加速度

// 乘法：单位指数相加
template<int M1, int L1, int T1, int M2, int L2, int T2>
Unit<M1 + M2, L1 + L2, T1 + T2> operator*(Unit<M1, L1, T1> a, Unit<M2, L2, T2> b) {
    return Unit<M1 + M2, L1 + L2, T1 + T2>(a.getValue() * b.getValue());
}

// 除法：单位指数相减
template<int M1, int L1, int T1, int M2, int L2, int T2>
Unit<M1 - M2, L1 - L2, T1 - T2> operator/(Unit<M1, L1, T1> a, Unit<M2, L2, T2> b) {
    return Unit<M1 - M2, L1 - L2, T1 - T2>(a.getValue() / b.getValue());
}

// 使用
Meter distance(100.0);
Second time(10.0);
Velocity v = distance / time;  // 自动推导为 Velocity
std::cout << "Velocity: " << v.getValue() << " m/s" << std::endl;

Kilogram mass(5.0);
// Force f = mass * v;  // 这会正确计算为 Force
// Second s = mass * v;  // 编译错误：类型不匹配！
```

### 10.4 实现观察者模式

```cpp
template<typename... Args>
class Signal {
private:
    using Slot = std::function<void(Args...)>;
    std::vector<Slot> slots;
    
public:
    // 连接槽函数
    void connect(Slot slot) {
        slots.push_back(std::move(slot));
    }
    
    // 发射信号
    void emit(Args... args) {
        for (auto& slot : slots) {
            slot(args...);
        }
    }
    
    // 操作符重载
    void operator()(Args... args) {
        emit(args...);
    }
};

// 使用
Signal<int, std::string> signal;

signal.connect([](int num, std::string str) {
    std::cout << "Slot 1: " << num << ", " << str << std::endl;
});

signal.connect([](int num, std::string str) {
    std::cout << "Slot 2: " << num * 2 << ", " << str << std::endl;
});

signal(42, "hello");  // 触发两个槽函数
```

### 10.5 实现编译期字符串哈希

```cpp
// constexpr 哈希函数
constexpr uint32_t hash_string(const char* str, uint32_t hash = 2166136261u) {
    return *str ? hash_string(str + 1, (hash ^ *str) * 16777619u) : hash;
}

// 编译期字符串
template<uint32_t Hash>
struct CompileTimeString {
    static constexpr uint32_t hash = Hash;
};

// 宏简化使用
#define CT_STRING(str) CompileTimeString<hash_string(str)>

// 使用编译期哈希实现高效的字符串比较
template<typename StringType>
void process_command(StringType) {
    if constexpr (StringType::hash == hash_string("start")) {
        std::cout << "Starting..." << std::endl;
    }
    else if constexpr (StringType::hash == hash_string("stop")) {
        std::cout << "Stopping..." << std::endl;
    }
    else {
        std::cout << "Unknown command" << std::endl;
    }
}

int main() {
    process_command(CT_STRING("start")());  // 编译期确定分支
    process_command(CT_STRING("stop")());
}
```

---

## 总结

### 核心概念回顾

1. **模板基础**
   - 泛型编程，类型参数化
   - 编译期代码生成
   - 零开销抽象

2. **模板类型**
   - 函数模板：泛型函数
   - 类模板：泛型类
   - 变参模板：任意数量参数

3. **高级特性**
   - SFINAE：选择性启用模板
   - 模板元编程：编译期计算
   - 完美转发：保持值类别
   - Concepts：约束模板参数

4. **最佳实践**
   - 优先使用 `typename` 而不是 `class`
   - 使用 `if constexpr` 替代 SFINAE（C++17+）
   - 使用 Concepts 提供清晰的约束（C++20+）
   - 模板定义通常放在头文件中
   - 避免过度使用模板元编程

### 学习路径

```
入门阶段
  ├─ 函数模板基础
  ├─ 类模板基础
  └─ 模板特化

进阶阶段
  ├─ 非类型模板参数
  ├─ 变参模板
  ├─ SFINAE 与 enable_if
  └─ 类型萃取

高级阶段
  ├─ 模板元编程
  ├─ 完美转发
  ├─ 折叠表达式
  └─ Concepts（C++20）

实战阶段
  ├─ STL 容器实现
  ├─ 智能指针实现
  ├─ 元编程库设计
  └─ 编译期计算
```

### 常见陷阱

1. **模板定义分离**：模板定义通常需要在头文件中
2. **依赖名称查找**：使用 `typename` 和 `template` 关键字消除歧义
3. **实例化开销**：每个类型都会生成代码，导致二进制膨胀
4. **编译错误**：模板错误信息通常很长，需要耐心分析

### 推荐资源

- 《C++ Templates: The Complete Guide》（最权威的模板书籍）
- 《Effective Modern C++》（现代 C++ 最佳实践）
- cppreference.com（查阅标准库文档）
- Compiler Explorer（在线查看模板实例化结果）

---

*文档版本：v1.0*  
*最后更新：2025年11月19日*
