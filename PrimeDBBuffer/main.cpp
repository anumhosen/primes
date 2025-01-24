#include <iostream>
#include <vector>
#include <cmath>
#include <sqlite3.h>

using namespace std;

int main() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    const char *dbFilename = "Primes.db";

    // Open the SQLite database
    if (sqlite3_open(dbFilename, &db)) {
        cerr << "Error opening database: " << sqlite3_errmsg(db) << endl;
        return 1;
    }

    // Create the primes table if it doesn't exist
    const char *createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS primes (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            prime_number INTEGER NOT NULL UNIQUE
        );
    )";
    if (sqlite3_exec(db, createTableSQL, nullptr, nullptr, nullptr) != SQLITE_OK) {
        cerr << "Error creating table: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return 1;
    }

    // Get the last prime number from the database
    long long num = 3; // Default to 3 if no primes exist
    const char *getLastPrimeSQL = "SELECT MAX(prime_number) FROM primes;";
    if (sqlite3_prepare_v2(db, getLastPrimeSQL, -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            if (sqlite3_column_type(stmt, 0) != SQLITE_NULL) { // Check if the result is not NULL
                num = sqlite3_column_int64(stmt, 0) + 2; // Start with the next odd number
            }
        }
        sqlite3_finalize(stmt);
    } else {
        cerr << "Error retrieving last prime: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return 1;
    }

    cout << "Starting with: " << num << endl;

    long long limit = 1000000; // Generate primes up to 1 billion
    vector<int> primes;

    // Load only primes <= sqrt(limit) into memory
    const char *getPrimesSQL = "SELECT prime_number FROM primes WHERE prime_number <= ?;";
    if (sqlite3_prepare_v2(db, getPrimesSQL, -1, &stmt, nullptr) == SQLITE_OK) {
        sqlite3_bind_int64(stmt, 1, static_cast<long long>(sqrt(limit)));
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            primes.push_back(sqlite3_column_int(stmt, 0));
        }
        sqlite3_finalize(stmt);
    } else {
        cerr << "Error loading primes: " << sqlite3_errmsg(db) << endl;
        sqlite3_close(db);
        return 1;
    }

    // Prepare buffer and batch insert logic
    vector<long long> buffer;
    const int bufferSize = 1000; // Buffer size for batch inserts

    // Generate new primes
    while (num < limit) {
        bool is_prime = true;

        // Check divisibility by primes in memory
        for (int prime : primes) {
            if (prime * prime > num) break;
            if (num % prime == 0) {
                is_prime = false;
                break;
            }
        }

        if (is_prime) {
            buffer.push_back(num);

            // Insert buffer into database if full
            if (buffer.size() >= bufferSize) {
                sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
                for (long long prime : buffer) {
                    const char *insertSQL = "INSERT INTO primes (prime_number) VALUES (?);";
                    if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr) == SQLITE_OK) {
                        sqlite3_bind_int64(stmt, 1, prime);
                        sqlite3_step(stmt);
                        sqlite3_finalize(stmt);
                    }
                }
                sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
                buffer.clear();
            }
        }

        num += 2; // Skip even numbers
    }

    // Insert remaining primes in the buffer into the database
    if (!buffer.empty()) {
        sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
        for (long long prime : buffer) {
            const char *insertSQL = "INSERT INTO primes (prime_number) VALUES (?);";
            if (sqlite3_prepare_v2(db, insertSQL, -1, &stmt, nullptr) == SQLITE_OK) {
                sqlite3_bind_int64(stmt, 1, prime);
                sqlite3_step(stmt);
                sqlite3_finalize(stmt);
            }
        }
        sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr);
    }

    cout << "Prime generation complete." << endl;

    // Close the database
    sqlite3_close(db);
    return 0;
}
