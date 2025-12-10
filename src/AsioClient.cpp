#include <AsioClient.hpp>

AsioClient::AsioClient() : socket_(io_context_)
{
}

bool AsioClient::Connect(const std::string& host, const std::string& port)
{
	try
	{
		boost::asio::ip::tcp::resolver resolver(io_context_);
		boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, port);
		boost::asio::connect(socket_, endpoints);
		return true;
	}
	catch (std::exception& e)
	{
		return false;
	}
}

std::string AsioClient::ReadLine()
{
	boost::asio::streambuf buf;
	try {
		boost::asio::read_until(socket_, buf, '\n');
	}
	catch (boost::system::system_error& e) {
		return "";
	}

	std::istream is(&buf);
	std::string line;
	std::getline(is, line);
	if (!line.empty() && line.back() == '\r') // handle carriage return on Windows
		line.pop_back();
	return line;
}

void AsioClient::WriteLine(const std::string& msg)
{
	std::string out_msg = msg + "\n";
	try {
		boost::asio::write(socket_, boost::asio::buffer(out_msg));
	}
	catch (boost::system::system_error& e)
	{

	}
}
