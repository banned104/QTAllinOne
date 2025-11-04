#ifndef _VIRTUAL_FUNCTION_TEST_HPP_
#define _VIRTUAL_FUNCTION_TEST_HPP_

#include <vector>
#include <iostream>
#include <memory>

class SortStrategy {
public:
    virtual void sort( std::vector<int> &array ) = 0;
};

// 快排
class _QuickSort : public SortStrategy {
public:
    // void sort( std::vector<int> array ) override  ---> 少了 &; 导致与基类方法参数类型不一致; 所以无法override;
    void sort( std::vector<int>& array ) override {
        std::cout<< "Quick sort!" << std::endl;
    }
};

// 归并排序
class _MergeSort : public SortStrategy {
public:
    void sort( std::vector<int>& array ) override {
        std::cout<< "Merge sort!" << std::endl;
    }
};

class Sorter {
    SortStrategy* m_s;
    Sorter( SortStrategy* s ) : m_s(s) { }

    void useSort( std::vector<int>& a ) {
        m_s->sort(a);
    }
};


/* 使用策略模式 */
void useStrategyTest() {
    // 只需要改一处 即可在后面的业务逻辑中无感切换方法的具体实现
    std::unique_ptr<SortStrategy> p_uniqe_sort = std::make_unique<_QuickSort>();

    std::vector<int> array = {1,2,3};
    std::vector<int>& p_array = array;
    p_uniqe_sort.get()->sort(p_array);
}

#endif
