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

// 클라이언트 인터페이스를 상속받아서 이 애플리케이션 전용의 클라이언트 클레스를 만들 수 있음
// 이 시점에서 템플릿은 사라지게 됨
// client_interface 클레스에 CustomMsgTypes 를 전달함
class CustomClient : public han::net::client_interface<CustomMsgTypes>
{
public:
    // Connect, Disconnect, IsConnected 함수는 이미 재공됨
    // 바로 실제 로직을 구현할 수 있음

    // 서버에게 특정한 위치에 총알을 발사했다고 보내고 싶다고 함
    bool FireBullet(float x, float y) {
        // x, y 를 받고
        // 메세지를 만듬
        han::net::message<CustomMsgTypes> msg;
        msg.header.id = CustomMsgTypes::FireBullet;

        // 데이터 넣음
        msg << x << y;
        Send(msg);
    }
};

int main() {

    // 메인 게임 어딘가에서 커스텀 클라이언트 인스턴트를 생성
    CustomClient c;
    // 특정한 포트와 주소를 넣음
    c.Connect("community.onelonecoder.com", 60000);

    // 서버에게 총알이 발사됬다고 전해야 할 때는 아레의 함수를 호출함
    c.FireBullet(2.0f, 5.0f);

    // 현재 상태로는 컴파일되지 않음
    // 아직 대부분은 만들어지지 않았음
    // 그리도 대부분의 것들을 구현하지도 않음
    // 하지만 이 단계에서 우리의 프레임워크는 복잡한 것들,
    // 예를 들어 클라이언트와 서버 간의 관계 등을
    // 상당히 유연하게 줄였고, 간단하고 사용하기 쉽게 만들었음

    // 이건 항상 1단계의 경우임
    // 파트2에는 클라이언트, 서버, 연결을 전부 구현하고, 메세지를 실제로 이동해 볼 것
    // 우리가 미리 만들어둔 것은, 트랜잭션을 위한 멋진 확정된 메세징 시스템 프레임워크와 큐임
    // 지금 시점에선 우리가 해결해 나가야 하는 문제들을 아주 높은 레벨로만 봤음
    // 이제 우리는 타이밍이나 연결을 강하게 유지하는 일, 메세지 순서 보장에 대처할 것
    // 네트워크 끊김 또는, 클라이언트가 예측되지 않은 순간에 접속을 끊으면 어떻게 대처할 것인지

    // 그리고 진도가 잘 나가면, 로그인 시스템을 어떻게 구현하는지 알아볼 것

    // 파트 2에는 이 아저씨가 만든 Raycast 월드 엔진을 이용해 실제로 게임을 구현해 볼것
    // 이를 통해 많은 캐릭터가 움직이고 서로 공격하는 것을 가능하게 할것

    return 0;

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
