#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

using namespace std;

int main()
{
    ifstream GetLastPrime("Primes.txt", ios::in);
    string lastPrime;
    long long num;

    // Read the last prime number from the file
    if (GetLastPrime)
    {
        GetLastPrime.seekg(0, ios::end);
        long long pos = GetLastPrime.tellg();
        char ch;
        for (long long i = pos - 1; i >= 0; --i)
        {
            GetLastPrime.seekg(i);
            GetLastPrime.get(ch);
            if (ch == '\n' && i != pos - 1)
            {
                break;
            }
            lastPrime.insert(lastPrime.begin(), ch);
        }
        num = stoll(lastPrime) + 2; // Start with the next odd number
    }
    else
    {
        // If file doesn't exist or is empty, start with the first prime number
        num = 3;
    }
    GetLastPrime.close();

    cout << "Starting With : " << num << endl;

    long long limit = 10000; // Generate primes up to 1 billion (1B)

    // Load only primes <= sqrt(limit) into memory
    vector<int> primes;
    ifstream PrimesFile("Primes.txt");
    string line;
    while (getline(PrimesFile, line))
    {
        int prime = stoi(line);
        if (prime * prime > limit) break; // Only load primes <= sqrt(1B)
        primes.push_back(prime);
    }
    PrimesFile.close();

    // Prepare for buffered output
    vector<string> buffer;
    const int bufferSize = 1000; // Buffer size for writing to file

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
            // Add prime to the buffer
            buffer.push_back(to_string(num));
            cout << num << endl;

            // Write buffer to file if full
            if (buffer.size() >= bufferSize)
            {
                ofstream WritingPrime("Primes.txt", ios::app);
                for (const string &prime : buffer)
                {
                    WritingPrime << "\n" << prime;
                }
                WritingPrime.close();
                buffer.clear(); // Clear the buffer
            }
        }

        num += 2; // Skip even numbers
    }

    // Write any remaining primes in the buffer to the file
    if (!buffer.empty())
    {
        ofstream WritingPrime("Primes.txt", ios::app);
        for (const string &prime : buffer)
        {
            WritingPrime << "\n" << prime;
        }
        WritingPrime.close();
    }

    return 0;
}
