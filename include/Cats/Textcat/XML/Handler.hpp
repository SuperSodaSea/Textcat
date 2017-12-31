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

#ifndef CATS_TEXTCAT_XML_HANDLER_HPP
#define CATS_TEXTCAT_XML_HANDLER_HPP


#include "Cats/Corecat/Text/String.hpp"


namespace Cats {
namespace Textcat{
inline namespace XML {

class XMLHandlerBase {
    
protected:
    
    using StringView8 = Corecat::StringView8;
    
public:
    
    void startDocument() {}
    void endDocument() {}
    void startElement(StringView8 /*name*/) {}
    void endElement(StringView8 /*name*/) {}
    void endAttributes(bool /*empty*/) {}
    void doctype() {}
    void attribute(StringView8 /*name*/, StringView8 /*value*/) {}
    void text(StringView8 /*value*/) {}
    void cdata(StringView8 /*value*/) {}
    void comment(StringView8 /*value*/) {}
    void processingInstruction(StringView8 /*name*/, StringView8 /*value*/) {}
    
};

}
}
}


#endif
