/*
 *
 * MIT License
 *
 * Copyright (c) 2016 The Cats Project
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

#include <iostream>

#include "Cats/Corecat/Stream.hpp"
#include "Cats/Textcat/XML.hpp"

using namespace Cats::Corecat;
using namespace Cats::Textcat;

int main() {
    
    std::ios::sync_with_stdio(false);
    
    auto wrapper = Stream::createWrapper(std::cout);
    XML::Serializer s(wrapper);
    s.startDocument();
        s.startElement("list");
            s.endAttributes();
            s.startElement("person");
                s.attribute("name", "SuperSodaSea");
                s.attribute("gender", "male");
                s.attribute("age", "16");
                s.endAttributes();
            s.endElement("person");
        s.endElement("list");
    s.endDocument();
    std::cout << std::endl;
    
    return 0;
    
}
