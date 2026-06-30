#include <iostream>

void run_allocation_benchmarks();
void run_lifecycle_benchmarks();
void run_marker_benchmarks();
void run_scope_benchmarks();
void run_reset_benchmarks();

int main() {
    run_allocation_benchmarks();
    run_lifecycle_benchmarks();
    run_marker_benchmarks();
    run_scope_benchmarks();
    run_reset_benchmarks();
    
    return 0;
}

