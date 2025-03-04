#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <fstream>
#include <sstream>

#define DIM 500
#define STEPS 400

std::vector<std::vector<uint8_t>> data(DIM+1, std::vector<uint8_t>(DIM+1));

void step(std::vector<std::vector<uint8_t>>& table) {
    std::vector<std::vector<uint8_t>> copy = table;
    
    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(0, 1);

    for (int i = 1; i < DIM; i++) {
        for (int j = 1; j < DIM; j++) {
            int sum = 0;
            sum += copy[i-1][j-1] + copy[i-1][j] + copy[i-1][j+1] 
                + copy[i][j-1] + copy[i][j+1] 
                + copy[i+1][j-1] + copy[i+1][j] + copy[i+1][j+1];
            if (sum < 4) {
                table[i][j] = 0;
            }
            else if (sum == 4) {
                table[i][j] = dist(rng);
            }
            else {
                table[i][j] = 1;
            }
        }
    }
    for (int i = 1; i < DIM; i++) {
        int j = 0;
        int sum = 0;
        sum += copy[i-1][j-1+DIM] + copy[i-1][j] + copy[i-1][j+1] 
            + copy[i][j-1+DIM]+ copy[i][j+1] 
            + copy[i+1][j-1+DIM] + copy[i+1][j] + copy[i+1][j+1];
        if (sum < 4) {
            table[i][j] = 0;
        }
        else if (sum == 4) {
            table[i][j] = dist(rng);
        }
        else {
            table[i][j] = 1;
        }
        j = DIM;
        sum = 0;
        sum += copy[i-1][j-1] + copy[i-1][j] + copy[i-1][j+1-DIM]
            + copy[i][j-1] + copy[i][j+1-DIM]
            + copy[i+1][j-1] + copy[i+1][j] + copy[i+1][j+1-DIM];
        if (sum < 4) {
            table[i][j] = 0;
        }
        else if (sum == 4) {
            table[i][j] = dist(rng);
        }
        else {
            table[i][j] = 1;
        }
    }
    for (int j = 1; j < DIM; j++) {
        int i = 0;
        int sum = 0;
        sum += copy[i-1+DIM][j-1] + copy[i-1+DIM][j] + copy[i-1+DIM][j+1]  
                + copy[i][j-1] + copy[i][j+1] 
                + copy[i+1][j-1] + copy[i+1][j] + copy[i+1][j+1];
        if (sum < 4) {
            table[i][j] = 0;
        }
        else if (sum == 4) {
            table[i][j] = dist(rng);
        }
        else {
            table[i][j] = 1;
        }
        i = DIM;
        sum = 0;
        sum += copy[i-1][j-1] + copy[i-1][j] + copy[i-1][j+1] 
                + copy[i][j-1] + copy[i][j+1]
                + copy[i+1-DIM][j-1] + copy[i+1-DIM][j] + copy[i+1-DIM][j+1];
        if (sum < 4) {
            table[i][j] = 0;
        }
        else if (sum == 4) {
            table[i][j] = dist(rng);
        }
        else {
            table[i][j] = 1;
        }
    }

    table[0][0] = (copy[0][1] + copy[1][0] + copy[1][1]) > 1 ? 1 : 0;
    table[0][DIM] = (copy[0][DIM-1] + copy[1][DIM] + copy[1][DIM-1]) > 1 ? 1 : 0;
    table[DIM][0] = (copy[DIM][1] + copy[DIM-1][0] + copy[DIM-1][1]) > 1 ? 1 : 0;
    table[DIM][DIM] = (copy[DIM-1][DIM] + copy[DIM][DIM-1] + copy[DIM-1][DIM-1]) > 1 ? 1 : 0;
}

void gen_data(std::vector<std::vector<uint8_t>>& table) {
    std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dist(0, 1);
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            table[i][j] = dist(rng);
        }
    }
}

void save(const std::string& filename) {
    std::ofstream ofs(filename, std::ios_base::out | std::ios_base::binary);
    ofs << "P6\n" << DIM << ' ' << DIM << "\n255\n";

    for (auto j = 0u; j < DIM; ++j) {
        for (auto i = 0u; i < DIM; ++i) {
            if (data[i][j] == 0) {
                ofs << static_cast<char>(0) << static_cast<char>(0) << static_cast<char>(0);
            }
            else {
                ofs << static_cast<char>(255) << static_cast<char>(255) << static_cast<char>(255);
            }
        }
    }
}

int digits(int number) {
    int digits = 0; while (number != 0) { number /= 10; digits++; }
    return digits;
}

std::string fileName(int fileIterator) {
    std::stringstream x;
    x << "images/output_";
    for (int i = 0; i < digits(STEPS) - digits(fileIterator); i++) {
        x << "0";
    }
    x << fileIterator;
    x << ".ppm";
    return x.str();
}

int countDensity(std::vector<std::vector<uint8_t>>& table) {
    int count = 0;
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            if (table[i][j] == 1)
                count++;
        }
    }
    return count;
}

int main() {
    gen_data(data);

    std::ofstream outFile("densityData.csv");
    
    for (int i = 0; i < STEPS; i++) {
        outFile << i << "," << countDensity(data) << std::endl;
        save(fileName(i));
        step(data);
    }
    
    outFile.close();
}