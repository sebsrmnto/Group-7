#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <iomanip>
#include <limits>

using namespace std;

const int MAX_FLOORS = 9;
const int MAX_REQUESTS = 3;
const int MAX_CAPACITY = 3;

struct Request {
    int from;
    int to;
};

mutex mtx;
condition_variable cv;
bool processing = false;
vector<Request> upRequests;
vector<Request> downRequests;

void simulateElevator() {
    unique_lock<mutex> lock(mtx);
    cv.wait(lock, [] { return processing; });

    int currentFloor = 1;
    int currentPassengers = 0;

    cout << "\n=== Elevator Simulation Starts ===\n";
    cout << "Elevator starts at floor 1\n";

    auto printPassengerCount = [&]() {
        cout << " [Passengers: " << currentPassengers << "]\n";
    };

    auto handleFloorActions = [&](map<int, int>& onFloors, map<int, int>& offFloors, int targetFloor) {
        // Handle passengers getting off
        if (offFloors[targetFloor] > 0) {
            int alighting = min(offFloors[targetFloor], currentPassengers);
            currentPassengers -= alighting;
            cout << setw(2) << targetFloor << " | Passengers getting off: " << alighting;
            printPassengerCount();
        }
        // Handle passengers getting on
        if (onFloors[targetFloor] > 0) {
            int boarding = min(onFloors[targetFloor], MAX_CAPACITY - currentPassengers);
            if (boarding > 0) {
                currentPassengers += boarding;
                cout << setw(2) << targetFloor << " | Passengers getting on:  " << boarding;
                printPassengerCount();
            }
        }
    };

    // Process up requests
    if (!upRequests.empty()) {
        sort(upRequests.begin(), upRequests.end(), [](const Request& a, const Request& b) {
            return a.from < b.from;
        });

        map<int, int> onFloors, offFloors;
        for (const auto& request : upRequests) {
            onFloors[request.from]++;
            offFloors[request.to]++;
        }

        cout << "\n--- Processing Up Requests ---\n";
        while (currentFloor <= MAX_FLOORS) {
            handleFloorActions(onFloors, offFloors, currentFloor);
            if (currentFloor == MAX_FLOORS) break; // Stop if the elevator is at the top floor
            cout << setw(2) << currentFloor << " >> ";
            currentFloor++;
        }
    }

    // Process down requests
    if (!downRequests.empty()) {
        sort(downRequests.begin(), downRequests.end(), [](const Request& a, const Request& b) {
            return a.from > b.from;
        });

        map<int, int> onFloors, offFloors;
        for (const auto& request : downRequests) {
            onFloors[request.from]++;
            offFloors[request.to]++;
        }

        cout << "\n--- Processing Down Requests ---\n";
        while (currentFloor >= 1) {
            handleFloorActions(onFloors, offFloors, currentFloor);
            if (currentFloor == 1) break; // Stop if the elevator is at the ground floor
            cout << setw(2) << currentFloor << " << ";
            currentFloor--;
        }
    }

    // Return to floor 1 and go idle
    if (currentFloor != 1) {
        cout << "\n--- Returning to Floor 1 ---\n";
        while (currentFloor > 1) {
            cout << setw(2) << currentFloor << " << ";
            currentFloor--;
        }
        cout << " 1\n";
    }

    cout << "\n=== Elevator is now idle at Floor 1 ===\n";
}

int main() {
    thread elevatorThread(simulateElevator);

    int requestCount = 0;

    cout << "=== Welcome to the Elevator System ===\n";
    cout << "Maximum floors: " << MAX_FLOORS << ", Maximum requests: " << MAX_REQUESTS << "\n\n";

    while (requestCount < MAX_REQUESTS) {
        Request request;

        // Input validation for "from" floor
        while (true) {
            cout << "Enter starting floor (from): ";
            cin >> request.from;
            if (cin.fail() || request.from < 1 || request.from > MAX_FLOORS) {
                cin.clear(); // Clear the error flag
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard invalid input
                cout << "Invalid input. Please enter a number between 1 and " << MAX_FLOORS << ".\n";
            } else {
                break;
            }
        }

        // Input validation for "to" floor
        while (true) {
            cout << "Enter destination floor (to): ";
            cin >> request.to;
            if (cin.fail() || request.to < 1 || request.to > MAX_FLOORS || request.from == request.to) {
                cin.clear(); // Clear the error flag
                cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Discard invalid input
                if (request.from == request.to) {
                    cout << "Invalid input. Starting and destination floors cannot be the same.\n";
                } else {
                    cout << "Invalid input. Please enter a number between 1 and " << MAX_FLOORS << ".\n";
                }
            } else {
                break;
            }
        }

        // Add the request
        if (request.from < request.to) {
            upRequests.push_back(request);
        } else {
            downRequests.push_back(request);
        }
        requestCount++;

        if (requestCount < MAX_REQUESTS) {
            cout << "Enter another request? (y/n): ";
            char moreRequests;
            cin >> moreRequests;
            if (moreRequests == 'n' || moreRequests == 'N') break;
        }
    }

    if (requestCount == MAX_REQUESTS) {
        cout << "Maximum number of requests reached.\n";
    }

    {
        lock_guard<mutex> lock(mtx);
        processing = true;
    }
    cv.notify_one();

    elevatorThread.join();
    return 0;
}
