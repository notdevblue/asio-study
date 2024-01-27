// https://youtu.be/2hNdkYInj4g?si=bqJr0AssVWlSPhq7&t=2172
// cmake 대충 보고 와야 할듯. 영샹이 dependency 관련 내용임

#include <iostream>
#include <han_net.h>
// 지금까지의 이론을 검증할 것


// enun class 를 만들어서 메세지 타입을 정의할 것
enum class CustomMsgTypes : uint32_t { // 그리고 uint32_t 타입으로 구현되게끔, 각 타입 id 가 4 바이트 이게끔
    FireBullet,
    MovePlayer
};

int main() {

    // 나만의 메세지를 만들고 싶다면 아레의 방법대로 함
    han::net::message<CustomMsgTypes> msg; // 위에서 만든 메세지 타입으로 메세지 객채를 만듬

    msg.header.id = CustomMsgTypes::FireBullet; // 메세지 타입만 allow 함
    // msg.header.id = 1; 다른거 넣으면 컴파일 에러 남

    int a = 1;
    bool b = true;
    float c = 3.14159f;

    struct {
        float x;
        float y;
    } d[5];

    msg << a << b << c << d;
    // 디버깅으로 직접 확인하면
    // msg 크기는 57 (int + bool + float + (float, float) * 5) 임
    // 아주 잘 들거간 것 처럼 보임

    a = 99;
    b = false;
    c = 1.54f;
    // 정확한 확인을 위해서 변수의 값을 바꾸고 다시 꺼내보면

    msg >> d >> c >> b >> a;
    // 아주 잘 꺼내져있는걸 확인할 수 있음

    return 0;
}

// 상당히 유연하고, 유저 친화적인 메세지 생성 방법을 개발함
// 파라미터도 항상 알고 있음.
// 얼마나 큰지, 무엇을 위한 메세지인지.
// 그리고 가능한 곳에서, 컴파일러가 메세지가 온전한지 확인도 해줌
// https://youtu.be/2hNdkYInj4g?si=wmM67P0QbD-kogmH&t=2433 여기부터 보면 됨
