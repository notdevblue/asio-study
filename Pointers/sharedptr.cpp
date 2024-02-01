#include <iostream>
#include <memory>

/*
C++ 에는 3가지의 스마트 포인터가 존재함
std::shared_ptr
std::weak_ptr
std::unique_ptr
std::auto_ptr 은 삭제됨
*/


class MyClass {
public:
    inline MyClass() {
        std::cout << "만들어짐!\n";
    }

    inline virtual ~MyClass() {
        std::cout << "사라짐!\n";
    }

    inline void Say() {
        std::cout << "Call count: " << ++m_call_count << "\n";
    }

private:
    int m_call_count = 0;
};  


int main()
{

    // std::shared_ptr
    /*
    shared_ptr 은 포인터를 통해 소유권을 공유함
    여러 shared_ptr 오브젝트는 같은 오브젝트를 소유할 수 있음
    소유한 포인터는 get() 을 사용해서 접근할 수 있음

    오브젝트가 파괴되고 메모리 할당이 해제될 때는
    1. 오브젝트를 소유하고 있는 마지막 shared_ptr 가 파괴됬을 때
    2. 오브젝트를 소유하고 있는 마지막 shared_ptr 가 다른 오브젝트를 할당받았거나, reset() 을 호출

    모든 멤버 함수 (복사 생성자와 복사 대입 포함)은 추가적인 동기 작업 없이 각각 다른 스래드에서 다른 shared_ptr 인스턴스를 통해 호출할 수 있음.

    만약 각각 다른 스래드에서 같은 shared_ptr 인스턴스를 동기 작업 없이 접근하고, non-const 멤버 함수를 사용하게 된다면, race condition.
    std::atomic<shared_ptr> 을 이용해서 data race 를 막을 수 있음
    */

    std::shared_ptr<MyClass> origin = std::make_shared<MyClass>(); // MyClass 를 생성

    printf("Use count: %ld\n", origin.use_count());
    origin.get()->Say();

    printf("Creating more...\n"); // origin 을 복사함
    std::shared_ptr<MyClass> p1 = origin;
    std::shared_ptr<MyClass> p2 = origin;
    std::shared_ptr<MyClass> p3 = origin;

    printf("Use count: %ld\n", origin.use_count()); // 4개의 shared_ptr 인스턴스 (본인 포함)가 origin 을 사용하고 있으니 4를 출력함
    p1.get()->Say(); // 같은 인스턴스임을 확인할 수 있음
    p2.get()->Say();
    p3.get()->Say();

    origin.reset();
    p1.reset();
    p2.reset();
    p3.reset();
    // 오브젝트를 소유하고 있는 마지막 shared_ptr 가 파괴되었기 때문에, 소유하고 있던 오브젝트를 파괴함.

    printf("Exitting\n");

    // origin.get()->Say(); // segfault (reset 호출 이후에 접근)

    return 0;
}
