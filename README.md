# Tiny Database

A mini database system developed in C++ that supports table management, CRUD operations, CSV-based persistence, and column data type validation.

## Features

- Create tables with user-defined columns.
- Support for `INT`, `FLOAT`, and `VARCHAR` data types.
- Validate values during record insertion and updates.
- Insert, view, search, update, and delete records.
- Save table data to CSV files.
- Load existing tables after restarting the program.
- Drop tables when they are no longer required.
- Handle invalid inputs, duplicate column names, and missing tables.

## Technologies Used

- **Language:** C++
- **Storage:** CSV files
- **Version Control:** Git and GitHub

## How to Compile and Run

Make sure a C++ compiler such as GCC is installed.

1. Clone this repository:

   ```bash
   git clone https://github.com/AshutoshAryan266/Tiny-Database.git
   ```

2. Navigate into the project folder:

   ```bash
   cd Tiny-Database
   ```

3. Compile the program:

   ```bash
   g++ main.cpp -o main
   ```

4. Run it on Windows:

   ```powershell
   .\main.exe
   ```

## Storage Format

Table data is stored in CSV files. Each table is saved separately, allowing data to be loaded again when the program restarts.

## Project Purpose

This project demonstrates fundamental database concepts, including table management, record operations, input validation, and file-based persistence.

## Future Improvements

- Support for additional data types.
- Better handling of values containing spaces and commas.
- More advanced query operations.
- Improved storage and schema management.
