#pragma once

#include <string>
#include <vector>

struct Client
{
    int id;
    std::string firstName;
    std::string lastName;
    std::string email;
    std::vector<std::string> phones;
};

class ClientManager
{
public:

    ClientManager(
        const std::string& host,
        const std::string& port,
        const std::string& database,
        const std::string& user,
        const std::string& password
    );

    void createTables();

    int addClient(
        const std::string& firstName,
        const std::string& lastName,
        const std::string& email
    );

    void addPhone(
        int clientId,
        const std::string& phone
    );

    void updateClient(
        int clientId,
        const std::string& firstName,
        const std::string& lastName,
        const std::string& email
    );

    void deletePhone(
        int clientId,
        const std::string& phone
    );

    void deleteClient(
        int clientId
    );

    std::vector<Client> findClients(
        const std::string& value
    );

private:

    std::string connectionString;
};