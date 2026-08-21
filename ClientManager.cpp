#include "ClientManager.h"

#include <pqxx/pqxx>

#include <algorithm>
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
}


int ClientManager::addClient(
    const std::string& firstName,
    const std::string& lastName,
    const std::string& email
)
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


void ClientManager::addPhone(
    int clientId,
    const std::string& phone
)
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


void ClientManager::updateClient(
    int clientId,
    const std::string& firstName,
    const std::string& lastName,
    const std::string& email
)
{
    pqxx::connection connection(connectionString);

    pqxx::work transaction(connection);

    transaction.exec_params(
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

    transaction.commit();
}


void ClientManager::deletePhone(
    int clientId,
    const std::string& phone
)
{
    pqxx::connection connection(connectionString);

    pqxx::work transaction(connection);

    transaction.exec_params(
        R"(
            DELETE FROM phones
            WHERE client_id = $1
              AND phone = $2;
        )",
        clientId,
        phone
    );

    transaction.commit();
}


void ClientManager::deleteClient(
    int clientId
)
{
    pqxx::connection connection(connectionString);

    pqxx::work transaction(connection);

    transaction.exec_params(
        R"(
            DELETE FROM clients
            WHERE id = $1;
        )",
        clientId
    );

    transaction.commit();
}


std::vector<Client> ClientManager::findClients(
    const std::string& value
)
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