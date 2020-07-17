# Style Guide
## Headers

```cpp
// Use pragma once instaed of header defines
#pragma once
```

## Classes

```cpp
class ExampleBase {
public:
    virtual ~ExampleBase() {}; // <= Use virtual destrucotrs in base class
};

class ExampleClass : public ExampleBase { // <= Always use public access-specifier 
public:
    ExampleClass();
    void PublicMethod() override; // <= Always use override on derived virtual methods
private:
    int member_var = 0;
};
```

## Enums 

```cpp
// Use the smallest type if possible
// Always prefix plain enum members with the enum name
// Use only plain enums for flagging
enum Color : uint8_t { 
    COLOR_RED,
    COLOR_GREEN,
    COLOR_BLUE
};

// Prefix not neceseary on enum classes
enum class Access : uint8_t {
    READ,
    WRITE
};
```

## Namespaces

```cpp
// Everything should be placed inside namespaces
// Every module has its own namespace inside the main namespace: Squid
namespace Squid {
namespace ModuleName {

    // Code...

} // namespace ModuleName
} // namespace Squid
```

