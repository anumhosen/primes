#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;

int main()
{
    ifstream GetLastPrime("Primes.txt", ios::in);
    string lastPrime;
    int num;

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
        num = stoi(lastPrime) + 2;
    }
    else
    {
        // If file doesn't exist or is empty, start with the first prime number
        num = 3;
    }
    GetLastPrime.close();

    cout << "Starting With : " << num << endl;

    int limit = 1000000; // Generate primes up to this limit
    vector<int> primes;

    // Load existing primes into memory
    ifstream PrimesFile("Primes.txt");
    string line;
    while (getline(PrimesFile, line))
    {
        primes.push_back(stoi(line));
    }
    PrimesFile.close();

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
            // primes.push_back(num); // Add to memory
            ofstream WritingPrime("Primes.txt", ios::app);
            WritingPrime << "\n" << num; // Append to file
            WritingPrime.close();
            cout << num << endl;
        }

        num += 2; // Skip even numbers
    }

    return 0;
}
