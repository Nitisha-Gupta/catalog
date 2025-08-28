#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <map>
#include <sstream>
using namespace std;

// Function to convert from any base to decimal (handles large numbers)
long long baseToDecimal(const string& num, int base) {
    long long result = 0;
    long long power = 1;
    
    // Process from right to left
    for (int i = num.length() - 1; i >= 0; i--) {
        char digit = num[i];
        int digitValue;
        
        if (digit >= '0' && digit <= '9') {
            digitValue = digit - '0';
        } else if (digit >= 'A' && digit <= 'Z') {
            digitValue = digit - 'A' + 10;
        } else if (digit >= 'a' && digit <= 'z') {
            digitValue = digit - 'a' + 10;
        } else {
            continue; // Skip invalid characters
        }
        
        if (digitValue >= base) {
            continue; // Skip invalid digits for this base
        }
        
        result += digitValue * power;
        power *= base;
    }
    
    return result;
}

// Simple JSON parser for our specific format
map<int, pair<int, string> > parseJSON(const string& filename) {
    map<int, pair<int, string> > points;
    ifstream file(filename.c_str());
    
    if (!file.is_open()) {
        return points;
    }
    
    string content;
    string line;
    
    // Read entire file
    while (getline(file, line)) {
        content += line;
    }
    file.close();
    
    // Parse each numbered entry (1 to 20)
    for (int i = 1; i <= 20; i++) {
        stringstream ss;
        ss << i;
        string keyPattern = "\"" + ss.str() + "\"";
        size_t keyPos = content.find(keyPattern);
        if (keyPos == string::npos) continue;
        
        // Find the opening brace after the key
        size_t objStart = content.find("{", keyPos);
        if (objStart == string::npos) continue;
        
        // Find the matching closing brace
        int braceCount = 1;
        size_t pos = objStart + 1;
        while (pos < content.length() && braceCount > 0) {
            if (content[pos] == '{') braceCount++;
            else if (content[pos] == '}') braceCount--;
            pos++;
        }
        
        if (braceCount > 0) continue;
        
        string objContent = content.substr(objStart, pos - objStart);
        
        // Extract base
        int base = 10;
        size_t basePos = objContent.find("\"base\"");
        if (basePos != string::npos) {
            size_t colonPos = objContent.find(":", basePos);
            size_t startQuote = objContent.find("\"", colonPos);
            size_t endQuote = objContent.find("\"", startQuote + 1);
            if (startQuote != string::npos && endQuote != string::npos) {
                string baseStr = objContent.substr(startQuote + 1, endQuote - startQuote - 1);
                stringstream baseStream(baseStr);
                baseStream >> base;
            }
        }
        
        // Extract value
        string value;
        size_t valuePos = objContent.find("\"value\"");
        if (valuePos != string::npos) {
            size_t colonPos = objContent.find(":", valuePos);
            size_t startQuote = objContent.find("\"", colonPos);
            size_t endQuote = objContent.find("\"", startQuote + 1);
            if (startQuote != string::npos && endQuote != string::npos) {
                value = objContent.substr(startQuote + 1, endQuote - startQuote - 1);
            }
        }
        
        if (!value.empty()) {
            points[i] = make_pair(base, value);
        }
    }
    
    return points;
}

// Get k value from JSON
int getKValue(const string& filename) {
    ifstream file(filename.c_str());
    if (!file.is_open()) return 3; // default
    
    string content;
    string line;
    while (getline(file, line)) {
        content += line;
    }
    file.close();
    
    size_t kPos = content.find("\"k\"");
    if (kPos != string::npos) {
        size_t colonPos = content.find(":", kPos);
        size_t startQuote = content.find("\"", colonPos);
        if (startQuote != string::npos) {
            size_t endQuote = content.find("\"", startQuote + 1);
            if (endQuote != string::npos) {
                string kStr = content.substr(startQuote + 1, endQuote - startQuote - 1);
                stringstream kStream(kStr);
                int k;
                kStream >> k;
                return k;
            }
        }
        
        // Try without quotes (numeric value)
        size_t commaPos = content.find_first_of(",}", colonPos);
        if (commaPos != string::npos) {
            string kStr = content.substr(colonPos + 1, commaPos - colonPos - 1);
            // Remove whitespace
            kStr.erase(0, kStr.find_first_not_of(" \t"));
            kStr.erase(kStr.find_last_not_of(" \t") + 1);
            stringstream kStream(kStr);
            int k;
            kStream >> k;
            return k;
        }
    }
    
    return 3; // default
}

// Lagrange interpolation to find constant term (value at x=0)
double lagrangeInterpolation(const vector<pair<int, long long> >& points) {
    double result = 0.0;
    int n = points.size();
    
    // For each point, calculate its contribution to f(0)
    for (int i = 0; i < n; i++) {
        double term = (double)points[i].second; // y_i
        
        // Calculate the Lagrange basis polynomial L_i(0)
        for (int j = 0; j < n; j++) {
            if (i != j) {
                double numerator = (0.0 - (double)points[j].first);
                double denominator = ((double)points[i].first - (double)points[j].first);
                term *= numerator / denominator;
            }
        }
        
        result += term;
    }
    
    return result;
}

int main() {
    string filename = "testcase1.json"; // Change this to test different files
    
    // Get k value
    int k = getKValue(filename);
    
    // Parse JSON file
    map<int, pair<int, string> > rawPoints = parseJSON(filename);
    
    if (rawPoints.empty()) {
        return 1;
    }
    
    // Decode the points and take only first k points
    vector<pair<int, long long> > points;
    int count = 0;
    
    for (map<int, pair<int, string> >::iterator it = rawPoints.begin(); 
         it != rawPoints.end() && count < k; ++it) {
        int x = it->first;
        int base = it->second.first;
        string value = it->second.second;
        
        long long y = baseToDecimal(value, base);
        points.push_back(make_pair(x, y));
        count++;
    }
    
    // Use Lagrange interpolation to find the constant term c
    double constant = lagrangeInterpolation(points);
    
    // Print only the constant term (rounded to nearest integer)
    long long result = (long long)round(constant);
    cout << result << endl;
    
    return 0;
}