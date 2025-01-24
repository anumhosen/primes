#include <iostream>
#include <vector>
#include <cmath>
#include <sqlite3.h> // Include SQLite3 header

using namespace std;

int main()
{
    // Open SQLite database (it will create the database if it doesn't exist)
    sqlite3 *db;
    char *errMessage = 0;
    int rc = sqlite3_open("Primes.db", &db);

    if (rc) {
        cerr << "Can't open database: " << sqlite3_errmsg(db) << endl;
        return 1;
    } else {
        cout << "Opened database successfully!" << endl;
    }

    // Create table if it doesn't exist
    const char* createTableSQL = "CREATE TABLE IF NOT EXISTS primes (id INTEGER PRIMARY KEY, prime_number INTEGER);";
    rc = sqlite3_exec(db, createTableSQL, 0, 0, &errMessage);
    if (rc != SQLITE_OK) {
        cerr << "SQL error: " << errMessage << endl;
        sqlite3_free(errMessage);
        sqlite3_close(db);
        return 1;
    }

    // Fetch the last prime number from the database
    long long num;
    const char* fetchLastPrimeSQL = "SELECT MAX(prime_number) FROM primes;";
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, fetchLastPrimeSQL, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return 1;
    }

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        num = sqlite3_column_int64(stmt, 0);
        if (num == 0) {
            // If no primes exist in the database, start with 3
            num = 3;
        } else {
            num += 2; // Start with the next odd number
        }
    } else {
        cerr << "Failed to fetch last prime: " << sqlite3_errmsg(db) << endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return 1;
    }
    sqlite3_finalize(stmt);

    cout << "Starting With : " << num << endl;

    long long limit = 10000; // Generate primes up to 1 billion (1B)

    // Load only primes <= sqrt(limit) into memory
    vector<int> primes;
    const char* fetchPrimesSQL = "SELECT prime_number FROM primes WHERE prime_number <= ?;";
    rc = sqlite3_prepare_v2(db, fetchPrimesSQL, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        cerr << "Failed to prepare fetch primes statement: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return 1;
    }

    sqlite3_bind_int64(stmt, 1, static_cast<long long>(sqrt(limit)));

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        primes.push_back(sqlite3_column_int(stmt, 0));
    }
    sqlite3_finalize(stmt);

    // Generate new primes
    while (num < limit)
    {
        bool is_prime = true;

        // Check divisibility by primes less than or equal to sqrt(num)
        for (int prime : primes)
        {
            if (prime * prime > num) break;
            if (num % prime == 0)
            {
                is_prime = false;
                break;
            }
        }

        if (is_prime)
        {
            // Insert prime into the database
            const char* insertSQL = "INSERT INTO primes (prime_number) VALUES (?);";
            rc = sqlite3_prepare_v2(db, insertSQL, -1, &stmt, 0);
            if (rc != SQLITE_OK) {
                cerr << "Failed to prepare insert statement: " << sqlite3_errmsg(db) << endl;
                sqlite3_close(db);
                return 1;
            }

            sqlite3_bind_int64(stmt, 1, num);
            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                cerr << "Execution failed: " << sqlite3_errmsg(db) << endl;
                sqlite3_finalize(stmt);
                sqlite3_close(db);
                return 1;
            }

            sqlite3_finalize(stmt);
            cout << num << endl;
        }

        num += 2; // Skip even numbers
    }

    sqlite3_close(db); // Close the database

    return 0;
}
