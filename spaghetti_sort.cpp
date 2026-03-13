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
 * DUPLICATE HANDLING (fixes #1):
 *   When two strands have equal length, both threads sleep the same duration
 *   and wake at approximately the same time — OS scheduling determines which
 *   appends first. We add a tiny sub-millisecond jitter seeded from thread ID
 *   to break ties deterministically without meaningfully affecting sort order.
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
#include <functional>  // std::hash

// ─── Configuration ────────────────────────────────────────────────────────────

// Each unit of value = this many milliseconds of sleep.
// Lower = faster simulation; higher = more visually obvious timing gaps.
constexpr int TIME_UNIT_MS = 20;

// Maximum microsecond jitter added to break ties between equal-value threads.
// Must be much less than TIME_UNIT_MS * 1000 to preserve sort order.
constexpr int TIE_BREAK_JITTER_US = 500;  // 0.5 ms max — safe for TIME_UNIT_MS >= 5

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
 * For duplicate values, a tiny deterministic jitter (derived from thread ID)
 * is added so equal-valued strands don't race for the mutex. This fixes #1.
 *
 * @param value     The numeric value this strand represents
 * @param strand_id Unique index of this strand in the original input (for jitter)
 */
void strand_thread(int value, size_t strand_id) {
    // ── Step 1: "Stand upright" ──────────────────────────────────────────────
    // Primary sleep: proportional to value (this IS the sort key).
    std::this_thread::sleep_for(std::chrono::milliseconds(value * TIME_UNIT_MS));

    // ── Step 1b: Tie-breaking jitter (fix for #1) ────────────────────────────
    // For equal values, add a sub-millisecond offset derived from strand_id
    // so threads don't all hammer the mutex at exactly the same instant.
    // The jitter is tiny compared to TIME_UNIT_MS so it cannot change order.
    size_t jitter_us = (std::hash<size_t>{}(strand_id) % TIE_BREAK_JITTER_US);
    std::this_thread::sleep_for(std::chrono::microseconds(jitter_us));

    // ── Step 2: "Hand touches this strand" ──────────────────────────────────
    // The thread woke up — it's the shortest remaining strand.
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

    for (size_t i = 0; i < input.size(); ++i) {
        // Each strand gets its own thread, sleeping proportionally to its value.
        // strand_id (i) is passed so equal-valued strands get unique jitter.
        threads.emplace_back(strand_thread, input[i], i);
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

    // ── Test 5: All duplicates (regression test for #1) ─────────────────────
    {
        std::vector<int> data = {5, 5, 5, 5, 5};
        std::cout << "\n── Test 5: All duplicates (regression for #1) ──\n";
        print_vec("Input ", data);
        auto result = spaghetti_sort(data);
        print_vec("Output", result);
        std::cout << "Sorted correctly: " << (is_sorted_asc(result) ? "✅ YES" : "❌ NO") << "\n";
    }

    // ── Test 6: Mixed duplicates ─────────────────────────────────────────────
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
