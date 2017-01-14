/*
 *
 * MIT License
 *
 * Copyright (c) 2016-2017 The Cats Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "Cats/Corecat/String.hpp"
#include "Cats/Textcat/XML.hpp"

using namespace Cats::Corecat;
using namespace Cats::Textcat;

class Handler : public XML::HandlerBase {
    
private:
    
    int level;
    
private:
    
    void indent() { for(int i = 0; i < level; ++i) std::cout << "    "; }
    
public:
    
    void startDocument() { level = 0; std::cout << "startDocument()\n"; }
    void endDocument() { std::cout << "endDocument()\n"; }
    void startElement(StringView name) {
        
        indent(); std::cout << "startElement(\"" << name << "\")\n"; ++level;
        
    }
    void endElement(StringView name) {
        
        --level; indent(); std::cout << "endElement(\"" << name << "\")\n";
        
    }
    void endAttributes(bool empty) {
        
        indent(); std::cout << "endAttributes(" << std::boolalpha << empty << ")\n"; if(empty) --level;
        
    }
    void doctype() { indent(); std::cout << "doctype()\n"; }
    void attribute(StringView name, StringView value) {
        
        indent(); std::cout << "attribute(\"" << name << "\", \"" << value << "\")\n";
        
    }
    void text(StringView value) {
        
        indent(); std::cout << "text(\"" << value << "\")\n";
        
    }
    void cdata(StringView value) {
        
        indent(); std::cout << "cdata(\"" << value << "\")\n";
        
    }
    void comment(StringView value) {
        
        indent(); std::cout << "comment(\"" << value << "\")\n";
        
    }
    void processingInstruction(StringView name, StringView value) {
        
        indent(); std::cout << "processingInstruction(\"" << name << "\", \"" << value << "\")\n";
        
    }
    
};

void readFile(const char* name, std::vector<char>& data) {
    
    std::size_t size;
    std::ifstream is(name, std::ios::binary);
    if(!is) throw std::runtime_error("can't read file");
    is.seekg(0, std::ios::end);
    size = is.tellg();
    is.seekg(0);
    data.resize(size + 1);
    is.read(data.data(), size);
    data[size] = 0;
    
}

int main(int argc, char** argv) {
    
    std::ios::sync_with_stdio(false);
    
    if(argc < 2) {
        
        std::cout << "error: file name needed" << std::endl;
        return 1;
        
    }
    try {
        
        std::vector<char> data;
        readFile(argv[1], data);
        XML::Parser parser;
        Handler handler;
        parser.parse<>(data.data(), handler);
        
    } catch(std::exception& e) {
        
        std::cout << "exception: " << e.what() << std::endl;
        
    }
    
    return 0;
    
}
