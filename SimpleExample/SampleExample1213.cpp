#include <iostream>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

// �����Ͱ� �󸶳� ū�� �ѹ��� �� �� ������ �����ϰ� ū �������� ���۸� ����� �����Ұ���
// �׸��� ������ �б⸦ asio �� ����ϰ� �� �Լ��� ���� �����̱� ������ ���� �Լ� ������ �ۿ� ����� �� ��
std::vector<char> vBuffer(1 * 1024);
// ������ ũ�Ⱑ �����ͺ��� �۾Ƶ�
// ���� ũ�� ��ŭ ��� �о� ����
// �׸��� ũ�Ⱑ ���� ���� �۾Ƶ� ���� ���� ũ�� ��ŭ ����

void GrapSomeData(asio::ip::tcp::socket& socket) {

	// asio �� synchronous �� ����� ���� �� �Լ��� �����
	// socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);
	
	// asynchronous �� ����ϱ� ���� async_read_some �� ȣ����
	// ó���� ���� ���۸� ������. ������ ���� �Լ��� �����ؾ� ��
	// �Ʒ� �Լ��� async �̱� ������ �ٷ� ���� ���� ����
	// ��� ������ ���� �����͸� ���� �غ� �Ǿ��ٸ� �� �����͸� ���� �غ� �� 
	// �׸��� �츮�� �� ���� �����ϰ� ���Ͽ��� �����͸� �а� ���� ���� ���ۿ� �ְ�, ȭ�鿡 ����ϴ°���
	// �׷��� ���ٸ� ������
	socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
		[&](std::error_code ec, std::size_t length) { // ���� �ڵ�� read_some �Լ����� ���� �������� ���̸� ������
			if (!ec) {
				std::cout << "\n\nRead " << length << " bytes\n\n";

				for (int i = 0; i < length; ++i) {
					std::cout << vBuffer[i];
				}

				GrapSomeData(socket);
			}

			// ���� �ڵ尡 ���ٸ� �󸶳� ���� ����Ʈ�� �޾Ҵ��� ǥ���ϰ�,
			// length ��ŭ ���۸� ��ȯ�ؼ� �ֿܼ� ǥ����
			// �׸��� �ٽ� GrapSomeData �Լ��� ȣ���Ұ���
			// ��û���� ��¼�� ����Լ���� ������ �� ������
			// async_read_some �Լ��� asio context ���� �� ���� �Ѱ��ְ� �ٷ� ��ȯ��
			// �� asio �� ��׶��忡�� ���ư� ��
			// ���Ͽ� �����Ͱ� ����� ���, �츮�� ���� �Լ� �ȿ� �� �ڵ带 ������ ��
		}
	);

	// �񵿱������� �����ϸ�
	// �츮�� �ð� ������ ���� ������ �̽��� �غ���
}

int main() {

	asio::error_code ec;

	asio::io_context context;
	// �츮�� context �� ��� ���ǿ� ���� ����ؾ��Ѵٰ� ��
	// �׸��� ���õ� �ڵ带 �����ϰ� ��
	// ������ ������ ����� ���� ���α׷��� �帧�� block ��.
	// �׸��� �츮�� ����� ������ �׻� block �ϴ°� ������ ����
	// �� ������ �ذ��ϱ� ���� �츮�� �� �� �ִ� ���� ����� �� ������ �����
	// asio context �� �ڱ⸸�� �����忡�� ������ ����
	// �̷��� context �� ���� �����带 ���� �ʰ� �ڽŸ��� �ڵ���� ���� �� �ִ� ������ ��� ��

	asio::io_context::work idleWork(context);
	// asio ���� �������� �༭ context �� ������ �ʰ� ��
	
	std::thread thrContext = std::thread([&]() { context.run(); });
	// �����带 �ϳ� ����� context �� run �Լ��� ȣ����
	// run �Լ��� context �� �� ���� �� �������� �� ��ȯ��
	// �׷��� �츮�� context �� ����� ����ϴ°��� ������ ���� ������ ���� ����
	// �����̵� asio �� �̸� �ذ��ϴ� ����� ��

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
		
		// �������� �� �͵��� ���� �ʿ����


		//GrapSomeData(socket);
		// ��û�� ������ ���� GrapSomeData �Լ��� ������
		// �׸��� �Լ��� async �� ���𰡸� �б� ���� ����� ���� (context ����) �� �غ���
		// ������ �츮 ���α׷��� ���𰡸� �ϱ� ���� ��������
		// ���⼭ ���� async �� �ణ �ر򸮰� ��
		// ����� ���� �غ� ��û�� ������ ������ �ű����

		using namespace std::chrono_literals;
		std::this_thread::sleep_for(20000ms);
		// ����� ��� �����͸� �ޱ� ������ ���α׷��� ������ �ʰ� ��
	}

	return 0;
}

// ������ ���ø� ���� �񵿱������� �����Ͽ� ��� ������ �� �ִٴ°� �� �� ����
// ������ �����Ե� ������� ��
// �񵿱� ���α׷��� ����� �Ǹ� �Ϲ����� ���α׷����� ���� �ٸ��� �����ؾ� ��
// �츮�� ������ ������� ���� �귯���� �ʰ� ��
// �̰��� ���� ���α׷����� ��������
// ������ �̷��� ���� ����ǰ� ���𰡸� �غ��ص� �ٴ� ����
// �ð� ������ ���� ������ ���� �������� ����ϱ� ���� ����
// https://youtu.be/2hNdkYInj4g?si=lzSgCme9ACJL9F2i&t=1373