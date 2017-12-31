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

#ifndef CATS_TEXTCAT_XML_SERIALIZER_HPP
#define CATS_TEXTCAT_XML_SERIALIZER_HPP


#include <cassert>
#include <cstring>

#include <iostream>

#include "Cats/Corecat/Data/Stream/OutputStream.hpp"
#include "Cats/Corecat/Text/String.hpp"

#include "Handler.hpp"


namespace Cats {
namespace Textcat{
inline namespace XML {

class XMLSerializer : public XMLHandlerBase {
    
private:
    
    template <typename T>
    using OutputStream = Corecat::OutputStream<T>;
    using StringView8 = Corecat::StringView8;
    
private:
    
    OutputStream<char>* stream;
    
public:
    
    XMLSerializer(OutputStream<char>& stream_) : stream(&stream_) {}
    
    void startDocument() {}
    void endDocument() {}
    void startElement(StringView8 name) {
        
        stream->writeAll("<", 1);
        stream->writeAll(name.getData(), name.getLength());
        
    }
    void endElement(StringView8 name) {
        
        stream->writeAll("</", 2);
        stream->writeAll(name.getData(), name.getLength());
        stream->writeAll(">", 1);
        
    }
    void endAttributes(bool empty) {
        
        if(empty) stream->writeAll("/>", 2);
        else stream->writeAll(">", 1);
        
    }
    void doctype() {}
    void attribute(StringView8 name, StringView8 value) {
        
        stream->writeAll(" ", 1);
        stream->writeAll(name.getData(), name.getLength());
        stream->writeAll("=\"", 2);
        stream->writeAll(value.getData(), value.getLength());
        stream->writeAll("\"", 1);
        
    }
    void text(StringView8 value) {
        
        stream->writeAll(value.getData(), value.getLength());
        
    }
    void cdata(StringView8 value) {
        
        stream->writeAll("<![CDATA[", 9);
        stream->writeAll(value.getData(), value.getLength());
        stream->writeAll("]]>", 3);
        
    }
    void comment(StringView8 value) {
        
        stream->writeAll("<!--", 4);
        stream->writeAll(value.getData(), value.getLength());
        stream->writeAll("-->", 3);
        
    }
    void processingInstruction(StringView8 name, StringView8 value) {
        
        stream->writeAll("<?", 2);
        stream->writeAll(name.getData(), name.getLength());
        stream->writeAll(" ", 1);
        stream->writeAll(value.getData(), value.getLength());
        stream->writeAll("?>", 2);
        
    }
    
    OutputStream<char>& getStream() { return *stream; }
    void setStream(OutputStream<char>& stream_) { stream = &stream_; }
    
};

}
}
}


#endif
