#include <vector>
#include <array>
#include <iostream>
#include <random>
#include <chrono>
#include <fstream>

#define RZUTY 100

int randomNumber;
int p[40] = {0};

void saveToCSV(int* data, size_t N, const std::string& fileName) {
    std::ofstream outFile(fileName);

    if (!outFile) {
        std::cerr << "Error" << std::endl;
        return;
    }

    for (size_t i = 0; i < N; ++i) {
        outFile << i << "," << data[i] << std::endl;
    }

    outFile.close();
}

int main() {
    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(2, 12);

    int currentPosition = 0;
    int throws = 0;

    while (throws < RZUTY) {
        randomNumber = dist(rng);
        currentPosition += randomNumber;
        if (currentPosition > 39) {
            currentPosition = currentPosition - 40;
        }
        p[currentPosition]++;
        if (currentPosition == 30) {
            currentPosition = 10;
            p[currentPosition]++;
        }
        
        throws++;
    }

    saveToCSV(p, 40, "output.csv");
}