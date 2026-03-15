// Author: PAUL WAMBUGU | Reg No: EB3/61578/22
// spaghetti_sort.cpp — Simulates Spaghetti Sort using threads (each sleeps proportionally to its value)

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <random>
#include <functional>

// TIME_UNIT_MS: sleep duration per value unit. TIE_BREAK_JITTER_US: sub-ms offset for equal values.
constexpr int TIME_UNIT_MS = 20;
constexpr int TIE_BREAK_JITTER_US = 500;

std::vector<int> sorted_output;
std::mutex output_mutex;

// Each thread sleeps for (value * TIME_UNIT_MS) ms, then appends to sorted_output — shorter sleeps = smaller values arrive first.
void strand_thread(int value, size_t strand_id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(value * TIME_UNIT_MS));
    size_t jitter_us = (std::hash<size_t>{}(strand_id) % TIE_BREAK_JITTER_US);
    std::this_thread::sleep_for(std::chrono::microseconds(jitter_us));
    std::lock_guard<std::mutex> lock(output_mutex);
    sorted_output.push_back(value);
}

// Spawns one thread per element simultaneously; joins all threads; returns values in ascending order.
std::vector<int> spaghetti_sort(const std::vector<int>& input) {
    sorted_output.clear();
    std::vector<std::thread> threads;
    threads.reserve(input.size());

    std::cout << "\n[spaghetti_sort] Dropping " << input.size() << " strands onto the table...\n";

    for (size_t i = 0; i < input.size(); ++i)
        threads.emplace_back(strand_thread, input[i], i);

    for (auto& t : threads)
        t.join();

    std::cout << "[spaghetti_sort] All strands collected. Sorting complete.\n\n";
    return sorted_output;
}

void print_vec(const std::string& label, const std::vector<int>& v) {
    std::cout << label << ": [ ";
    for (int x : v) std::cout << x << " ";
    std::cout << "]\n";
}

bool is_sorted_asc(const std::vector<int>& v) {
    return std::is_sorted(v.begin(), v.end());
}

int main() {
    std::cout << "╔══════════════════════════════════════════╗\n";
    std::cout << "║        🍝  Spaghetti Sort Demo           ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n";

    // Test 1: Hand-crafted input
    {
        std::vector<int> data = {8, 3, 7, 1, 5, 2, 9, 4, 6};
        std::cout << "\n── Test 1: Hand-crafted input ──\n";
        print_vec("Input ", data);
        auto result = spaghetti_sort(data);
        print_vec("Output", result);
        std::cout << "Sorted correctly: " << (is_sorted_asc(result) ? "✅ YES" : "❌ NO") << "\n";
    }

    // Test 2: Already sorted
    {
        std::vector<int> data = {1, 2, 3, 4, 5};
        std::cout << "\n── Test 2: Already sorted input ──\n";
        print_vec("Input ", data);
        auto result = spaghetti_sort(data);
        print_vec("Output", result);
        std::cout << "Sorted correctly: " << (is_sorted_asc(result) ? "✅ YES" : "❌ NO") << "\n";
    }

    // Test 3: Reverse sorted
    {
        std::vector<int> data = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
        std::cout << "\n── Test 3: Reverse sorted input ──\n";
        print_vec("Input ", data);
        auto result = spaghetti_sort(data);
        print_vec("Output", result);
        std::cout << "Sorted correctly: " << (is_sorted_asc(result) ? "✅ YES" : "❌ NO") << "\n";
    }

    // Test 4: Random input (seed=42)
    {
        std::mt19937 rng(42);
        std::uniform_int_distribution<int> dist(1, 15);
        std::vector<int> data(12);
        std::generate(data.begin(), data.end(), [&]() { return dist(rng); });
        std::cout << "\n── Test 4: Random input (seed=42) ──\n";
        print_vec("Input ", data);
        auto result = spaghetti_sort(data);
        print_vec("Output", result);
        std::cout << "Sorted correctly: " << (is_sorted_asc(result) ? "✅ YES" : "❌ NO") << "\n";
    }

    // Test 5: All duplicates (regression for equal-value race condition)
    {
        std::vector<int> data = {5, 5, 5, 5, 5};
        std::cout << "\n── Test 5: All duplicates ──\n";
        print_vec("Input ", data);
        auto result = spaghetti_sort(data);
        print_vec("Output", result);
        std::cout << "Sorted correctly: " << (is_sorted_asc(result) ? "✅ YES" : "❌ NO") << "\n";
    }

    // Test 6: Mixed duplicates
    {
        std::vector<int> data = {4, 2, 4, 1, 2, 3, 1};
        std::cout << "\n── Test 6: Mixed duplicates ──\n";
        print_vec("Input ", data);
        auto result = spaghetti_sort(data);
        print_vec("Output", result);
        std::cout << "Sorted correctly: " << (is_sorted_asc(result) ? "✅ YES" : "❌ NO") << "\n";
    }

    std::cout << "\n╔══════════════════════════════════════════╗\n";
    std::cout << "║           All tests complete             ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n";

    return 0;
}
