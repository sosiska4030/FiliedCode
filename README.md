# FiliedCode Compiler (fcc)

A compiled programming language built with C++ and LLVM.

## Example

```fc
func main() -> void {
    variable int x == 40 + 2;
    println(x);
    variable string msg == "Hello world";
    println(msg);
}
```

## Building the Compiler

**Requirements:**
- LLVM 22+
- MinGW / GCC
- CMake 3.20+

```bash
mkdir build && cd build
cmake ..
make -j4
```

## Usage

```bash
# Compile a .fc file
fcc build hello.fc

# Compile and run immediately  
fcc build_and_run hello.fc

# Show compiler version
fcc --version

# Check compilation
fcc check hello.fc

# Run .fc file without .exe file
fcc run_without_file hello.fc
```

## Language Features

### Functions
```fc
func main() -> void {
    ...
}
```

### Variables
```fc
variable int x == 42;
variable string msg == "Hello world";
```

### Output
```fc
printf("text");   // print without newline
println("text");  // print with newline
println(x);       // print variable
```

### Arithmetic
```fc
variable int result == 10 + 5;
variable int result == 10 - 5;
variable int result == 10 * 5;
variable int result == 10 / 5;
```

## Project Structure
FiliedCode/
src/      - compiler source code
tests/    - example .fc files

