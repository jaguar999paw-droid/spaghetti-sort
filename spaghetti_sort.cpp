/**
 * spaghetti_sort.cpp
 *
 * Spaghetti Sort — A physical/analog sorting algorithm simulated in C++
 *
 * CONCEPT:
 *   In the physical world, you cut spaghetti strands to lengths proportional
 *   to the values you want to sort. You drop the bundle on a table (gravity
 *   aligns all bottoms), then sweep your hand downward from above. The first
 *   strand you touch is the maximum. Repeat for each subsequent maximum.
 *
 *   The trick: ALL strands are "compared" simultaneously by gravity — O(1) per
 *   extraction, O(n) total. A computer can't do true physical parallelism, so
 *   we simulate it by sleeping each value's thread for a duration proportional
 *   to its value. Smaller values wake up sooner → they're "shorter strands".
 *
 * SIMULATION STRATEGY:
 *   - Each element spawns a thread.
 *   - The thread sleeps for (value * TIME_UNIT) milliseconds.
 *   - On wake, the thread pushes its value into a shared output vector.
 *   - Result: values arrive in ascending order — sorted!
 *
 * COMPLEXITY (this simulation):
 *   Time:  O(max_value) — bounded by the longest sleep, not n
 *   Space: O(n)         — one thread per element
 *
 * BUILD:
 *   g++ -std=c++17 -pthread spaghetti_sort.cpp -o spaghetti_sort
 */

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <algorithm>
#include <random>

// ─── Configuration ────────────────────────────────────────────────────────────

// Each unit of value = this many milliseconds of sleep.
// Lower = faster simulation; higher = more visually obvious timing gaps.
constexpr int TIME_UNIT_MS = 20;

// ─── Shared state ─────────────────────────────────────────────────────────────

std::vector<int> sorted_output;   // Strands "land" here in order of height
std::mutex output_mutex;          // Guards sorted_output from concurrent writes

// ─── The "strand" function ────────────────────────────────────────────────────

/**
 * strand_thread()
 *
 * Simulates a single spaghetti strand of length `value`.
 *
 * A shorter strand (smaller value) will sleep less and therefore push itself
 * into sorted_output earlier — exactly as a shorter physical strand would be
 * "picked last" when sweeping from the top (we collect in ascending order here).
 *
 * @param value  The numeric value this strand represents
 */
void strand_thread(int value) {
    // ── Step 1: "Stand upright" ──────────────────────────────────────────────
    // Sleep duration is proportional to the strand's length (value).
    // This is the gravity simulation: all threads start simultaneously,
    // but shorter ones finish sleeping first.
    std::this_thread::sleep_for(std::chrono::milliseconds(value * TIME_UNIT_MS));

    // ── Step 2: "Hand touches this strand" ──────────────────────────────────
    // The thread woke up, meaning it's the shortest remaining strand.
    // Safely append to the shared sorted result.
    {
        std::lock_guard<std::mutex> lock(output_mutex);
        sorted_output.push_back(value);
    }
}

// ─── Main spaghetti_sort function ─────────────────────────────────────────────

/**
 * spaghetti_sort()
 *
 * Sorts a vector of non-negative integers using the spaghetti sort simulation.
 *
 * @param input   The unsorted values (analogous to the uncut bundle of spaghetti)
 * @return        A new vector containing values in ascending sorted order
 */
std::vector<int> spaghetti_sort(const std::vector<int>& input) {
    sorted_output.clear();

    // ── Step 1: "Cut the strands" ────────────────────────────────────────────
    // Spawn one thread per element. All threads start simultaneously —
    // this is the moment you "drop the bundle" onto the table.
    std::vector<std::thread> threads;
    threads.reserve(input.size());

    std::cout << "\n[spaghetti_sort] Dropping " << input.size()
              << " strands onto the table...\n";

    for (int val : input) {
        // Each strand gets its own thread, sleeping proportionally to its value
        threads.emplace_back(strand_thread, val);
    }

    // ── Step 2: "Wait for all strands to fall" ───────────────────────────────
    // Join all threads — we wait for every strand to have been "touched"
    // (i.e., every thread has pushed its value to sorted_output).
    for (auto& t : threads) {
        t.join();
    }

    // ── Step 3: Return the result ────────────────────────────────────────────
    // sorted_output now contains all values in ascending order
    std::cout << "[spaghetti_sort] All strands collected. Sorting complete.\n\n";
    return sorted_output;
}

// ─── Helpers ──────────────────────────────────────────────────────────────────

/** Pretty-print a vector */
void print_vec(const std::string& label, const std::vector<int>& v) {
    std::cout << label << ": [ ";
    for (int x : v) std::cout << x << " ";
    std::cout << "]\n";
}

/** Verify result is actually sorted */
bool is_sorted_asc(const std::vector<int>& v) {
    return std::is_sorted(v.begin(), v.end());
}

// ─── Entry point ──────────────────────────────────────────────────────────────

int main() {
    std::cout << "╔══════════════════════════════════════════╗\n";
    std::cout << "║        🍝  Spaghetti Sort Demo           ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n";

    // ── Test 1: Small hand-crafted input ────────────────────────────────────
    {
        std::vector<int> data = {8, 3, 7, 1, 5, 2, 9, 4, 6};

        std::cout << "\n── Test 1: Hand-crafted input ──\n";
        print_vec("Input ", data);

        auto result = spaghetti_sort(data);

        print_vec("Output", result);
        std::cout << "Sorted correctly: " << (is_sorted_asc(result) ? "✅ YES" : "❌ NO") << "\n";
    }

    // ── Test 2: Already sorted ───────────────────────────────────────────────
    {
        std::vector<int> data = {1, 2, 3, 4, 5};

        std::cout << "\n── Test 2: Already sorted input ──\n";
        print_vec("Input ", data);

        auto result = spaghetti_sort(data);

        print_vec("Output", result);
        std::cout << "Sorted correctly: " << (is_sorted_asc(result) ? "✅ YES" : "❌ NO") << "\n";
    }

    // ── Test 3: Reverse sorted ───────────────────────────────────────────────
    {
        std::vector<int> data = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

        std::cout << "\n── Test 3: Reverse sorted input ──\n";
        print_vec("Input ", data);

        auto result = spaghetti_sort(data);

        print_vec("Output", result);
        std::cout << "Sorted correctly: " << (is_sorted_asc(result) ? "✅ YES" : "❌ NO") << "\n";
    }

    // ── Test 4: Random input ─────────────────────────────────────────────────
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

    std::cout << "\n╔══════════════════════════════════════════╗\n";
    std::cout << "║           All tests complete             ║\n";
    std::cout << "╚══════════════════════════════════════════╝\n";

    return 0;
}
