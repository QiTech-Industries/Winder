# C++ Coding Conventions
In order to make code more readable we need to streamline our styles. While you can deviate from the rules below, make sure to have sufficient reason to do so and discuss/explain as needed. This guide is not perfect and maybe it needs adjustment after all.
* Unless explicitly stated otherwise english is used for naming, commenting and documentation

## Naming schemes
* All names should be descriptive, that doesn't mean they must be long
   * ``Configuration`` -> ``Config``
   * ``rotationsPerMinute`` -> ``rpm``
   * ``Utilities`` -> ``Utils``
   * ...
* Avoid hard to distinguish names
  * Bad:  ``WiFi`` and ``Wifi``
  * Good: ``WiFi`` and ``QiWifi``
  * in case function names are different in a singular and plural noun only consider using an overload
* Variables
    * Lowercase camelCase, starting with an underscore for private class attributes
    * Correct: ``int myVarFoo``
    * Wrong: ``int MyVarFoo``
    * Wrong: ``int my_var_foo``
* Constants
    * Uppercase separated by underscores
    * Define variables as constants wherever possible
    * Correct: ``const int MY_CONSTANT_FOO``
    * Wrong: ``const int my_constant_foo``
    * Wrong: ``const int MYCONSTANTFOO``
* Structs
    * Lowercase camelCase, ending with ``_s``
    * Correct: ``myStructFoo_s``
* Enumerations
    * Lowercase camelCase, ending with ``_e``
    * Correct: ``myEnumFoo_e``
* Types
    * Lowercase camelCase, ending with ``_t``
    * Correct: ``myTypeFoo_t``
* Functions / Methods
    * Lowercase camelCase
    * functions that perform actions should use verbs in their names
        * Bad: ``rotation()``
        * Good: ``rotate()``, ``doRotation()``
    * Use intuitive prefixes
        * ``getX()``: All attribute-getters without any direct side effects
        * ``setX()``: All attribute-setters without any direct side effects
        * ``isX()``: All functions returning boolean without any direct side effects
        * ``canX()``: Check for authorization
        * ``init()``: Initialisation and setup code, one per class
        * ``handle()``: Main loop that handles the current state and its changes
        * ``handleX()``: All private sub loops that must be called from main handle loop
        * ``onX()``: All functions that register a callback
    * Converter functions
        * Good: ``xToY()``
        * Bad:  ``x2Y()``
    * Try to use matching words for matching functions
        * Good: ``start()``/``stop()``, ``activate()``/``deactivate()``
        * Bad: ``off()``/``power()``
* Classes
    * Uppercase CamelCase
* TODO: Namespaces
    * TODO: To be decided

## Editor Settings
* VsCode
    * C_Cpp.clang_format_fallbackStyle: ``{ BasedOnStyle: Google, IndentWidth: 4, ColumnLimit: 140 }``
        * use `Shift + Alt + F` to format files according to style
    * files.insertFinalNewline: true
    * files.eol: \n
## Code
* For conditionals use the following blocks and spacing
```javascript
// Good
if (a == 1) {
    foo();
} else if (b == 2) {
    bar();
}

// Bad
if(a==2){ // Too few spaces
    foo();
} // else if not here
else if ( b == 2 ) { // Too many spaces
    bar();
}
```
* Prefer guard-statements of complex conditional cases
```javascript
// Good
if(foo) return 1;
if(bar) return 2;

// Bad
if(foo) {
    return 1;
} else if(bar) {
    return 2;
}
```
* Include the names of parameters in the header-definition of functions
    * Good: ``bool isFoo(bool bar);``
    * Bad: ``bool isFoo(bool);``
* Define the constructor in a class-definition as early as possible
* Move general Purpose functions to ``Utils.cpp`` (e.g. conversion functions)
* Files with 500+ lines are most often better split up into multiple files
* Switch statements must always contain a `default` option
* Tripple nested if structures can **always** be written cleaner
* Give thought to an intuitive order of parameters, for example according to the order in the name of the function
* For single if-statements do not use a ``{}``-block
    * Good: ``if(foo) return 1;``
    * Bad: ``if(foo) {return 1;}``
* Prefer precise data-types (see ``<stdint.h>``) over generic ones
    * Bad: ``int foo``
    * Okay: ``byte foo``
    * Good: ``uint8 foo``
* In a microcontroller-context only use as much size for variables as needed
    * ``float`` instead of ``double`` by default
* Use ``char*`` instead of ``std::string``
* ``#define`` guard
    * use ``#pragma once`` if possible
    * To guarantee uniqueness, use the following format based on the project's source tree: ``<PROJECT>_<PATH>_<FILE>_H_``
    * Example: ``#define FOO_BAR_BAZ_H_``
* Includes
   * Prefer ``#include <timer.h>`` over ``#include "timer.h"``
* For non-trivial parameterlists try to use structs as these are often more hand and readable
* all functions related to debugging should only be compiled when ``DEBUG`` environment variable is set

## Comments
* Explain what is done not how it is done
* Wherever possible, comments for functions, classes etc. are made in the header-files
* Use javadoc-styled annotations (such as ``@param``), as it helps with generation documentation files
* Always comment the purpose, parameters, return-value and sideeffects of a function in, unless it is a trivial method such as getters/setters
* Use ``TODO`` whenever something needs to be checked or implemented, as it can be found much easier that way. A code without TODOs is assumed to be done
* TODOS should be finished when merging into master/main
* Pay attention to punctuation, spelling and grammer, as documentation does not to become more painful to read than it already is
* imports must not be commented as it leads to many duplicate comments

## Documentation
* For reusable code such as a library, make sure to provide a readme that explains the usage (preferably with examples), so a new user won't have to dive into the code
* For complex applications and systems, provide a description and preferably graphical representation of the structure, such as architecture-, code- and network-structure
* For anything that will be used by customers, provide a manual that is styled in corporate design

## Versioning
A version is meant to be more than just an incremented number that disambigously distinguishes different configurations from each other. Therefore we use Semantic Versioning:

``<major>.<minor>.<patch>``

**Major:** API incompatible changes (breaking changes are introduced) that require the user to change their code

**Minor:** New API features are added however the old API stays still functional and in tact

**Patch:** Some bugs have been fixed/patched the api did not change at all
