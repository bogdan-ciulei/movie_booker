#pragma once
#include <boost/asio.hpp>
#include <string>
#include <IMovieBooker.hpp>

/// Simple implementation of an async IO server that accepts movie booking commands, modelled after the Boost.Asio examples
/**
 * @file AsioServer.hpp
 * @brief Async TCP server that accepts simple text commands for movie booking.
 *
 * The server is modeled after the Boost.Asio examples and exposes a small set
 * of text commands (list_movies, select_movie, list_theaters, select_theater,
 * book_seats) which are delegated to an `IMovieBooker` implementation.
 */



/**
 * @class tcp_connection
 * @brief Per-connection handler for the Asio server.
 *
 * Instances are created for each accepted client and manage async reads and
 * writes on the socket. The connection holds a reference to an
 * `IMovieBooker` to perform queries and booking operations.
 */
class tcp_connection : public std::enable_shared_from_this<tcp_connection>
{
public:
    typedef std::shared_ptr<tcp_connection> pointer;

    ~tcp_connection();

    /**
     * @brief Create a new connection object.
     * @param io_context The io_context used for socket operations.
     * @param booker Reference to the movie booker used to service requests.
     * @return Shared pointer to the created connection.
     */
    static pointer create(boost::asio::io_context& io_context, IMovieBooker& booker);

    /**
     * @brief Access the underlying socket for accept operations.
     * @return Reference to the internal tcp socket.
     */
    boost::asio::ip::tcp::socket& socket();

    /**
     * @brief Start the connection by sending an initial prompt and scheduling reads.
     */
    void start();

private:
    tcp_connection(boost::asio::io_context& io_context_, IMovieBooker& booker);
    

    /**
     * @brief Asynchronously write a message to the client.
     * @param msg Message to send.
     */
    void write_out(const std::string&);

    /**
     * @brief Trim a trailing '\r' from a string if present.
     * @param s String to trim.
     * @return Reference to the trimmed string.
     */
    static std::string& trim_trailing(std::string&, char);

    void handle_write_end(const boost::system::error_code&, size_t);
    void handle_read_end(const boost::system::error_code&, size_t);

    IMovieBooker& booker_;
    boost::asio::ip::tcp::socket socket_;
    std::string command_buffer, out_buffer;
    std::string last_movie, last_theater;
};


/**
 * @class AsioServer
 * @brief TCP server that accepts client connections and delegates commands to an IMovieBooker.
 *
 * Construct with a reference to an `IMovieBooker` implementation. Call `Run()`
 * to start the server's event loop. `Stop()` will stop the io_context.
 */
class AsioServer
{
public:
    AsioServer() = delete;

    /**
     * @brief Construct the server.
     * @param booker Reference to an IMovieBooker used to service requests.
     */
    AsioServer(IMovieBooker &booker);

    ~AsioServer();

    /**
     * @brief Run the server event loop (blocks until stopped).
     */
    void Run();

    /**
     * @brief Stop the server's io_context and return from Run().
     */
    void Stop();

private:
    void start_accept();
    void handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error);

	std::vector<tcp_connection::pointer> connections;
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor;
    IMovieBooker& booker_;
    bool run_once;
};


