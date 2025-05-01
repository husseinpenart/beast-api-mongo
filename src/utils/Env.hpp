#pragma once
#include <string>
#include <unordered_map>
#include <fstream>
#include <stdexcept>
#include <iostream>

class Env
{
public:
    static std::unordered_map<std::string, std::string> load(const std::string &filename, bool verbose = true)
    {
        std::unordered_map<std::string, std::string> env;
        std::ifstream file(filename);
        if (!file.is_open())
        {
            if (verbose)
            {
                std::cerr << "Warning: Failed to open .env file: " << filename << ". Falling back to empty env." << std::endl;
            }
            return env; // Return empty map instead of throwing
        }

        std::string line;
        while (std::getline(file, line))
        {
            if (line.empty() || line[0] == '#')
                continue;

            size_t pos = line.find('=');
            if (pos == std::string::npos)
                continue;

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            key.erase(0, key.find_first_not_of(" \t"));
            key.erase(key.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);

            if (!key.empty())
            {
                env[key] = value;
                if (verbose)
                {
                    std::cout << "Loaded env: " << key << "=..." << std::endl; // Hide value for security
                }
            }
        }

        file.close();
        return env;
    }

    static std::string get(const std::unordered_map<std::string, std::string> &env, const std::string &key, const std::string &default_value = "")
    {
        auto it = env.find(key);
        if (it == env.end())
        {
            std::cerr << "Warning: Environment variable not found: " << key << ". Using default: " << default_value << std::endl;
            return default_value;
        }
        return it->second;
    }
};