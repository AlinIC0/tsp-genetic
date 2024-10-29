//GENETIC

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <random>
#include <numeric>

using namespace std;

struct City {
    string name;
    double x, y;
};

//Incarca datele despre orase dintr-un fisier text
vector<City> loadCitiesFromText(const string& filename) {
    ifstream file(filename);
    vector<City> cities;
    string line, name;
    double x, y;

    if (!file.is_open()) {
        cerr << "Nu s-a gasit fisierul txt: " << filename << endl;
        return cities;
    }

    while (getline(file, line)) {
        stringstream ss(line);
        ss >> name >> x >> y;
        cities.push_back({ name, x, y });
    }
    file.close();
    return cities;
}

//Calculeaza distanta euclidian intre doua orase
double euclidean_distance(const City& a, const City& b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

//Construieste o matrice de distante intre toate perechile de orase
vector<vector<double>> calculate_distance_matrix(const vector<City>& cities) {
    int num_cities = cities.size();
    vector<vector<double>> dist_matrix(num_cities, vector<double>(num_cities, 0.0));

    for (int i = 0; i < num_cities; ++i) {
        for (int j = 0; j < num_cities; ++j) {
            if (i != j) {
                dist_matrix[i][j] = euclidean_distance(cities[i], cities[j]);
            }
        }
    }
    return dist_matrix;
}

//Calculeaza distanta toatala pentru un tur dat
double calculate_total_distance(const vector<int>& tour, const vector<vector<double>>& dist_matrix) {
    double total_distance = 0.0;
    for (size_t i = 0; i < tour.size() - 1; ++i) {
        total_distance += dist_matrix[tour[i]][tour[i + 1]];
    }
    total_distance += dist_matrix[tour.back()][tour[0]];  // return to the starting city
    return total_distance;
}

//Genereaza un tur aleatoriu al oraselor
vector<int> random_tour(int num_cities) {
    vector<int> tour(num_cities);
    iota(tour.begin(), tour.end(), 0);
    random_device rd;
    mt19937 g(rd());
    shuffle(tour.begin(), tour.end(), g);
    return tour;
}

//Genereaza o populatie de ture aleatorii
vector<vector<int>> generate_population(int population_size, int num_cities) {
    vector<vector<int>> population;
    for (int i = 0; i < population_size; ++i) {
        population.push_back(random_tour(num_cities));
    }
    return population;
}

//Selecteaza un individ din populatie folosind metoda turneului
vector<int> tournament_selection(const vector<vector<int>>& population, const vector<double>& fitness_values, int tournament_size) {
    random_device rd;
    mt19937 g(rd());
    uniform_int_distribution<> dis(0, population.size() - 1);

    vector<int> best = population[dis(g)];
    double best_fitness = numeric_limits<double>::max();

    for (int i = 0; i < tournament_size; ++i) {
        int idx = dis(g);
        if (fitness_values[idx] < best_fitness) {
            best_fitness = fitness_values[idx];
            best = population[idx];
        }
    }
    return best;
}
//Realizeaza crossover intre doi parinti pentru a crea un copil

vector<int> crossover(const vector<int>& parent1, const vector<int>& parent2) {
    //Implementarea crossover-ului
    int size = parent1.size();
    vector<int> child(size, -1);

    random_device rd;
    mt19937 g(rd());
    uniform_int_distribution<> dis(0, size - 1);

    int start = dis(g);
    int end = dis(g);

    if (start > end) {
        swap(start, end);
    }

    for (int i = start; i <= end; ++i) {
        child[i] = parent1[i];
    }

    int current_pos = (end + 1) % size;
    for (int i = 0; i < size; ++i) {
        int city = parent2[(end + 1 + i) % size];
        if (find(child.begin(), child.end(), city) == child.end()) {
            child[current_pos] = city;
            current_pos = (current_pos + 1) % size;
        }
    }

    return child;
}

//Aplica mutatii asupra unui tur
void mutate(vector<int>& tour, double mutation_rate) {
    random_device rd;
    mt19937 g(rd());
    uniform_real_distribution<> dis(0.0, 1.0);

    for (size_t i = 0; i < tour.size(); ++i) {
        if (dis(g) < mutation_rate) {
            uniform_int_distribution<> swap_pos(0, tour.size() - 1);
            int j = swap_pos(g);
            swap(tour[i], tour[j]);
        }
    }
}

int main() {
    string filename = "tsp.txt";  
    vector<City> cities = loadCitiesFromText(filename);
    vector<vector<double>> dist_matrix = calculate_distance_matrix(cities);

    if (cities.empty()) {
        cout << "Nu s-au incarcat orase din txt." << endl;
        return 1;
    }

    int population_size = 100;
    int generations = 500;
    double mutation_rate = 0.01;
    int tournament_size = 5;

    vector<vector<int>> population = generate_population(population_size, cities.size());

    for (int generation = 0; generation < generations; ++generation) {
        vector<double> fitness_values(population.size());

        for (size_t i = 0; i < population.size(); ++i) {
            fitness_values[i] = calculate_total_distance(population[i], dist_matrix);
        }

        vector<vector<int>> new_population;
        for (size_t i = 0; i < population.size(); ++i) {
            vector<int> parent1 = tournament_selection(population, fitness_values, tournament_size);
            vector<int> parent2 = tournament_selection(population, fitness_values, tournament_size);
            vector<int> child = crossover(parent1, parent2);
            mutate(child, mutation_rate);
            new_population.push_back(child);
        }

        population = new_population;

       
        double best_distance = *min_element(fitness_values.begin(), fitness_values.end());
        cout << "Generatia " << generation + 1 << ": Cea mai buna distanta = " << best_distance << endl;
    }

   
    double best_distance = numeric_limits<double>::max();
    vector<int> best_tour;
    for (const auto& tour : population) {
        double distance = calculate_total_distance(tour, dist_matrix);
        if (distance < best_distance) {
            best_distance = distance;
            best_tour = tour;
        }
    }

    double fuel_consumption_per_km = 7.5 / 100.0;  
    double fuel_cost_per_liter = 1.5; 
    double total_fuel_needed = best_distance * fuel_consumption_per_km;
    double total_fuel_cost = total_fuel_needed * fuel_cost_per_liter;
    double vendor_request = 100.0;

    cout << "Cea mai scurta distanta: " << best_distance << " km" << endl;
    cout << "Costul estimat al combustibilului: " << total_fuel_cost << " $" << endl;
    cout << "Cost cerut de vanzator: " << vendor_request << " $" << endl;

    if (total_fuel_cost < vendor_request) {
        cout << "Compania ar trebui sa ofere o contraoferta de: " << total_fuel_cost << " $" << endl;
    }
    else {
        cout << "Compania ar trebui sa ofere o contraoferta de: " << vendor_request << " $" << endl;
    }

    return 0;
}
