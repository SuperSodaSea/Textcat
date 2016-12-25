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

#ifndef CATS_TEXTCAT_XML_SERIALIZER_HPP
#define CATS_TEXTCAT_XML_SERIALIZER_HPP


#include <cassert>
#include <cstring>

#include <iostream>

#include "Cats/Corecat/Stream.hpp"

#include "Handler.hpp"


namespace Cats {
namespace Textcat{
namespace XML {

class Serializer : public HandlerBase {
    
private:
    
    Corecat::Stream::Base* stream;
    
public:
    
    Serializer() = default;
    Serializer(Corecat::Stream::Base& stream_) : stream(&stream_) { assert(stream->isWriteable()); }
    
    Corecat::Stream::Base& getStream() { return *stream; }
    void setStream(Corecat::Stream::Base& stream_) { stream = &stream_; }
    
    void startDocument() {}
    void endDocument() {}
    void startElement(const char* name, std::size_t nameLength) {
        
        stream->write("<", 1);
        stream->write(name, nameLength);
        
    }
    void startElement(const char* name) { startElement(name, std::strlen(name)); }
    void endElement(const char* name, std::size_t nameLength) {
        
        stream->write("</", 2);
        stream->write(name, nameLength);
        stream->write(">", 1);
        
    }
    void endElement(const char* name) { endElement(name, std::strlen(name)); }
    void endAttributes() {
        
        stream->write(">", 1);
        
    }
    void doctype() {}
    void attribute(const char* name, std::size_t nameLength, const char* value, std::size_t valueLength) {
        
        stream->write(" ", 1);
        stream->write(name, nameLength);
        stream->write("=\"", 2);
        stream->write(value, valueLength);
        stream->write("\"", 1);
        
    }
    void attribute(const char* name, const char* value) { attribute(name, std::strlen(name), value, std::strlen(value)); }
    void text(const char* value, std::size_t valueLength) {
        
        stream->write(value, valueLength);
        
    }
    void text(const char* value) { text(value, std::strlen(value)); }
    void cdata(const char* value, std::size_t valueLength) {
        
        stream->write("<![CDATA[", 9);
        stream->write(value, valueLength);
        stream->write("]]>", 3);
        
    }
    void cdata(const char* value) { cdata(value, std::strlen(value)); }
    void comment(const char* value, std::size_t valueLength) {
        
        stream->write("<!--", 4);
        stream->write(value, valueLength);
        stream->write("-->", 3);
        
    }
    void comment(const char* value) { comment(value, std::strlen(value)); }
    void processingInstruction(const char* name, std::size_t nameLength, const char* value, std::size_t valueLength) {
        
        stream->write("<?", 2);
        stream->write(name, nameLength);
        stream->write(" ", 1);
        stream->write(value, valueLength);
        stream->write("?>", 2);
        
    }
    void processingInstruction(const char* name, const char* value) { processingInstruction(name, std::strlen(name), value, std::strlen(value)); }
    
};

}
}
}


#endif
