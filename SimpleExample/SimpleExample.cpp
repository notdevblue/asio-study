#include <iostream>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
// 윈도우 버전 마다 소켓 헨들링이 다르기 때문에
// 미리 정의를 해줌
// 기본값도 있긴 함
#endif

#define ASIO_STANDALONE // 부스트 라이브러리 안 씀

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
// 메모리 헨들링
#include <asio/ts/internet.hpp>
// 네트워크 통신을 위한 것들을 해줌

int main() {

	asio::error_code ec;

	asio::io_context context;
	// unique instance of asio
	// OS 종속적인 것들이 이 안에서 처리됨

	// 어딘가 연결되고 싶은 주소를 가져옴
	// 이름에서 알다시피 asio 는 네트워크에 한정되지 않고 많은 입출력을 지원함
	// 연결될 수 있는 무언가들은 endpoint 라고 함
	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), 80);
	// tcp 스타일의 endpoint 를 만듬
	// ip 주소와 port 로 endpoint 가 정의됨
	// make_address 함수를 이용해 IP 문자열을 asio 가 알아들을 수 있는 무언가로 바꿈
	// 무언가 잘못될 때를 대비해서 에러코드 변수도 전달함
	
	// OS 의 네트워크 드라이버의 훅
	// 우리가 연결한 네트워크의 통로로 사용될 것
	asio::ip::tcp::socket socket(context); // context 가 socket 의 구현을 전달할 것
	
	// 소켓에게 endpoint 에 연결하라고 함
	socket.connect(endpoint, ec);

	if (!ec) {
		std::cout << "Hello Internet!" << std::endl;
	}
	else {
		std::cout << "Failed to connect to address:\n" << ec.message() << std::endl;
		// error_code 변수는 런타임에 무슨 에러가 발생했는지 알아볼 수 있음
		// 에러 메세지가 서술적이기 때문에 유용함
	}

	if (socket.is_open()) {
		// 살아있는 커넥션
		// 웹사이트에 연결하려고 하기 때문에, 서버는 http request 를 기다리고 있음

		std::string sRequest =
			"GET /index.html HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Connection: close\r\n\r\n";

		socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);
		// write_some 함수는 이 데이터를 가능한 만큼 보내달라는 의미
		// asio 에서 데이터 입출력을 작업할 때, asio buffer 를 사용함
		// 그냥 byte 배열임. (문자열의 byte 들과 문자열 크기)

		// 수정 전에는 서버에게 데이터를 보내고 바로 응답을 읽기를 시도함.
		// 요청은 보내고, 처리하고, 답장하는데 시간이 걸림

		socket.wait(socket.wait_read);
		// 응답이 돌아올 때 까지 이 스레드를 블락 함

		// 서버에게 데이터를 보냈다면 서버가 뭔가를 돌려주겠지
		size_t bytes = socket.available(); // 뭔가 읽을 bytes 가 있다면
		std::cout << "Bytes Available: " << bytes << std::endl;

		// 요청과 서버에서의 지연 시간 때문에
		// 아무때나 바이트들이 응답으로 넘어올 수 있음
		// 위의 코드는 대충 뭐 읽을거 생기면 읽어라 방식이라서 모든 데이터를 받아올 수 없음

		// 네트워킹의 두가지 큰 문재점을 알 수 있음
		// 1. 언제 무언가 일어날지 모름
		//	  우리 컴퓨터를 떠나는 순간 뭐 어케 할 수 없음
		//	  모든건 시간이 걸림
		// 2. 서버가 얼마나 응답할지 모름
		//    얼마나 많은 데이터를 받기 위해 준비해야 하는지 모름
		//    버퍼 크기 어캄? 모름.

		// 코드에 wait 과 blocking statement 들을 추가하는건 아주 좋지 못한 행동임
		// 여기서 "as"io 의 asynchronous 를 확인할 타이밍임
		// https://youtu.be/2hNdkYInj4g?si=GqqoNHPYbEFxpg2I&t=919

		if (bytes > 0) {
			std::vector<char> vBuffer(bytes); // available 한 bytes 크기 만큼 vector 만듬 
			socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);
			// write_some 에서 write 를 read 로 바꾼거임 

			for (auto c : vBuffer) {
				std::cout << c; // 버퍼 표시
			}
		}
	}

	return 0;
}