// System Design LLD code for ride sharing system
// File name: Ride.systemDeisgn.cpp
// Author: Shreedhar Goyal

#include <string>
#include <algorithm>
#include <stdexcept>
#include <cctype>
using namespace std;

struct Location {
    int x;
    int y;

    bool operator==(const Location& other) const {
        return x == other.x && y == other.y;
    }
};

enum class Gender { MALE, FEMALE, OTHER };
enum class VehicleType { BIKE, AUTO, SEDAN, SUV };
enum class RideCycle { ONGOING, CANCELLED, COMPLETED, WAITING };

class Vehicle {
private:
    string vehicleID;
    string numberPlate;
    string model; //`{Company}+{CarName}+{Model}`
    VehicleType type;

    void validatePlate(const string& plate){
        if(plate.empty())
            throw invalid_argument("Invalid plate number");
    }
public:
    Vehicle(const string& ID, const string& plate, const string& mod, VehicleType vt):
        vehicleID(ID), numberPlate(plate), model(mod), type(vt){
            validatePlate(plate);
        }
    
    string getVehicleID() const { return vehicleID; }
    string getNumberPlate() const { return numberPlate; }
    string getModel() const { return model; }
    VehicleType getType() const { return type; }
};

class User {
protected:
    string userID;
    string name;
    Gender gender;
    string mobileNumber;
    string aadharNumber;
    double rating;

    void validateNumber(const string& mobile){
        if(mobile.size() != 10 || !all_of(mobile.begin(), mobile.end(), ::isdigit))
            throw invalid_argument("Invalid mobile number");
    }

    void validateAadhar(const string& aadhar){
        if(aadhar.size() != 12 || !all_of(aadhar.begin(), aadhar.end(), ::isdigit))
            throw invalid_argument("Invalid Aadhar number");
    }

public:
    User(const string& id, const string& name, Gender gender, const string& mobile, const string& aadhar, double initialRating)
        : userID(id), name(name), gender(gender), rating(initialRating) {
        validateNumber(mobile);
        validateAadhar(aadhar);

        this->mobileNumber = mobile;
        this->aadharNumber = aadhar;
    }

    virtual ~User() = default;

    string getID() const { return userID; }
    string getName() const { return name; }
    string getMobileNumber() const { return mobileNumber; }
    double getRating() const { return rating; }
    
    virtual void updateRating(double newRating) = 0;
    virtual string getRole() const = 0;
};

class Rider : public User {
public:
    Rider(const string& id, const string& name, Gender gender, const string& mobile, const string& aadhar, double initialRating)
        : User(id, name, gender, mobile, aadhar, initialRating) {}

    void updateRating(double newRating) override {
        rating = rating*0.9 + newRating*0.1;

        if(rating > 5.0) rating = 5.0;
        if(rating < 1.0) rating = 1.0;
    }

    string getRole () const override {
        return "RIDER";
    }
};

class Driver : public User {
private:
    Vehicle* vehicle;
    bool availability;
public:
    Driver(const string& id, const string& name, Gender gender, const string& mobile, const string& aadhar, double initialRating):
        User(id, name, gender, mobile, aadhar, initialRating) , vehicle(nullptr), availability(true) {}

    bool getAvailability() const {  return availability; }
    Vehicle* getVehicle() const { return vehicle; }
    
    void updateRating(double newRating) override {
        if (newRating > rating) // good behaviour is rewarded quickly
            rating = rating*0.9 + newRating*0.2;
        else if ( newRating < rating) // bad reviews aren't penalise dratically
            rating = rating*0.9 + newRating*0.05;
        
        if(rating > 5.0) rating = 5.0;
        if(rating < 1.0) rating = 1.0;
    }

    void assignVehicle(Vehicle* v) { this->vehicle = v; }
    void updateAvailability ( bool newAvailability ) { availability = newAvailability; }

    string getRole() const override { return "DRIVER"; }
};

class baseFareCalculator {
public:
    virtual ~baseFareCalculator() = default;
    virtual double calculate(double distance) const = 0;
};

class baseBike : public baseFareCalculator {
public:
    double calculate(double distance) const override {
        return 10 + distance*10;
    }
};

class baseAuto : public baseFareCalculator {
public:
    double calculate(double distance) const override {
        return 20 + distance*15;
    }
};

class baseSedan : public baseFareCalculator {
public:
    double calculate(double distance) const override {
        return 50 + distance*30;
    }
};

class baseSUV : public baseFareCalculator {
public:
    double calculate(double distance) const override {
        return 100 + distance*50;
    }
};

baseFareCalculator* getBaseFare(VehicleType type){
    static baseBike bike;
    static baseAuto autoCalc;
    static baseSedan sedan;
    static baseSUV suv;

    switch(type){
        case VehicleType::BIKE : return &bike;
        case VehicleType::AUTO : return &autoCalc;
        case VehicleType::SEDAN : return &sedan;
        case VehicleType::SUV : return &suv;
        default : throw invalid_argument("No matching vehicle type");
    }
}

class Ride {
private:
    string rideID;
    Driver* driver;
    Rider* rider;
    RideCycle status;

    double payment;
    double rideDistance;

    Location startLoc;
    Location endLoc;

public:
    Ride(const string& id,Rider* r, const Location& startingLocation, const Location& endingLocation) :
        rideID(id), rider(r), driver(nullptr), status( RideCycle::WAITING ), payment(0.0), rideDistance(0.0), 
        startLoc(startingLocation), endLoc(endingLocation) {
            if(!rider)
                throw invalid_argument("Ride must have a rider!");
            if(startingLocation == endingLocation)
                throw invalid_argument("Ride must have different start and end locations");
        }
    
    // Setters
    void setDriver(Driver* d){ driver = d; }
    void setStatus(RideCycle newStatus){ status = newStatus; }
    void setPayment(double amount){ payment = amount; }

    // Getters
    string getRideID() const { return rideID; }
    Driver* getDriver() const { return driver; }
    Rider* getRider() const { return rider; }
    double getPayment() const { return payment; }
    RideCycle getStatus() const { return status; }
    Location getStartLocation() const { return startLoc; }
    Location getEndLocation() const { return endLoc; }
};

class RideService {
public:
    Ride* createRide( const string& rideId,Rider* rider,const Location& startLoc,const Location& endLoc){
        return new Ride(rideId, rider, startLoc, endLoc);
    }

    void assignDriverToRide(Ride* ride, Driver* driver){
        if(!ride)
            throw invalid_argument("Ride is null");
        if(!driver || !driver->getAvailability())
            throw invalid_argument("Driver is not available");
        if(ride->getStatus() != RideCycle::WAITING)
            throw invalid_argument("Ride is not in WAITING status");

        ride->setDriver(driver);
        driver->updateAvailability(false);
        ride->setStatus(RideCycle::ONGOING);
    }

    void cancelRide(Ride* ride){
        if(!ride)
            throw invalid_argument("Ride is null");
        if(ride->getStatus() != RideCycle::WAITING)
            throw invalid_argument("Only WAITING rides can be cancelled");

        ride->setStatus(RideCycle::CANCELLED);
        if(ride->getDriver())
            ride->getDriver()->updateAvailability(true);
    }

    void startRide(Ride* ride) {
        if(ride->getStatus() != RideCycle::WAITING)
            throw invalid_argument("Ride not in WAITING");
        ride->setStatus(RideCycle::ONGOING);
    }

    void completeRide(Ride* ride){
        if(!ride)
            throw invalid_argument("Ride is null");
        if(ride->getStatus() != RideCycle::ONGOING)
            throw invalid_argument("Ride is not in ONGOING status");

        // Calculate distance (Manhattan distance for simplicity)
        Location startLoc = ride->getStartLocation();
        Location endLoc = ride->getEndLocation();
        double distance = abs(endLoc.x - startLoc.x) + abs(endLoc.y - startLoc.y);

        // Calculate fare
        VehicleType vType = ride->getDriver()->getVehicle()->getType();
        baseFareCalculator* fareCalc = getBaseFare(vType);
        double fare = fareCalc->calculate(distance);

        ride->setPayment(fare);
        ride->setStatus(RideCycle::COMPLETED);
        ride->getDriver()->updateAvailability(true);
    }
};