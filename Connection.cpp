#include "Connection.hpp"

Connection::Connection(void) {}

Connection::Connection(int a_clientSocket) : m_clientSocket(a_clientSocket) 
{
	std::cout << "New connection on fd: " << m_clientSocket << '\n';
}

Connection::Connection(const Connection &a_other)
	: m_clientSocket(a_other.m_clientSocket), m_head(a_other.m_head), m_body(a_other.m_body) {}

Connection &Connection::operator=(const Connection &a_other)
{
	if (this != &a_other)
	{
		m_clientSocket = a_other.m_clientSocket;
		m_head = a_other.m_head;
		m_body = a_other.m_body;
	}
	return (*this);
}

Connection::~Connection(void) {}

std::string	Connection::readUntilSep(std::string& a_ouput, const std::string& a_seperator)
{
	char		buffer[BUFFER_SIZE];
	std::size_t	sepPos = std::string::npos;
	std::string	remainder;
	int			recvRet = 0;

	do
	{
		recvRet = recv(m_clientSocket, buffer, BUFFER_SIZE, 0);
		if (recvRet == -1)
			throw (std::runtime_error("Error: recv bad socket?"));
		a_ouput.append(buffer, recvRet);
		sepPos = a_ouput.find(a_seperator);
		// std::cout << "Condition1: " << (recvRet == BUFFER_SIZE) << " Conditon2: " << (sepPos == std::string::npos) << '\n';
	} while (recvRet == BUFFER_SIZE && sepPos == std::string::npos);

	if (sepPos != std::string::npos)
	{
		remainder.append(a_ouput.begin() + sepPos + 4, a_ouput.end());
		a_ouput.erase(a_ouput.begin() + sepPos, a_ouput.end());
	}
	return (remainder);
}

int Connection::getSocketFd(void) const
{
	return (m_clientSocket);
}

int Connection::reciveRequestRaw(void)
{
	try
	{
		std::string remainder = readUntilSep(m_head, "\r\n\r\n");
		if (!remainder.empty())
		{
			m_body.append(remainder);
			readUntilSep(m_body, "\r\n\r\n");
		}
		m_request = Request(m_head, m_body, m_clientSocket);
		m_response = Response(m_request);
		m_response.createResponseMessage();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return (-1);
	}
	
	return (0);
}

int Connection::sendResponse(void) const
{
	const std::string&	response = m_response.getResponse();
	std::cout << "Response: " << response << '\n';
	return (send(m_clientSocket, response.data(), response.size(), 0));
}

bool Connection::operator==(const int a_fd) const
{
	return (m_clientSocket == a_fd);
}

void Connection::printHeadNBody(void) const
{
	std::cout << "Head:\n" << m_head << '\n';
	std::cout << "Body:\n" << m_body << '\n';
}