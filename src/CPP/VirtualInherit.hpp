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

static void TestEcho() {
    // 两个独立的实例 虚继承是在一个实例内避免Ambiguous的操作...
    std::shared_ptr<DerivedD> testD = std::make_shared<DerivedD>();
    std::shared_ptr<DerivedC> testC = std::make_shared<DerivedC>();
    testD.get()->Echo();
    testD.get()->data = 150000;
    testD.get()->Echo();
    testC->Echo();
}

#endif
