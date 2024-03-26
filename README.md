# Directory and File Trees

The Directory and File Trees project involves practical exercises focused on code comprehension, systematic debugging, thorough internal testing, and informed design decisions. It consists of three parts, each involving different tree data structures.

## Description
The project consists of three parts, each involving different tree data structures.

**Part 1:** Debugging Binary Directory Trees (BDT) using the GDB debugger.

**Part 2:** Enhancing an internal validation module to identify invalid states in faulty Directory Trees (DT).

**Part 3:** Implementing the File Tree (FT) interface based on the Directory Tree (DT) implementation from Part 2.

Collaboration with a partner was a key aspect of this project.

## Languages & Tools
- C programming language
- GDB debugger
- Valgrind (for memory management analysis)

## File Descriptions

### Shared Files:
- `path.c`: Contains the implementation of path-related functions. (Provided in the assignment)
- `path.h`: Defines the interface for path-related functions. (Provided in the assignment)
- `dynarray.c`: Implements dynamic array data structure. (Provided in the assignment)
- `dynarray.h`: Contains the declarations for dynamic array functions. (Provided in the assignment)
- `a4def.h`: Defines constants and data structures used throughout the project. (Provided in the assignment)

### Part 2:
- `checkerDT.h`: Declares the interface for the internal validation module.
- `checkerDT.c`: Implements the internal validation module for the Directory Tree.

### Part 3:
- `ft.c`: Represents the implementation of the File Tree (FT) interface.
- `nodeFT.c`: Defines the structure and implementation of a node in the File Tree.
- `nodeFT.h`: Provides the interface for the node in the File Tree.
