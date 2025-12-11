## Description
This project is a ( test project ) backend service that will manage a list of theaters each running one or more movies. Clients can book one or more seats for the selected theater and movie.
It uses Boost.Asio for the network code.

When built, it will create 3 binaries : 
- movie_booker  ( main service  - use with a json containing the movies description )
- movie_booker_client ( interactive command line client )
- movie_booker_tests

## Dependencies:
- CMake
- Conan 2 Package Manager
- Visual Studio 2022 ( Windows )
- gcc or other modern compiler ( Linux )
- Boost.Asio ( configured with conanfile.txt )
- Gtest ( for tests, configured within conanfile.txt )
- Rapidjson ( for loading a list of movies from a json )

## JSon file:
  If none is specified , movie_booking will try to load movies.json from the current directory.  
  Example json:  
>		{  
>			"movies": [  
>			{ "title": "Movie1", "theaters": ["T1","T2"] },  
>			{ "title": "Movie2", "theaters": ["T3"] }  
>			]  
>		}
>

## Building:
1) Windows:  
   Install Conan 2 Package Manager https://docs.conan.io/2/index.html  
   Copy conanfile.txt from msvc/ to the root of the project ( this will change the build system to MSBuild ).  
   Run  
>		conan profile detect -f  
   This should autodetect your system configuration succesfully. If it fails and compilation fails, replace the %USERPROFILE%\.conan\profiles\default with the one from msvc\conan\profiles **making sure to edit everything according to your machine configuration**.  
   Run  
>		conan install . -s build_type=Debug --build=missing -of build\x64\debug\props  
>		conan install . -s build_type=Release --build=missing -of build\x64\release\props  
   This will build the dependencies and create the props files that are used by the Visual Studio projects.  
   Now you can use movie_booking.sln from the msvc folder.

2) Linux:  
   	Use Cmake and Conan 2  
	Similar steps with Windows except that you don't need to replace any files.  
	Run  
>		conan install . --output-folder=build --build=missing  
>		cd build  
>		cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release  
>		cmake --build .  
		
3)	Both:
	Copy movies.json from project root to output folder for testing
	
## Code description  

IMovieBooker.hpp - interface for a bookings manager  
MovieBooker.cpp and MovieBooker.hpp  - actual implementation of that interface  
AsioServer.cpp and AsioServer.hpp - service network server , uses Boost.Asio to create an async TCP server  
MovieBookerMain.cpp - main for the service, also code to populate movies from a json file  
AsioClient.cpp and AsioClient.hpp - network TCP client using Boost.Asio , synchronous  
movie_booker_client.cpp - command line client main  
tests.cpp - gtest tests  

	
