#ifndef _VIRTURAL_INHERIT_HPP_
#define _VIRTURAL_INHERIT_HPP_
    #include <memory>
    #include <iostream>

class BaseA {
public:
    float data;

    BaseA()
        : data(1.5)
    {
        std::cout << "BaseA Initialize" << std::endl;
    }

    void Echo() {
        std::cout << "BaseA: " << this->data << std::endl;
    }

    virtual ~BaseA(){};
};

class DerivedB : virtual public BaseA {
public:
    DerivedB() {
        std::cout << "DerivedB Initialize" << std::endl;
    }
};

class DerivedC : public virtual BaseA {
public:
    DerivedC() {
        std::cout << "DerivedC Initialize" << std::endl;
    }
};

class DerivedD : public DerivedB, DerivedC {
public:
    DerivedD() {
        std::cout << "DerivedD Initialize" << std::endl;
    }
};

class DerivedE : public BaseA {

};

static void TestEcho() {
    // 两个独立的实例 虚继承是在一个实例内避免Ambiguous的操作...
    std::shared_ptr<DerivedD> testD = std::make_shared<DerivedD>();
    std::shared_ptr<DerivedC> testC = std::make_shared<DerivedC>();
    testD.get()->Echo();
    testD.get()->data = 150000;
    testD.get()->Echo();
    testC->Echo();

    // dynamic_cast 以及 static_cast
    BaseA* a = new BaseA;
    // 虚继承的派生类(BCD都不行)无法使用 static 原因复杂
    // DerivedB* b1 = static_cast<DerivedB*>( a );

    DerivedE* e1 = static_cast<DerivedE*>(a);
    DerivedB* b1 = dynamic_cast<DerivedB*>( a );

    if ( b1 ) {
        std::cout << b1 << std::endl;
    } else {
        // dynamic_cast 不允许 downcasting [ 基类指针指向基类对象时不能 dynamic_cast 为 派生类 ]
        std::cout << "nullptr: "  << b1 << std::endl;
    }
}

#endif
