#ifndef _DEEP_COPY_HPP_
#define _DEEP_COPY_HPP_

#include <cstring>
#include <iostream>
#include <memory>

class StringClass {
public:
    StringClass( const char* str ) {
        data = new char[ std::strlen(str) + 1 ];
        std::strcpy( data, str );
    }

    // 深拷贝构造函数
    StringClass( const StringClass& other ) {
        data = new char[ std::strlen( other.data ) + 1 ];
        std::strcpy( data, other.data );

        std::cout << "length= " << std::strlen( data ) << std::endl;
        ReverseString(data);
    }

    // 深拷贝运算符重载
    StringClass& operator=( const StringClass& other ) {
        if ( this == &other ) {
            return *this;
        }

        delete[] data;
        data = new char[ std::strlen(other.data) + 1 ];
        std::strcpy( data, other.data );
        ReverseString(data);

        return *this;
    }

    void SetString( const char* str ) {
        if ( data != NULL ) {
            delete[] data;
        }

        data = new char[ std::strlen(str) + 1 ];
        std::strcpy( data, str );
    }

    void ReverseString( char* str ) {
        int length = std::strlen( str );
        std::cout << "length= " << length << std::endl;
        char temp = '\0';

        for( int i = 0; i < length/2; i++ ) {
            temp = str[length - i - 1];
            str[length - i - 1] = str[i];
            str[i] = temp;
        }
    }

    void PrintString(void) {
        std::cout << data << std::endl;
    }

    ~StringClass() {
        delete[] data;
    }

private:
    char* data;
};


static void TestStringClass() {
    const char* originalString = "hello";
    std::shared_ptr<StringClass> stringOriginClass = std::make_shared<StringClass>(originalString);
    std::shared_ptr<StringClass> stringClassTest = std::make_shared<StringClass>("halo");

    stringClassTest->PrintString();
    // stringClassTest = std::make_shared<StringClass>(originalString);
    // stringClassTest = stringOriginClass; ---> 这里触发的是 shared_ptr 的赋值运算 而不是底层的自己编写的拷贝构造函数
    stringClassTest->PrintString();

    StringClass a("");
    StringClass b("KoBeee");
    a = b;
    a.PrintString();



    return;
}


#endif
