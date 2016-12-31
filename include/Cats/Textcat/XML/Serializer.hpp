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

#ifndef CATS_TEXTCAT_XML_SERIALIZER_HPP
#define CATS_TEXTCAT_XML_SERIALIZER_HPP


#include <cassert>
#include <cstring>

#include <iostream>

#include "Cats/Corecat/Stream.hpp"
#include "Cats/Corecat/String.hpp"

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
    void startElement(Corecat::StringView name) {
        
        stream->write("<", 1);
        stream->write(name.getData(), name.getLength());
        
    }
    void endElement(Corecat::StringView name) {
        
        stream->write("</", 2);
        stream->write(name.getData(), name.getLength());
        stream->write(">", 1);
        
    }
    void endAttributes(bool empty) {
        
        if(empty) stream->write("/>", 2);
        else stream->write(">", 1);
        
    }
    void doctype() {}
    void attribute(Corecat::StringView name, Corecat::StringView value) {
        
        stream->write(" ", 1);
        stream->write(name.getData(), name.getLength());
        stream->write("=\"", 2);
        stream->write(value.getData(), value.getLength());
        stream->write("\"", 1);
        
    }
    void text(Corecat::StringView value) {
        
        stream->write(value.getData(), value.getLength());
        
    }
    void cdata(Corecat::StringView value) {
        
        stream->write("<![CDATA[", 9);
        stream->write(value.getData(), value.getLength());
        stream->write("]]>", 3);
        
    }
    void comment(Corecat::StringView value) {
        
        stream->write("<!--", 4);
        stream->write(value.getData(), value.getLength());
        stream->write("-->", 3);
        
    }
    void processingInstruction(Corecat::StringView name, Corecat::StringView value) {
        
        stream->write("<?", 2);
        stream->write(name.getData(), name.getLength());
        stream->write(" ", 1);
        stream->write(value.getData(), value.getLength());
        stream->write("?>", 2);
        
    }
    
};

}
}
}


#endif
