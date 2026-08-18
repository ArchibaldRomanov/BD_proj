#include "ClientManager.h"

#include <pqxx/pqxx>

#include <algorithm>
#include <iostream>
#include <stdexcept>


ClientManager::ClientManager(
    const std::string& host,
    const std::string& port,
    const std::string& database,
    const std::string& user,
    const std::string& password
)
{
    connectionString =
        "host=" + host +
        " port=" + port +
        " dbname=" + database +
        " user=" + user +
        " password=" + password;
}


void ClientManager::createTables()
{
    try
    {
        pqxx::connection connection(connectionString);

        pqxx::work transaction(connection);

        transaction.exec(R"(
            CREATE TABLE IF NOT EXISTS clients
            (
                id SERIAL PRIMARY KEY,
                first_name VARCHAR(100) NOT NULL,
                last_name VARCHAR(100) NOT NULL,
                email VARCHAR(255) NOT NULL UNIQUE
            );
        )");

        transaction.exec(R"(
            CREATE TABLE IF NOT EXISTS phones
            (
                id SERIAL PRIMARY KEY,
                client_id INTEGER NOT NULL,
                phone VARCHAR(50) NOT NULL,

                CONSTRAINT fk_client
                    FOREIGN KEY (client_id)
                    REFERENCES clients(id)
                    ON DELETE CASCADE,

                CONSTRAINT unique_client_phone
                    UNIQUE (client_id, phone)
            );
        )");

        transaction.commit();

        std::cout << "Database tables created successfully.\n";
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(
            std::string("Error creating tables: ") + e.what()
        );
    }
}


int ClientManager::addClient(
    const std::string& firstName,
    const std::string& lastName,
    const std::string& email
)
{
    try
    {
        pqxx::connection connection(connectionString);

        pqxx::work transaction(connection);

        pqxx::result result = transaction.exec_params(
            R"(
                INSERT INTO clients
                    (first_name, last_name, email)
                VALUES
                    ($1, $2, $3)
                RETURNING id;
            )",
            firstName,
            lastName,
            email
        );

        transaction.commit();

        return result[0][0].as<int>();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(
            std::string("Error adding client: ") + e.what()
        );
    }
}


void ClientManager::addPhone(
    int clientId,
    const std::string& phone
)
{
    try
    {
        pqxx::connection connection(connectionString);

        pqxx::work transaction(connection);

        transaction.exec_params(
            R"(
                INSERT INTO phones
                    (client_id, phone)
                VALUES
                    ($1, $2);
            )",
            clientId,
            phone
        );

        transaction.commit();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(
            std::string("Error adding phone: ") + e.what()
        );
    }
}


void ClientManager::updateClient(
    int clientId,
    const std::string& firstName,
    const std::string& lastName,
    const std::string& email
)
{
    try
    {
        pqxx::connection connection(connectionString);

        pqxx::work transaction(connection);

        pqxx::result result = transaction.exec_params(
            R"(
                UPDATE clients
                SET
                    first_name = $1,
                    last_name = $2,
                    email = $3
                WHERE id = $4;
            )",
            firstName,
            lastName,
            email,
            clientId
        );

        if (result.affected_rows() == 0)
        {
            throw std::runtime_error(
                "Client with specified ID does not exist."
            );
        }

        transaction.commit();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(
            std::string("Error updating client: ") + e.what()
        );
    }
}


void ClientManager::deletePhone(
    int clientId,
    const std::string& phone
)
{
    try
    {
        pqxx::connection connection(connectionString);

        pqxx::work transaction(connection);

        pqxx::result result = transaction.exec_params(
            R"(
                DELETE FROM phones
                WHERE client_id = $1
                  AND phone = $2;
            )",
            clientId,
            phone
        );

        if (result.affected_rows() == 0)
        {
            throw std::runtime_error(
                "Phone was not found."
            );
        }

        transaction.commit();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(
            std::string("Error deleting phone: ") + e.what()
        );
    }
}


void ClientManager::deleteClient(
    int clientId
)
{
    try
    {
        pqxx::connection connection(connectionString);

        pqxx::work transaction(connection);

        pqxx::result result = transaction.exec_params(
            R"(
                DELETE FROM clients
                WHERE id = $1;
            )",
            clientId
        );

        if (result.affected_rows() == 0)
        {
            throw std::runtime_error(
                "Client was not found."
            );
        }

        transaction.commit();
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(
            std::string("Error deleting client: ") + e.what()
        );
    }
}


std::vector<Client> ClientManager::findClients(
    const std::string& value
)
{
    try
    {
        pqxx::connection connection(connectionString);

        pqxx::work transaction(connection);

        pqxx::result result = transaction.exec_params(
            R"(
                SELECT
                    c.id,
                    c.first_name,
                    c.last_name,
                    c.email,
                    p.phone
                FROM clients c
                LEFT JOIN phones p
                    ON c.id = p.client_id
                WHERE
                    c.first_name ILIKE '%' || $1 || '%'
                    OR c.last_name ILIKE '%' || $1 || '%'
                    OR c.email ILIKE '%' || $1 || '%'
                    OR p.phone ILIKE '%' || $1 || '%'
                ORDER BY c.id;
            )",
            value
        );

        transaction.commit();

        std::vector<Client> clients;

        for (const auto& row : result)
        {
            int id = row["id"].as<int>();

            auto it = std::find_if(
                clients.begin(),
                clients.end(),
                [id](const Client& client)
                {
                    return client.id == id;
                }
            );

            if (it == clients.end())
            {
                Client client;

                client.id = id;
                client.firstName =
                    row["first_name"].as<std::string>();

                client.lastName =
                    row["last_name"].as<std::string>();

                client.email =
                    row["email"].as<std::string>();

                if (!row["phone"].is_null())
                {
                    client.phones.push_back(
                        row["phone"].as<std::string>()
                    );
                }

                clients.push_back(client);
            }
            else
            {
                if (!row["phone"].is_null())
                {
                    it->phones.push_back(
                        row["phone"].as<std::string>()
                    );
                }
            }
        }

        return clients;
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(
            std::string("Error searching clients: ") + e.what()
        );
    }
}