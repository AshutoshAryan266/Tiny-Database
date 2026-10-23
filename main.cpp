

#include <iostream>
#include <vector>
#include <string>
#include <limits>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cmath>
using namespace std;

// Finds a column by name and returns its index, or -1 if it does not exist.
int findColumn(const vector<string> &columns, const string &columnName) {
    for (int i = 0; i < static_cast<int>(columns.size()); i++) {
        if (columns[i] == columnName) return i;
    }
    return -1;
}

// Splits a simple comma-separated line. This version does not support quoted commas.
vector<string> splitCSVLine(const string &line) {
    vector<string> values;
    string value;
    for (char ch : line) {
        if (ch == ',') {
            values.push_back(value);
            value.clear();
        } else {
            value += ch;
        }
    }
    values.push_back(value);
    return values;
}

bool tableExists(const string &tableName) {
    ifstream file(tableName + ".csv");
    return file.is_open();
}

bool isValidType(const string &type) {
    return type == "INT" || type == "FLOAT" || type == "VARCHAR";
}

// Validates a value according to its declared column type.
bool validateValue(const string &value, const string &type) {
    if (type == "VARCHAR") return !value.empty();

    if (type == "INT") {
        if (value.empty()) return false;
        size_t i = (value[0] == '+' || value[0] == '-') ? 1 : 0;
        if (i == value.size()) return false;
        for (; i < value.size(); i++) {
            if (!isdigit(static_cast<unsigned char>(value[i]))) return false;
        }
        char *end = nullptr;
        strtol(value.c_str(), &end, 10);
        return end != value.c_str() && *end == '\0';
    }

    if (type == "FLOAT") {
        if (value.empty()) return false;
        char *end = nullptr;
        strtod(value.c_str(), &end);
        return end != value.c_str() && *end == '\0' && isfinite(strtod(value.c_str(), nullptr));
    }

    return false;
}

// Saves names, type metadata, and records to a CSV file.
bool saveTable(const string &tableName, const vector<string> &columns,
               const vector<string> &dataTypes, const vector<vector<string>> &records) {
    ofstream file(tableName + ".csv");
    if (!file.is_open()) {
        cout << "Error: Could not save CSV file!" << endl;
        return false;
    }

    for (int i = 0; i < static_cast<int>(columns.size()); i++) {
        if (i > 0) file << ",";
        file << columns[i];
    }
    file << "\n#TYPES";
    for (const string &type : dataTypes) file << "," << type;
    file << "\n";

    for (const auto &record : records) {
        for (int j = 0; j < static_cast<int>(record.size()); j++) {
            if (j > 0) file << ",";
            file << record[j];
        }
        file << "\n";
    }

    file.close();
    if (file.fail()) {
        cout << "Error: Could not finish writing CSV file!" << endl;
        return false;
    }
    return true;
}

// Loads both the current format and older CSV files created before data types were added.
bool loadTable(const string &newTableName, vector<string> &tableColumns,
               vector<string> &tableDataTypes, vector<vector<string>> &tableRecords) {
    ifstream file(newTableName + ".csv");
    if (!file.is_open()) {
        cout << "Table not found!" << endl;
        return false;
    }

    string line;
    if (!getline(file, line) || line.empty()) {
        cout << "Invalid or empty CSV file!" << endl;
        return false;
    }

    vector<string> newColumns = splitCSVLine(line);
    for (const string &column : newColumns) {
        if (column.empty()) {
            cout << "Invalid column name in CSV file!" << endl;
            return false;
        }
    }
    for (int i = 0; i < static_cast<int>(newColumns.size()); i++) {
        for (int j = i + 1; j < static_cast<int>(newColumns.size()); j++) {
            if (newColumns[i] == newColumns[j]) {
                cout << "Duplicate column in CSV file!" << endl;
                return false;
            }
        }
    }

    vector<string> newDataTypes(newColumns.size(), "VARCHAR");
    vector<vector<string>> newRecords;
    bool firstLineAfterHeader = true;
    bool hasTypeMetadata = false;

    while (getline(file, line)) {
        if (line.empty()) continue;
        vector<string> fields = splitCSVLine(line);

        if (firstLineAfterHeader) {
            firstLineAfterHeader = false;
            if (!fields.empty() && fields[0] == "#TYPES") {
                hasTypeMetadata = true;
                if (fields.size() != newColumns.size() + 1) {
                    cout << "Invalid data type metadata in CSV file!" << endl;
                    return false;
                }
                for (size_t i = 0; i < newColumns.size(); i++) {
                    newDataTypes[i] = fields[i + 1];
                    if (!isValidType(newDataTypes[i])) {
                        cout << "Unknown data type in CSV file!" << endl;
                        return false;
                    }
                }
                continue;
            }
        }

        if (fields.size() != newColumns.size()) {
            cout << "Invalid record in CSV file!" << endl;
            return false;
        }
        for (size_t i = 0; i < fields.size(); i++) {
            if (hasTypeMetadata && !validateValue(fields[i], newDataTypes[i])) {
                cout << "Record contains a value that does not match its column type!" << endl;
                return false;
            }
        }
        newRecords.push_back(fields);
    }

    if (file.bad()) {
        cout << "Error while reading CSV file!" << endl;
        return false;
    }

    tableColumns = newColumns;
    tableDataTypes = newDataTypes;
    tableRecords = newRecords;
    return true;
}

void createTable(string &tableName, vector<string> &columns, vector<string> &dataTypes,
                 vector<vector<string>> &records, bool &tableCreated) {
    string newTableName;
    int numberOfColumns;
    cout << "Enter table name: ";
    cin >> newTableName;

    if (tableExists(newTableName)) {
        cout << "A table with this name already exists!" << endl;
        return;
    }

    cout << "Enter number of columns: ";
    if (!(cin >> numberOfColumns)) {
        cout << "Invalid number of columns!" << endl;
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }
    if (numberOfColumns <= 0) {
        cout << "Number of columns must be positive!" << endl;
        return;
    }

    vector<string> newColumns;
    vector<string> newDataTypes;
    for (int i = 0; i < numberOfColumns; i++) {
        string columnName, type;
        cout << "Enter column " << i + 1 << " name: ";
        cin >> columnName;
        if (findColumn(newColumns, columnName) != -1) {
            cout << "Duplicate column name! Table not created." << endl;
            return;
        }

        cout << "Enter data type for " << columnName << " (INT, FLOAT, VARCHAR): ";
        cin >> type;
        for (char &ch : type) ch = static_cast<char>(toupper(static_cast<unsigned char>(ch)));
        if (!isValidType(type)) {
            cout << "Invalid data type! Use INT, FLOAT, or VARCHAR. Table not created." << endl;
            return;
        }
        newColumns.push_back(columnName);
        newDataTypes.push_back(type);
    }

    vector<vector<string>> newRecords;
    if (!saveTable(newTableName, newColumns, newDataTypes, newRecords)) return;

    tableName = newTableName;
    columns = newColumns;
    dataTypes = newDataTypes;
    records.clear();
    tableCreated = true;
    cout << "Table created and saved successfully!" << endl;
}

void openTable(string &tableName, vector<string> &columns, vector<string> &dataTypes,
               vector<vector<string>> &records, bool &tableCreated) {
    string newTableName;
    cout << "Enter the name of the table to load: ";
    cin >> newTableName;

    vector<string> newColumns, newDataTypes;
    vector<vector<string>> newRecords;
    if (!loadTable(newTableName, newColumns, newDataTypes, newRecords)) return;

    tableName = newTableName;
    columns = newColumns;
    dataTypes = newDataTypes;
    records = newRecords;
    tableCreated = true;
    cout << "Table loaded successfully!" << endl;
}

void insertRecord(string &tableName, vector<string> &columns, vector<string> &dataTypes,
                  vector<vector<string>> &records) {
    vector<string> record;
    for (size_t i = 0; i < columns.size(); i++) {
        string value;
        while (true) {
            cout << "Enter value for " << columns[i] << " (" << dataTypes[i] << "): ";
            cin >> value;
            if (validateValue(value, dataTypes[i])) break;
            cout << "Invalid value! Column '" << columns[i] << "' requires type " << dataTypes[i] << ". Try again." << endl;
        }
        record.push_back(value);
    }

    records.push_back(record);
    if (saveTable(tableName, columns, dataTypes, records)) {
        cout << "Record inserted and saved successfully!" << endl;
    } else {
        records.pop_back();
    }
}

void viewRecords(const string &tableName, const vector<string> &columns,
                 const vector<string> &dataTypes, const vector<vector<string>> &records) {
    cout << "\nTable: " << tableName << endl;
    for (size_t i = 0; i < columns.size(); i++) cout << "+----------";
    cout << "+\n|";
    for (size_t i = 0; i < columns.size(); i++) cout << " " << columns[i] << " (" << dataTypes[i] << ")\t|";
    cout << "\n";
    for (size_t i = 0; i < columns.size(); i++) cout << "+----------";
    cout << "+\n";

    if (records.empty()) cout << "No records available." << endl;
    else {
        for (const auto &record : records) {
            cout << "|";
            for (const string &value : record) cout << " " << value << "\t|";
            cout << "\n";
        }
    }
    for (size_t i = 0; i < columns.size(); i++) cout << "+----------";
    cout << "+\n";
}

void searchRecord(const vector<string> &columns, const vector<vector<string>> &records) {
    string searchColumn, searchValue;
    cout << "Enter column name to search: ";
    cin >> searchColumn;
    int columnIndex = findColumn(columns, searchColumn);
    if (columnIndex == -1) {
        cout << "Column not found!" << endl;
        return;
    }

    cout << "Enter value to search: ";
    cin >> searchValue;
    bool found = false;
    for (const auto &record : records) {
        if (record[columnIndex] == searchValue) {
            cout << "Record found: ";
            for (const string &value : record) cout << value << " ";
            cout << endl;
            found = true;
        }
    }
    if (!found) cout << "No matching record found!" << endl;
}

void updateRecord(string &tableName, vector<string> &columns, vector<string> &dataTypes,
                  vector<vector<string>> &records) {
    string searchColumn, searchValue, updateColumn, newValue;
    cout << "Enter column to identify record: ";
    cin >> searchColumn;
    int searchIndex = findColumn(columns, searchColumn);
    if (searchIndex == -1) {
        cout << "Column not found!" << endl;
        return;
    }
    cout << "Enter value to identify record: ";
    cin >> searchValue;

    for (size_t i = 0; i < records.size(); i++) {
        if (records[i][searchIndex] == searchValue) {
            cout << "Enter column to update: ";
            cin >> updateColumn;
            int updateIndex = findColumn(columns, updateColumn);
            if (updateIndex == -1) {
                cout << "Column to update not found!" << endl;
                return;
            }

            while (true) {
                cout << "Enter new value (" << dataTypes[updateIndex] << "): ";
                cin >> newValue;
                if (validateValue(newValue, dataTypes[updateIndex])) break;
                cout << "Invalid value! Column '" << updateColumn << "' requires type " << dataTypes[updateIndex] << ". Try again." << endl;
            }

            string oldValue = records[i][updateIndex];
            records[i][updateIndex] = newValue;
            if (saveTable(tableName, columns, dataTypes, records)) {
                cout << "Record updated and saved successfully!" << endl;
            } else {
                records[i][updateIndex] = oldValue;
            }
            return;
        }
    }
    cout << "Record not found!" << endl;
}

void deleteRecord(string &tableName, const vector<string> &columns, const vector<string> &dataTypes,
                  vector<vector<string>> &records) {
    string deleteColumn, deleteValue;
    cout << "Enter column to identify record: ";
    cin >> deleteColumn;
    int deleteIndex = findColumn(columns, deleteColumn);
    if (deleteIndex == -1) {
        cout << "Column not found!" << endl;
        return;
    }
    cout << "Enter value to identify record: ";
    cin >> deleteValue;

    for (size_t i = 0; i < records.size(); i++) {
        if (records[i][deleteIndex] == deleteValue) {
            vector<string> deletedRecord = records[i];
            records.erase(records.begin() + static_cast<long>(i));
            if (saveTable(tableName, columns, dataTypes, records)) {
                cout << "Record deleted and saved successfully!" << endl;
            } else {
                records.insert(records.begin() + static_cast<long>(i), deletedRecord);
            }
            return;
        }
    }
    cout << "Record not found!" << endl;
}

void dropTable(string &tableName, vector<string> &columns, vector<string> &dataTypes,
               vector<vector<string>> &records, bool &tableCreated) {
    char confirm;
    cout << "Are you sure you want to drop table '" << tableName << "'? (Y/N): ";
    cin >> confirm;
    if (confirm == 'Y' || confirm == 'y') {
        if (remove((tableName + ".csv").c_str()) != 0) {
            cout << "Error: Could not delete the CSV file. Table retained." << endl;
            return;
        }
        tableName.clear();
        columns.clear();
        dataTypes.clear();
        records.clear();
        tableCreated = false;
        cout << "Table dropped successfully!" << endl;
    } else {
        cout << "Drop table cancelled." << endl;
    }
}

int main() {
    string tableName;
    vector<string> columns;
    vector<string> dataTypes;
    vector<vector<string>> records;
    bool tableCreated = false;
    char continueChoice = 'Y';
    int choice;

    while (continueChoice == 'Y' || continueChoice == 'y') {
        cout << "\n===== Tiny Database =====" << endl;
        cout << "1. Create Table" << endl;
        cout << "2. Load Existing Table" << endl;
        cout << "3. Insert Record" << endl;
        cout << "4. View Records" << endl;
        cout << "5. Search Record" << endl;
        cout << "6. Update Record" << endl;
        cout << "7. Delete Record" << endl;
        cout << "8. Drop Table" << endl;
        cout << "9. Exit" << endl;
        cout << "Enter your choice: ";

        if (!(cin >> choice)) {
            cout << "Invalid input! Please enter a number." << endl;
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            continue;
        }

        if (choice == 1) {
            createTable(tableName, columns, dataTypes, records, tableCreated);
        } else if (choice == 2) {
            openTable(tableName, columns, dataTypes, records, tableCreated);
        } else if (choice >= 3 && choice <= 7) {
            if (!tableCreated) {
                cout << "Please create or load a table first!" << endl;
            } else if (choice == 3) {
                insertRecord(tableName, columns, dataTypes, records);
            } else if (choice == 4) {
                viewRecords(tableName, columns, dataTypes, records);
            } else if (choice == 5) {
                searchRecord(columns, records);
            } else if (choice == 6) {
                updateRecord(tableName, columns, dataTypes, records);
            } else if (choice == 7) {
                deleteRecord(tableName, columns, dataTypes, records);
            }
        } else if (choice == 8) {
            if (!tableCreated) cout << "No active table to drop!" << endl;
            else dropTable(tableName, columns, dataTypes, records, tableCreated);
        } else if (choice == 9) {
            cout << "Thank you for using Tiny Database!" << endl;
            break;
        } else {
            cout << "Invalid choice!" << endl;
        }

        cout << "\nReturn to menu? (Y/N): ";
        cin >> continueChoice;
        if (continueChoice != 'Y' && continueChoice != 'y' &&
            continueChoice != 'N' && continueChoice != 'n') {
            cout << "Invalid choice. Exiting program." << endl;
            break;
        }
    }

    cout << "Program ended." << endl;
    return 0;
}
