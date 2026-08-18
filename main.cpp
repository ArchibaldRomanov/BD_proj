#include "ClientManager.h"

#include <iostream>
#include <string>


void printClients(const std::vector<Client>& clients)
{
    if (clients.empty())
    {
        std::cout << "No clients found.\n";
        return;
    }

    for (const Client& client : clients)
    {
        std::cout << "\n";
        std::cout << "ID: " << client.id << "\n";
        std::cout << "First name: "
            << client.firstName << "\n";

        std::cout << "Last name: "
            << client.lastName << "\n";

        std::cout << "Email: "
            << client.email << "\n";

        std::cout << "Phones: ";

        if (client.phones.empty())
        {
            std::cout << "none";
        }
        else
        {
            for (const std::string& phone : client.phones)
            {
                std::cout << phone << " ";
            }
        }

        std::cout << "\n";
    }
}


int main()
{
    try
    {
        ClientManager manager(
            "localhost",
            "5432",
            "clients_db",
            "postgres",
            "2905"
        );

        std::cout << "Creating database tables...\n";

        manager.createTables();


        std::cout << "\n";
        std::cout << "Adding clients...\n";

        int ivanId = manager.addClient(
            "Ivan",
            "Petrov",
            "ivan@example.com"
        );

        int annaId = manager.addClient(
            "Anna",
            "Ivanova",
            "anna@example.com"
        );


        std::cout << "Ivan ID: "
            << ivanId << "\n";

        std::cout << "Anna ID: "
            << annaId << "\n";


        std::cout << "\n";
        std::cout << "Adding phones...\n";

        manager.addPhone(
            ivanId,
            "+37111111111"
        );

        manager.addPhone(
            ivanId,
            "+37122222222"
        );

        manager.addPhone(
            annaId,
            "+37133333333"
        );


        std::cout << "\n";
        std::cout << "Searching Ivan...\n";

        auto clients = manager.findClients("Ivan");

        printClients(clients);


        std::cout << "\n";
        std::cout << "Searching by phone...\n";

        clients = manager.findClients("+37111111111");

        printClients(clients);


        std::cout << "\n";
        std::cout << "Updating Ivan...\n";

        manager.updateClient(
            ivanId,
            "Ivan",
            "Sidorov",
            "ivan.sidorov@example.com"
        );


        std::cout << "\n";
        std::cout << "Searching updated client...\n";

        clients = manager.findClients(
            "ivan.sidorov@example.com"
        );

        printClients(clients);


        std::cout << "\n";
        std::cout << "Deleting one phone...\n";

        manager.deletePhone(
            ivanId,
            "+37111111111"
        );


        std::cout << "\n";
        std::cout << "Searching Ivan after deleting phone...\n";

        clients = manager.findClients("Sidorov");

        printClients(clients);


        std::cout << "\n";
        std::cout << "Deleting Anna...\n";

        manager.deleteClient(annaId);


        std::cout << "\n";
        std::cout << "Searching Anna after deletion...\n";

        clients = manager.findClients("Anna");

        printClients(clients);


        std::cout << "\n";
        std::cout << "Program completed successfully.\n";
    }
    catch (const std::exception& e)
    {
        std::cerr << "\nERROR:\n";
        std::cerr << e.what() << "\n";

        return 1;
    }

    return 0;
}