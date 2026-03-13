# 🍝 Spaghetti Sort

A C++ simulation of the **Spaghetti Sort** algorithm — a physical/analog sorting thought experiment that achieves O(n) time complexity by exploiting physical parallelism.

## How It Works

Imagine a bundle of uncooked spaghetti strands, each cut to a length proportional to its value:

1. Hold the bundle loosely and drop it on a flat surface — all strands stand upright, bottoms aligned by gravity
2. Lower your hand from above until it touches the tallest strand — that's your **maximum**
3. Remove it, repeat — each pass gives you the next largest in O(1) time

The magic: all strands "compare" themselves **simultaneously** via gravity. A computer can't do that natively, so we simulate it using **threads sleeping proportional to their value**.

## Algorithm Complexity

| Model        | Time Complexity |
|--------------|----------------|
| Physical     | O(n) — parallel gravity |
| This simulation | O(max_value) — thread sleep time |
| Naive CPU    | O(n²) equivalent |

## Files

| File | Description |
|------|-------------|
| `spaghetti_sort.cpp` | Core implementation with detailed comments |
| `CMakeLists.txt` | Build configuration |

## Build & Run

```bash
mkdir build && cd build
cmake ..
make
./spaghetti_sort
```

Or compile directly:

```bash
g++ -std=c++17 -pthread spaghetti_sort.cpp -o spaghetti_sort
./spaghetti_sort
```

## License

MIT
