#pragma once
#include "net_common.h"
#include "net_message.h"
#include "net_tsqueue.h"
#include "net_connection.h"

namespace han
{
    namespace net
    {
        // 클라이언트 인터페이스는 asio 설정과 연결을 담당함
        // 그리고 앱이 접근할 수 있는 서버와의 엑세스 포인트로도 행동함
        template <typename T>
        class client_interface
        {
        public:
            // 그리고 생성자와 파괴자가 필요함
            client_interface() : m_socket(m_context) {
                // 소켓 초기화
            }

            virtual ~client_interface() {
                // 클라이언트가 파괴됬으면 항상 서버와의 연결을 종료하려고 시도
                Disconnect();
            }

            // 프레임워크의 대부분의 구현이 존재하지 않음에도 불구하고
            // 그냥 직감적으로(intuitively) 함수 안쪽을 채워 넣을 수 있음

        public:
            // 연결이 있으면 좋음
            bool Connect(const std::string& host, const uint16_t port) {
                // 연결은 약간 여러움. 대부분은 다음 영상에서 다룰 것
                // 이 함수에선 asio 가 물리적으로 서버와 연결하도록 할 것

                // 에러 코드를 사용하는 것 보단 예외를 캐치 할 것
                // asio 에러의 좋은 점은 상당히 자세하게 나와 있다는 것임

                try
                {
                    // 여기선 많은 것들을 할 것
                    // 1. 연결 오브젝트 생성
                    m_connection = std::make_unique<connection<T>>(); // TODO

                    // 이제 실제로 어디로 연결하는지에 대한 주소를 만들 것
                    // 영상 초반을 보면 ip 주소로 특정함

                    // 하지만 asio 는 resolver 라는 오브젝트를 제공함
                    // url 이나 string 형식이나 ip address 를 가지고
                    // 네트워크를 통해 연결할 수 있는 무언가로 바꿔줌
                    // endpoints 를 생성함

                    // 하지만 resolver 는 실패할 수 있음
                    // 만약 ip 주소로 줄일 수 없는 url 을 입력했다면 예외를 던짐.
                    asio::ip::tcp::resolver resolver(m_context);
                    m_endpoints = resolver.resolve(host, std::to_string(port));

                    // 유효한 주소로 변환됬다면
                    // 서버와 연결하기 함수를 호출할 수 있음
                    m_connection->ConnectToServer(m_endpoints);

                    // 그리고 최종적으로 asio 가 일할 수 있는 스레드를 생성함
                    thrContext = std::thread([this]() { m_context.run(); });

                    // 조금 빠진 곳이 있지만 다음 영상에서 할 것
                }
                catch(std::exception& e)
                {
                    std::cerr << "Client Exception: " << e.what() << '\n';
                    return false;
                }
                
                return true;
            }

            // 연결이 있으면 해제도 존재함
            void Disconnect() {
                // 연결 해제는 그냥 정중한거임

                // 연결이 실제로 이루어졌는지 확인하고
                if (IsConnected()) {
                    // 그렇다면 연결 해제함
                    m_connection->Disconnect();
                }

                // 위에 작동과는 상관없이
                // asio context 가 돌아가지 않도록 확실히 하고,
                // 돌리고 있는 스레드도 종료할 것
                m_context.stop();
                if (thrContext.joinable())
                    thrContext.join();

                // 연결 해제가 완료됬다면 커넥션 오브젝트도 더이상 사용하지 않음
                // unique_pointer 를 릴리즈 할 것
                m_connection.release();
            }

            // 연결이 유효한지 알 수 있으면 유용함ㅁ
            bool IsConnected() {
                // 이 함수에서 우리가 해야 할 것은
                // 연결이 존재하고, 연결이 연결되었는지 확인할 함수를 호출함
                if (m_connection)
                    return m_connection->IsConnected();
                else
                    return false;
            }

            

            // 그리고 클라이언트는 아레의 큐에 접근할 수 있어야 함
            // private 이니 access function 을 만들 것
            tsqueue<owned_message<T>>& Incoming() {
                return m_qMessagesIn;
            }
        
        protected:
            // 클라이언트 인터페이스는 asio 컨텍스트를 가짐
            asio::io_context m_context;

            // 컨텍스트는 자기가 일할 스레드도 필요함
            std::thread thrContext;

            // 이렇게 생각할 수 있음.
            // 커넥션이 asio 관련 모든 것들을 담당한다고 하지 않았음?
            // ㅇㅇ 하지만 클라이언트는 연결을 설정해야 함
            // 그리고 유효한 경우에만 존재할 것임
            // 그래서 클라이언트가 서버와의 교섭에 필요한 asio 관련 것들을 담당하고 연결을 이루어 낼 것임

            // 그래서 클라이언트는 소켓을 하나 가지고 시작할 것
            asio::ip::tcp::socket m_socket;

            // 그리고 연결이 성사되면, unique pointer 형식으로 만들 것,
            // 그리고 asio 관련 것들을 넘길 것임
            std::unique_ptr<connection<T>> m_connection;

            // 강의하는 사람이 이렇게 하기로 한 것
            // 다른 방식으로 해도 됨
        
        private:
            // 서버와 클라이언트는 들어오는 메세지를 보관하기 위한 물리적인 큐가 필요함
            tsqueue<owned_message<T>> m_qMessagesIn;
        };
    }
}

// 조금 실망스러울 수 있다는 것에 대해 알고 있음
// 대부분 영상 끝에는 완벽하게 돌아가는 예시를 줌
// 하지만 이건 시리즈 중에 하나이니, 그럴 일은 없음.
// 돌아가는 결과물을 위해서는 한 영상에 전부 집어넣기는 너무 코드가 많음
// 그래서 클라이언트를 작성하는 것이 어떤 식으로 이루어지는지 보여줌으로 끝낼 것
// 그리고 실질적인 단순화라는 것을 동의할 것이라고 의심하지 않음
