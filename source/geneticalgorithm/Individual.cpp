#include <vector>
#include <iostream>
#include <numeric>

#include "GAUtils.cpp"

class Individual {
private:
    std::vector<int> genotype;
    int numAmbulances;
    int numDepots;

public:
    /**
     * @brief Construct a new Individual object with specified number of depots.
     * @param numDepots Number of depots in the genotype.
     */
    Individual(int numDepots) : genotype(numDepots, 0) {}

    /**
     * @brief Randomizes the number of ambulances at each depot.
     */
    void randomizeAmbulances() {
        std::fill(genotype.begin(), genotype.end(), 0); // Reset all depots to 0 ambulances

        for (int i = 0; i < numAmbulances; ++i) {
            int depotIndex = RandomUtil::randomInt(0, genotype.size() - 1);
            genotype[depotIndex]++;
        }
    }

    /**
     * @brief Checks if the individual's genotype is valid.
     * @return true if the total number of ambulances matches numAmbulances, otherwise false.
     */
    bool isValid() const {
        int totalAmbulances = std::accumulate(genotype.begin(), genotype.end(), 0);
        return totalAmbulances == numAmbulances;
        }

    void printChromosome() const {
        for (int i = 0; i < genotype.size(); ++i) {
            std::cout << "Depot " << i << ": " << genotype[i] << " ambulances" << std::endl;
        }
    }

    /**
     * @brief Prints the chromosome (genotype) of the individual.
     */
    const std::vector<int>& getGenotype() const {
        return genotype;
    }

    void setGenotype(const std::vector<int>& newGenotype) {
        genotype = newGenotype;
    }

    void setAmbulancesAtDepot(int depotIndex, int count) {
        if (depotIndex >= 0 && depotIndex < genotype.size()) {
            genotype[depotIndex] = count;
        }
    }

    int getAmbulancesAtDepot(int depotIndex) const {
        return (depotIndex >= 0 && depotIndex < genotype.size()) ? genotype[depotIndex] : -1;
    }

    int getNumAmbulances() const {
        return numAmbulances;
    }

    void setNumAmbulances(int newNumAmbulances) {
        numAmbulances = newNumAmbulances;
    }

    int getNumDepots() const {
        return numDepots;
    }

    void setNumDepots(int newNumDepots) {
        numDepots = newNumDepots;
    }
};