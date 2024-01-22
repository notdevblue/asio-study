#include <iostream>
#include <thread>

#define ASIO_STANDALONE

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

// 데이터가 얼마나 큰지 한번에 알 수 없으니 적당하게 큰 사이즈의 버퍼를 만들고 재사용할거임
// 그리고 데이터 읽기를 asio 가 담당하게 할 함수를 만들 예정이기 때문에 메인 함수 스코프 밖에 만들어 둘 것
std::vector<char> vBuffer(1 * 1024);
// 버퍼의 크기가 데이터보다 작아도
// 버퍼 크기 만큼 계속 읽어 나감
// 그리고 크기가 버퍼 보다 작아도 문제 없이 크기 많큼 읽음

void GrapSomeData(asio::ip::tcp::socket& socket) {

	// asio 를 synchronous 로 사용할 때는 이 함수를 사용함
	// socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);
	
	// asynchronous 로 사용하기 위해 async_read_some 를 호출함
	// 처음과 같이 버퍼를 전달함. 하지만 람다 함수도 전달해야 함
	// 아레 함수는 async 이기 때문에 바로 무언갈 하지 않음
	// 대신 소켓이 무언가 데이터를 읽을 준비가 되었다면 그 데이터를 읽을 준비를 함 
	// 그리고 우리가 줄 일은 간단하게 소켓에서 데이터를 읽고 위에 만든 버퍼에 넣고, 화면에 출력하는거임
	// 그래서 람다를 전달함
	socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
		[&](std::error_code ec, std::size_t length) { // 에러 코드와 read_some 함수에서 읽은 데이터의 길이를 전달함
			if (!ec) {
				std::cout << "\n\nRead " << length << " bytes\n\n";

				for (int i = 0; i < length; ++i) {
					std::cout << vBuffer[i];
				}

				GrapSomeData(socket);
			}

			// 에러 코드가 없다면 얼마나 많은 바이트를 받았는지 표시하고,
			// length 만큼 버퍼를 순환해서 콘솔에 표시함
			// 그리고 다시 GrapSomeData 함수를 호출할거임
			// 엄청나게 개쩌는 재귀함수라고 생각할 수 있지만
			// async_read_some 함수는 asio context 에게 할 일을 넘겨주고 바로 반환함
			// 즉 asio 는 백그라운드에서 돌아갈 것
			// 소켓에 데이터가 날라온 경우, 우리가 람다 함수 안에 둔 코드를 구현할 것
		}
	);

	// 비동기적으로 생각하면
	// 우리는 시간 지연과 버퍼 사이즈 이슈를 극복함
}

int main() {

	asio::error_code ec;

	asio::io_context context;
	// 우리는 context 가 어떠한 조건에 의해 대기해야한다고 함
	// 그리고 관련된 코드를 실행하게 함
	// 하지만 뭔가를 대기할 때는 프로그램의 흐름을 block 함.
	// 그리고 우리는 입출력 문맥이 항상 block 하는걸 원하지 않음
	// 이 문제를 해결하기 위해 우리가 할 수 있는 여러 방법중 한 가지의 방법은
	// asio context 를 자기만의 스레드에서 돌리는 거임
	// 이러면 context 가 메인 스레드를 막지 않고 자신만의 코드들을 돌릴 수 있는 공간을 얻게 됨

	asio::io_context::work idleWork(context);
	// asio 에게 구라일을 줘서 context 가 끝나지 않게 함
	
	std::thread thrContext = std::thread([&]() { context.run(); });
	// 스레드를 하나 만들고 context 의 run 함수를 호출함
	// run 함수는 context 가 할 일이 다 떨어졌을 때 반환함
	// 그래서 우리가 context 에 명령을 등록하는것을 끝내자 마자 실행이 끝날 것임
	// 다행이도 asio 는 이를 해결하는 방법을 줌

	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), 80);

	asio::ip::tcp::socket socket(context);

	socket.connect(endpoint, ec);

	if (!ec) {
		std::cout << "Hello Internet!" << std::endl;
	}
	else {
		std::cout << "Failed to connect to address:\n" << ec.message() << std::endl;
	}

	if (socket.is_open()) {
		GrapSomeData(socket);

		std::string sRequest =
			"GET /index.html HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Connection: close\r\n\r\n";

		socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);
		
		// 지난번에 한 것들은 이제 필요없고


		//GrapSomeData(socket);
		// 요청을 보내자 마자 GrapSomeData 함수를 실행함
		// 그리고 함수는 async 로 무언가를 읽기 위해 입출력 문맥 (context 변수) 을 준비함
		// 하지만 우리 프로그램이 무언가를 하기 전에 끝나버림
		// 여기서 이제 async 가 약간 해깔리게 들어감
		// 입출력 문맥 준비를 요청을 보내기 전으로 옮길거임

		using namespace std::chrono_literals;
		std::this_thread::sleep_for(20000ms);
		// 충분히 모든 데이터를 받기 전까지 프로그램이 끝나지 않게 함
	}

	return 0;
}

// 간단한 예시를 통해 비동기적으로 생각하여 얻는 이익이 꽤 있다는걸 알 수 있음
// 하지만 복잡함도 따라오게 됨
// 비동기 프로그램을 만들게 되면 일반적인 프로그램과는 조금 다르게 생각해야 함
// 우리가 생각한 순서대로 일이 흘러가지 않게 됨
// 이것이 동기 프로그램과의 차이점임
// 하지만 미래에 무언가 실행되게 무언가를 준비해둔 다는 점은
// 시간 지연과 버퍼 사이즈 같은 변수들을 담당하기 아주 좋음
// https://youtu.be/2hNdkYInj4g?si=lzSgCme9ACJL9F2i&t=1373