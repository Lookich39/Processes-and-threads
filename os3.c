#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define N 10000000
#define BLOCK_SIZE (3316130)

double pi = 0.0;
int current_iteration = 0;
HANDLE mutex;

DWORD WINAPI CalculatePi(LPVOID param) {
    double local_pi = 0.0;
    int start, end;

    while (1) {
        WaitForSingleObject(mutex, INFINITE);
        start = current_iteration;
        end = (current_iteration += BLOCK_SIZE);
        ReleaseMutex(mutex);

        if (start >= N) break;

        if (end > N) end = N;

        for (int i = start; i < end; i++) {
            double x = (i + 0.5) * (1.0 / N);
            local_pi += 4.0 / (1.0 + x * x);
        }
    }

    WaitForSingleObject(mutex, INFINITE);
    pi += local_pi;
    ReleaseMutex(mutex);

    return 0;
}

int main() {
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    int num_threads = 1;

    HANDLE* threads = (HANDLE*)malloc(num_threads * sizeof(HANDLE));
    mutex = CreateMutex(NULL, FALSE, NULL);
    current_iteration = 0;

    for (int i = 0; i < num_threads; i++) {
        threads[i] = CreateThread(NULL, 0, CalculatePi, NULL, 0, NULL);
        if (!threads[i]) {
            printf("Ошибка создания потока %d\n", i);
            return 1;
        }
    }

    WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE);

    pi /= N;
    QueryPerformanceCounter(&end);
    double time = (double)(end.QuadPart - start.QuadPart) / freq.QuadPart;

    printf("Calculated pi = %.15f\n", pi);
    printf("Execution time: %.3f seconds\n", time);
    printf("Threads used: %d\n", num_threads);

    for (int i = 0; i < num_threads; i++) CloseHandle(threads[i]);
    free(threads);
    CloseHandle(mutex);

    return 0;
}