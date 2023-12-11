#include <iostream>

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
// ������ ���� ���� ���� ��鸵�� �ٸ��� ������
// �̸� ���Ǹ� ����
// �⺻���� �ֱ� ��
#endif

#define ASIO_STANDALONE // �ν�Ʈ ���̺귯�� �� ��

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
// �޸� ��鸵
#include <asio/ts/internet.hpp>
// ��Ʈ��ũ ����� ���� �͵��� ����

int main() {

	asio::error_code ec;

	asio::io_context context;
	// unique instance of asio
	// OS �������� �͵��� �� �ȿ��� ó����

	// ��� ����ǰ� ���� �ּҸ� ������
	// �̸����� �˴ٽ��� asio �� ��Ʈ��ũ�� �������� �ʰ� ���� ������� ������
	// ����� �� �ִ� ���𰡵��� endpoint ��� ��
	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), 80);
	// tcp ��Ÿ���� endpoint �� ����
	// ip �ּҿ� port �� endpoint �� ���ǵ�
	// make_address �Լ��� �̿��� IP ���ڿ��� asio �� �˾Ƶ��� �� �ִ� ���𰡷� �ٲ�
	// ���� �߸��� ���� ����ؼ� �����ڵ� ������ ������
	
	// OS �� ��Ʈ��ũ ����̹��� ��
	// �츮�� ������ ��Ʈ��ũ�� ��η� ���� ��
	asio::ip::tcp::socket socket(context); // context �� socket �� ������ ������ ��
	
	// ���Ͽ��� endpoint �� �����϶�� ��
	socket.connect(endpoint, ec);

	if (!ec) {
		std::cout << "Hello Internet!" << std::endl;
	}
	else {
		std::cout << "Failed to connect to address:\n" << ec.message() << std::endl;
		// error_code ������ ��Ÿ�ӿ� ���� ������ �߻��ߴ��� �˾ƺ� �� ����
		// ���� �޼����� �������̱� ������ ������
	}

	if (socket.is_open()) {
		// ����ִ� Ŀ�ؼ�
		// ������Ʈ�� �����Ϸ��� �ϱ� ������, ������ http request �� ��ٸ��� ����

		std::string sRequest =
			"GET /index.html HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Connection: close\r\n\r\n";

		socket.write_some(asio::buffer(sRequest.data(), sRequest.size()), ec);
		// write_some �Լ��� �� �����͸� ������ ��ŭ �����޶�� �ǹ�
		// asio ���� ������ ������� �۾��� ��, asio buffer �� �����
		// �׳� byte �迭��. (���ڿ��� byte ��� ���ڿ� ũ��)

		// ���� ������ �������� �����͸� ������ �ٷ� ������ �б⸦ �õ���.
		// ��û�� ������, ó���ϰ�, �����ϴµ� �ð��� �ɸ�

		socket.wait(socket.wait_read);
		// ������ ���ƿ� �� ���� �� �����带 ��� ��

		// �������� �����͸� ���´ٸ� ������ ������ �����ְ���
		size_t bytes = socket.available(); // ���� ���� bytes �� �ִٸ�
		std::cout << "Bytes Available: " << bytes << std::endl;

		// ��û�� ���������� ���� �ð� ������
		// �ƹ����� ����Ʈ���� �������� �Ѿ�� �� ����
		// ���� �ڵ�� ���� �� ������ ����� �о�� ����̶� ��� �����͸� �޾ƿ� �� ����

		// ��Ʈ��ŷ�� �ΰ��� ū �������� �� �� ����
		// 1. ���� ���� �Ͼ�� ��
		//	  �츮 ��ǻ�͸� ������ ���� �� ���� �� �� ����
		//	  ���� �ð��� �ɸ�
		// 2. ������ �󸶳� �������� ��
		//    �󸶳� ���� �����͸� �ޱ� ���� �غ��ؾ� �ϴ��� ��
		//    ���� ũ�� ��į? ��.

		// �ڵ忡 wait �� blocking statement ���� �߰��ϴ°� ���� ���� ���� �ൿ��
		// ���⼭ "as"io �� asynchronous �� Ȯ���� Ÿ�̹���
		// https://youtu.be/2hNdkYInj4g?si=GqqoNHPYbEFxpg2I&t=919

		if (bytes > 0) {
			std::vector<char> vBuffer(bytes); // available �� bytes ũ�� ��ŭ vector ���� 
			socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);
			// write_some ���� write �� read �� �ٲ۰��� 

			for (auto c : vBuffer) {
				std::cout << c; // ���� ǥ��
			}
		}
	}

	return 0;
}