#include <vector>
#include <random>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>
#include <future>
#include <iomanip>
#include <mutex>
#include <filesystem>
#include <map>

#define STEPS 1000
//{10, 100, 200, 500, 1000}
int DIM = 200;
//{0.05, 0.1, 0.3, 0.5, 0.6, 0.75, 0.8, 0.95}
double p0 = 0.3;

std::mutex mtx;

void init(std::vector<std::vector<uint8_t>>& vec) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dist(p0);
    
    for (int i = 0; i < DIM; i++) {
        for (int j = 0; j < DIM; j++) {
            vec[i][j] = static_cast<uint8_t>(dist(gen));
        }
    }
}

uint8_t checkNeighbors(int x, int y, std::vector<std::vector<uint8_t>>& vec) {
    int neighbors = 0;
    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
            if (!(i == 0 && j == 0)) {
                int c_x = x + i;
                int c_y = y + j;

                if (c_x < 0) c_x = DIM - 1;
                else if (c_x >= DIM) c_x = 0;

                if (c_y < 0) c_y = DIM - 1;
                else if (c_y >= DIM) c_y = 0;

                if (vec[c_x][c_y] == 1) {
                    neighbors++; 
                }
            }
        }
    }
    return neighbors;
}

void step(std::vector<std::vector<uint8_t>>& current, std::vector<std::vector<uint8_t>>& next) {
    for (auto i = 0u; i < DIM; i++) {
        for (auto j = 0u; j < DIM; j++) {
            auto n = checkNeighbors(i, j, current); 
            if ((current[i][j] == 0 && n == 3) || (current[i][j] == 1 && (n == 2 || n == 3))) {
                next[i][j] = 1;
            }
            else {
                next[i][j] = 0;
            }
            
        }
    }
}

void save(const std::string& filename, std::vector<std::vector<uint8_t>>* vec) {
    std::ofstream ofs(filename, std::ios_base::out | std::ios_base::binary);
    std::lock_guard<std::mutex> lock(mtx);
    ofs << "P6\n" << DIM << ' ' << DIM << "\n255\n";

    for (auto& row : *vec) {
        for (auto cell : row) {
            char p = cell ? 255 : 0;
            ofs << 0 << p << 0;
        }
    }

    ofs.close();
}

std::string fileName(int fileIterator) {
    std::ostringstream x;
    x << "images/output_" << std::setw(4) << std::setfill('0') << fileIterator << ".ppm";
    return x.str();
}

void saveDensities(std::vector<double> dV) {
    std::ostringstream x;
    x << "dd_" << p0*100 << ".txt";
    std::ofstream out(x.str());
    for (double d : dV) {
        out << d << "\n";
    }
    out.close();
}

double countDensity(std::vector<std::vector<uint8_t>>& vec) {
    double count = 0;
    for (auto i = 0u; i < DIM; i++) {
        for (auto j = 0u; j < DIM; j++) {
            if (vec[i][j] == 1)
                count++;
        }
    }
    return count/(DIM*DIM);
}

double mean(const std::vector<double>& data) {
    double sum = 0.0;
    for (double val : data) {
        sum += val;
    }
    return sum/data.size();
}

double stdVar(const std::vector<double>& data, double mean) {
    double var = 0.0;
    for (double val : data) {
        var += (val - mean) * (val - mean);
    }
    var /= data.size();
    return std::sqrt(var);
}

double stdErr(const std::vector<double>& data) {
    if (data.empty()) return 0.0;

    return stdVar(data, mean(data)) / std::sqrt(data.size());
}

void makeAndFrames() {
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<uint8_t>> grid1(DIM, std::vector<uint8_t>(DIM));
    std::vector<std::vector<uint8_t>> grid2(DIM, std::vector<uint8_t>(DIM));    
    auto* current = &grid1;
    auto* next = &grid2;

    init(*current);
    std::vector<std::future<void>> futures;

    futures.push_back(std::async(std::launch::async, save, fileName(0), current));
    for (int i = 1; i <= STEPS; i++) {
        step(*current, *next);
        futures.push_back(std::async(std::launch::async, save, fileName(i), current));
        std::swap(current, next);
    }

    for (auto& f : futures) f.get();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "M&F [p = " << p0 << "], [size = " << DIM << "]: Elapsed time: " << elapsed.count() << "s\n";
}

double makeAndCount() {
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<uint8_t>> grid1(DIM, std::vector<uint8_t>(DIM));
    std::vector<std::vector<uint8_t>> grid2(DIM, std::vector<uint8_t>(DIM));    
    auto* current = &grid1;
    auto* next = &grid2;

    std::vector<double> densityValues(STEPS);
    
    init(*current);

    for (int i = 0; i < STEPS; i++) {
        densityValues[i] = countDensity(*current);
        step(*current, *next);
        std::swap(current, next);
    }

    // saveDensities(densityValues);

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    std::cout << "M&C [p = " << p0 << "], [size = " << DIM << "]: Elapsed time: " << elapsed.count() << "s\n";
    return stdErr(densityValues);
}

void runSimulationForSize(int size, std::map<int, std::vector<double>>& results) {
    DIM = size;
    std::vector<double> errors;

    for (int i = 0; i < 100; i++) {
        errors.push_back(makeAndCount());
    }

    results[size] = errors;
}

int main() {
    std::vector<int> size_vals = {10, 100, 200, 500, 1000};
    std::map<int, std::vector<double>> results;

    std::vector<std::future<void>> futures;
    for (int s : size_vals) {
        runSimulationForSize(s, results);
    }

    for (auto& f : futures) f.get();

    std::ofstream out("results.txt");
    for (const auto& [s, errors] : results) {
        double meanError = mean(errors);
        out << "Size = " << s << ", MeanStdError = " << meanError << "\n";
    }
    out.close();

    std::cout << "Results written to results.txt\n";
    return 0;
}