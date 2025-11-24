#ifndef _TEMPLATE_TEST_HPP_
#define _TEMPLATE_TEST_HPP_

#include <iostream>
#include <type_traits>
#include <vector>
#include <string>


/*
 * 编写一个模板函数 SaveObject(T& obj)。如果对象有 Serialize() 函数，就调用它；如果没有，就打印“不可序列化”。
 */

/*
 * 模板编程时是没有变量这一说法的 所以只能使用 std::true_type/false_type
 */

// --- SFINAE ---
// SFINAE 替换失败不是错误 Substitution Failure Is Not An Error
// 萃取 目标: 实现静态反射 编译阶段运行 用于“检测是否存在某个函数”
/*
 * 模板中的 void 只是用于占位, 给其他的特化提供位置
 */
template <typename T, typename U=void>                /* 主模板 没有第二个参数时默认为void */
struct HasSerialize : std::false_type{};

template <typename T>                               /* 特化模板 告诉编译器 我要对某个 T 进行特化 */
/*
 * std::void_t 用于判断是否合法, 合法则让第二个参数变为 void, 不合法则编译器会将其丢掉, 进而导入void_t 让编译器忽略这个特化, 继续寻找其他特化, 没有了就回到最初版本;
 *
 * 所谓的合法就是编译器是否认为没有语法错误
 * 特化时需要把 所有模板参数 再写一遍 <T, ...>; 特化 即 提供一份专门的实现
 * 如果 Serialize()不存在, 这个类型就不是 true_type, 返回到默认的主模板的 false_type
 */
struct HasSerialize<T, std::void_t< decltype(std::declval<T>().Serialize()) >> : std::true_type {};

// std::declval<T>() -> 使用 decval 在编译器 “模拟” 一个对象的实例     (C++11)
// .Serialize() ->  尝试调用这个实例的 .Serialize() 函数
// decltype() -> 用于获取表达式的类型; 编译器会尝试推导 T.Serialize() 的返回值类型  (C++ 11)
// std::void_t<..> -> 在 <...> 中, 不管给什么类型, 只要是合法的, 就将其变为 void 类型 (C++ 17)

// --- constexpr --- (C++ 17)
template <typename T>
void SaveObject( T& Obj ) {
    /* constexpr : 如果不为真 花括号内的所有代码都会被编译器丢弃 */
    if constexpr ( HasSerialize<T>::value ) {
        std::cout << "[System] Detected Serialize(). Saving..." << std::endl;
        Obj.Serialize();
    } else {
        std::cout << "[System] Object does NOT have Serialize(). Skipping..." << std::endl;
    }
}

struct PlayerData {
    int Hp;
    void Serialize() { std::cout << "-> PlayerData Serialzed!" << std::endl; }
};

struct SimpleObject {
    // 这个 struct 没有 Serialize 函数
    float size;
};

static void TemplateTest() {
    PlayerData P1;
    SimpleObject S1;

    SaveObject(P1);     // 存在 并调用 Serialize()
    SaveObject(S1);
}

#endif

/*
编译器会先找特化, 找不到特化 才回到主模板

template<class T, class U>
struct X { static constexpr bool value = false; };

// 全特化：两个参数都固定
template<>
struct X<int, double> { static constexpr bool value = true; };

// 部分特化：只固定一部分，留一个参数待填
template<class T>
struct X<T, T> { static constexpr bool value = true; };  // 两个实参相同才命中
 */
