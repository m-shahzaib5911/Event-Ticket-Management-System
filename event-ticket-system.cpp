#include <iostream>
#include <vector>
#include <regex>
#include <cctype>
#include <iomanip>
#include <limits>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
#include <sys/stat.h>
using namespace std;

// =============== HELPER FUNCTIONS ===============
void clearScreen() {
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif
}

void waitForEnter() {
    // Clear any existing input first
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    // Wait for exactly one Enter press
    while (cin.get() != '\n') {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

void showScreenHeader(const string& title) {
    clearScreen();
    cout << "\n===== " << title << " =====" << endl;
}

string getlineinput(const string& prompt) {
    string input;
    cout << prompt;
    getline(cin, input);
    return input;
}

int getMenuChoice(const string& prompt, int minChoice, int maxChoice) {
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);
        
        // Skip empty input (just Enter pressed)
        if (input.empty()) {
            continue;
        }
        
        // Check if input is numeric
        bool isNumeric = true;
        for (char c : input) {
            if (!isdigit(c)) {
                isNumeric = false;
                break;
            }
        }
        
        if (!isNumeric) {
            cout << "Invalid input! Please enter a number between " 
                 << minChoice << " and " << maxChoice << ".\n";
            continue;
        }
        
        // Convert to integer
        stringstream ss(input);
        int choice;
        ss >> choice;
        
        // Validate range
        if (choice < minChoice || choice > maxChoice) {
            cout << "Please enter a number between " 
                 << minChoice << " and " << maxChoice << ".\n";
            continue;
        }
        
        return choice;
    }
}

// =============== CORE FUNCTIONALITY ===============
bool validdate(const string& date) {
    // First check the format
    regex datePattern("^(0[1-9]|[12][0-9]|3[01])-(0[1-9]|1[0-2])-\\d{4}$");
    
    if (!regex_match(date, datePattern)) {
        cout << "Error: Date must be in DD-MM-YYYY format (e.g., 15-06-2025).\n";
        return false;
    }

    // Extract components
    int day = stoi(date.substr(0, 2));
    int month = stoi(date.substr(3, 2));
    int year = stoi(date.substr(6, 4));

    // Check year (must be 2025 or later)
    if (year < 2025) {
        cout << "Invalid year! Events can only be created for 2025 or later.\n";
        return false;
    }

    // Check month
    if (month < 1 || month > 12) {
        cout << "Invalid month! Month must be between 01 and 12.\n";
        return false;
    }

    // Check day
    if (day < 1 || day > 31) {
        cout << "Invalid day! Day must be between 01 and 31.\n";
        return false;
    }

    // Check months with 30 days
    if ((month == 4 || month == 6 || month == 9 || month == 11) && day > 30) {
        cout << "Invalid day! This month only has 30 days.\n";
        return false;
    }

    // Check February
    if (month == 2) {
        bool isLeap = (year % 400 == 0) || (year % 100 != 0 && year % 4 == 0);
        if (day > (isLeap ? 29 : 28)) {
            cout << "Invalid day! February only has " << (isLeap ? 29 : 28) << " days in " << year << ".\n";
            return false;
        }
    }

    return true;
}

class Event {
public:
    int eventID;
    string eventName;
    string eventLocation;
    string eventDate;
    map<string, pair<float, int>> ticketTiers;
    
    Event(int id, string name, string location, string date) {
        eventID = id;
        eventName = name;
        eventLocation = location;
        eventDate = date;
    }

    void addTicketTier(string tierName, float price, int quantity) {
        ticketTiers[tierName] = make_pair(price, quantity);
    }

    int getTotalTickets() const {
        int total = 0;
        for (const auto& tier : ticketTiers) {
            total += tier.second.second;
        }
        return total;
    }

    void displayEvent() const {
        cout << "\n===== Event Details =====\n";
        cout << "Event ID: " << eventID << endl;
        cout << "Event Name: " << eventName << endl;
        cout << "Location: " << eventLocation << endl;
        cout << "Date: " << eventDate << endl;
        cout << "Available Tickets:\n";
        for (const auto& tier : ticketTiers) {
            cout << " - " << tier.first << ": $" << fixed << setprecision(2) 
                 << tier.second.first << " (" << tier.second.second << " available)\n";
        }
        cout << "Total Tickets: " << getTotalTickets() << endl;
        cout << "======================\n";
    }
};

class User {
public:
    int UserId;
    string UserName;

    User(int id, string name) {
        UserId = id;
        UserName = name;
    }

    void displayUser() const {
        cout << "\n---- User Information ----\n";
        cout << "User ID: " << UserId << endl;
        cout << "User Name: " << UserName << endl;
        cout << "==============================" << endl;
    }
};

class Booking {
public:
    int bookingId;
    int userId;
    int eventId;
    int tickets;
    float totalPrice;
    string status;
    string ticketTier;

    Booking(int bId, int uId, int eId, int tic, float price, string tier) {
        bookingId = bId;
        userId = uId;
        eventId = eId;
        tickets = tic;
        totalPrice = price;
        status = "Confirmed";
        ticketTier = tier;
    }

    void displayBooking() const {
        cout << "\n----- Booking Details -----\n";
        cout << "Booking ID: " << bookingId << endl;
        cout << "Event ID: " << eventId << endl;
        cout << "User ID: " << userId << endl;
        cout << "Ticket Tier: " << ticketTier << endl;
        cout << "Tickets: " << tickets << endl;
        cout << "Total Price: $" << fixed << setprecision(2) << totalPrice << endl;
        cout << "Status: " << status << endl;
    }
};

// =============== DATA MANAGEMENT ===============
void ensureFileExists(const string& filename) {
    ifstream file(filename);
    if (!file.good()) {
        ofstream createFile(filename);
        createFile.close();
    }
}

void initializeDataFiles() {
    ensureFileExists("users.txt");
    ensureFileExists("events.txt");
    ensureFileExists("bookings.txt");
}

void saveUsers(const vector<User>& Userlist) {
    ofstream outfile("users.txt");
    if (!outfile) {
        cerr << "Error saving users\n";
        return;
    }
    for (const auto& user : Userlist) {
        outfile << user.UserId << "," << user.UserName << "\n";
    }
    outfile.close();
}

void saveEvents(const vector<Event>& eventlist) {
    ofstream outfile("events.txt");
    if (!outfile) {
        cerr << "Error saving events\n";
        return;
    }
    for (const auto& event : eventlist) {
        outfile << event.eventID << ","
                << event.eventName << ","
                << event.eventLocation << ","
                << event.eventDate;

        for (const auto& tier : event.ticketTiers) {
            outfile << "," << tier.first << ":" << fixed << setprecision(2) 
                    << tier.second.first << ":" << tier.second.second;
        }
        outfile << "\n";
    }
    outfile.close();
}

void saveBookings(const vector<Booking>& bookings) {
    ofstream outFile("bookings.txt");
    if (!outFile) {
        cerr << "Error saving bookings\n";
        return;
    }
    for (const auto& booking : bookings) {
        outFile << booking.bookingId << ","
                << booking.userId << ","
                << booking.eventId << ","
                << booking.tickets << ","
                << fixed << setprecision(2) << booking.totalPrice << ","
                << booking.status << ","
                << booking.ticketTier << "\n";
    }
    outFile.close();
}

vector<User> loadUsers() {
    vector<User> Userlist;
    ifstream inFile("users.txt");
    string line;
    while (getline(inFile, line)) {
        size_t pos = line.find(',');
        if (pos != string::npos) {
            int id = stoi(line.substr(0, pos));
            string name = line.substr(pos + 1);
            Userlist.emplace_back(id, name);
        }
    }
    return Userlist;
}

vector<Event> loadEvents() {
    vector<Event> eventlist;
    ifstream inFile("events.txt");
    string line;

    while (getline(inFile, line)) {
        stringstream ss(line);
        string token;
        vector<string> tokens;
        
        while (getline(ss, token, ',')) {
            tokens.push_back(token);
        }
        
        if (tokens.size() >= 4) {
            Event event(stoi(tokens[0]), tokens[1], tokens[2], tokens[3]);
            for (size_t i = 4; i < tokens.size(); i++) {
                size_t firstColon = tokens[i].find(':');
                size_t secondColon = tokens[i].rfind(':');
                if (firstColon != string::npos && secondColon != string::npos) {
                    string tierName = tokens[i].substr(0, firstColon);
                    float price = stof(tokens[i].substr(firstColon + 1, secondColon - firstColon - 1));
                    int quantity = stoi(tokens[i].substr(secondColon + 1));
                    event.addTicketTier(tierName, price, quantity);
                }
            }
            eventlist.push_back(event);
        }
    }
    return eventlist;
}

vector<Booking> loadBookings() {
    vector<Booking> bookings;
    ifstream inFile("bookings.txt");
    if (!inFile) return bookings;

    string line;
    while (getline(inFile, line)) {
        if (line.empty() || line[0] == '#') continue;

        vector<string> tokens;
        string token;
        size_t start = 0;
        size_t end = line.find(',');
        
        while (end != string::npos) {
            token = line.substr(start, end - start);
            tokens.push_back(token);
            start = end + 1;
            end = line.find(',', start);
        }
        tokens.push_back(line.substr(start));

        if (tokens.size() != 7) continue;

        try {
            bookings.emplace_back(
                stoi(tokens[0]),
                stoi(tokens[1]),
                stoi(tokens[2]),
                stoi(tokens[3]),
                stof(tokens[4]),
                tokens[6]
            );
            bookings.back().status = tokens[5];
        } catch (...) {
            continue;
        }
    }
    return bookings;
}

// =============== BUSINESS LOGIC ===============
int getNextEventID(const vector<Event>& eventlist) {
    if (eventlist.empty()) return 1;
    int maxID = 0;
    for (const auto& event : eventlist) {
        if (event.eventID > maxID) maxID = event.eventID;
    }
    return maxID + 1;
}

int getnextUserID(const vector<User>& Userlist) {
    if (Userlist.empty()) return 1;
    int maxID = 0;
    for (const auto& user : Userlist) {
        if (user.UserId > maxID) maxID = user.UserId;
    }
    return maxID + 1;
}

bool adminlogin() {
    const string ADMIN_UserN = "admin";
    const string ADMIN_Pass = "admin123";

    showScreenHeader("ADMIN LOGIN");
    string username = getlineinput("Username: ");
    
    // Password input
    cout << "Password: ";
    #ifdef _WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hStdin, &mode);
    SetConsoleMode(hStdin, mode & (~ENABLE_ECHO_INPUT));
    #else
    termios oldt;
    tcgetattr(STDIN_FILENO, &oldt);
    termios newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    #endif

    string password;
    getline(cin, password);
    
    #ifdef _WIN32
    SetConsoleMode(hStdin, mode);
    #else
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    #endif

    cout << endl;
    
    if (username != ADMIN_UserN || password != ADMIN_Pass) {
        cout << "Access denied! Invalid credentials. Press Enter to return...";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return false;
    }
    return true;
}
void displayAllEvents(const vector<Event>& eventlist) {
    showScreenHeader("ALL EVENTS");
    
    if (eventlist.empty()) {
        cout << "No events registered yet.\n";
        return;
    }

    cout << "\n" << string(102, '=') << "\n";
    cout << left << setw(6) << "ID" 
         << setw(30) << "EVENT NAME" 
         << setw(26) << "LOCATION" 
         << setw(17) << "DATE" 
         << setw(22) << "TOTAL TICKETS AVAILABLE"
         << "\n" << string(102, '=') << "\n";

    for (const auto& event : eventlist) {
        // Basic event info
        cout << left << setw(6) << event.eventID 
             << setw(30) << (event.eventName.length() > 24 ? event.eventName.substr(0, 21) + "..." : event.eventName)
             << setw(26) << (event.eventLocation.length() > 19 ? event.eventLocation.substr(0, 16) + "..." : event.eventLocation)
             << setw(17) << event.eventDate
             << setw(22) << event.getTotalTickets();

        // Ticket tiers (display first 2 tiers with ellipsis if more)
        int tierCount = 0;
        for (const auto& tier : event.ticketTiers) {
            if (tierCount < 2) {
                cout << "\n     - " << left << setw(12) << tier.first 
                     << "$" << fixed << setprecision(2) << tier.second.first
                     << " (" << tier.second.second << ")";
            }
            tierCount++;
        }
        if (tierCount > 2) {
            cout << "\n     + " << (tierCount - 2) << " more tiers...";
        }

        cout << "\n" << string(102, '-') << "\n";
    }
}
void displayAllUsers(const vector<User>& userList) {
    showScreenHeader("ALL USERS");
    
    if (userList.empty()) {
        cout << "No users registered.\n";
        return;
    }

    // Table header
    cout << "\n" << string(21, '=') << "\n";
    cout << left << setw(10) << "USER ID" 
         << setw(30) << "USER NAME" 
         << "\n" << string(21, '=') << "\n";

    // Table rows
    for (const auto& user : userList) {
        cout << left << setw(10) << user.UserId 
             << setw(30) << user.UserName 
             << "\n" << string(21, '-') << "\n";
    }

    // Footer with count
    cout << "\nTotal Users: " << userList.size() << "\n";
}
void viewAllBookingsAdmin(const vector<Booking>& bookings, const vector<User>& Userlist, const vector<Event>& eventlist) {
    showScreenHeader("ALL BOOKINGS - ADMIN VIEW");
    
    if (bookings.empty()) {
        cout << "No bookings found.\n";
        cout << "\nPress Enter to return...";
        waitForEnter();
        return;
    }

    // Group bookings by user ID
    map<int, vector<const Booking*>> userBookings;
    for (const auto& booking : bookings) {
        userBookings[booking.userId].push_back(&booking);
    }

    // Display grouped bookings
    for (const auto& [userId, bookingsList] : userBookings) {
        // Find user details
        string userName = "Unknown";
        for (const auto& user : Userlist) {
            if (user.UserId == userId) {
                userName = user.UserName;
                break;
            }
        }

        cout << "\n===== USER: " << userName << " (ID: " << userId << ") =====\n";
        cout << "-------------------------------------------------------------\n";
        cout << left << setw(12) << "Booking ID" 
             << setw(12) << "Event ID" 
             << setw(20) << "Event Name" 
             << setw(12) << "Date" 
             << setw(15) << "Ticket Tier" 
             << setw(8) << "Tickets" 
             << setw(12) << "Total Price" 
             << setw(12) << "Status" 
             << endl;
        cout << "-------------------------------------------------------------\n";

        float userTotal = 0;
        int userTickets = 0;

        for (const auto& booking : bookingsList) {
            // Find event details
            string eventName = "Unknown";
            string eventDate = "Unknown";
            for (const auto& event : eventlist) {
                if (event.eventID == booking->eventId) {
                    eventName = event.eventName;
                    eventDate = event.eventDate;
                    break;
                }
            }

            cout << left << setw(12) << booking->bookingId 
                 << setw(12) << booking->eventId 
                 << setw(20) << eventName 
                 << setw(12) << eventDate 
                 << setw(15) << booking->ticketTier 
                 << setw(8) << booking->tickets 
                 << "$" << fixed << setprecision(2) << setw(11) << booking->totalPrice 
                 << setw(12) << booking->status 
                 << endl;

            if (booking->status == "Confirmed") {
                userTotal += booking->totalPrice;
                userTickets += booking->tickets;
            }
        }

        cout << "-------------------------------------------------------------\n";
        cout << "USER TOTALS: " << userTickets << " tickets | $" 
             << fixed << setprecision(2) << userTotal << "\n";
    }

    // Add system-wide totals
    float systemTotal = 0;
    int systemTickets = 0;
    int confirmedBookings = 0;
    int cancelledBookings = 0;
    
    for (const auto& booking : bookings) {
        if (booking.status == "Confirmed") {
            systemTotal += booking.totalPrice;
            systemTickets += booking.tickets;
            confirmedBookings++;
        } else {
            cancelledBookings++;
        }
    }

    cout << "\n===== SYSTEM TOTALS =====\n";
    cout << "TOTAL BOOKINGS: " << bookings.size() << " (Confirmed: " << confirmedBookings 
         << ", Cancelled: " << cancelledBookings << ")\n";
    cout << "TOTAL CONFIRMED TICKETS: " << systemTickets << "\n";
    cout << "TOTAL REVENUE: $" << fixed << setprecision(2) << systemTotal << "\n";

}

void adminPanel(vector<Event>& eventlist, vector<User> Userlist, vector<Booking>& bookings, int& nextBookingId) {
    int choice;
    do {
        showScreenHeader("ADMIN PANEL");
        cout << "1. Register New Event\n"
             << "2. View All Events\n"
             << "3. View All Users\n"
             << "4. View All Bookings\n"
             << "5. Return to Main Menu\n";
        
        choice = getMenuChoice("Enter your choice: ", 1, 5);

        switch(choice) {
            case 1: {
                showScreenHeader("REGISTER NEW EVENT");
                int id = getNextEventID(eventlist);
                cout << "Event ID: " << id << endl;

                string name = getlineinput("Enter Event Name: ");
                string location = getlineinput("Enter Event Location: ");

                string date;
                bool validDate = false;
                do {
                    date = getlineinput("Enter Event Date (DD-MM-YYYY): ");
                } while (!validdate(date));

                Event newEvent(id, name, location, date);
                
                while (true) {
                    cout << "\nAdd Ticket Catagory (or 'done' to finish):\n";
                    string tierName = getlineinput("Enter Catagory Name: ");
                    if (tierName == "done") break;

                    float price;
                    cout << "Enter price for this catagory: $";
                    while (!(cin >> price)) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "Invalid input. Please enter a number: ";
                    }
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    int quantity;
                    cout << "Enter available quantity of tickets: ";
                    while (!(cin >> quantity)) {
                        cin.clear();
                        cin.ignore(numeric_limits<streamsize>::max(), '\n');
                        cout << "Invalid input. Please enter a number: ";
                    }
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');

                    newEvent.addTicketTier(tierName, price, quantity);
                }

                if (newEvent.ticketTiers.empty()) {
                    cout << "\nAdding default ticket catagory...\n";
                    float price;
                    cout << "Enter default ticket price: $";
                    cin >> price;
                    clearInput();

                    int quantity;
                    cout << "Enter available quantity: ";
                    cin >> quantity;
                    clearInput();

                    newEvent.addTicketTier("Standard", price, quantity);
                }

                eventlist.push_back(newEvent);
                saveEvents(eventlist);
                cout << "\nEvent registered successfully!\n";
                cout << "\nPress Enter to return...";
                cin.get();  // Wait for exactly one Enter press
                break;
            }
            case 2: {
                showScreenHeader("ALL EVENTS");
                if (eventlist.empty()) {
                    cout << "No events registered yet.\n";
                } else {
                    
                        displayAllEvents(eventlist);
                    
                }
                cout << "\nPress Enter to return...";
                cin.get();  // Wait for exactly one Enter press
                break;
            }
            case 3: {
                showScreenHeader("ALL USERS");
                if (Userlist.empty()) {
                    cout << "No users registered.\n";
                } else {
                    displayAllUsers(Userlist);
                }
                cout << "\nPress Enter to return...";
                cin.get();  // Wait for exactly one Enter press
                break;
            }
            case 4: {
                viewAllBookingsAdmin(bookings, Userlist, eventlist);
                cout << "\nPress Enter to return...";
                cin.get();  // Wait for exactly one Enter press
                break;
            }
            case 5:
                return;
        }
    } while (true);
}

void userBookTicket(vector<Event>& eventlist, vector<User>& Userlist, vector<Booking>& bookings, int& nextBookingId) {
    showScreenHeader("BOOK TICKETS");
    
    if (eventlist.empty()) {
        cout << "No events available for booking.\n";
        waitForEnter();
        return;
    }
    if (Userlist.empty()) {
        cout << "No users registered. Please register first.\n";
        waitForEnter();
        return;
    }

    int UserId;
    cout << "Enter User ID: ";
    cin >> UserId;
    clearInput();

    bool userFound = false;
    for (const auto& user : Userlist) {
        if (user.UserId == UserId) {
            userFound = true;
            break;
        }
    }

    if (!userFound) {
        cout << "User ID not found.\n";
        waitForEnter();
        return;
    }

    cout << "\n===== Available Events =====\n";
    for (const auto& event : eventlist) {
        if (event.getTotalTickets() > 0) {
            cout << "ID: " << event.eventID << " | " << event.eventName 
                 << " (" << event.eventDate << " at " << event.eventLocation << ")\n";
        }
    }

    int eventID;
    cout << "\nEnter Event ID to book: ";
    cin >> eventID;
    clearInput();

    Event* eventPtr = nullptr;
    for (auto& event : eventlist) {
        if (event.eventID == eventID) {
            eventPtr = &event;
            break;
        }
    }

    if (!eventPtr) {
        cout << "Event ID not found.\n";
        waitForEnter();
        return;
    }

    if (eventPtr->getTotalTickets() <= 0) {
        cout << "This event is sold out.\n";
        waitForEnter();
        return;
    }

    cout << "\n===== Available Ticket Tiers =====\n";
    cout << "0. Go back\n";
    vector<pair<string, pair<float, int>>> availableTiers;
    int index = 1;
    
    for (const auto& tier : eventPtr->ticketTiers) {
        if (tier.second.second > 0) {
            cout << index << ". " << tier.first << " - $" << fixed << setprecision(2)
                 << tier.second.first << " (" << tier.second.second << " available)\n";
            availableTiers.push_back(tier);
            index++;
        }
    }

    if (availableTiers.empty()) {
        cout << "No ticket tiers available.\n";
        waitForEnter();
        return;
    }

    int tierChoice = getMenuChoice(
        "\nSelect ticket tier (0-" + to_string(availableTiers.size()) + "): ", 
        0, 
        availableTiers.size()
    );
    
    if (tierChoice == 0) {
        cout << "Returning to menu.\n";
        waitForEnter();
        return;
    }

    const auto& selectedTier = availableTiers[tierChoice-1];
    string tierName = selectedTier.first;
    float tierPrice = selectedTier.second.first;
    int tierAvailable = selectedTier.second.second;

    int ticketQuantity;
    while (true) {
        cout << "Number of tickets to book: ";
        cin >> ticketQuantity;
        clearInput();
        
        if (ticketQuantity <= 0) {
            cout << "Please enter at least 1 ticket.\n";
        } else if (ticketQuantity > tierAvailable) {
            cout << "Only " << tierAvailable << " tickets available.\n";
        } else {
            break;
        }
    }

    float totalPrice = ticketQuantity * tierPrice;

    cout << "\n===== Booking Summary =====\n";
    cout << "Event: " << eventPtr->eventName << endl;
    cout << "Date: " << eventPtr->eventDate << endl;
    cout << "Location: " << eventPtr->eventLocation << endl;
    cout << "Ticket Tier: " << tierName << endl;
    cout << "Quantity: " << ticketQuantity << endl;
    cout << "Price per Ticket: $" << fixed << setprecision(2) << tierPrice << endl;
    cout << "Total Price: $" << fixed << setprecision(2) << totalPrice << endl;

    char confirm;
    do {
        cout << "\nConfirm booking? (Y/N): ";
        cin >> confirm;
        confirm = toupper(confirm);
        
    } while (confirm != 'Y' && confirm != 'N');

    if (confirm == 'Y') {
        eventPtr->ticketTiers[tierName].second -= ticketQuantity;
        bookings.emplace_back(nextBookingId++, UserId, eventID, ticketQuantity, totalPrice, tierName);
        saveEvents(eventlist);
        saveBookings(bookings);
        
        cout << "\nBooking confirmed!\n";
        cout << "Booking ID: " << nextBookingId-1 << endl;
    } else {
        cout << "Booking cancelled.\n";
    }
    cout << "\nPress Enter to return...";
    waitForEnter();
}

void cancelBooking(vector<Event>& eventlist, vector<Booking>& bookings) {
    showScreenHeader("CANCEL BOOKING");
    
    if (bookings.empty()) {
        cout << "No bookings to cancel.\n";
        waitForEnter();
        return;
    }

    int bookingId;
    cout << "Enter Booking ID to cancel: ";
    cin >> bookingId;

    bool found = false;
    for (auto& booking : bookings) {
        if (booking.bookingId == bookingId && booking.status == "Confirmed") {
            for (auto& event : eventlist) {
                if (event.eventID == booking.eventId) {
                    event.ticketTiers[booking.ticketTier].second += booking.tickets;
                    break;
                }
            }
            booking.status = "Cancelled";
            found = true;
            
            cout << "\n===== Cancellation Summary =====\n";
            cout << "Booking ID: " << booking.bookingId << " cancelled\n";
            cout << booking.tickets << " tickets released\n";
            
            saveEvents(eventlist);
            saveBookings(bookings);
            break;
        }
    }

    if (!found) {
        cout << "Booking not found or already cancelled.\n";
    }
    cout << "\nPress Enter to return...";
    waitForEnter();
}

void viewUserBookings(const vector<Booking>& bookings, const vector<User>& Userlist, const vector<Event>& eventlist) {
    showScreenHeader("MY BOOKINGS");
    
    if (bookings.empty()) {
        cout << "No bookings found.\n";
        cout << "\nPress Enter to return...";
        waitForEnter();
        return;
    }

    int userId;
    cout << "Enter your User ID: ";
    cin >> userId;

    // Find user
    string userName = "No user found with this id";
    for (const auto& user : Userlist) {
        if (user.UserId == userId) {
            userName = user.UserName;
            break;
        }
    }

    cout << "\nUser: " << userName << " (ID: " << userId << ")\n\n";
    
    // Table header
    cout << left << setw(12) << "Booking ID" 
         << setw(15) << "Status" 
         << setw(15) << "Event" 
         << setw(12) << "Date" 
         << setw(15) << "Location" 
         << setw(12) << "Tier" 
         << setw(8) << "Tickets" 
         << setw(12) << "Total Price" 
         << endl;

    cout << string(95, '-') << endl;

    bool hasBookings = false;
    for (const auto& booking : bookings) {
        if (booking.userId == userId) {
            hasBookings = true;
            
            // Find event details
            string eventName = "Unknown";
            string eventDate = "Unknown";
            string eventLocation = "Unknown";
            
            for (const auto& event : eventlist) {
                if (event.eventID == booking.eventId) {
                    eventName = event.eventName;
                    eventDate = event.eventDate;
                    eventLocation = event.eventLocation;
                    break;
                }
            }

            cout << left << setw(12) << booking.bookingId 
                 << setw(15) << booking.status 
                 << setw(15) << eventName 
                 << setw(12) << eventDate 
                 << setw(15) << eventLocation 
                 << setw(12) << booking.ticketTier 
                 << setw(8) << booking.tickets 
                 << "$" << fixed << setprecision(2) << setw(11) << booking.totalPrice 
                 << endl;
        }
    }

    if (!hasBookings) {
        cout << "No bookings found for User ID: " << userId << endl;
    }
    cout << "\nPress Enter to return...";
    waitForEnter();
}

int main() {
    initializeDataFiles();
    vector<Event> eventlist = loadEvents();
    vector<User> Userlist = loadUsers();
    vector<Booking> bookings = loadBookings();

    int nextBookingId = 1;
    if (!bookings.empty()) {
        nextBookingId = bookings.back().bookingId + 1;
    }

    int choice;
    do {
        showScreenHeader("EVENT TICKETING SYSTEM");
        cout << "1. Register User\n"
             << "2. View All Events\n"
             << "3. Book Tickets\n"
             << "4. Cancel Booking\n"
             << "5. View My Bookings\n"
             << "6. Admin Login\n"
             << "7. Exit\n";
        
        choice = getMenuChoice("Enter your choice: ", 1, 7);

        switch(choice) {
            case 1: {
                showScreenHeader("USER REGISTRATION");
                int UserId = getnextUserID(Userlist);
                cout << "Your User ID: " << UserId <<endl ;

                string UserName = getlineinput("Enter User Name: ");
                User newUser(UserId, UserName);
                Userlist.push_back(newUser);
                saveUsers(Userlist);
                
                cout << "\nUser registered successfully!\n";
                cout << "\nPress Enter to return...";
                cin.get();
                break;
            }
            case 2:
                showScreenHeader("AVAILABLE EVENTS");
                if (eventlist.empty()) {
                    cout << "No events available.\n";
                } else {
                    displayAllEvents(eventlist);  // Show compact event list
                }
                cout << "\nPress Enter to return...";
                cin.get();  // Wait for exactly one Enter press
                break;
            case 3:
                userBookTicket(eventlist, Userlist, bookings, nextBookingId);
                break;
            case 4:
                cancelBooking(eventlist, bookings);
                break;
            case 5:
                viewUserBookings(bookings, Userlist, eventlist);
                break;
            case 6:
                if (!adminlogin()) {
                    break;  // Just break if login fails (adminlogin() handles the prompt)
                }
                adminPanel(eventlist, Userlist, bookings, nextBookingId);
                break;
            case 7:
                saveUsers(Userlist);
                saveEvents(eventlist);
                saveBookings(bookings);
                cout << "\nExiting program. Goodbye!\n";
                break;
            default:
                cout << "Invalid choice! Please try again.\n";
                waitForEnter();
        }
    } while (choice != 7);

    return 0;
}