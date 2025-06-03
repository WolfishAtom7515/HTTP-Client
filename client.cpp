#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>

#include <string>
#include <iostream>
#include <vector>

#include "helper.hpp"
#include "requests.hpp"
#include "json.hpp"

#define PORT 8081
#define HOST "63.32.125.183"


/* ================  < Auxiliary functions >  =============== */

int get_id(const std::string response)
{
    std::string ext_response = basic_extract_json_response((char *)response.c_str());
    nlohmann::json body_response = nlohmann::json::parse(ext_response);

    // save the id of the collection
    int token = body_response["id"];

    return token;
}

void print_collection(const std::string response)
{
    std::string ext_response = basic_extract_json_response((char *)response.c_str());
    nlohmann::json body_response = nlohmann::json::parse(ext_response);

    std::string title = body_response["title"];
    std::string owner = body_response["owner"];

    std::cout << "title: " << title << '\n';
    std::cout << "owner: " << owner << '\n';

    auto movies = body_response["movies"];

    // print the movies in the collection
    for (auto movie : movies)
    {
        std::string title = movie["title"];
        std::cout << "#" << movie["id"] << ": " << title << '\n';
    }
}

void print_movie(const std::string response)
{
    std::string ext_response = basic_extract_json_response((char *)response.c_str());
    nlohmann::json body_response = nlohmann::json::parse(ext_response);

    std::string title = body_response["title"];
    std::string description = body_response["description"];
    std::string year = body_response["year"].dump();
    std::string rating = body_response["rating"];

    // print the movie details
    std::cout << "title: " << title << '\n';
    std::cout << "year: " << std::stoi(year) << '\n';
    std::cout << "description: " << description << '\n';
    std::cout << "rating: " << std::stod(rating) << '\n';
}

std::string get_JWT(const std::string response)
{
    std::string ext_response = basic_extract_json_response((char *)response.c_str());
    nlohmann::json body_response = nlohmann::json::parse(ext_response);

    // save the JWT token
    std::string token = body_response["token"];
    return token;
}

std::string get_cookiee(const std::string response)
{
    int text_until_cookiee = response.find("Set-Cookie: ");
    int text_end_cookiee = response.find(";", text_until_cookiee + 12);

    // return the cookiee
    return response.substr(text_until_cookiee + 12, text_end_cookiee - text_until_cookiee - 12);
}

std::string add_movie_to_collection_index(int sockfd, int id_collection, int id_movie, std::string &JWT_token)
{
    // check if the collection exists
    std::string url = "/api/v1/tema/library/collections/" + std::to_string(id_collection) + "/movies";

    nlohmann::ordered_json collection_format;
    collection_format["id"] = id_movie;

    std::string collection_format_dump = collection_format.dump(4);
    char *collection_format_content = (char *)collection_format_dump.c_str();
    char *JWT_content = (char *)JWT_token.c_str();

    std::string message = compute_post_request((char *)HOST, (char *)url.c_str(),
                                                (char *)"application/json", &collection_format_content, 1, NULL, 0, JWT_content);
 
    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // return response
    return response;
}

bool get_code(const std::string response, const std::string posible_output)
{
    std::string ext_response = basic_extract_json_response((char *)response.c_str());
    nlohmann::json body_response = nlohmann::json::parse(ext_response);

    std::istringstream aux(response);
    std::string text;
    int code;

    aux >> text >> code >> text;

    // check if the response is valid
    if (code / 100 == 2)
    {
        std::cout << "SUCCESS: ";

        // check if the response contains a message
        if(body_response.contains("message"))
        {
            std::string message = body_response["message"];
            std::cout << message << '\n';
        } 
        else
        {
            std::cout << posible_output << '\n';
        }

        return true;
    }
    else
    {
        std::cout << "ERROR: ";

        // check if the response contains a error
        if(body_response.contains("error"))
        {
            std::string message = body_response["error"];
            std::cout << message << '\n';
        }

        return false;
    }    
}

/* ================  < Admin functions >  =============== */

void login_admin(int sockfd, std::string &cookiee, bool &is_admin)
{
    nlohmann::ordered_json credentials;
    std::string password, username;

    // check if user is logged in
    if (cookiee != "no cookiee")
    {   
        if (is_admin)
            std::cout << "Admin already log in.\n";
        else 
            std::cout << "User already log in.\n";
        return; 
    }

    std::cout << "username=";
    std::getline(std::cin, username);
    credentials["username"] = username;

    std::cout << "password=";
    std::getline(std::cin, password);
    credentials["password"] = password;

    std::string credentials_dump = credentials.dump(4);
    char *credentials_content = (char *)credentials_dump.c_str();    

    std::string message = compute_post_request((char *)HOST,(char *)"/api/v1/tema/admin/login",
                                                (char *)"application/json",&credentials_content, 1, NULL, 0, NULL);

    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // print response and reset cookiee & is_admin
    if (get_code(response, ""))
    {
        cookiee = get_cookiee(response);
        is_admin = true;
    }

}

void add_user(int sockfd, std::string &cookiee, bool &is_admin)
{
    nlohmann::ordered_json credentials;
    std::string password, username;

    // check if user is logged in and it is admin
    if (is_admin == false)
    {
        std::cout << "You are not admin.\n";
        return; 
    }

    std::cout << "username=";
    std::getline(std::cin, username);
    credentials["username"] = username;

    std::cout << "password=";
    std::getline(std::cin, password);
    credentials["password"] = password;

    std::string credentials_dump = credentials.dump(4);
    char *credentials_content = (char *)credentials_dump.c_str();    

    char *cookiee_content = (char *)cookiee.c_str();

    std::string message = compute_post_request((char *)HOST, (char *)"/api/v1/tema/admin/users",
                                                (char *)"application/json", &credentials_content, 1, &cookiee_content, 1, NULL);

    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // print response
    get_code(response, "");
}

void get_users(int sockfd, std::string &cookiee, bool &is_admin)
{
    // check if user is logged in and it is admin
    if (is_admin == false)
    {
        std::cout << "You are not admin.\n";
        return; 
    }

    char *cookiee_content = (char *)cookiee.c_str();

    std::string message = compute_get_request((char *)HOST, (char *)"/api/v1/tema/admin/users",
                                                NULL, &cookiee_content, 1, NULL);

    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // print response
    get_code(response, "List of users");

    std::string ext_response = basic_extract_json_response((char *)response.c_str());
    nlohmann::json body_response = nlohmann::json::parse(ext_response);

    nlohmann::ordered_json users = body_response["users"];

    // print the rest of the response
    for (auto user : users)
    {
        std::string username = user["username"];
        std::string password = user["password"];
        std::cout << "#" << user["id"] << " " << username << ":" << password << '\n';
    }  
}

void delete_user(int sockfd, std::string &cookiee, bool &is_admin)
{
    std::string username;

    // check if user is logged in and it is admin
    if (is_admin == false)
    {
        std::cout << "You are not admin.\n";
        return; 
    }

    std::cout << "username=";
    std::getline(std::cin, username);

    // check if the user exists
    char *cookiee_content = (char *)cookiee.c_str();
    std::string url = "/api/v1/tema/admin/users/" + username;

    std::string message = compute_delete_request((char *)HOST, (char*)url.c_str(), NULL, &cookiee_content, 1, NULL);

    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // print response
    get_code(response, "");
}

void logout_admin(int sockfd, std::string &cookiee, bool &is_admin) 
{
    // check if user is logged in and it is admin
    if (is_admin == false)
    {
        std::cout << "You are not admin.\n";
        return; 
    }

    char *cookiee_content = (char *)cookiee.c_str();

    std::string message = compute_get_request((char *)HOST, (char *)"/api/v1/tema/admin/logout",
                                                NULL, &cookiee_content, 1, NULL);

    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // print response
    get_code(response, "");

    // reset cookiee and is_admin
    cookiee = "no cookiee";
    is_admin = false;
}

/* ================  < Client functions > =============== */

void login(int sockfd, std::string &cookiee)
{
    nlohmann::ordered_json credentials;
    std::string password, username, admin_username;

    // check if user is logged in
    if (cookiee != "no cookiee")
    {
        std::cout << "User already log in.\n";
        return; 
    }

    std::cout << "admin_username=";
    std::getline(std::cin, admin_username);
    credentials["admin_username"] = admin_username;

    std::cout << "username=";
    std::getline(std::cin, username);
    credentials["username"] = username;

    std::cout << "password=";
    std::getline(std::cin, password);
    credentials["password"] = password;

    std::string credentials_dump = credentials.dump(4);
    char *credentials_content = (char *)credentials_dump.c_str();    

    std::string message = compute_post_request((char *)HOST, (char *)"/api/v1/tema/user/login",
                                                (char *)"application/json", &credentials_content, 1, NULL, 0, NULL);

    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // print response
    if (get_code(response, ""))
    {
        cookiee = get_cookiee(response);
    }

}

void get_access(int sockfd, std::string &cookiee, std::string &JWT_token, bool is_admin)
{
    // check if user is logged in
    if (cookiee == "no cookiee")
    {
        std::cout << "You need to log in first.\n";
        return; 
    }

    char *cookiee_content = (char *)cookiee.c_str();

    std::string message = compute_get_request((char *)HOST, (char *)"/api/v1/tema/library/access",
                                                NULL, &cookiee_content, 1, NULL);

    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // print response
    get_code(response, "Token JWT received");
    JWT_token = get_JWT(response);
}

void get_movies(int sockfd, std::string &cookiee, std::string &JWT_token)
{
    // check if user is logged in and has access
    if (cookiee == "no cookiee")
    {
        std::cout << "You need to log in first.\n";
        return; 
    }

    if (JWT_token == "no token")
    {
        std::cout << "You don't have access sir.\n";
        return; 
    }

    char *JWT_content = (char *)JWT_token.c_str();

    std::string message = compute_get_request((char *)HOST, (char *)"/api/v1/tema/library/movies",
                                                NULL, NULL, 0, JWT_content);

    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // print response
    get_code(response, "List of movies");

    std::string ext_response = basic_extract_json_response((char *)response.c_str());
    nlohmann::json body_response = nlohmann::json::parse(ext_response);

    nlohmann::ordered_json movies = body_response["movies"];

    // print the rest of the response
    for (auto movie : movies)
    {
        std::string title = movie["title"];
        std::cout << "#" << movie["id"] << " " << title << '\n';
    }
}

void get_movie(int sockfd, std::string &cookiee, std::string &JWT_token)
{
    std::string id;
    size_t end_pos;

    // check if user is logged in and has access
    if (cookiee == "no cookiee")
    {
        std::cout << "You need to log in first.\n";
        return; 
    }

    if (JWT_token == "no token")
    {
        std::cout << "You don't have access sir.\n";
        return; 
    }

    // check if input is valid
    std::cout << "id=";
    std::getline(std::cin, id);
    std::stoi(id, &end_pos);

    if (end_pos != id.length())
    {
        std::cout << "Id should be a number(int)!\n";
        return;
    }

    // check if the movie exists
    char *JWT_content = (char *)JWT_token.c_str();
    std::string url = "/api/v1/tema/library/movies/" + id;

    std::string message = compute_get_request((char *)HOST, (char *)url.c_str(),
                                                NULL, NULL, 0, JWT_content);

    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // print response
    if (!get_code(response, "Movie details")) 
    {
        return;
    }

    // print the rest of the response
    print_movie(response);

}

void add_movie(int sockfd, std::string &cookiee, std::string &JWT_token)
{
    nlohmann::ordered_json movie_format;
    std::string title, year, description, rating;
    double rating_nr;
    int year_nr;
    size_t end_pos;

    // check if user is logged in and has access
    if (cookiee == "no cookiee")
    {
        std::cout << "User already log in.\n";
        return; 
    }

    if (JWT_token == "no token")
    {
        std::cout << "You don't have access sir.\n";
        return; 
    }

    // check if input is valid
    std::cout << "title=";
    std::getline(std::cin, title);
    movie_format["title"] = title;

    std::cout << "year=";
    std::getline(std::cin, year);
    year_nr = std::stod(year, &end_pos);
    movie_format["year"] = year_nr;

    if (end_pos != year.length())
    {
        std::cout << "The year should be a number!\n";
        return;
    }

    if (0 > year_nr  || year_nr > 3000)
    {
        std::cout << "You sure you got the year right? Try again\n";
        return;
    }

    std::cout << "description=";
    std::getline(std::cin, description);
    movie_format["description"] = description;

    std::cout << "rating=";
    std::getline(std::cin, rating);
    rating_nr = std::stof(rating, &end_pos);
    movie_format["rating"] = rating_nr;

    if (end_pos != rating.length())
    {
        std::cout << "The rating should be a number(double)!\n";
        return;
    }

    if (0 > rating_nr  || rating_nr > 9.9)
    {
        std::cout << "ERROR: Rating must be between 0 and 9.9!\n";
        return;
    }

    std::string movie_format_dump = movie_format.dump(4);
    char *movie_format_content = (char *)movie_format_dump.c_str();    
    char *JWT_content = (char *)JWT_token.c_str();

    std::string message = compute_post_request((char *)HOST, (char *)"/api/v1/tema/library/movies",
                                                (char *)"application/json", &movie_format_content, 1, NULL, 0, JWT_content);

    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // print response
    get_code(response, "Movie added");
}

void delete_movie(int sockfd, std::string &cookiee, std::string &JWT_token)
{
    std::string id;
    size_t end_pos;

    // check if user is logged in and has access
    if (cookiee == "no cookiee")
    {
        std::cout << "You need to log in first.\n";
        return; 
    }

    if (JWT_token == "no token")
    {
        std::cout << "You don't have access sir.\n";
        return; 
    }

    // check if input is valid
    std::cout << "id=";
    std::getline(std::cin, id);
    std::stoi(id, &end_pos);

    if (end_pos != id.length())
    {
        std::cout << "Id should be a number(int)!\n";
        return;
    }

    // check if the movie exists
    char *JWT_content = (char *)JWT_token.c_str();
    std::string url = "/api/v1/tema/library/movies/" + id;

    std::string message = compute_delete_request((char *)HOST, (char *)url.c_str(),
                                                NULL, NULL, 0, JWT_content);

    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // print response
    get_code(response, "");
}

void update_movie(int sockfd, std::string &cookiee, std::string &JWT_token)
{
    nlohmann::ordered_json movie_format;
    std::string id, title, year, description, rating;
    size_t end_pos;
    double rating_nr;
    int year_nr;

    // check if user is logged in and has access
    if (cookiee == "no cookiee")
    {
        std::cout << "You need to log in first.\n";
        return; 
    }

    if (JWT_token == "no token")
    {
        std::cout << "You don't have access sir.\n";
        return; 
    }

    // check if input is valid
    std::cout << "id=";
    std::getline(std::cin, id);
    std::stoi(id, &end_pos);

    if (end_pos != id.length())
    {
        std::cout << "Id should be a number(int)!\n";
        return;
    }

        std::cout << "title=";
    std::getline(std::cin, title);
    movie_format["title"] = title;

    std::cout << "year=";
    std::getline(std::cin, year);
    year_nr = std::stod(year, &end_pos);
    movie_format["year"] = year_nr;

    if (end_pos != year.length())
    {
        std::cout << "The year should be a number!\n";
        return;
    }

    if (0 > year_nr  || year_nr > 3000)
    {
        std::cout << "You sure you got the year right? Try again\n";
        return;
    }

    std::cout << "description=";
    std::getline(std::cin, description);
    movie_format["description"] = description;

    std::cout << "rating=";
    std::getline(std::cin, rating);
    rating_nr = std::stof(rating, &end_pos);
    movie_format["rating"] = rating_nr;

    if (end_pos != rating.length())
    {
        std::cout << "The rating should be a number(double)!\n";
        return;
    }

    if (0 > rating_nr  || rating_nr > 9.9)
    {
        std::cout << "Rating must be between 0 and 9.9!\n";
        return;
    }

    // check if the movie exists
    std::string movie_format_dump = movie_format.dump(4);
    char *movie_format_content = (char *)movie_format_dump.c_str();    
    char *JWT_content = (char *)JWT_token.c_str();
    std::string url = "/api/v1/tema/library/movies/" + id;

    std::string message = compute_put_request((char *)HOST, (char *)url.c_str(),
                                                (char *)"application/json", &movie_format_content, 1, NULL, 0, JWT_content);

    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // print response
    get_code(response, "Movie updated");
}

void get_collections(int sockfd, std::string &cookiee, std::string &JWT_token)
{
    // check if user is logged in and has access
    if (cookiee == "no cookiee")
    {
        std::cout << "You need to log in first.\n";
        return; 
    }

    if (JWT_token == "no token")
    {
        std::cout << "You don't have access sir.\n";
        return; 
    }

    char *JWT_content = (char *)JWT_token.c_str();

    std::string message = compute_get_request((char *)HOST, (char *)"/api/v1/tema/library/collections",
                                                NULL, NULL, 0, JWT_content);

    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // print response
    get_code(response, "List of collections");

    std::string ext_response = basic_extract_json_response((char *)response.c_str());
    nlohmann::json body_response = nlohmann::json::parse(ext_response);

    nlohmann::ordered_json collections = body_response["collections"];

    // print the rest of the response
    for (auto collection : collections)
    {
        std::string title = collection["title"];
        std::cout << "#" << collection["id"] << ": " << title << '\n';
    }
}

void get_collection(int sockfd, std::string &cookiee, std::string &JWT_token)
{
    std::string id;
    size_t end_pos;

    // check if user is logged in and has access
    if (cookiee == "no cookiee")
    {
        std::cout << "You need to log in first.\n";
        return; 
    }

    if (JWT_token == "no token")
    {
        std::cout << "You don't have access sir.\n";
        return; 
    }

    // check if input is valid
    std::cout << "id=";
    std::getline(std::cin, id);
    std::stoi(id, &end_pos);

    if (end_pos != id.length())
    {
        std::cout << "Id should be a number(int)!\n";
        return;
    }

    char *JWT_content = (char *)JWT_token.c_str();
    std::string url = "/api/v1/tema/library/collections/" + id;

    std::string message = compute_get_request((char *)HOST, (char *)url.c_str(),
                                                NULL, NULL, 0, JWT_content);

    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // check if the collection exists
    if (!get_code(response, "Collection details")) 
    {   
        return;
    }

    // print response
    print_collection(response);
}

void add_collection(int sockfd, std::string &cookiee, std::string &JWT_token)
{
    nlohmann::ordered_json collection_format;
    std::string title, num_movies;
    int num_movies_nr;
    size_t end_pos;
    std::vector<int> movies_id;

    // check if user is logged in and has access
    if (cookiee == "no cookiee")
    {
        std::cout << "User already log in.\n";
        return; 
    }

    if (JWT_token == "no token")
    {
        std::cout << "You don't have access sir.\n";
        return; 
    }

    // check if input is valid
    std::cout << "title=";
    std::getline(std::cin, title);
    collection_format["title"] = title;

    std::cout << "num_movies=";
    std::getline(std::cin, num_movies);
    num_movies_nr = std::stoi(num_movies, &end_pos);


    if (end_pos != num_movies.length())
    {
        std::cout << "Nr. movies should be a number!\n";
        return;
    }

    for (int i = 0; i < num_movies_nr; i++)
    {
        int j;
        std::cout << "movie_id[" << i << "]=";
        std::cin >> j;

        movies_id.push_back(j);
    }

    // check if the collection exists
    std::string collection_format_dump = collection_format.dump(4);
    char *collection_format_content = (char *)collection_format_dump.c_str();
    char *JWT_content = (char *)JWT_token.c_str();

    std::string message = compute_post_request((char *)HOST, (char *)"/api/v1/tema/library/collections",
                                                (char *)"application/json", &collection_format_content, 1, NULL, 0, JWT_content);

    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // add movies to collection via index
    int id = get_id(response);
    for (int i = 0; i < num_movies_nr; i++)
    {
        response = add_movie_to_collection_index(sockfd, id, movies_id[i], JWT_token);

        std::istringstream aux(response);
        std::string text;
        int code;

        aux >> text >> code >> text;

        if (code / 100 != 2)
        {
            break;
        }
    }

    // print response
    get_code(response, "Collection added");
}

void delete_collection(int sockfd, std::string &cookiee, std::string &JWT_token)
{
    std::string id;
    size_t end_pos;

    // check if user is logged in and has access
    if (cookiee == "no cookiee")
    {
        std::cout << "You need to log in first.\n";
        return; 
    }

    if (JWT_token == "no token")
    {
        std::cout << "You don't have access sir.\n";
        return; 
    }

    // check if input is valid
    std::cout << "id=";
    std::getline(std::cin, id);
    std::stoi(id, &end_pos);

    if (end_pos != id.length())
    {
        std::cout << "Id should be a number(int)!\n";
        return;
    }

    // check if the collection exists
    char *JWT_content = (char *)JWT_token.c_str();
    std::string url = "/api/v1/tema/library/collections/" + id;

    std::string message = compute_delete_request((char *)HOST, (char *)url.c_str(),
                                                NULL, NULL, 0, JWT_content);

    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // print response
    get_code(response, ""); 
}

void add_movie_to_collection(int sockfd, std::string &cookiee, std::string &JWT_token)
{
    int collection_id, movie_id;

    // check if user is logged in and has access
    if (cookiee == "no cookiee")
    {
        std::cout << "User already log in.\n";
        return; 
    }

    if (JWT_token == "no token")
    {
        std::cout << "You don't have access sir.\n";
        return; 
    }

    std::cout << "collection_id=";
    std::cin >> collection_id;

    std::cout << "movie_id=";
    std::cin >> movie_id;

    // add movie to collection via index
    std::string response = add_movie_to_collection_index(sockfd, collection_id, movie_id, JWT_token);

    // print response
    get_code(response, "");
}

void delete_movie_from_collection(int sockfd, std::string &cookiee, std::string &JWT_token)
{
    std::string id_collection, id_movie;
    size_t end_pos;

    // check if user is logged in and has access
    if (cookiee == "no cookiee")
    {
        std::cout << "You need to log in first.\n";
        return; 
    }

    if (JWT_token == "no token")
    {
        std::cout << "You don't have access sir.\n";
        return; 
    }

    // check if input is valid
    std::cout << "collection_id=";
    std::getline(std::cin, id_collection);
    std::stoi(id_collection, &end_pos);

    if (end_pos != id_collection.length())
    {
        std::cout << "Id should be a number(int)!\n";
        return;
    }

    std::cout << "movie_id=";
    std::getline(std::cin, id_movie);
    std::stoi(id_movie, &end_pos);

    if (end_pos != id_movie.length())
    {
        std::cout << "Id should be a number(int)!\n";
        return;
    }

    // check if the collection exists
    char *JWT_content = (char *)JWT_token.c_str();
    std::string url = "/api/v1/tema/library/collections/" + id_collection +  "/movies/" + id_movie;

    std::string message = compute_delete_request((char *)HOST, (char *)url.c_str(),
                                                NULL, NULL, 0, JWT_content);

    // send request
    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // print response
    get_code(response, "");
}

void logout(int sockfd, std::string &cookiee, std::string &JWT_token)
{
    // check if user is logged in
    if (cookiee == "no cookiee")
    {
        std::cout << "User already log in.\n";
        return; 
    }

    // compute the request
    char *cookiee_content = (char *)cookiee.c_str();
    std::string message = compute_get_request((char *)HOST, (char *)"/api/v1/tema/user/logout",
                                                NULL, &cookiee_content, 1, NULL);

    send_to_server(sockfd, (char *)message.c_str());
    std::string response = receive_from_server(sockfd);

    // reset cookiee and JWT_token
    cookiee = "no cookiee";
    JWT_token = "no token";

    // print response
    get_code(response, "");
}

/* ================  < Main > =============== */

int main(int argc, char *argv[])
{
    std::string command;
    std::string cookiee = "no cookiee", JWT_token = "no token";
    bool is_admin = false;

    // open connection to the server
    int sockfd = open_connection((char *)HOST, PORT, AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "ERROR: Failed to open connection to the server." << '\n';
        return -1;
    }

    while (true)
    {
        std::getline(std::cin, command);

        if (command == "login_admin")/* 1 */
        {
            /* admin login */
            login_admin(sockfd, cookiee, is_admin);
        } else if (command == "add_user")/* 2 */
        {
            /* admin add user */
            add_user(sockfd, cookiee, is_admin);
        } else if (command == "get_users")/* 3 */
        {
            /* admin get users */
            get_users(sockfd, cookiee, is_admin);
        } else if (command == "delete_user")/* 4 */
        {
            /* admin delete user */
            delete_user(sockfd, cookiee, is_admin);
        } else if (command == "logout_admin")/* 5 */
        {
            /* admin logout */
            logout_admin(sockfd, cookiee, is_admin);
        } else if (command == "login")/* 6 */
        {
            /* user login */
            login(sockfd, cookiee);
        } else if (command == "get_access")/* 7 */
        {
            /* user get access */
            get_access(sockfd, cookiee, JWT_token, is_admin);
        } else if (command == "get_movies")/* 8 */
        {
            /* user get movies */
            get_movies(sockfd, cookiee, JWT_token);
        } else if (command == "get_movie")/* 9 */
        {
            /* user get movie */
            get_movie(sockfd, cookiee, JWT_token);
        } else if (command == "add_movie")/* 10 */
        {
            /* user add movie */
            add_movie(sockfd, cookiee, JWT_token);
        } else if (command == "delete_movie")/* 11 */
        {
            /* user delete movie */
            delete_movie(sockfd, cookiee, JWT_token);
        } else if (command == "update_movie")/* 12 */
        {
            /* user update movie */
            update_movie(sockfd, cookiee, JWT_token);
        } else if (command == "get_collections")/* 13 */
        {
            /* user get collections */
            get_collections(sockfd, cookiee, JWT_token);
        } else if (command == "get_collection")/* 14 */
        {
            /* user get collection */
            get_collection(sockfd, cookiee, JWT_token);
        } else if (command == "add_collection")/* 15 */
        {
            /* user add collection */
            add_collection(sockfd, cookiee, JWT_token);
        } else if (command == "delete_collection")/* 16 */
        {
            /* user delete collection */
            delete_collection(sockfd, cookiee, JWT_token);
        } else if (command == "add_movie_to_collection")/* 17 */
        {
            /* user add movie to collection */
            add_movie_to_collection(sockfd, cookiee, JWT_token);
        } else if (command == "delete_movie_from_collection")/* 18 */
        {
            /* user delete movie to collection */
            delete_movie_from_collection(sockfd, cookiee, JWT_token);
        } else if (command == "logout")/* 19 */
        {
            /* user logout */
            logout(sockfd, cookiee, JWT_token);
        } else if (command == "exit")/* 20 */
        {
            /* exit */
            break;
        }
    }

    // close connection to the server
    close_connection(sockfd);
    return 0;
}
