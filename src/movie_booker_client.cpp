// movie_booker_client.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <AsioClient.hpp>

int main()
{
    AsioClient client;
    if (!client.Connect("127.0.0.1", "8080")) {
        std::cerr << "Failed to connect to server 127.0.0.1:8080\n";
        return 1;
    }

    // Read initial server greeting (if any)
    std::string serverLine = client.ReadLine();
    if (!serverLine.empty())
        std::cout << serverLine << std::endl;

    std::cout << "Type commands (type 'quit' or 'exit' to close):" << std::endl;
    while (true) 
    {
        std::cout << "> ";
        std::string input;
        if (!std::getline(std::cin, input))
            break; // EOF or error on stdin

        if (input == "quit" || input == "exit")
            break;

        if (input.empty())
            continue;

        client.WriteLine(input);

        // Read single-line response from server
        std::string resp = client.ReadLine();
        if (resp.empty())
            std::cout << "(no response)" << std::endl;
        else
            std::cout << resp << std::endl;
    }

    return 0;
}

