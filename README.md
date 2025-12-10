## Description
This project is a ( test project ) backend service that will manage a list of theaters each running one or more movies. Clients can book one or more seats for the selected theater and movie.
It uses Boost.Asio for the network code.

When built, it will create 3 binaries : 
- movie_booking  ( main service  - use with a json containing the movies description )
- movie_booker_client ( interactive command line client )
- tests

## Dependencies:
- CMake ( Linux )
- Conan Package Manager
- Boost.Asio ( configured with conanfile.txt )
- Gtest ( for tests, configured within conanfile.txt )
- Rapidjson ( for loading a list of movies from a json )

## JSon file:
  If none is specified , movie_booking will try to load movies.json from the current directory
  Example json:
	{
		"movies": [
		{ "title": "Movie1", "theaters": ["T1","T2"] },
		{ "title": "Movie2", "theaters": ["T3"] }
		]
	}

## Building:
