#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define N 10000000
#define BLOCK_SIZE (10 * 12345) // Замените 12345 на номер вашего студбилета

// Глобальные переменные
double pi = 0.0;
int current_iteration = 0;
int active_threads = 0;
HANDLE mutex;
HANDLE resume_semaphore;

DWORD WINAPI CalculatePi(LPVOID param) {
    while (1) {
        WaitForSingleObject(mutex, INFINITE);
        
        if (current_iteration >= N) {
            active_threads--;
            ReleaseMutex(mutex);
            break;
        }
        
        int start = current_iteration;
        int end = (start + BLOCK_SIZE < N) ? start + BLOCK_SIZE : N;
        current_iteration = end;
        ReleaseMutex(mutex);
        
        double partial_sum = 0.0;
        for (int i = start; i < end; i++) {
            double x = (i + 0.5) * (1.0 / N);
            partial_sum += 4.0 / (1.0 + x * x);
        }
        
        WaitForSingleObject(mutex, INFINITE);
        pi += partial_sum;
        ReleaseMutex(mutex);
        
        WaitForSingleObject(resume_semaphore, INFINITE);
    }
    return 0;
}

int main() {
    // Инициализация таймера
    LARGE_INTEGER frequency, start_time, end_time;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start_time);

    int num_threads = 4; // Можно изменять количество потоков
    HANDLE* threads = (HANDLE*)malloc(num_threads * sizeof(HANDLE));
    
    mutex = CreateMutex(NULL, FALSE, NULL);
    resume_semaphore = CreateSemaphore(NULL, 0, num_threads, NULL);
    
    active_threads = num_threads;
    for (int i = 0; i < num_threads; i++) {
        threads[i] = CreateThread(NULL, 0, CalculatePi, NULL, 0, NULL);
        if (threads[i] == NULL) {
            printf("Error creating thread %d\n", i);
            return 1;
        }
    }
    
    while (active_threads > 0) {
        ReleaseSemaphore(resume_semaphore, active_threads, NULL);
        Sleep(10);
        
        WaitForSingleObject(mutex, INFINITE);
        int work_remaining = (current_iteration < N);
        ReleaseMutex(mutex);
        
        if (!work_remaining) {
            ReleaseSemaphore(resume_semaphore, active_threads, NULL);
            break;
        }
    }
    
    WaitForMultipleObjects(num_threads, threads, TRUE, INFINITE);
    
    pi /= N;
    
    // Замер времени окончания
    QueryPerformanceCounter(&end_time);
    double elapsed_time = (double)(end_time.QuadPart - start_time.QuadPart) / frequency.QuadPart;
    
    printf("Calculated pi = %.15f\n", pi);
    printf("Execution time: %.5f seconds\n", elapsed_time);
    printf("Threads used: %d\n", num_threads);
    
    // Освобождение ресурсов
    for (int i = 0; i < num_threads; i++) {
        CloseHandle(threads[i]);
    }
    free(threads);
    CloseHandle(mutex);
    CloseHandle(resume_semaphore);
    
    return 0;
}