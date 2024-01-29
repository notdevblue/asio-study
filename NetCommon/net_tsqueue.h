#include <mutex>
#include <deque>

// thread-safe 해야 하는 이유는
// 아무 때나 연결(연결 담당하는 그거)이나 클라이언트에서 접근 하기 때문
// 특별히 서버에선 엄청나게 많은 연결이 큐에 작성하려 하고, 서버는 필요할때 읽기 때문임
// 이 모든 것들은 여러 시점에서 이루어짐
// 그리고, 언제 일어나게 될지 제어할 순 없음, 처음에 봣던 것 처럼 (html 불러오는 코드)
// 데이터는 이동하는데 시간이 걸림, 그리고 근본적으로 <비동기 입출력 라이브러리> 에 우리는 전부 기반하고 있음
// 우리가 때때로 일어나는 병렬적인 요청들을 받아 큐에 직렬화 시키는 작업을 위해선, 큐가 thread-safe 해야 함

// https://youtu.be/2hNdkYInj4g?si=YT5KhW76QLu9D9R2&t=2701
// thread-safe Queue 를 구현하는건 사실 그렇게 어렵지 않음
// 강의하는사람은 lock 을 사용해서 구현할 것
// 뭔가가 큐에 작성하려고 하면, 다른 모든것들은 그 큐에서 읽을 수 없다는 것임
// lock-free thread-safe Queue 도 존재함. 하지만 간단하고 접근하기 쉽게 만들 것

namespace han
{
    namespace net
    {
        // template class 인걸 볼 수 있음
        // threadsafe queue 를 만드는 김에 메세지만 보관하는게 아니라
        // 거의 모든것들을 보관할 수 있게 할 예정임
        template<typename T>
        class tsqueue {
            public:
                // 기본 생성자를 사용할 것
                // 그리고 복사는 막을 것. 멤버 변수로 뮤텍스가 존재하기 때문임
                tsqueue() = default;
                tsqueue(const tsqueue<T>&) = delete;

                // clear 함수가 존재하기 때문에, 파괴자도 추가할 수 있음
                // 큐가 파괴될 때 clear 함수가 호출되게끔.
                virtual ~tsqueue() { clear(); }

            public:
                // threadsafe queue 를 구현하는건 그냥
                // double ended queue 에서 제공하는 표준 함수의 보호를 제공하는 것임
                // queue 맨 앞의 오브젝트의 레퍼런스를 얻어 옴
                const T& front() {
                    // scoped lock 과 mutex 를 통해 dequeue.front() 가 실행되는 동안 다른 것들이 돌아가는걸 방지함
                    std::scoped_lock lock(muxQueue);
                    return deqQueue.front();
                }

                // front 가 있으니 back 도 있음
                const T& back() {
                    std::scoped_lock lock(muxQueue);
                    return deqQueue.back();
                }

                // double ended queue 이니 뒤에 무언가를 추가할 능력이 있음
                void push_back(const T& item) {
                    std::scoped_lock lock(muxQueue);
                    deqQueue.emplace_back(std::move(item));
                }
                
                // 앞에도 추가할 수 있음
                void push_front(const T& item) {
                    std::scoped_lock lock(muxQueue);
                    deqQueue.emplace_front(std::move(item));
                }

                // 편리한 함수도 추가할 수 있음
                bool empty() {
                    std::scoped_lock lock(muxQueue);
                    return deqQueue.empty();
                }

                // 큐의 요소 개수를 샘
                size_t count() {
                    std::scoped_lock lock(muxQueue);
                    return deqQueue.size();
                }

                // 큐를 비움
                void clear() {
                    std::scoped_lock lock(muxQueue);
                    deqQueue.clear();
                }

                // double ended queue 에 재공된 기본 함수의 변형을 두개 추가할 것임
                
                // 지우는게 아니라 반환받고 싶기 때문
                T pop_front() {
                    std::scoped_lock lock(muxQueue);
                    auto t = std::move(deqQueue.front());
                    deqQueue.pop_front();
                    return t;
                }

                // 위와 비슷한 방식으로 pop_back 도 만듬
                T pop_back() {
                    std::scoped_lock lock(muxQueue);
                    auto t = std::move(deqQueue.back());
                    deqQueue.pop_back();
                    return t;
                }
            

            protected:
                // 근본적으로 우리의 큐는 double ended queue 형식으로 데이터를 보관할 것 (또는 deque)
                // 그리고 mutex 를 이용하여 deque 의 접근을 보호할 것
                std::mutex muxQueue;
                std::deque<T> deqQueue;
        };
    }
}

// scoped lock 보호 기능을 일반적인 double ended queue 기능에 추가한 것
// 메세지가 시스템 여기저기서 푸쉬되는 동안
// 메세지가 서버에 도착하게 되면
// 서버는 메세지를 전송한 클라이언트에게 답장해야하는 경우가 있을 수 있으니 메세지가 어디서 도착했는지 알아야 함
// 서버의 관점에서 보면 클라이언트를 식별하게 하는 것은 연결임
// 그래서 메세지가 도착한 연결에 대한 포인터를 포함하고 있는 변형된 메세지 타입을 추가할 것
// 동일한 것을 클라이언트에도 적용할 수 있음.
// 하지만 지금 상황에선 조금 과함.
// 왜냐하면 클라이언트는 서버를 향한 오로지 하나의 커넥션을 가지기 때문임.
