#include <vector>

class Individual {
public:
    // Genotype: Vector representing the number of ambulances at each depot
    std::vector<int> genotype;
    int numAmbulances;
    int numDepots;

    // Constructor to initialize an individual with a specific number of depots
    Individual(int numDepots) : genotype(numDepots, 0) {}

    // Method to set ambulances at a specific depot
    void setAmbulancesAtDepot(int depotIndex, int count) {
        if (depotIndex >= 0 && depotIndex < genotype.size()) {
            genotype[depotIndex] = count;
        }
    }

    // Method to get the number of ambulances at a specific depot
    int getAmbulancesAtDepot(int depotIndex) const {
        return (depotIndex >= 0 && depotIndex < genotype.size()) ? genotype[depotIndex] : -1;
    }
};