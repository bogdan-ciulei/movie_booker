## Description
This project is a ( test project ) backend service that will manage a list of theaters each running one or more movies. Clients can book one or more seats for the selected theater and movie.
It uses Boost.Asio for the network code.

When built, it will create 3 binaries : 
- movie_booking  ( main service  - use with a json containing the movies description )
- movie_booker_client ( interactive command line client )
- tests

## Dependencies:
- CMake ( Linux )
- Conan 2 Package Manager
- Boost.Asio ( configured with conanfile.txt )
- Gtest ( for tests, configured within conanfile.txt )
- Rapidjson ( for loading a list of movies from a json )

## JSon file:
  If none is specified , movie_booking will try to load movies.json from the current directory.  
  Example json:  
>	{  
>		"movies": [  
>		{ "title": "Movie1", "theaters": ["T1","T2"] },  
>		{ "title": "Movie2", "theaters": ["T3"] }  
>		]  
>	}
>

## Building:
1) Windows:  
   Install Conan 2 Package Manager https://docs.conan.io/2/index.html  
   Copy conanfile.txt from msvc/ to the root of the project ( this will change the build system to MSBuild ).  
   Run  
>   	conan profile detect -f  
   This should autodetect your system configuration succesfully. If it fails and compilation fails, replace the ~\.conan\profiles\default with the one from msvc\conan\profiles **making sure to edit everything according to your machine configuration**
   Run  
>		conan install . -s build_type=Debug --build=missing -of build/x64/debug/props  
>		conan install . -s build_type=Release --build=missing -of build/x64/release/props  
   This will build the dependencies and create the props files that are used by the Visual Studio projects.  
   Now you can use movie_booking.sln from the msvc folder.

2) Linux:  
   	Use Cmake and Conan 2
