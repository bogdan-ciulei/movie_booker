#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <boost/asio.hpp>
#include "AsioServer.hpp"
#include "IMovieBooker.hpp"
#include <thread>
#include <chrono>
#include <random>

using ::testing::Return;
using boost::asio::ip::tcp;

// Mock implementation of IMovieBooker used to control backend behavior in tests.
class MockMovieBooker : public IMovieBooker
{
public:
    MOCK_METHOD(bool, AddMovie, (const std::string&, const std::vector<std::string>&), (override));
    MOCK_METHOD(std::vector<std::string>, GetMovies, (), (override));
    MOCK_METHOD(std::vector<std::string>, GetTheatersForMovie, (const std::string&), (override));
    MOCK_METHOD(std::vector<unsigned int>, GetFreeSeats, (const std::string&, const std::string&), (override));
    MOCK_METHOD(bool, BookSeats, (const std::string&, const std::string&, const std::vector<unsigned int>&), (override));
    MOCK_METHOD(bool, IsTheater, (const std::string&), (override));
};

// Small helper that trims a trailing carriage return from a string (\r).
static std::string trim_cr(const std::string& s)
{
    if (!s.empty() && s.back() == '\r')
        return s.substr(0, s.size() - 1);
    return s;
}

static void connect_to_localhost(boost::asio::io_context& io, boost::asio::ip::tcp::socket& sock, const std::string& port)
{
    boost::asio::ip::tcp::resolver resolver(io);
    auto endpoints = resolver.resolve("127.0.0.1", port);
    boost::asio::connect(sock, endpoints);
}

// Helper to start the AsioServer on a background thread so tests can connect a client socket.
static std::thread start_server(AsioServer* server)
{
    return std::thread([server]() {
        server->Run();
    });
}

// Generate a random ephemeral port in the range 20000-40000 to avoid collisions
static unsigned short random_port()
{
    static std::mt19937 rng((std::random_device())());
    std::uniform_int_distribution<int> dist(20000, 40000);
    return static_cast<unsigned short>(dist(rng));
}

// Test: when a client connects, the server immediately sends an initial prompt line.
TEST(AsioServerTest, InitialPromptIsSentOnConnect)
{
    MockMovieBooker mock;
    unsigned short port = random_port();
    AsioServer server(mock, port);

    auto thr = start_server(&server);

    // Give server a moment to start and bind
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    boost::asio::io_context io;
    tcp::socket sock(io);
    connect_to_localhost(io, sock, std::to_string(server.GetPort()));


    boost::asio::streambuf buf;
    boost::asio::read_until(sock, buf, '\n');
    std::istream is(&buf);
    std::string line;
    std::getline(is, line);
    line = trim_cr(line);

    // Server prompt contains this substring
    EXPECT_NE(line.find("Hello! Input command("), std::string::npos);

    server.Stop();
    thr.join();
}

// Test: sending the "list_movies" command invokes GetMovies() on the backend and returns the movie list.
TEST(AsioServerTest, ListMoviesCommandCallsBackendAndReturnsList)
{
    MockMovieBooker mock;
    // include a movie name containing a space to test handling of names with whitespace
    EXPECT_CALL(mock, GetMovies()).WillOnce(Return(std::vector<std::string>{"Movie A", "MovieB"}));

    unsigned short port = random_port();
    AsioServer server(mock, port);
    auto thr = start_server(&server);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    boost::asio::io_context io;
    tcp::socket sock(io);
    connect_to_localhost(io, sock, std::to_string(server.GetPort()));

    // read initial prompt
    boost::asio::streambuf buf;
    boost::asio::read_until(sock, buf, '\n');
    std::istream is1(&buf);
    std::string prompt;
    std::getline(is1, prompt);

    // send command
    std::string cmd = "list_movies\n";
    boost::asio::write(sock, boost::asio::buffer(cmd));

    // read response line
    boost::asio::streambuf buf2;
    boost::asio::read_until(sock, buf2, '\n');
    std::istream is2(&buf2);
    std::string response;
    std::getline(is2, response);
    response = trim_cr(response);

    // The server concatenates movie names with commas, expect both movie names present
    EXPECT_NE(response.find("Movie A"), std::string::npos);
    EXPECT_NE(response.find("MovieB"), std::string::npos);

    server.Stop();
    thr.join();
}

// Test: sending an unknown command returns an error message starting with "Error!"
TEST(AsioServerTest, UnknownCommandReturnsError)
{
    MockMovieBooker mock;
    unsigned short port = random_port();
    AsioServer server(mock, port);
    auto thr = start_server(&server);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    boost::asio::io_context io;
    tcp::socket sock(io);
    connect_to_localhost(io, sock, std::to_string(server.GetPort()));

    // read initial prompt
    boost::asio::streambuf buf;
    boost::asio::read_until(sock, buf, '\n');
    std::istream is(&buf);
    std::string prompt;
    std::getline(is, prompt);

    // send an unrecognized command
    std::string cmd = "this_command_does_not_exist\n";
    boost::asio::write(sock, boost::asio::buffer(cmd));

    // read response and verify it starts with "Error!"
    boost::asio::streambuf buf2;
    boost::asio::read_until(sock, buf2, '\n');
    std::istream is2(&buf2);
    std::string response;
    std::getline(is2, response);
    response = trim_cr(response);

    EXPECT_EQ(response.find("Error!"), 0u);

    server.Stop();
    thr.join();
}

// Test: attempting to select an invalid movie returns an error message starting with "Error!"
TEST(AsioServerTest, SelectMovieInvalidReturnsError)
{
    MockMovieBooker mock;

    // Expect the backend to be queried for the movie and return empty list to indicate not found
    EXPECT_CALL(mock, GetTheatersForMovie("NoSuchMovie")).WillOnce(Return(std::vector<std::string>{}));

    // No further expectation needed: server should respond with an error when movie not provided/recognized
    unsigned short port = random_port();
    AsioServer server(mock, port);
    auto thr = start_server(&server);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    boost::asio::io_context io;
    tcp::socket sock(io);
    connect_to_localhost(io, sock, std::to_string(server.GetPort()));

    // read initial prompt
    boost::asio::streambuf buf;
    boost::asio::read_until(sock, buf, '\n');
    std::istream is(&buf);
    std::string prompt;
    std::getline(is, prompt);

    // send select_movie with an invalid movie name
    std::string cmd = "select_movie NoSuchMovie\n";
    boost::asio::write(sock, boost::asio::buffer(cmd));

    // read response and verify it starts with "Error!"
    boost::asio::streambuf buf2;
    boost::asio::read_until(sock, buf2, '\n');
    std::istream is2(&buf2);
    std::string response;
    std::getline(is2, response);
    response = trim_cr(response);

    EXPECT_EQ(response.find("Error!"), 0u);

    server.Stop();
    thr.join();
}

// Test: booking seats and then listing free seats reflects the booking
TEST(AsioServerTest, BookSeatsAndListFreeSeats)
{
    MockMovieBooker mock;

    // Enforce call order: select_movie -> select_theater -> book_seats -> get_free_seats
    ::testing::InSequence seq;
    EXPECT_CALL(mock, GetTheatersForMovie("MovieX")).WillOnce(Return(std::vector<std::string>{"Theater1"}));
    EXPECT_CALL(mock, IsTheater("Theater1")).WillOnce(Return(true));
    EXPECT_CALL(mock, BookSeats("Theater1", "MovieX", ::testing::ElementsAre(5u, 6u))).WillOnce(Return(true));

    // After booking, GetFreeSeats should not include 5 and 6
    std::vector<unsigned int> freeSeats;
    for (unsigned int i = 1; i <= 20; ++i)
        if (i != 5 && i != 6)
            freeSeats.push_back(i);
    EXPECT_CALL(mock, GetFreeSeats("Theater1", "MovieX")).WillOnce(Return(freeSeats));

    unsigned short port = random_port();
    AsioServer server(mock, port);
    auto thr = start_server(&server);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    boost::asio::io_context io;
    tcp::socket sock(io);
    connect_to_localhost(io, sock, std::to_string(server.GetPort()));

    // consume initial prompt
    boost::asio::streambuf buf;
    boost::asio::read_until(sock, buf, '\n');
    std::istream is0(&buf);
    std::string initial;
    std::getline(is0, initial);

    // select movie
    boost::asio::write(sock, boost::asio::buffer(std::string("select_movie MovieX\n")));
    boost::asio::streambuf r1;
    boost::asio::read_until(sock, r1, '\n');
    std::istream is1(&r1);
    std::string resp1; std::getline(is1, resp1);
    resp1 = trim_cr(resp1);
    EXPECT_NE(resp1.find("Movie MovieX selected"), std::string::npos);

    // select theater
    boost::asio::write(sock, boost::asio::buffer(std::string("select_theater Theater1\n")));
    boost::asio::streambuf r2;
    boost::asio::read_until(sock, r2, '\n');
    std::istream is2(&r2);
    std::string resp2; std::getline(is2, resp2);
    resp2 = trim_cr(resp2);
    EXPECT_NE(resp2.find("Theater Theater1 selected"), std::string::npos);

    // book seats 5 and 6
    boost::asio::write(sock, boost::asio::buffer(std::string("book_seats 5,6\n")));
    boost::asio::streambuf r3;
    boost::asio::read_until(sock, r3, '\n');
    std::istream is3(&r3);
    std::string resp3; std::getline(is3, resp3);
    resp3 = trim_cr(resp3);
    EXPECT_NE(resp3.find("Seats booked successfully"), std::string::npos);

    // request free seats
    boost::asio::write(sock, boost::asio::buffer(std::string("get_free_seats\n")));
    boost::asio::streambuf r4;
    boost::asio::read_until(sock, r4, '\n');
    std::istream is4(&r4);
    std::string resp4;
    std::getline(is4, resp4);
    resp4 = trim_cr(resp4);

    // response should not contain booked seats 5 and 6, but contain some other seat (e.g., 7)
    EXPECT_EQ(resp4.find(",5,"), std::string::npos);
    EXPECT_EQ(resp4.find(",6,"), std::string::npos);
    EXPECT_EQ(resp4.find(",7,"), 7);

    server.Stop();
    thr.join();
}

// Test: selecting a theater calls IsTheater on the backend and returns selection confirmation
TEST(AsioServerTest, SelectTheaterCallsIsTheaterAndReturnsSelected)
{
    MockMovieBooker mock;

    // Expect IsTheater to be called and return true indicating theater exists
    EXPECT_CALL(mock, IsTheater("TheaterExist")).WillOnce(Return(true));

    unsigned short port = random_port();
    AsioServer server(mock, port);
    auto thr = start_server(&server);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    boost::asio::io_context io;
    tcp::socket sock(io);
    connect_to_localhost(io, sock, std::to_string(server.GetPort()));

    // consume initial prompt
    boost::asio::streambuf buf;
    boost::asio::read_until(sock, buf, '\n');
    std::istream is(&buf);
    std::string prompt;
    std::getline(is, prompt);

    // send select_theater command for an existing theater
    std::string cmd = "select_theater TheaterExist\n";
    boost::asio::write(sock, boost::asio::buffer(cmd));

    // read response
    boost::asio::streambuf resp_buf;
    boost::asio::read_until(sock, resp_buf, '\n');
    std::istream is2(&resp_buf);
    std::string response;
    std::getline(is2, response);
    response = trim_cr(response);

    EXPECT_NE(response.find("Theater TheaterExist selected"), std::string::npos);

    server.Stop();
    thr.join();
}
