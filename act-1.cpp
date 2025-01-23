#include <iostream>
#include <vector>
#include <algorithm>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>

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

    cout << "Elevator starts at floor 1\n";

    auto printPassengerCount = [&]() {
        cout << " [Passengers: " << currentPassengers << "]\n";
    };

    // Process all up requests
    if (!upRequests.empty()) {
        sort(upRequests.begin(), upRequests.end(), [](const Request& a, const Request& b) {
            return a.from < b.from;
        });

        map<int, int> onFloors, offFloors;

        for (const auto& request : upRequests) {
            onFloors[request.from]++;
            offFloors[request.to]++;
        }

        for (auto it = onFloors.begin(); it != onFloors.end(); ++it) {
            while (currentFloor < it->first) {
                cout << currentFloor << " >> ";
                currentFloor++;
            }
            if (currentFloor == it->first) {
                int boarding = min(it->second, MAX_CAPACITY - currentPassengers);
                if (boarding > 0) {
                    currentPassengers += boarding;
                    cout << currentFloor << " (getting on: " << boarding << " passengers)";
                    printPassengerCount();
                }
            }
        }

        for (auto it = offFloors.begin(); it != offFloors.end(); ++it) {
            while (currentFloor < it->first) {
                cout << currentFloor << " >> ";
                currentFloor++;
            }
            if (currentFloor == it->first) {
                int alighting = min(it->second, currentPassengers);
                currentPassengers -= alighting;
                cout << currentFloor << " (getting off: " << alighting << " passengers)";
                printPassengerCount();
            }
        }
    }

    // Process all down requests
    if (!downRequests.empty()) {
        sort(downRequests.begin(), downRequests.end(), [](const Request& a, const Request& b) {
            return a.from > b.from;
        });

        map<int, int> onFloors, offFloors;

        for (const auto& request : downRequests) {
            onFloors[request.from]++;
            offFloors[request.to]++;
        }

        for (auto it = onFloors.rbegin(); it != onFloors.rend(); ++it) {
            while (currentFloor < it->first) {
                cout << currentFloor << " >> ";
                currentFloor++;
            }
            while (currentFloor > it->first) {
                cout << currentFloor << " >> ";
                currentFloor--;
            }
            if (currentFloor == it->first) {
                int boarding = min(it->second, MAX_CAPACITY - currentPassengers);
                if (boarding > 0) {
                    currentPassengers += boarding;
                    cout << currentFloor << " (getting on: " << boarding << " passengers)";
                    printPassengerCount();
                }
            }
        }

        for (auto it = offFloors.rbegin(); it != offFloors.rend(); ++it) {
            while (currentFloor > it->first) {
                cout << currentFloor << " >> ";
                currentFloor--;
            }
            if (currentFloor == it->first) {
                int alighting = min(it->second, currentPassengers);
                currentPassengers -= alighting;
                cout << currentFloor << " (getting off: " << alighting << " passengers)";
                printPassengerCount();
            }
        }
    }

    cout << "\nElevator simulation complete.\n";
}

int main() {
    char moreRequests;
    thread elevatorThread(simulateElevator);

    do {
        if (upRequests.size() + downRequests.size() >= MAX_REQUESTS) {
            cout << "Maximum number of requests reached.\n";
            break;
        }

        Request request;
        cout << "Enter floor request (from-to): ";
        cin >> request.from >> request.to;

        if (request.from < 1 || request.from > MAX_FLOORS || request.to < 1 || request.to > MAX_FLOORS) {
            cout << "Invalid request: Floors must be between 1 and " << MAX_FLOORS << ".\n";
        } else if (request.from == request.to) {
            cout << "Invalid request: Starting and destination floors cannot be the same.\n";
        } else if (request.from < request.to) {
            upRequests.push_back(request);
        } else {
            downRequests.push_back(request);
        }

        cout << "Enter another request? (y/n): ";
        cin >> moreRequests;
    } while (moreRequests == 'y' || moreRequests == 'Y');

    {
        lock_guard<mutex> lock(mtx);
        processing = true;
    }
    cv.notify_one();

    elevatorThread.join();
    return 0;
}
