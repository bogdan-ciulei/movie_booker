#include <AsioServer.hpp>
#include <iostream>
#include <functional>


using boost::asio::ip::tcp;

constexpr char list_message[] = "Hello! Input command(\"list_movies\", \"select_movie <name>\", \"list_theaters\", \"select_theater <name>\", \"get_free_seats\", \"book_seat <s1,s2,..>\")\n\n";
constexpr char invalid_cmd_message[] = "Error! Enter a valid command\n";

tcp_connection::pointer tcp_connection::create(boost::asio::io_context& io_context_, IMovieBooker& booker)
{
    return tcp_connection::pointer(new tcp_connection(io_context_, booker));
}

tcp_connection::~tcp_connection()
{
	socket_.close();
}

tcp::socket& tcp_connection::socket() 
{
    return socket_;
}

 void tcp_connection::start()
 {
    write_out(std::string(list_message));
 }


tcp_connection::tcp_connection(boost::asio::io_context &io_context_, IMovieBooker &booker)
        : socket_(io_context_), booker_(booker)
{
    command_buffer.reserve(1024);
	out_buffer.reserve(1024);
}

void tcp_connection::handle_write_end(const boost::system::error_code& error, size_t size)
{
    if (!error)
    {
       boost::asio::async_read_until(socket_,
            boost::asio::dynamic_buffer(command_buffer), '\n',
            std::bind(&tcp_connection::handle_read_end, shared_from_this(),
            boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
	}
    else
	    socket_.close();
}

void tcp_connection::handle_read_end(const boost::system::error_code& error, std::size_t sz)
{
	if (!error)
	{
		size_t sz = command_buffer.find("\n");
		std::stringstream stream(command_buffer, std::ios::in);
		std::string command;

		if (stream >> command)
		{
			trim_trailing(command, '\r');                      // windows helper
			trim_trailing(command, ' ');                    
			// transform to lower case
			std::transform(command.begin(), command.end(), command.begin(),
				[](unsigned char c) {
				return static_cast<char>(std::tolower(c));
			});
			if (command == "list_movies")
			{
				auto movies = booker_.GetMovies();
				std::string list;
				for (auto t : movies)
					list += t + ",";
				if (list.empty())
					list = "No movies running";
				list += "\n";
				write_out(list);
			}
			else if (command == "select_movie")
			{
				stream >> std::ws;			// skip any whitespace in stream
				std::getline(stream, last_movie);
				trim_trailing(last_movie, '\r');
				trim_trailing(last_movie, ' ');
				if ( booker_.GetTheatersForMovie(last_movie).empty() )
				{
					write_out("Error! Select a valid movie\n");
					last_movie = "";
				}
				else
				{
					last_theater = "";                  // reset last selected theater
					write_out("Movie " + last_movie + " selected\n");
				}
			}
			else if (command == "list_theaters")
			{
				if (last_movie.size())
				{
					auto theaters = booker_.GetTheatersForMovie(last_movie);
					std::string list;
					for (auto t : theaters)
						list += t + ",";
					if (list.empty())
						list = "Movie is not running in any theater";
					list += "\n";
					write_out(list);
				}
				else
					write_out("Error! No valid movie selected\n");
			}
			else if (command == "select_theater")
			{
				stream >> std::ws;			// skip any whitespace in stream
				std::getline(stream, last_theater);
				trim_trailing(last_theater, '\r');
				trim_trailing(last_theater, ' ');
			
				if (!booker_.IsTheater(last_theater))
				{
					write_out("Error! Select a valid theater");
					last_theater = "";
				}
				else
					write_out("Theater " + last_theater + " selected\n");
			}
			else if (command == "get_free_seats")
			{
				if (!last_movie.size())
				{
					write_out("Error! No valid movie selected\n");
				}
				else if (!last_theater.size())
				{
					write_out("Error! No valid theater selected\n");
				}
				else
				{
					std::vector<unsigned int> free_seats = booker_.GetFreeSeats(last_theater, last_movie);
					std::string list;
					for ( auto seat : free_seats )
						list += std::to_string(seat) + ",";
					list += "\n";
					write_out(list);
				}
			}
			else if (command == "book_seats")
			{
				if (!last_theater.size())
				{
					write_out("Error! No valid theater selected\n");
				}
				if (!last_movie.size())
				{
					write_out("Error! No valid movie selected\n");
				}
				else
				{
					int seat = 0;
					std::vector<unsigned int> seats;
					stream >> std::ws; // skip any whitespace before seats
					while (stream >> seat)
					{
						if (seat < 1 || seat > 20)
						{
							write_out("Error! Seats not in range 1-20\n");
							seats.clear();
							break;
						}
						seats.push_back(seat);
						char c = stream.peek();
						if (c == ',')
							stream.ignore(1);
						else
							break;
					}

					if (seats.empty())
					{
						write_out("Error! No valid seats specified\n");
					}
					else if (seats.size() > 20)
					{
						write_out("Error! Too many seats requested; request 1 to 20 unique seat ids\n");
					}
					else
					{
						if (booker_.BookSeats(last_theater, last_movie, seats))
						 write_out("Seats booked successfully\n");
						else
						 write_out("Error! Could not book seats\n");
					}
				}
			}
			else
			 write_out(invalid_cmd_message);
		}
		else
		{
			write_out(invalid_cmd_message);
		}
		command_buffer.erase(0, sz + 1); // remove processed command
	}
	else
		socket_.close();
}

// helper for responding to client
void tcp_connection::write_out(const std::string& msg)
{
	out_buffer = msg;
	boost::asio::async_write(socket_, boost::asio::buffer(out_buffer),
		std::bind(&tcp_connection::handle_write_end, shared_from_this(),
			boost::asio::placeholders::error,
			boost::asio::placeholders::bytes_transferred));
}

// helper on Windows in case carriage return is present before newline
std::string& tcp_connection::trim_trailing( std::string& str , char c)
{
	if ( !str.empty() )
		while (str.back() == c)
			str.pop_back();
	return str;
}



AsioServer::AsioServer(IMovieBooker& booker, unsigned short port) : booker_(booker), acceptor(io_context_), run_once(false), port_(port)
{
    // bind to the requested port (0 -> ephemeral)
    acceptor.open(tcp::v4());
    acceptor.set_option(boost::asio::socket_base::reuse_address(true));
    acceptor.bind(tcp::endpoint(tcp::v4(), port_));
    // update port_ in case ephemeral port was requested (port == 0)
    port_ = acceptor.local_endpoint().port();
    acceptor.listen();
}

AsioServer::~AsioServer()
{
    Stop();
	connections.clear();
}


void AsioServer::start_accept()
{
    //tcp_connection::pointer new_connection = tcp_connection::create(io_context_, booker_);
	connections.emplace_back(tcp_connection::create(io_context_, booker_));

    acceptor.async_accept(connections.back()->socket(), std::bind(&AsioServer::handle_accept, this, connections.back(),
            boost::asio::placeholders::error));
}

void AsioServer::handle_accept(tcp_connection::pointer new_connection, const boost::system::error_code& error)
{
    if (!error)
    {
        new_connection->start();
    }

    start_accept();
}

void AsioServer::Run()
{
    start_accept();
    if (run_once)
        io_context_.restart();
	io_context_.run();
	run_once = true;
}

void AsioServer::Stop()
{
    io_context_.stop();
}

unsigned short AsioServer::GetPort() const
{
    return port_;
}