// spaghetti_sort.cpp
//
// Simulates the Spaghetti Sort algorithm using C++ threads.
//
// The idea (physical analogy):
//   - Represent each number as a spaghetti strand cut to that length.
//   - Drop all strands on a table at once, then pick them up shortest-first.
//   - Here, each number becomes a thread that sleeps for (value × MS_PER_UNIT)
//     milliseconds. Shorter values wake up first and record themselves — producing
//     a sorted sequence without any comparison.
//
// Time complexity:  O(max_value)  — determined by the largest number, not n.
// Space complexity: O(n)          — one thread per element.
//
// Author: Paul Wambugu | Reg No: EB3/61578/22

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm>   // is_sorted, generate
#include <random>      // mt19937, uniform_int_distribution

using namespace std;

// Scale factor: 1 unit of value = how many milliseconds of sleep
const int MS_PER_UNIT = 20;

vector<int> sorted_output;   // threads append here as they finish
mutex output_lock;           // protects sorted_output from race conditions

// Each strand sleeps proportional to its value, then records itself.
// The tiny (id % 500) microsecond offset breaks ties among equal values.
void strand(int value, int id) {
    this_thread::sleep_for(chrono::milliseconds(value * MS_PER_UNIT));
    this_thread::sleep_for(chrono::microseconds(id % 500));  // tie-break
    lock_guard<mutex> lock(output_lock);
    sorted_output.push_back(value);
}

// Spawn one thread per element, wait for all to finish, return sorted result.
vector<int> spaghetti_sort(const vector<int>& input) {
    sorted_output.clear();
    vector<thread> threads;

    for (int i = 0; i < (int)input.size(); i++)
        threads.emplace_back(strand, input[i], i);

    for (auto& t : threads)
        t.join();   // block until every strand has reported in

    return sorted_output;
}

// Helper: print a labelled vector on one line
void print_vec(const string& label, const vector<int>& v) {
    cout << label << ": ";
    for (int x : v) cout << x << " ";
    cout << "\n";
}

// Helper: run one test case and report pass/fail
void run_test(const string& name, vector<int> data) {
    cout << "-- " << name << " --\n";
    print_vec("Input ", data);
    vector<int> result = spaghetti_sort(data);
    print_vec("Output", result);
    bool ok = is_sorted(result.begin(), result.end());
    cout << "Sorted correctly: " << (ok ? "YES" : "NO") << "\n\n";
}

int main() {
    // Test 1: General unsorted input
    run_test("General case", {8, 3, 7, 1, 5, 2, 9, 4, 6});

    // Test 2: Duplicate values — exercises the tie-break logic
    run_test("Duplicates", {4, 2, 4, 1, 2, 3, 1});

    // Test 3: Random input with a fixed seed for reproducibility
    mt19937 rng(42);
    uniform_int_distribution<int> dist(1, 15);
    vector<int> random_data(10);
    generate(random_data.begin(), random_data.end(), [&]() { return dist(rng); });
    run_test("Random (seed=42, n=10)", random_data);

    return 0;
}
