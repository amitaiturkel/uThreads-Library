
# uThreads Project

## Overview

The `uThreads` project is an implementation of user-level threads in C++. It includes a set of test programs to verify the functionality of the threading library. The project is designed to demonstrate the creation, management, and synchronization of threads at the user level.

## Project Structure

- `Thread.cpp` and `Thread.h`: Implementation and header file for the threading library.
- `uthreads.cpp` and `uthreads.h`: Implementation and header file for the main user-level threads functionality.
- `tests/`: Directory containing test source files.
  - `test0_sanity.cpp`: Basic sanity tests for the threading library.
  - `test2_two_thread.cpp`: Tests involving two threads.
  - `test0_sanity.txt` and `test2_two_thread.txt`: Expected outputs for the corresponding tests.
  - `uthreads_comprehensive_tests.cpp`: More comprehensive tests.
- `Makefile`: Build script to compile the project and run tests.

## Build Instructions

1. **Install Dependencies**: Ensure you have `g++` installed on your system.

2. **Build the Project**: Run the following command to compile the library and test executables:

   ```sh
   make
   ```

   This will create the static library `libuthreads.a` and compile the test executables in the `build` directory.

## Running Tests

To run the tests, use the `run_tests` target in the `Makefile`. This will compile the test executables if they are not already up-to-date and execute them. Use the following command:

```sh
make run_tests
```

The output of each test will be displayed in the terminal. Make sure that the test output matches the expected results in the `.txt` files located in the `tests/` directory.

## Cleaning Up

To remove all compiled files and executables, use the `clean` target:

```sh
make clean
```

This will delete all object files, the static library, and the test executables.

## Contributing

Feel free to contribute to the project by submitting pull requests or opening issues. Contributions are welcome to enhance the functionality and improve the tests.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contact

For any questions or inquiries, please contact [Amitai] at [Amitai.turkel@gmail.com].

```
