// movie_booking.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <string>
#include <MovieBooker.hpp>
#include <AsioServer.hpp>

// RapidJSON headers
#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include <filesystem>
#include <cstdlib>

#ifdef _WIN32
    #include <direct.h>
    #define getcwd _getcwd
    #define PATHSEP "\\"
#else
	#include <unistd.h>
    #define PATHSEP "/"
#endif

// Simple loader: expects a JSON file with structure:
// {
//   "movies": [
//     { "title": "Movie1", "theaters": ["T1","T2"] },
//     { "title": "Movie2", "theaters": ["T3"] }
//   ]
// }

class MovieDataLoader 
{
public:
    static bool LoadFromFile(const std::string& path, MovieBooker& booker) 
    {
        std::ifstream ifs;

		ifs.open(path);

        if (!ifs.is_open()) 
        {
            return false;
        }

        rapidjson::IStreamWrapper isw(ifs);
        rapidjson::Document doc;
        doc.ParseStream(isw);
        if (doc.HasParseError() || 
            !doc.IsObject()) 
        {
            return false;
        }

        if (!doc.HasMember("movies") || !doc["movies"].IsArray()) return false;

        for (const auto& mv : doc["movies"].GetArray()) 
        {
            if (!mv.IsObject()) 
                continue;
            if (!mv.HasMember("title") || !mv["title"].IsString()) 
                continue;
            std::string title = mv["title"].GetString();

            std::vector<std::string> theaters;
            if (mv.HasMember("theaters") && mv["theaters"].IsArray()) 
            {
                for (const auto& th : mv["theaters"].GetArray()) 
                    if (th.IsString()) 
                        theaters.emplace_back(th.GetString());
            }

            if (!theaters.empty()) {
                booker.AddMovie(title, theaters);
            }
        }

        return true;
    }
};

int main(int argc, char** argv)
{
    // determine data file path; if none provided, use movies.json in current directory

	std::string dataFile;

    if (argc > 1 && argv[1] != nullptr && argv[1][0] != '\0')
        dataFile = argv[1];
    else
        dataFile = "movies.json";

    MovieBooker booker;

    if (!MovieDataLoader::LoadFromFile(dataFile, booker)) 
    {
        std::cerr << "Warning: failed to load movie data from '" << dataFile << "'. Starting with empty catalog.\n";
    }

    try {
        AsioServer server(booker);
        std::cout << "Starting AsioServer on port 8080...\n";
        server.Run();
    }
    catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << "\n";
        return 2;
    }

    return 0;
}
