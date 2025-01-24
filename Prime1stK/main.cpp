#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main()
{
    int num = 3;
    int limit = 1000;    // 1st thousand
    string primeString;
    int prime = 0;
    while (num < limit)
    {
        bool is_prime = true;
        ifstream Primes("Primes.txt");

        while(getline(Primes, primeString))
        {
            prime = stoi(primeString);
            // Check divisibility by primes less than or equal to sqrt(num)
            if (prime * prime > num) break;

            if (num % prime == 0)
            {
                is_prime = false;
                break;
            }
        }
        Primes.close();

        if (is_prime)
        {
            ofstream WritingPrime("Primes.txt", ios::app);
            WritingPrime << "\n" << num;
            WritingPrime.close();
        }
        num += 2; // Increment by 2 to skip even numbers
    }
    return 0;
}
