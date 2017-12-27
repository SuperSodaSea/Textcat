# Textcat::XML

Textcat::XML contains the XML parser and serializer.


## About XML

XML stands for "Extensible Markup Language".

Specification for XML:

* [Extensible Markup Language (XML) 1.0](https://www.w3.org/TR/xml/)
* [Extensible Markup Language (XML) 1.1](https://www.w3.org/TR/xml11/)

Specification for DOM:

* [Document Object Model Level 1](https://www.w3.org/TR/REC-DOM-Level-1/)

## Features

* **Easy to use**. Textcat::XML provides both SAX and DOM style API.
* **High-performance**. Textcat::XML learned from RapidXml and RapidJSON, which are probably the fastest choices for XML and JSON. Under the same condition, it is sometimes even faster than RapidXml.
* **Header-only**. Textcat::XML is lightweight, and only require [Corecat][Corecat], which is the core of *The Cats Project* and is also header-only.


## Start in a minute

```cpp
#include <iostream>

#include "Cats/Textcat/XML.hpp"

using namespace Cats::Textcat::XML;

int main() {
    
    char data[] = R"(<list><person name="SuperSodaSea" gender="male" age="17"/></list>)";
	
    XMLDocument document;
    document.parse<>(data);
    std::cout << document << std::endl;
    
    return 0;
    
}
```
