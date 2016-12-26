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

#ifndef CATS_TEXTCAT_XML_DOM_HPP
#define CATS_TEXTCAT_XML_DOM_HPP


#include <cassert>
#include <cstring>

#include <new>
#include <iostream>

#include "Cats/Corecat/MemoryPool.hpp"
#include "Cats/Corecat/Stream.hpp"

#include "Handler.hpp"
#include "Parser.hpp"
#include "Serializer.hpp"


namespace Cats {
namespace Textcat{
namespace XML {

class Node;

namespace Impl {

template <typename T>
class List {
    
public:
    
    struct ListElement {
        
        T* prev;
        T* next;
        Node* parent;
        
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
        bool operator ==(Node* q) { return p == q; }
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
    
    T& append(Node& parent, T& child) {
        
        assert(!child.parent);
        if(first) { child.prev = last; last->next = &child; last = &child; }
        else first = last = &child;
        child.parent = &parent;
        return child;
        
    }
    
    T& insertBefore(T& child, T& ref) {
        
        assert(!child->parent && ref->parent);
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

class String {
    
private:
    
    const char* data;
    std::size_t length;
    
public:
    
    String() = default;
    String(const char* data_) : data(data_), length(std::strlen(data_)) {}
    String(const char* data_, std::size_t length_) : data(data_), length(length_) {}
    String(const String& src) : data(src.data), length(src.length) {}
    
    const char* getData() { return data; }
    std::size_t getLength() { return length; }
    void set(const char* data_) { assert(data); set(data_, std::strlen(data_)); }
    void set(const char* data_, std::size_t length_) { data = data_; length = length_; }
    
};

enum class Type : uint16_t {
    
    Element,
    Text,
    CDATA,
    Comment,
    ProcessingInstruction,
    Document,
    
};

class Node : public Impl::List<Node>::ListElement {
    
private:
    
    const Type type;
    Impl::List<Node> listChild;
    
public:
    
    Node(Type type_) : Impl::List<Node>::ListElement(), type(type_), listChild() {}
    Node(const Node& src) = delete;
    
    Type getType() const { return type; }
    
    Impl::List<Node>& child() { return listChild; }
    
    Node& getFirstChild() { return listChild.getFirst(); }
    Node& getLastChild() { return listChild.getLast(); }
    
    Node& appendChild(Node& child) { return listChild.append(*this, child); }
    Node& insertBefore(Node& child, Node& ref) { return listChild.insertBefore(child, ref); }
    Node& removeChild(Node& child) { return listChild.remove(child); }
    bool hasChildNodes() { return !listChild.empty(); }
    
};

class Attribute : public Impl::List<Attribute>::ListElement {
    
private:
    
    String name;
    String value;
    
public:
    
    Attribute() : Impl::List<Attribute>::ListElement(), name(), value() {}
    Attribute(const String& name_, const String& value_) :
        Impl::List<Attribute>::ListElement(), name(name_), value(value_) {}
    Attribute(const Attribute& src) = delete;
    
    String& getName() { return name; }
    String& getValue() { return value; }

};

class Element : public Node {
    
private:
    
    Impl::List<Attribute> listAttr;
    String name;
    
public:
    
    Element() : Node(Type::Element), listAttr(), name() {}
    Element(const String& name_) : Node(Type::Element), listAttr(), name(name_) {}
    Element(const Element& src) = delete;
    
    Impl::List<Attribute>& attribute() { return listAttr; }
    
    Attribute& getFirstAttribute() { return listAttr.getFirst(); }
    Attribute& getLastAttribute() { return listAttr.getLast(); }
    Attribute& appendAttribute(Attribute& attr) { return listAttr.append(*this, attr); }
    Attribute& removeAttribute(Attribute& attr) { return listAttr.remove(attr); }
    String& getName() { return name; }
    
};

class Text : public Node {
    
private:
    
    String value;
    
public:
    
    Text() : Node(Type::Text), value() {}
    Text(const String& value_) : Node(Type::Text), value(value_) {}
    Text(const Text& src) = delete;
    
    String& getValue() { return value; }
    
};

class CDATA : public Node {
    
private:
    
    String value;
    
public:
    
    CDATA() : Node(Type::CDATA), value() {}
    CDATA(const String& value_) : Node(Type::CDATA), value(value_) {}
    CDATA(const CDATA& src) = delete;
    
    String& getValue() { return value; }
    
};

class Comment : public Node {
    
private:
    
    String value;
    
public:
    
    Comment() : Node(Type::Comment), value() {}
    Comment(const String& value_) : Node(Type::Comment), value(value_) {}
    Comment(const Comment& src) = delete;
    
    String& getValue() { return value; }
    
};

class ProcessingInstruction : public Node {
    
private:
    
    String name;
    String value;
    
public:
    
    ProcessingInstruction() : Node(Type::ProcessingInstruction), name(), value() {};
    ProcessingInstruction(const String& name_, const String& value_) :
        Node(Type::ProcessingInstruction), name(name_), value(value_) {}
    ProcessingInstruction(const ProcessingInstruction& src) = delete;
    
    String& getName() { return name; }
    String& getValue() { return value; }
    
};

class Document : public Node {
    
private:
    
    Corecat::MemoryPoolFast<> memoryPool;
    
public:
    
    Document() : Node(Type::Document), memoryPool() {}
    Document(const Document& src) = delete;
    
    Element& createElement(const String& name) {
        
        return *new(memoryPool.allocate(sizeof(Element))) Element(name);
        
    }
    Attribute& createAttribute(const String& name, const String& value) {
        
        return *new(memoryPool.allocate(sizeof(Attribute))) Attribute(name, value);
        
    }
    Text& createText(const String& value) {
        
        return *new(memoryPool.allocate(sizeof(Text))) Text(value);
        
    }
    CDATA& createCDATA(const String& value) {
        
        return *new(memoryPool.allocate(sizeof(CDATA))) CDATA(value);
        
    }
    Comment& createComment(const String& value) {
        
        return *new(memoryPool.allocate(sizeof(Comment))) Comment(value);
        
    }
    ProcessingInstruction& createProcessingInstruction(const String& name, const String& value) {
        
        return *new(memoryPool.allocate(sizeof(ProcessingInstruction))) ProcessingInstruction(name, value);
        
    }
    
    void clear() {
        
        memoryPool.clear();
        
    }
    
    template <Parser::Flag F>
    void parse(char* data) {
        
        class Handler : public HandlerBase {
            
        private:
            
            Document* document;
            Node* cur;
            
        public:
            
            Handler(Document* document_) : document(document_), cur(nullptr) {}
            
            void startDocument() { cur = document; }
            void startElement(const char* name, std::size_t nameLength) {
                
                auto& element = document->createElement({name, nameLength});
                cur->appendChild(element);
                cur = &element;
                
            }
            void endElement(const char* /*name*/, std::size_t /*nameLength*/) {
                
                cur = cur->parent;
                
            }
            void attribute(const char* name, std::size_t nameLength, const char* value, std::size_t valueLength) {
                
                static_cast<Element*>(cur)->appendAttribute(document->createAttribute({name, nameLength}, {value, valueLength}));
                
            }
            void text(const char* value, std::size_t valueLength) {
                
                cur->appendChild(document->createText({value, valueLength}));
                
            }
            void cdata(const char* value, std::size_t valueLength) {
                
                cur->appendChild(document->createCDATA({value, valueLength}));
                
            }
            void comment(const char* value, std::size_t valueLength) {
                
                cur->appendChild(document->createComment({value, valueLength}));
                
            }
            void processingInstruction(const char* name, std::size_t nameLength, const char* value, std::size_t valueLength) {
                
                cur->appendChild(document->createProcessingInstruction({name, nameLength}, {value, valueLength}));
                
            }
            
        };
        
        assert(data);
        
        clear();
        Parser parser;
        Handler handler(this);
        parser.parse<F>(data, handler);
        
    }
    
    void serialize(Corecat::Stream::Base& stream) {
        
        Serializer serializer(stream);
        serializer.startDocument();
        if(hasChildNodes()) {
            
            Node* cur = &getFirstChild();
            while(true) {
                
                switch(cur->getType()) {
                
                case Type::Element: {
                    
                    auto& element = static_cast<Element&>(*cur);
                    serializer.startElement(element.getName().getData(), element.getName().getLength());
                    for(auto& attr : element.attribute()) {
                        
                        auto& name = attr.getName();
                        auto& value = attr.getValue();
                        serializer.attribute(name.getData(), name.getLength(), value.getData(), value.getLength());
                        
                    }
                    serializer.endAttributes();
                    if(cur->hasChildNodes()) { cur = &cur->getFirstChild(); continue; }
                    else serializer.endElement(element.getName().getData(), element.getName().getLength());
                    break;
                    
                }
                case Type::Text: {
                    
                    auto& text = static_cast<Text&>(*cur);
                    auto& value = text.getValue();
                    serializer.text(value.getData(), value.getLength());
                    break;
                    
                }
                case Type::CDATA: {
                    
                    auto& cdata = static_cast<CDATA&>(*cur);
                    auto& value = cdata.getValue();
                    serializer.cdata(value.getData(), value.getLength());
                    break;
                    
                }
                case Type::Comment: {
                    
                    auto& comment = static_cast<Comment&>(*cur);
                    auto& value = comment.getValue();
                    serializer.comment(value.getData(), value.getLength());
                    break;
                    
                }
                case Type::ProcessingInstruction: {
                    
                    auto& pi = static_cast<ProcessingInstruction&>(*cur);
                    auto& name = pi.getName();
                    auto& value = pi.getValue();
                    serializer.processingInstruction(name.getData(), name.getLength(), value.getData(), value.getLength());
                    break;
                    
                }
                default: throw std::runtime_error("invalid type");
                
                }
                while(!cur->next) {
                    
                    cur = cur->parent;
                    if(cur == this) break;
                    auto& name = static_cast<Element*>(cur)->getName();
                    serializer.endElement(name.getData(), name.getLength());
                    
                }
                if(cur == this) break;
                cur = cur->next;
                
            }
            
        }
        serializer.endDocument();
        
    }
    
};

inline std::ostream& operator <<(std::ostream& stream, Document& document) {
    
    auto wrapper = Corecat::Stream::createWrapper(stream);
    document.serialize(wrapper);
    return stream;
    
}

}
}
}


#endif
