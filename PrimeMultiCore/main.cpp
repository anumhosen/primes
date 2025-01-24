#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <thread>
#include <mutex>
#include <atomic>

using namespace std;

// Mutex to ensure safe file writing
mutex writeMutex;

// Function to check if a number is prime using the given primes
bool isPrime(long long num, const vector<int>& primes) {
    for (int prime : primes) {
        if (prime * prime > num) break;  // No need to check beyond sqrt(num)
        if (num % prime == 0) return false;  // n is divisible by a smaller prime
    }
    return true;
}

// Worker function to generate primes in a given range
void generatePrimesInRange(long long start, long long end, const vector<int>& primes, vector<string>& buffer, atomic<int>& count, long long limit) {
    for (long long num = start; num < end; num += 2) {  // Skip even numbers
        bool is_prime = true;

        // Check divisibility by primes less than or equal to sqrt(num)
        if (isPrime(num, primes)) {
            buffer.push_back(to_string(num));

            // Safely update the count for processed primes
            count++;

            // If buffer is full, write to file and clear it
            if (buffer.size() >= 1000) {
                lock_guard<mutex> lock(writeMutex);
                ofstream WritingPrime("Primes.txt", ios::app);
                for (const string& prime : buffer) {
                    WritingPrime << "\n" << prime;
                }
                WritingPrime.close();
                buffer.clear();
            }
        }
    }
}

int main() {
    ifstream GetLastPrime("Primes.txt", ios::in);
    string lastPrime;
    long long num;

    // Read the last prime number from the file
    if (GetLastPrime) {
        GetLastPrime.seekg(0, ios::end);
        long long pos = GetLastPrime.tellg();
        char ch;
        for (long long i = pos - 1; i >= 0; --i) {
            GetLastPrime.seekg(i);
            GetLastPrime.get(ch);
            if (ch == '\n' && i != pos - 1) {
                break;
            }
            lastPrime.insert(lastPrime.begin(), ch);
        }
        num = stoll(lastPrime) + 2; // Start with the next odd number
    } else {
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
    while (getline(PrimesFile, line)) {
        int prime = stoi(line);
        if (prime * prime > limit) break; // Only load primes <= sqrt(1B)
        primes.push_back(prime);
    }
    PrimesFile.close();

    // Prepare for buffered output
    vector<string> buffer;
    const int numThreads = 2; // Number of threads (adjust according to your machine's core count)

    // Atomic variable to keep track of how many primes have been processed
    atomic<int> count(0);

    // Divide the work among threads
    vector<thread> threads;
    long long rangeSize = (limit - num) / numThreads;

    for (int i = 0; i < numThreads; ++i) {
        long long start = num + i * rangeSize;
        long long end = (i == numThreads - 1) ? limit : (start + rangeSize); // Last thread takes the remaining range
        threads.push_back(thread(generatePrimesInRange, start, end, ref(primes), ref(buffer), ref(count), limit));
    }

    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();
    }

    // Write any remaining primes in the buffer to the file
    if (!buffer.empty()) {
        lock_guard<mutex> lock(writeMutex);
        ofstream WritingPrime("Primes.txt", ios::app);
        for (const string& prime : buffer) {
            WritingPrime << "\n" << prime;
        }
        WritingPrime.close();
    }

    cout << "Done. Processed " << count.load() << " primes." << endl;
    return 0;
}
