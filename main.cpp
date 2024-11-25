#include <iostream>
#include <thread>
#include <mutex>
#include <stdexcept>
#include <chrono>

// Struct used by both threads
struct SharedData {
    int x, y, z;
    int ready;
    std::mutex lock;
};

void thread_children(SharedData* data) {
    try {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::lock_guard<std::mutex> lk(data->lock);
            if (data->ready == 1) {
                // Calculate z = x + y
                data->z = data->x + data->y;
                std::cout << "Child thread: Calculated x + y = " << data->z << std::endl;

                // Set ready to 0
                data->ready = 0;
                break;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Child thread error: " << e.what() << std::endl;
    }
}

int main() {
    SharedData data;
    data.ready = 0;
    std::thread child_thread(thread_children, &data);

    try {
        // Parent thread reads values for x and y
        {
            std::lock_guard<std::mutex> lk(data.lock);
            std::cout << "Parent thread: Enter value for x: ";
            if (!(std::cin >> data.x)) {
                throw std::runtime_error("Invalid input for x");
            }
            std::cout << "Parent thread: Enter value for y: ";
            if (!(std::cin >> data.y)) {
                throw std::runtime_error("Invalid input for y");
            }

            // Set ready to 1
            data.ready = 1;
        }

        // Wait until ready is 0
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            std::lock_guard<std::mutex> lk(data.lock);
            if (data.ready == 0) {
                break;
            }
        }

        // Print the value of z
        std::cout << "Parent thread: Value of z = " << data.z << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Parent thread error: " << e.what() << std::endl;
        child_thread.join();
        return 1;
    }

    // Wait for the child thread to finish
    child_thread.join();

    return 0;
}