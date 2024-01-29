#pragma once

// 클라이언트와 서버 둘다 connection 에 의존할 것

#include "net_common.h"
#include "net_tsqueue.h"
#include "net_message.h"

namespace han
{
    namespace net
    {
        // 근본적으로 connection 은 template class 임
        // 하지만 요상한 클래스를 상속할 것
        // std::enable_shared_from_this 는
        // connection 객채 내에서 shared pointer 를 만들 수 있게 해줌
        // this 키워드랑 비슷함, 하지만 생 pointer 대신 shared pointer 를 제공함

        template <typename T>
        class connection : public std::enable_shared_from_this<connection<T>>
        {
        public:
            connection()
            {}

            virtual ~connection()
            {}

        public:
            bool ConnectToServer(); // 오로지 클라이언트에서만 호출할 것
            bool Disconnect(); // 서버, 클라이언트 둘다 호출이 가능하고, 연결을 끊을 것
            bool IsConnected() const; // 연결이 유효하고, 성사되었고, 활성화되었는지 상태를 반환함

        public:
            // 연결 개채는 메세지를 보낼 수 있음
            // 그리고 message 객채가 template 변수 T 를 통해 매개변수화 되어 있다는 것을 알 수 있음
            bool Send(const message<T>& msg);

        protected:
            // 연결을 asio 관련된 것들도 담당함
            // 각 연결은 socket 을 가짐
            asio::ip::tcp::socket m_socket;

            // io context 없인 asio 는 작동할 수 없고, socket 은 기능할수 없다는걸 암
            asio::io_context& m_asioContext;

            // 여기서 약간 이상해짐
            // 서버는 여러 연결을 가질 수 있음
            // 하지만 여러 asio context 를 가지고 싶진 않음
            // asio context 가 tandem(둘 이상 앞뒤로 나란히, 앞뒤로 나란히 서서) 처럼 작동하기를 원함
            // 그래서 서버 또는 클라이언트에서의 연결은 asio context 를 통해 제공될 것이고,
            // 연결에서 발송되는 메세지의 큐만 가지고 있을 것

            // asio 와의 연결은 아레의 queue 를 확인하여 필요한 경우 메세지를 전송할 것
            tsqueue<message<T>> m_qMessagesOut;

            // 서버로 들어오는 메세지들은
            // 클라이언트의 서버에 의해 큐에 저장되어 있음
            // 하지만 연결은 큐가 어디에 있는지 알아야 함
            // 그래서 owned_message 로 이루어진 tsqueue 를 사용함
            // 하지만 그냥 큐의 레퍼런스임
            // 이 큐는 서버 또는 클라이언트에 의해 제공되길 원함
            tsqueue<owned_message>& m_qMessagesIn;

            // 효율적으로 연결 오브젝트는 풀임

            // https://youtu.be/2hNdkYInj4g?si=1kjsaphoz4eBwZNJ&t=3087
        };
    }
}
