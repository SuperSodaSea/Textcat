/*
 *
 * MIT License
 *
 * Copyright (c) 2016-2018 The Cats Project
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
#include <vector>

#include "Cats/Textcat/XML.hpp"

using namespace Cats::Corecat;
using namespace Cats::Textcat;

std::vector<char> readFile(const char* name) {
    
    std::ifstream is(name, std::ios::binary);
    if(!is) throw IOException("Cannot read file");
    is.seekg(0, std::ios::end);
    std::size_t size = static_cast<std::size_t>(is.tellg());
    is.seekg(0);
    std::vector<char> data(size + 1);
    is.read(data.data(), size);
    data[size] = 0;
    return data;
    
}

int main(int argc, char** argv) {
    
    try {
        
        if(argc < 2) throw InvalidArgumentException("File name needed");
        
        for(int i = 1; i < argc; ++i) {
            
            auto data = readFile(argv[i]);
            XMLDocument document;
            document.parse<>(data.data());
            std::cout << document << std::endl;
            
        }
        
    } catch(std::exception& e) { std::cerr << e.what() << std::endl; return 1; }
    
    return 0;
    
}
