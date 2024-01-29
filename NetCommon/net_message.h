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
            // uint32 는 우리가 전달할 데이터 크기를 나타내기에 충분하 다고 생각
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

            // 사용하기 쉽게 메세지를 만들고 싶음
            // 지금까지 한거로 볼때,
            // 메세지를 정의하기 위해선
            // enum class 를 넘겨주며 메세지 구조체의 인스턴스를 생성함
            // 그러면 그에 따라서 메세지를 생성할 것
            // 메세지의 body 의 작업을 위해선, 바로 vector 를 만져도 됨
            // 하지만 편리한 연산자 오버로딩을 추가할 것
            // vector 가 stack 인것 처럼 사용할 수 있게 할 것임

            // 변수가 있다고 침 (float x, y)
            // 메세지 오브젝트를 가지고 pipe into it 할것 (msg << x << y)
            // 데이터를 가져올 땐 거꾸로 할 것 (msg >> y >> x)
            // 그리고 모든 데이터 타입이 가능하게 할 것

            // body vector 에 데이터 넣는 작업
            // template 함수로 만들 것
            // float 을 넣게 되면 타입이 float 이 되고
            // int 을 넣게 되면 int 가 됨
            // struct 를 넣게 되면 struct 가 됨
            // 하지만 C++ 에서 직렬화 불가능한 타입도 있음
            // static 변수를 가진 클레스 일 수 있음
            // 또는 복잡한 포인터들
            template<typename DataType>
            friend message<T>& operator << (message<T>& msg, const DataType& data) {
                // 그래서 모던 c++ 를 사용해서 오버라이드에 전달된 타입이 너무 복잡하지 않은지 확인할 것
                static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pushed into vector.");

                // 그리고 현재 vector 의 사이즈를 캐싱 할 것
                // 처음엔 0 이지만, 데이터를 넣으면 넣을수록 넣은 데이터만큼 커질 것
                size_t i = msg.body.size();

                // 그리고 들어가는 데이터들을 위한 공간을 추가로 만들어야 함
                msg.body.resize(msg.body.size() + sizeof(DataType));

                // vector.data() 는 첫 요소의 주소를 줌
                // vector 의 기존 크기를 알고 있으니, 어디서부터 새 데이터를 넣으면 되는지 알 수 있음
                // 첫 요소 주소 + 크기 = 쓸수 있는 빈 공간
                std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

                // body vector 의 사이즈가 변경되었으니
                // header 의 size 변수도 변경함
                // 이렇게 하면 header 의 size 는 항상 body 사이즈와 같음
                msg.header.size = msg.size();

                // 메세지의 레퍼런스를 반환하도록 오버로딩 함
                // 이는 임의의 오브젝트의 임의의 타입이 vector 에 들어가도록 chained 되게 함
                return msg;

                // 이런 방식의 장점은 대부분의 데이터 타입을 지원함
                // 그리고 메세지 vector 의 크기를 자동으로 변경 후 할당함
                // 단점 중 하나는 퍼포먼스임
                // 메세지 body vector 에 무언가를 추가할 때 마다 리사이드 되어야 함
                // 하지만 vector 는 자기가 커지면 선형적으로 커지지 않을 정도로 똑똑함
                // 실제로 보면 이 방식은 오버헤드는 적음
            }

            // body vector 데이터를 꺼낼 때에는 넣을 때와 같이 msg >> x >> y 가 편리하다고 생각할 수 있음
            // 이 방식의 구현을 보고 왜 별로인지 확인할 것

            template<typename DataType>
            friend message<T>& operator >> (message<T>& msg, DataType& data) {
                static_assert(std::is_standard_layout<DataType>::value, "Data is too complex to be pop out of a vector.");

                // 이번에는 vector 의 끝에서 데이터를 가져옴
                // 아레의 방식으로 위치를 캐싱할 것
                size_t i = msg.body.size() - sizeof(DataType); 

                // 물리적으로 유저의 변수에 vector 의 값을 복사함
                std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

                // 데이터를 빼냈으니 vector 의 크기를 줄임
                msg.body.resize(i);
                
                // 여기선 퍼포먼스에 영향이 가지 않음
                // 기존의 크기보다 vector 를 줄이는 것은 reallocation 이 일어나지 않음
                // 만약 앞에서 데이터를 빼게 된다면, vector 의 앞쪽에서 데이터가 빠지게 되어야 하고
                // 상당한 reallocation 이 데이터를 뺄 때 마다 일어날 것
                // vector 를 stack 처럼 사용하게 되면, 불필요한 reallocation 를 피할 수 있음

                // 헤더 사이즈 바꾸고 레퍼런스 반환함
                msg.header.size = msg.size();
                return msg;
            }

            // GO-SU 라면 iterator 를 사용하는 방식도 있음
            // vector 에 데이터를 넣은 순서대로 빼낼 수 있음
            // 하지만 데이터 빼내는 과정 중에서 iterator 를 어디에 저장할 것인지 생각해야 함
        };

        // 아레에서 사용되지만 정의되지 않았으니
        // 전방 선언을 함
        // 아레에 사용된걸 보면 이것도 template class 임
        template <typename T>
        class connection;

        // 근본적으로 owned_message 는 간단히 일반 메세지를 캡슐화 하지만
        // 연결 오브젝트의 shared pointer 를 가지고 있음 (connection 오브젝트를 아직 구현하진 않았음)
        template <typename T>
        struct owned_message {
            std::shared_ptr<connection<T>> remote = nullptr; // 정의되진 않았지만 사용하고 있음
            message<T> msg;

            // 완전성을 위해 output stream 을 overload 할 것
            // 이러면 cout 을 통해 출력이 가능함
            friend std::ostream& operator<<(std::ostream& os, const owned_message<T>& msg) {
                os << msg.msg;
                return os;
            }
        };

        // 이 프레임워크에서 사용되는 모든 것들은
        // 근본적으로 메세지를 정의하는 enum class 에 의존하는 템플릿 클레스 일 것임

        // 대채로 클라이언트나 서버에 도착하는 메세지들은
        // 도착하는데 사용된 연결 오브젝트의 태그를 들고 있을 것 (위에 shared_ptr 말하는 거임. 영어 어렵다...)
        // 이를 통해 필요한 경우, 메세지를 전송할 연결을 특정할 수 있음
    }
}