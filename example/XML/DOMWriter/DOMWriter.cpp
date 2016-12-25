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

#include "Cats/Textcat/XML.hpp"

using namespace Cats::Textcat;

int main() {
    
    std::ios::sync_with_stdio(false);
    
    XML::Document document;
    XML::Element list("list"); document.appendChild(list);
        XML::Element person("person"); list.appendChild(person);
            XML::Attribute name("name", "SuperSodaSea"); person.appendAttribute(name);
            XML::Attribute gender("gender", "male"); person.appendAttribute(gender);
            XML::Attribute age("age", "16"); person.appendAttribute(age);
    std::cout << document << std::endl;
    
    return 0;
    
}
