// 메세지 오브젝트의 정의를 include 할 것

#pragma once
#include "net_common.h"
// 강의하는 사람이 프레임워크를 헤더 온리로 만드는건 자기의 마음임
// 픽셀 게임 익스텐션으로 나중에 릴리즈 하고 싶으시다고함
// 그래서 꼭 헤더 온리로 하지 않아도 됨

namespace han
{
    namespace net
    {
        // T 는 enum class 를 대신할 것
        template <typename T>
        struct message_header
        {
            T id{}; // T 가 뭐든 간에 id 라는 이름을 가진 인스턴스를 생성함
            uint32_t size = 0; // 그리고 uint32_t 형식의 integer 로 크기를 저장함
            // size_t 를 고의로 선택하지 않음
            // uint32 는 우리가 전달할 데이터 크기를 나타내기에 충분하다고 생각
            // 32bit 과 64bit 에서 size_t 가 동일한지 장담할 수 없음
            // 원칙적으로 같은 프레임워크 코드는 32, 64bit 플렛폼 둘다 컴파일 될 것임.
            // 효과적으로 통신하기 위해서 통신 구조체 타입들의 크기는 바뀌지 않아야 함
            // 네트워킹 관련해선 이를 자주 생각하게 될 것
        };

        // id 의 본질적인 형식으로 enum class 를 선택했기 때문에,
        // 메세지 헤더에 의존하는 모든 것들은 template 여야 함.
        // 이는 knock-on(치어서 넘어트리다) 효과가 있음
        // 메세지에 의존하는 것들은 전부 template 여야 함.
        // 아닌거 같으면 int 로 해도 되지만, 헤더 파일로만 정의된 프레임워크를 개발할 때에는 적절하다고 강의하는 사람이 생각함
        template <typename T>
        struct message
        {
            message_header<T> header{}; // 메세지는 메세지 헤더가 존재함
            std::vector<uint8_t> body; // 그리고 바디도 존재함, unsigned 8bit integers 의 vector 를 통해 byte 로 개발할 수 있게 함
            // vector 는 데이터의 배열을 다루는 유연한 방법임

            // 메세지의 크기를 byte 로 반환함
            size_t size() const {
                return sizeof(message_header<T>) + body.size();
            }

            // 디버깅을 하는 동안 페킷을 직접 볼 수 있다면 상당히 유용할 것
            // cout 를 이용하여 우리의 메세지를 보기 쉽게 콘솔에 표시함
            // 이 << 는 수많은 곳에서 수많은 것들의 의해 접근되니 friend 접근 지정자가 사용됨.
            friend std::ostream& operator << (std::ostream& os, const message<T>& msg) {
                os << "ID:" << int(msg.header.id) << " Size:" << msg.header.size;
                return os;
            }

            // https://youtu.be/2hNdkYInj4g?si=WiVebTsCDOiicoDd&t=1884
            // 이거 봐야 함
        };
    }
}