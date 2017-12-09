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

#ifndef CATS_TEXTCAT_XML_DOM_HPP
#define CATS_TEXTCAT_XML_DOM_HPP


#include <cassert>
#include <cstring>

#include <new>
#include <iostream>

#include "Cats/Corecat/MemoryPool.hpp"
#include "Cats/Corecat/Stream.hpp"
#include "Cats/Corecat/String.hpp"

#include "Handler.hpp"
#include "Parser.hpp"
#include "Serializer.hpp"


namespace Cats {
namespace Textcat{
namespace XML {

class XMLNode;
class XMLElement;
class XMLText;
class XMLCDATA;
class XMLComment;
class XMLProcessingInstruction;
class XMLDocument;

namespace Impl {

template <typename T>
class List {
    
public:
    
    struct ListElement {
        
        T* prev;
        T* next;
        XMLNode* parent;
        
        ListElement() : prev(), next(), parent() {}
        
    };
    
    class Iterator {
        
    private:
        
        List* list;
        ListElement* p;
        
    public:
        
        Iterator(List* list_, ListElement* p_) : list(list_), p(p_) {}
        Iterator(const Iterator& src) : list(src.list), p(src.p) {}
        
        T& operator *() const { return static_cast<T&>(*p); }
        T* operator ->() const { return static_cast<T*>(p); }
        bool operator ==(const Iterator& it) { return p == it.p; }
        bool operator ==(XMLNode* q) { return p == q; }
        bool operator !=(const Iterator& it) { return p != it.p; }
        Iterator& operator ++() { p = p->next; return *this; }
        Iterator operator ++(int) { Iterator tmp = *this; ++*this; return tmp; }
        Iterator& operator --() { p = p ? p->prev : list->last; return *this; }
        Iterator operator --(int) { Iterator tmp = *this; --*this; return tmp; }
        
    };
    
private:
    
    T* first;
    T* last;
    
public:
    
    List() : first(), last() {}
    List(const List& src) = delete;
    ~List() = default;
    
    T& append(XMLNode& parent, T& child) {
        
        assert(!child.parent);
        if(first) { child.prev = last; last->next = &child; last = &child; }
        else first = last = &child;
        child.parent = &parent;
        return child;
        
    }
    
    T& insertBefore(T& child, T& ref) {
        
        assert(!child.parent && ref.parent);
        auto pPrev = ref.prev;
        child.prev = pPrev;
        child.next = &ref;
        if(pPrev) pPrev->next = &child;
        ref.prev = &child;
        child.parent = ref.parent;
        return child;
        
    }
    
    T& remove(T& child) {
        
        auto pPrev = child.prev;
        auto pNext = child.next;
        if(pPrev) pPrev->next = pNext;
        else first = pNext;
        if(pNext) pNext->next = pPrev;
        else last = pPrev;
        child.prev = nullptr;
        child.next = nullptr;
        child.parent = nullptr;
        return child;
        
    }
    
    T& getFirst() { return *first; }
    T& getLast() { return *last; }
    
    bool empty() const { return !first; }
    
    Iterator begin() { return Iterator(this, first); }
    Iterator end() { return Iterator(this, nullptr); }
    
};

}

enum class Type : uint16_t {
    
    XMLElement,
    XMLText,
    XMLCDATA,
    XMLComment,
    XMLProcessingInstruction,
    XMLDocument,
    
};

class XMLNode : public Impl::List<XMLNode>::ListElement {
    
private:
    
    const Type type;
    Impl::List<XMLNode> listChild;
    
public:
    
    XMLNode(Type type_) : Impl::List<XMLNode>::ListElement(), type(type_), listChild() {}
    XMLNode(const XMLNode& src) = delete;
    ~XMLNode() = default;
    
    Type getType() const { return type; }
    
    Impl::List<XMLNode>& child() { return listChild; }
    
    XMLNode& getFirstChild() { return listChild.getFirst(); }
    XMLNode& getLastChild() { return listChild.getLast(); }
    
    XMLNode& appendChild(XMLNode& child) { return listChild.append(*this, child); }
    XMLNode& insertBefore(XMLNode& child, XMLNode& ref) { return listChild.insertBefore(child, ref); }
    XMLNode& removeChild(XMLNode& child) { return listChild.remove(child); }
    bool hasChildNodes() { return !listChild.empty(); }
    
    XMLElement& asElement() noexcept { return reinterpret_cast<XMLElement&>(*this); }
    const XMLElement& asElement() const noexcept { return reinterpret_cast<const XMLElement&>(*this); }
    XMLText& asText() noexcept { return reinterpret_cast<XMLText&>(*this); }
    const XMLText& asText() const noexcept { return reinterpret_cast<const XMLText&>(*this); }
    XMLCDATA& asCDATA() noexcept { return reinterpret_cast<XMLCDATA&>(*this); }
    const XMLCDATA& asCDATA() const noexcept { return reinterpret_cast<const XMLCDATA&>(*this); }
    XMLComment& asComment() noexcept { return reinterpret_cast<XMLComment&>(*this); }
    const XMLComment& asComment() const noexcept { return reinterpret_cast<const XMLComment&>(*this); }
    XMLProcessingInstruction& asProcessingInstruction() noexcept { return reinterpret_cast<XMLProcessingInstruction&>(*this); }
    const XMLProcessingInstruction& asProcessingInstruction() const noexcept { return reinterpret_cast<const XMLProcessingInstruction&>(*this); }
    XMLDocument& asDocument() noexcept { return reinterpret_cast<XMLDocument&>(*this); }
    const XMLDocument& asDocument() const noexcept { return reinterpret_cast<const XMLDocument&>(*this); }
    
};

class XMLAttribute : public Impl::List<XMLAttribute>::ListElement {
    
private:
    
    using StringView8 = Cats::Corecat::StringView8;
    
private:
    
    StringView8 name;
    StringView8 value;
    
public:
    
    XMLAttribute() : Impl::List<XMLAttribute>::ListElement(), name(), value() {}
    XMLAttribute(StringView8 name_, StringView8 value_) :
        Impl::List<XMLAttribute>::ListElement(), name(name_), value(value_) {}
    XMLAttribute(const XMLAttribute& src) = delete;
    ~XMLAttribute() = default;
    
    StringView8 getName() const { return name; }
    void setName(StringView8 name_) { name = name_; }
    StringView8 getValue() const { return value; }
    void setValue(StringView8 value_) { value = value_; }

};

class XMLElement : public XMLNode {
    
private:
    
    using StringView8 = Cats::Corecat::StringView8;
    
private:
    
    Impl::List<XMLAttribute> listAttr;
    StringView8 name;
    
public:
    
    XMLElement() : XMLNode(Type::XMLElement), listAttr(), name() {}
    XMLElement(StringView8 name_) : XMLNode(Type::XMLElement), listAttr(), name(name_) {}
    XMLElement(const XMLElement& src) = delete;
    ~XMLElement() = default;
    
    Impl::List<XMLAttribute>& attribute() { return listAttr; }
    
    StringView8 getName() const { return name; }
    void setName(StringView8 name_) { name = name_; }
    
    XMLAttribute& getFirstAttribute() { return listAttr.getFirst(); }
    XMLAttribute& getLastAttribute() { return listAttr.getLast(); }
    XMLAttribute& appendAttribute(XMLAttribute& attr) { return listAttr.append(*this, attr); }
    XMLAttribute& removeAttribute(XMLAttribute& attr) { return listAttr.remove(attr); }
    
};

class XMLText : public XMLNode {
    
private:
    
    using StringView8 = Cats::Corecat::StringView8;
    
private:
    
    StringView8 value;
    
public:
    
    XMLText() : XMLNode(Type::XMLText), value() {}
    XMLText(StringView8 value_) : XMLNode(Type::XMLText), value(value_) {}
    XMLText(const XMLText& src) = delete;
    ~XMLText() = default;
    
    StringView8 getValue() const { return value; }
    void setValue(StringView8 value_) { value = value_; }
    
};

class XMLCDATA : public XMLNode {
    
private:
    
    using StringView8 = Cats::Corecat::StringView8;
    
private:
    
    StringView8 value;
    
public:
    
    XMLCDATA() : XMLNode(Type::XMLCDATA), value() {}
    XMLCDATA(StringView8 value_) : XMLNode(Type::XMLCDATA), value(value_) {}
    XMLCDATA(const XMLCDATA& src) = delete;
    ~XMLCDATA() = default;
    
    StringView8 getValue() const { return value; }
    void setValue(StringView8 value_) { value = value_; }
    
};

class XMLComment : public XMLNode {
    
private:
    
    using StringView8 = Cats::Corecat::StringView8;
    
private:
    
    StringView8 value;
    
public:
    
    XMLComment() : XMLNode(Type::XMLComment), value() {}
    XMLComment(StringView8 value_) : XMLNode(Type::XMLComment), value(value_) {}
    XMLComment(const XMLComment& src) = delete;
    ~XMLComment() = default;
    
    StringView8 getValue() const { return value; }
    void setValue(StringView8 value_) { value = value_; }
    
};

class XMLProcessingInstruction : public XMLNode {
    
private:
    
    using StringView8 = Cats::Corecat::StringView8;
    
private:
    
    StringView8 name;
    StringView8 value;
    
public:
    
    XMLProcessingInstruction() : XMLNode(Type::XMLProcessingInstruction), name(), value() {};
    XMLProcessingInstruction(StringView8& name_, StringView8& value_) :
        XMLNode(Type::XMLProcessingInstruction), name(name_), value(value_) {}
    XMLProcessingInstruction(const XMLProcessingInstruction& src) = delete;
    ~XMLProcessingInstruction() = default;
    
    StringView8 getName() const { return name; }
    void setName(StringView8 name_) { name = name_; }
    StringView8 getValue() const { return value; }
    void setValue(StringView8 value_) { value = value_; }
    
};

class XMLDocument : public XMLNode {
    
private:
    
    using StringView8 = Cats::Corecat::StringView8;
    template <typename T>
    using OutputStream = Cats::Corecat::Stream::OutputStream<T>;
    
public:
    
    class Exception : public std::exception {
        
    private:
        
        const char* str;
        
    public:
        
        Exception(const char* str_) : str(str_) {}
        Exception(const Exception& src) : str(src.str) {}
        
        const char* what() const noexcept final { return str; }
        
    };
    
private:
    
    Corecat::MemoryPoolFast<> memoryPool;
    
public:
    
    XMLDocument() : XMLNode(Type::XMLDocument), memoryPool() {}
    XMLDocument(const XMLDocument& src) = delete;
    ~XMLDocument() = default;
    
    XMLElement& createElement(StringView8 name) {
        
        return *new(memoryPool.allocate(sizeof(XMLElement))) XMLElement(name);
        
    }
    XMLAttribute& createAttribute(StringView8 name, StringView8 value) {
        
        return *new(memoryPool.allocate(sizeof(XMLAttribute))) XMLAttribute(name, value);
        
    }
    XMLText& createText(StringView8 value) {
        
        return *new(memoryPool.allocate(sizeof(XMLText))) XMLText(value);
        
    }
    XMLCDATA& createCDATA(StringView8 value) {
        
        return *new(memoryPool.allocate(sizeof(XMLCDATA))) XMLCDATA(value);
        
    }
    XMLComment& createComment(StringView8 value) {
        
        return *new(memoryPool.allocate(sizeof(XMLComment))) XMLComment(value);
        
    }
    XMLProcessingInstruction& createProcessingInstruction(StringView8 name, StringView8 value) {
        
        return *new(memoryPool.allocate(sizeof(XMLProcessingInstruction))) XMLProcessingInstruction(name, value);
        
    }
    
    void clear() {
        
        memoryPool.clear();
        
    }
    
    XMLElement& getRootElement() {
        
        for(auto& node : child()) if(node.getType() == Type::XMLElement) return static_cast<XMLElement&>(node);
        throw Exception("root element not found");
        
    }
    
    template <XMLParser::Flag F = XMLParser::Flag::Default>
    void parse(char* data) {
        
        class Handler : public XMLHandlerBase {
            
        private:
            
            XMLDocument* document;
            XMLNode* cur;
            
        public:
            
            Handler(XMLDocument* document_) : document(document_), cur(nullptr) {}
            
            void startDocument() { cur = document; }
            void startElement(StringView8 name) {
                
                auto& element = document->createElement(name);
                cur->appendChild(element);
                cur = &element;
                
            }
            void endElement(StringView8 /*name*/) {
                
                cur = cur->parent;
                
            }
            void endAttributes(bool empty) {
                
                if(empty) cur = cur->parent;
                
            }
            void attribute(StringView8 name, StringView8 value) {
                
                static_cast<XMLElement*>(cur)->appendAttribute(document->createAttribute(name, value));
                
            }
            void text(StringView8 value) {
                
                cur->appendChild(document->createText(value));
                
            }
            void cdata(StringView8 value) {
                
                cur->appendChild(document->createCDATA(value));
                
            }
            void comment(StringView8 value) {
                
                cur->appendChild(document->createComment(value));
                
            }
            void processingInstruction(StringView8 name, StringView8 value) {
                
                cur->appendChild(document->createProcessingInstruction(name, value));
                
            }
            
        };
        
        assert(data);
        
        clear();
        XMLParser parser;
        Handler handler(this);
        parser.parse<F>(data, handler);
        
    }
    
    void serialize(OutputStream<char>& stream) {
        
        XMLSerializer serializer(stream);
        serializer.startDocument();
        if(hasChildNodes()) {
            
            XMLNode* cur = &getFirstChild();
            while(true) {
                
                switch(cur->getType()) {
                
                case Type::XMLElement: {
                    
                    auto& element = static_cast<XMLElement&>(*cur);
                    serializer.startElement(element.getName());
                    for(auto& attr : element.attribute()) {
                        
                        serializer.attribute(attr.getName(), attr.getValue());
                        
                    }
                    bool empty = !cur->hasChildNodes();
                    serializer.endAttributes(empty);
                    if(!empty) { cur = &cur->getFirstChild(); continue; }
                    break;
                    
                }
                case Type::XMLText: {
                    
                    auto& text = static_cast<XMLText&>(*cur);
                    serializer.text(text.getValue());
                    break;
                    
                }
                case Type::XMLCDATA: {
                    
                    auto& cdata = static_cast<XMLCDATA&>(*cur);
                    serializer.cdata(cdata.getValue());
                    break;
                    
                }
                case Type::XMLComment: {
                    
                    auto& comment = static_cast<XMLComment&>(*cur);
                    serializer.comment(comment.getValue());
                    break;
                    
                }
                case Type::XMLProcessingInstruction: {
                    
                    auto& pi = static_cast<XMLProcessingInstruction&>(*cur);
                    serializer.processingInstruction(pi.getName(), pi.getValue());
                    break;
                    
                }
                default: throw std::runtime_error("invalid type");
                
                }
                while(!cur->next) {
                    
                    cur = cur->parent;
                    if(cur == this) break;
                    auto name = static_cast<XMLElement*>(cur)->getName();
                    serializer.endElement(name);
                    
                }
                if(cur == this) break;
                cur = cur->next;
                
            }
            
        }
        serializer.endDocument();
        
    }
    
};

inline std::ostream& operator <<(std::ostream& stream, XMLDocument& document) {
    
    auto wrapper = Corecat::Stream::createWrapperOutputStream(stream);
    document.serialize(wrapper);
    return stream;
    
}

}
}
}


#endif
