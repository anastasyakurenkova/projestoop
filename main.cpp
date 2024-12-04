#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <algorithm>
#include <cmath>
#include <cassert>
#include <sstream>
#include <set>
#include <fstream>

using namespace std;


// --- Генерация уникального ID для каждого авто ---
string generateID() {
    static int counter = 1;
    return "V" + to_string(counter++);
}

// --- Шаблонный класс Location<T> ---
template <typename T = double>
class Location {
    T latitude;
    T longitude;

public:
    Location() : latitude(0), longitude(0) {}

    Location(T lat, T lon) : latitude(lat), longitude(lon) {}

    T getLatitude() const { return latitude; }
    T getLongitude() const { return longitude; }

    // Преобразование координат в строку "latitude,longitude"
    std::string toString() const {
        return std::to_string(latitude) + "," + std::to_string(longitude);
    }

    // Загрузка координат из файла или иного источника
    static Location loadFromGPSData(const std::string& filePath) {
        // Пример чтения GPS-координат из текстового файла
        std::ifstream file(filePath);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to open GPS data file.");
        }

        T lat, lon;
        file >> lat >> lon;
        file.close();

        return Location(lat, lon);
    }

    // Расчет расстояния
    double distanceTo(const Location& other) const {
        constexpr double EARTH_RADIUS = 6371.0; // Радиус Земли (км)
        double lat_diff = (other.latitude - latitude) * M_PI / 180.0;
        double lon_diff = (other.longitude - longitude) * M_PI / 180.0;

        double a = sin(lat_diff / 2) * sin(lat_diff / 2) +
                   cos(latitude * M_PI / 180.0) * cos(other.latitude * M_PI / 180.0) *
                   sin(lon_diff / 2) * sin(lon_diff / 2);

        double c = 2 * atan2(sqrt(a), sqrt(1 - a));

        return EARTH_RADIUS * c;
    }
};


// --- Базовый класс ---
class Vehicle {
protected:
    string vehicle_id;
    string make_and_model;       // Модель
    double mileage;              // Пробег (км)
    double fuel_efficiency;      // Расход топлива (км/л)
    bool available;              // Доступность

public:

    Vehicle() : make_and_model("Generic"), mileage(0), fuel_efficiency(0), available(true) {
        vehicle_id = generateID();
    }

    Vehicle(const string& make, const string& model, double mileage, double efficiency)
        : make_and_model(make + " " + model), mileage(mileage), fuel_efficiency(efficiency), available(true) {
        vehicle_id = generateID();
    }

    virtual ~Vehicle() = default;

    virtual void describe() const = 0; // Чисто виртуальный метод

    // Проверка доступности
    virtual bool isAvailable() const { return available; }
    virtual double getFuelEfficiency() const { return fuel_efficiency; }

    // Для анализа данных о пробеге
    virtual double analyzeUsageEfficiency() const {
        return mileage / fuel_efficiency; // Общий расход топлива на расстояние
    }

    // Получение ID
    string getID() const { return vehicle_id; }

    // Перегрузка оператора сравнения (<): сортировка по пробегу
    bool operator<(const Vehicle& other) const {
        return mileage < other.mileage;
    }

    // --- Перегрузка оператора "==" для сравнения по ID ---
    bool operator==(const Vehicle& other) const {
        return vehicle_id == other.vehicle_id;
    }

    // --- Перегрузка оператора "!=" ---
    bool operator!=(const Vehicle& other) const {
        return !(*this == other);
    }

    // --- Перегрузка оператора "+=" для увеличения пробега ---
    Vehicle& operator+=(double additionalMileage) {
        if (additionalMileage > 0) {
            mileage += additionalMileage;
        }
        return *this;
    }

    // Вывод в поток (перегрузка <<)
    friend ostream& operator<<(ostream& os, const Vehicle& v) {
        os << "ID: " << v.vehicle_id << ", Model: " << v.make_and_model
           << ", Mileage: " << v.mileage << " км, Efficiency: " << v.fuel_efficiency
           << " км/л, Available: " << (v.available ? "Yes" : "No");
        return os;
    }
};

// --- Класс Car ---
class Car : public Vehicle {
    int passenger_capacity; // Вмещение пассажиров

public:
    Car() : Vehicle(), passenger_capacity(0) {}
    Car(const string& make, const string& model, double mileage, double efficiency, int capacity)
        : Vehicle(make, model, mileage, efficiency), passenger_capacity(capacity) {}

    void describe() const override {
        cout << "Car: " << make_and_model 
             << " (Passengers: " << passenger_capacity 
             << ", Mileage: " << mileage 
             << " км, Efficiency: " << fuel_efficiency << " км/л)" << endl;
    }
};

// --- Класс Truck ---
class Truck : public Vehicle {
    double load_capacity; // Грузоподъёмность (тонны)

public:
    Truck() : Vehicle(), load_capacity(0) {}
    Truck(const string& make, const string& model, double mileage, double efficiency, double loadCapacity)
        : Vehicle(make, model, mileage, efficiency), load_capacity(loadCapacity) {}

    void describe() const override {
        cout << "Truck: " << make_and_model 
             << " (Capacity: " << load_capacity << " тонн"
             << ", Mileage: " << mileage 
             << " км, Efficiency: " << fuel_efficiency << " км/л)" << endl;
    }
};

// --- Класс Motorcycle ---
class Motorcycle : public Vehicle {
    string type;          // Тип мотоцикла (sport, cruiser, touring, ...)
    int cylinder_count;   // Количество цилиндров

public:
    Motorcycle() : Vehicle(), type("Generic"), cylinder_count(0) {}

    Motorcycle(const string& make, const string& model, double mileage, double efficiency, 
               const string& type, int cylinder_count)
        : Vehicle(make, model, mileage, efficiency), type(type), cylinder_count(cylinder_count) {}

    void describe() const override {
        cout << "Motorcycle: " << make_and_model 
             << " (Type: " << type 
             << ", Cylinders: " << cylinder_count
             << ", Mileage: " << mileage 
             << " км, Efficiency: " << fuel_efficiency << " км/л)" << endl;
    }

    const string& getType() const {
        return type;
    }
};

// --- Класс MaintenanceManager ---
class MaintenanceManager {
    map<string, int> maintenanceSchedule; // ID -> пробег до следующего ТО
    map<string, bool> maintenanceRequired; // ID -> требуется ли ТО

public:
    void addVehicle(const string& id, int mileageToNextService) {
        maintenanceSchedule[id] = mileageToNextService;
        maintenanceRequired[id] = false;
    }

    void updateMileage(const string& id, int mileage) {
        if (maintenanceSchedule.find(id) != maintenanceSchedule.end()) {
            maintenanceSchedule[id] -= mileage;
            if (maintenanceSchedule[id] <= 0) {
                maintenanceRequired[id] = true;
            }
        }
    }

    bool needsMaintenance(const string& id) const {
        auto it = maintenanceRequired.find(id);
        return it != maintenanceRequired.end() ? it->second : false;
    }

    void performMaintenance(const string& id, int mileageToNextService) {
        if (maintenanceRequired.find(id) != maintenanceRequired.end()) {
            maintenanceRequired[id] = false;
            maintenanceSchedule[id] = mileageToNextService;
            cout << "Maintenance completed for vehicle " << id << endl;
        }
    }

    void showStatus() const {
        cout << "--- Maintenance Status ---" << endl;
        for (const auto& [id, required] : maintenanceRequired) {
            cout << "Vehicle " << id << ": " << (required ? "Needs Maintenance" : "OK") << endl;
        }
        cout << endl;
    }
};

// --- Singleton FleetManager ---
class FleetManager {
private:
    vector<shared_ptr<Vehicle>> vehicles;
    map<string, Location<double>> locations;
    MaintenanceManager maintenanceManager;
    static FleetManager* instance;
    
    FleetManager() = default;
    
    // Приватный метод для выполнения HTTP-запроса к API OSRM
    std::string queryRouteAPI(const std::string& start, const std::string& end) const {
        std::string command = "curl -s \"http://router.project-osrm.org/route/v1/driving/" + start + ";" + end + "?overview=full\"";
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) {
            throw std::runtime_error("Failed to call OSRM API.");
        }
    
        char buffer[128];
        std::string result;
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }
        pclose(pipe);
        return result;
    }

public:
    // Метод для получения единственного экземпляра FleetManager
    static FleetManager* getInstance() {
        if (!instance) // Если экземпляр еще не создан,
            instance = new FleetManager(); // создаем новый
        return instance; // Возвращаем экземпляр
    }

    // Метод для добавления нового транспортного средства в автопарк
    void addVehicle(shared_ptr<Vehicle> vehicle, int mileageToNextService) {
        vehicles.push_back(vehicle); // Добавляем транспортное средство в список
        maintenanceManager.addVehicle(vehicle->getID(), mileageToNextService); // Добавляем информацию о техническом обслуживании в менеджер
        cout << "Added: " << *vehicle << endl; // Выводим информацию о добавленном транспортном средстве
    }

    // Построение маршрута между двумя точками
    void buildRoute(const string& startID, const string& endID, const std::string& outputFilePath) {
        if (locations.find(startID) == locations.end() || locations.find(endID) == locations.end()) {
            throw std::runtime_error("Invalid start or end location ID.");
        }

        auto startLocation = locations[startID];
        auto endLocation = locations[endID];

        // Генерация маршрута через OSRM API
        std::string routeResponse = queryRouteAPI(startLocation.toString(), endLocation.toString());

        // Сохранение ответа API в файл
        std::ofstream file(outputFilePath);
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open output file.");
        }
        file << routeResponse;
        file.close();

        std::cout << "Route between " << startID << " and " << endID << " saved to " << outputFilePath << std::endl;
    }

    map<string, Location<double>>& getLocations() {
        return locations;
    }

    // Метод для вывода всех транспортных средств в автопарке
    void listVehicles() const {
        for (const auto& vehicle : vehicles) // Проходим по всем транспортным средствам
            cout << *vehicle << endl; // Выводим информацию о каждом
    }

    // Метод для проверки статуса технического обслуживания
    void maintenanceCheck() {
        maintenanceManager.showStatus(); // Показываем статус техобслуживания через менеджер
    }

    // Метод для обновления пробега транспортного средства по его ID
    void updateMileage(const string& id, int mileage) {
        maintenanceManager.updateMileage(id, mileage); // Передаем обновленный пробег в менеджер
    }

    // Метод для проверки необходимости технического обслуживания по ID
    bool needsMaintenance(const string& id) const {
        return maintenanceManager.needsMaintenance(id); // Проверяем необходимость ТО через менеджер
    }

    // Метод для выполнения технического обслуживания по ID
    void performMaintenance(const string& id, int mileageToNextService) {
        maintenanceManager.performMaintenance(id, mileageToNextService); // Выполняем ТО через менеджер
    }
    
};

// Инициализация статического экземпляра класса
FleetManager* FleetManager::instance = nullptr;


void runTests() {
    // 1. Тестирование класса Location
    Location<> loc1(55.7558, 37.6176); // Москва
    Location<> loc2(48.8566, 2.3522);  // Париж
    Location<> loc3(40.7128, -74.0060); // Нью-Йорк
    Location<> locDefault; // Локация по умолчанию

    // Проверяем расчет расстояния между двумя локациями
    assert(fabs(loc1.distanceTo(loc2) - 2486.0) < 50); // Примерное расстояние Москва-Париж
    assert(fabs(loc1.distanceTo(loc3) - 7510.0) < 50); // Примерное расстояние Москва-Нью-Йорк
    assert(locDefault.getLatitude() == 0); // Проверка значения по умолчанию
    assert(locDefault.getLongitude() == 0); // Проверка значения по умолчанию

    cout << "Location tests passed!" << endl;

    // 2. Тестирование базового класса Vehicle через Car, Truck и Motorcycle
    shared_ptr<Car> car = make_shared<Car>("Toyota", "Camry", 12000, 12.0, 5); // Создание автомобиля
    shared_ptr<Truck> truck = make_shared<Truck>("Volvo", "FH16", 80000, 5.0, 20.0); // Создание грузовика
    shared_ptr<Motorcycle> bike = make_shared<Motorcycle>("Yamaha", "YZF-R6", 15000, 18.0, "Sport", 4); // Создание мотоцикла

    // Проверка на уникальность ID для транспортных средств
    assert(car->getID().substr(0, 1) == "V");
    assert(truck->getID().substr(0, 1) == "V");
    assert(bike->getID().substr(0, 1) == "V");
    assert(car->getID() != truck->getID()); // Проверка, что ID у разных машин не совпадают

    // Тестируем эффективность использования топлива для каждого транспортного средства
    assert(car->analyzeUsageEfficiency() == 1000); // 12000 / 12 = 1000
    assert(truck->analyzeUsageEfficiency() == 16000); // 80000 / 5 = 16000
    assert(fabs(bike->analyzeUsageEfficiency() - 833.33) < 0.01); // 15000 / 18

    // Проверка доступности для каждого транспортного средства
    assert(car->isAvailable() == true);
    assert(truck->isAvailable() == true);
    assert(bike->isAvailable() == true);

    // Проверяем вывод описания объектов в консоль
    car->describe();   // Описание автомобиля
    truck->describe(); // Описание грузовика
    bike->describe();  // Описание мотоцикла

    // Проверка перегрузки оператора вывода (<<)
    cout << *car << endl;
    cout << *truck << endl;
    cout << *bike << endl;

    // Проверка перегрузки оператора "+=" для увеличения пробега
    *car += 500; // Увеличиваем пробег автомобиля
    *truck += 1500; // Увеличиваем пробег грузовика
    *bike += 100; // Увеличиваем пробег мотоцикла

    // Проверяем правильность расчета нового пробега
    assert(car->analyzeUsageEfficiency() > 1041);
    assert(car->analyzeUsageEfficiency() < 1042);// (12000 + 500) / 12
    assert(truck->analyzeUsageEfficiency() == 16300); // (80000 + 1500) / 5
    assert(bike->analyzeUsageEfficiency() > 838);
    assert(bike->analyzeUsageEfficiency() < 839);// (15000 + 100) / 18

    // Проверка сравнения (<) по пробегу
    assert(*car < *truck); // Проверяем, что у Car пробег меньше, чем у Truck

    // Проверка перегрузки операторов "==" и "!="
    shared_ptr<Car> car2 = make_shared<Car>("Honda", "Civic", 5000, 15.0, 4);
    assert(*car != *car2); // Проверка, что у Car и Car2 разные ID
    *car2 = *car; // Копируем данные, теперь они равны по ID
    assert(*car == *car2); // Проверка, что теперь car и car2 равны по ID

    cout << "Vehicle (Car, Truck, Motorcycle) tests passed!" << endl;

    // 3. Тестирование класса MaintenanceManager
    MaintenanceManager manager; // Инициализация менеджера обслуживания
    manager.addVehicle(car->getID(), 500);    // Добавляем автомобиль в менеджер
    manager.addVehicle(truck->getID(), 1000); // Добавляем грузовик
    manager.addVehicle(bike->getID(), 300);   // Добавляем мотоцикл

    // Проверка на необходимость техобслуживания
    assert(manager.needsMaintenance(car->getID()) == false); // Не требует ТО
    manager.updateMileage(car->getID(), 600); // Обновляем пробег
    assert(manager.needsMaintenance(car->getID()) == true); // Теперь требует ТО
    manager.performMaintenance(car->getID(), 800); // Выполняем ТО
    assert(manager.needsMaintenance(car->getID()) == false); // Теперь не требует ТО

    assert(manager.needsMaintenance(bike->getID()) == false); // Мотоцикл не требует ТО
    manager.updateMileage(bike->getID(), 350); // Превышаем пробег для ТО
    assert(manager.needsMaintenance(bike->getID()) == true); // Теперь требует ТО

    manager.showStatus(); // Отображаем статус автомобилей (вывод проверяется вручную)

    cout << "MaintenanceManager tests passed!" << endl;

    // 4. Тестирование Singleton FleetManager
    FleetManager* fleet = FleetManager::getInstance(); // Получаем единственный экземпляр FleetManager
    fleet->addVehicle(car, 500);    // Добавляем Car в автопарк
    fleet->addVehicle(truck, 1000); // Добавляем Truck
    fleet->addVehicle(bike, 300);   // Добавляем Motorcycle
    fleet->listVehicles();          // Список всех машин (проверяется вручную)

    // Проверяем обновление пробега в FleetManager
    fleet->updateMileage(bike->getID(), 350); // Превышаем пробег для ТО
    assert(fleet->needsMaintenance(bike->getID()) == true); // Проверяем необходимость ТО

    // Выполняем ТО для мотоцикла и проверяем его состояние
    fleet->performMaintenance(bike->getID(), 1000); // Выполняем ТО
    assert(fleet->needsMaintenance(bike->getID()) == false); // Проверяем состояние после ТО

    // Проверяем работу с локациями
    fleet->getLocations()[car->getID()] = loc1; // Местоположение автомобиля
    fleet->getLocations()[truck->getID()] = loc2; // Местоположение грузовика
    fleet->getLocations()[bike->getID()] = loc3;   // Местоположение мотоцикла

    // Проверяем корректность сохраненных локаций
    assert(fleet->getLocations()[car->getID()].getLatitude() == 55.7558); // Проверяем широту для автомобиля
    assert(fleet->getLocations()[truck->getID()].getLongitude() == 2.3522); // Проверяем долготу для грузовика

    cout << "FleetManager tests passed!" << endl;
    cout << "All tests passed successfully!" << endl; // Все тесты пройдены
}

// --- Main ---
int main() {
    runTests();
    // Создаем менеджер автопарка
    FleetManager* fleetManager = FleetManager::getInstance();

    // Добавляем местоположения транспортных средств
    fleetManager->getLocations()["A"] = Location<double>(55.7558, 37.6173); // Москва
    fleetManager->getLocations()["B"] = Location<double>(59.9343, 30.3351); // Санкт-Петербург

    // Строим маршрут между точками
    try {
        fleetManager->buildRoute("A", "B", "route.json");
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}




