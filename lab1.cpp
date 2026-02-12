#include <iostream>
#include <fstream>
#include <cmath>
#include <string>
#include <vector>
#include <stdexcept>

using namespace std;

// ==================== ІЄРАРХІЯ ВИНЯТКІВ ====================

class BaseException : public exception {
protected:
    string msg;
public:
    BaseException(const string& message) : msg(message) {}
    const char* what() const noexcept override { return msg.c_str(); }
};

class FileException : public BaseException {
public:
    FileException(const string& filename) : BaseException("File error: " + filename) {}
};

class RangeException : public BaseException {
private:
    double val;
public:
    RangeException(double value) : BaseException("Value out of range"), val(value) {}
    double getValue() const { return val; }
};

class ZeroDivideException : public BaseException {
public:
    ZeroDivideException(const string& func) : BaseException("Division by zero in " + func) {}
};

class AlgorithmChangeException : public BaseException {
public:
    AlgorithmChangeException(int toAlg) : BaseException("Switch to Algorithm " + to_string(toAlg)) {}
};

// ==================== РОБОТА З ТАБЛИЦЯМИ ====================

class TableData {
private:
    struct Point {
        double x, y;
    };
    vector<Point> data;
    string filename;
    
    void loadData() {
        ifstream file(filename);
        if (!file) throw FileException(filename);
        
        double x, y;
        while (file >> x >> y) {
            data.push_back({x, y});
        }
        file.close();
    }
    
public:
    TableData(const string& fname) : filename(fname) {
        try {
            loadData();
        }
        catch (const FileException&) {
            throw;
        }
    }
    
    double getValue(double x) const {
        if (data.empty()) throw runtime_error("No data loaded");
        
        // Точне співпадіння
        for (const auto& p : data) {
            if (fabs(p.x - x) < 1e-9) return p.y;
        }
        
        // Перевірка діапазону
        if (x < data.front().x || x > data.back().x) {
            throw RangeException(x);
        }
        
        // Інтерполяція
        for (size_t i = 0; i < data.size() - 1; i++) {
            if (data[i].x <= x && x <= data[i+1].x) {
                return data[i].y + (data[i+1].y - data[i].y) * 
                       (x - data[i].x) / (data[i+1].x - data[i].x);
            }
        }
        
        throw runtime_error("Interpolation failed");
    }
    
    static double Tbl(double x) {
        static TableData table("C:/Users/Kikisya/Desktop/lab1/dat_1.dat");
        if (x < -10 || x >= 10) throw RangeException(x);
        return table.getValue(x);
    }
};

// ==================== АЛГОРИТМ 1 ====================

double Krl(double x, double y, double z) {
    try {
        if (x > 0 && y <= 1) {
            if (fabs(z) < 1e-9) throw ZeroDivideException("Krl");
            return floor(TableData::Tbl(x) + TableData::Tbl(y) / z);
        }
        else if (y > 1) {
            if (fabs(x) < 1e-9) throw ZeroDivideException("Krl");
            return floor(TableData::Tbl(y) + TableData::Tbl(z) / x);
        }
        else if (x <= 0) {
            if (fabs(y) < 1e-9) throw ZeroDivideException("Krl");
            return floor(TableData::Tbl(z) + TableData::Tbl(x) / y);
        }
    }
    catch (const ZeroDivideException& e) {
        cerr << "Warning: " << e.what() << ", using alternative value" << endl;
        if (fabs(x) < 1e-9) return floor(y + z);
        return x + 1;
    }
    return 0;
}

double Nrl(double x, double y) {
    if (x > y) return 0.42 * Krl(x, y, x);
    else return 0.57 * Krl(y, x, y) - 0.42 * Krl(y, y, y);
}

double Grl(double x, double y, double z) {
    if (floor(x + y) == floor(z)) {
        throw AlgorithmChangeException(2);
    }
    
    if (x + y >= z) {
        return floor(x + y) + 0.4 * Nrl(x, z) + 0.6 * Nrl(y, z);
    } else {
        return floor(x + y) + 1.4 * Nrl(x, z) - 0.4 * Nrl(y * Nrl(y, 1), z);
    }
}

double algorithm1(double x, double y, double z) {
    try {
        if (x < -10 || x >= 10) throw AlgorithmChangeException(2);
        return x * Grl(x, y, z) + y * Grl(y, z, x) + z * Grl(z, x, y);
    }
    catch (const FileException& e) {
        throw AlgorithmChangeException(3);
    }
    catch (const RangeException& e) {
        throw AlgorithmChangeException(2);
    }
}

// ==================== АЛГОРИТМ 2 ====================

double Kr12(double x, double y, double z) {
    try {
        if (x > 0 && fabs(z) > 1e-9) {
            return TableData::Tbl(x) + TableData::Tbl(y) / z;
        }
        else if (x < 0 && y > 1 && fabs(x) > 1e-9) {
            return TableData::Tbl(y) + TableData::Tbl(z) / x;
        }
        else if (x <= 0 && y <= 1 && fabs(y) > 1e-9) {
            return TableData::Tbl(z) + TableData::Tbl(x) / y;
        }
        else {
            throw ZeroDivideException("Kr12");
        }
    }
    catch (const ZeroDivideException& e) {
        cerr << "Warning: " << e.what() << endl;
        if (fabs(x) < 1e-9) return floor(y + z);
        return x + 1;
    }
}

double Nr12(double x, double y) {
    double denom = sqrt(x*x + y*y);
    if (fabs(denom) < 1e-9) {
        cerr << "Error: Division by zero in Nr12" << endl;
        return -0.05;
    }
    
    if (x > y) {
        return 0.42 * Kr12(x/denom, y/denom, x/denom);
    } else {
        return 0.57 * Kr12(y/denom, x/denom, y/denom);
    }
}

double Gr12(double x, double y, double z) {
    if (x + y >= z) {
        return x + y + 0.3 * Nr12(x, z) + 0.7 * Nr12(y, z);
    } else {
        return x + y + 1.3 * Nr12(x, z) - 0.3 * Nr12(y, z);
    }
}

double algorithm2(double x, double y, double z) {
    try {
        if (x < -10 || x >= 10) {
            cerr << "Error: x out of range, using |x|/10" << endl;
            return fabs(x) / 10.0;
        }
        
        double result = x * Gr12(x, y, z) + 
                       y * Gr12(x, y, z) + 
                       y * Gr12(z, y, x) - 
                       x * y * z * Gr12(y, x, z);
        return result;
    }
    catch (const FileException& e) {
        throw AlgorithmChangeException(3);
    }
}

// ==================== АЛГОРИТМ 3 ====================

double algorithm3(double x, double y, double z) {
    return 1.3498 * z + 2.2362 * y - 2.348 * x * y;
}

// ==================== ОСНОВНА ФУНКЦІЯ ====================

double computeFun(double x, double y, double z) {
    cout << "Computing fun(" << x << ", " << y << ", " << z << ")" << endl;
    
    // Алгоритм 1
    try {
        cout << "Trying Algorithm 1..." << endl;
        return algorithm1(x, y, z);
    }
    catch (const AlgorithmChangeException& e) {
        cout << e.what() << endl;
        
        // Алгоритм 2
        try {
            cout << "Trying Algorithm 2..." << endl;
            return algorithm2(x, y, z);
        }
        catch (const AlgorithmChangeException& e2) {
            cout << e2.what() << endl;
            
            // Алгоритм 3
            cout << "Trying Algorithm 3..." << endl;
            return algorithm3(x, y, z);
        }
    }
    catch (const exception& e) {
        cout << "Unexpected error: " << e.what() << endl;
        cout << "Using Algorithm 3 as fallback..." << endl;
        return algorithm3(x, y, z);
    }
}

// ==================== MAIN ====================

int main() {
    cout << "=== Exception Handling Lab Work ===" << endl;
    
    // Створюємо тестовий файл
    ofstream testFile("C:/Users/Kikisya/Desktop/lab1/dat_1.dat");
    testFile << "-10 23.5\n-5 12.4\n0 10.1\n5 6.87\n10 1.21";
    testFile.close();
    
    // Тестові випадки
    vector<vector<double>> testCases = {
        {0, 2, 3},      // Нормальний випадок
        {15, 2, 3},     // x > 10 → Algorithm 2
        {1, 0.5, 0},    // Ділення на нуль
        {-20, 1, 1}     // x < -10 → Algorithm 2
    };
    
    for (const auto& test : testCases) {
        double x = test[0], y = test[1], z = test[2];
        cout << "\n--- Test: x=" << x << ", y=" << y << ", z=" << z << " ---" << endl;
        
        try {
            double result = computeFun(x, y, z);
            cout << "Result: " << result << endl;
        }
        catch (const exception& e) {
            cout << "Failed: " << e.what() << endl;
        }
    }
    
    // Інтерактивний режим
    cout << "\n=== Interactive Mode ===" << endl;
    cout << "Enter x y z (or -1 to exit): ";
    
    double x, y, z;
    while (cin >> x && x != -1) {
        cin >> y >> z;
        
        try {
            double result = computeFun(x, y, z);
            cout << "fun(" << x << ", " << y << ", " << z << ") = " << result << endl;
        }
        catch (const exception& e) {
            cout << "Error: " << e.what() << endl;
        }
        
        cout << "\nEnter x y z (or -1 to exit): ";
    }
    
    // Прибираємо тестовий файл
    remove("C:/Users/Kikisya/Desktop/lab1/dat_1.dat");
    
    cout << "\nProgram finished." << endl;
    return 0;
}
