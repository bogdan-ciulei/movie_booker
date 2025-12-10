#pragma once
#include <boost/asio.hpp>
#include <string>

// A syncronous TCP client modelled after the Boost.Asio examples

/**
 * @class AsioClient
 * @brief Synchronous TCP client wrapper around Boost.Asio socket operations.
 *
 * Typical usage:
 * @code
 * AsioClient c;
 * if (c.Connect("127.0.0.1", "8080")) {
 *   c.WriteLine("list_movies");
 *   std::string resp = c.ReadLine();
 * }
 * @endcode
 */
class AsioClient
{
public:
    /**
     * @brief Construct an AsioClient. The socket is initially unconnected.
     */
	AsioClient();
	
	/**
	 * @brief Destructor closes the socket if open.
	 */
	~AsioClient()
	{
		//socket_->close();
	}

	/**
	 * @brief Connect to a remote host and port (blocking).
	 * @param host Hostname or IP address (e.g. "127.0.0.1").
	 * @param port Service name or port number as string (e.g. "8080").
	 * @return true on successful connection, false on failure.
	 */
	bool Connect(const std::string& host, const std::string& port);

	/**
	 * @brief Read a single line from the connected socket.
	 *
	 * This call blocks until a '\n' is received or the connection is closed.
	 * Carriage-return characters ('\r') are stripped to support Windows servers.
	 * @return The line read (without the trailing '\n'); empty string on error/EOF.
	 */
	std::string ReadLine();

	/**
	 * @brief Write a single line to the connected socket.
	 * @param msg Message text to send (a terminating '\n' is appended by the
	 * method).
	 */
	void WriteLine(const std::string& msg);

private:
	
	boost::asio::io_context io_context_;
	boost::asio::ip::tcp::socket socket_;
};