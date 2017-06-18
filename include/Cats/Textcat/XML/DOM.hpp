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

class Node;
class Element;
class Text;
class CDATA;
class Comment;
class ProcessingInstruction;
class Document;

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
    ~List() = default;
    
    T& append(Node& parent, T& child) {
        
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
    ~Node() = default;
    
    Type getType() const { return type; }
    
    Impl::List<Node>& child() { return listChild; }
    
    Node& getFirstChild() { return listChild.getFirst(); }
    Node& getLastChild() { return listChild.getLast(); }
    
    Node& appendChild(Node& child) { return listChild.append(*this, child); }
    Node& insertBefore(Node& child, Node& ref) { return listChild.insertBefore(child, ref); }
    Node& removeChild(Node& child) { return listChild.remove(child); }
    bool hasChildNodes() { return !listChild.empty(); }
    
    Element& asElement() noexcept { return reinterpret_cast<Element&>(*this); }
    const Element& asElement() const noexcept { return reinterpret_cast<const Element&>(*this); }
    Text& asText() noexcept { return reinterpret_cast<Text&>(*this); }
    const Text& asText() const noexcept { return reinterpret_cast<const Text&>(*this); }
    CDATA& asCDATA() noexcept { return reinterpret_cast<CDATA&>(*this); }
    const CDATA& asCDATA() const noexcept { return reinterpret_cast<const CDATA&>(*this); }
    Comment& asComment() noexcept { return reinterpret_cast<Comment&>(*this); }
    const Comment& asComment() const noexcept { return reinterpret_cast<const Comment&>(*this); }
    ProcessingInstruction& asProcessingInstruction() noexcept { return reinterpret_cast<ProcessingInstruction&>(*this); }
    const ProcessingInstruction& asProcessingInstruction() const noexcept { return reinterpret_cast<const ProcessingInstruction&>(*this); }
    Document& asDocument() noexcept { return reinterpret_cast<Document&>(*this); }
    const Document& asDocument() const noexcept { return reinterpret_cast<const Document&>(*this); }
    
};

class Attribute : public Impl::List<Attribute>::ListElement {
    
private:
    
    using StringView8 = Cats::Corecat::StringView8;
    
private:
    
    StringView8 name;
    StringView8 value;
    
public:
    
    Attribute() : Impl::List<Attribute>::ListElement(), name(), value() {}
    Attribute(StringView8 name_, StringView8 value_) :
        Impl::List<Attribute>::ListElement(), name(name_), value(value_) {}
    Attribute(const Attribute& src) = delete;
    ~Attribute() = default;
    
    StringView8 getName() const { return name; }
    void setName(StringView8 name_) { name = name_; }
    StringView8 getValue() const { return value; }
    void setValue(StringView8 value_) { value = value_; }

};

class Element : public Node {
    
private:
    
    using StringView8 = Cats::Corecat::StringView8;
    
private:
    
    Impl::List<Attribute> listAttr;
    StringView8 name;
    
public:
    
    Element() : Node(Type::Element), listAttr(), name() {}
    Element(StringView8 name_) : Node(Type::Element), listAttr(), name(name_) {}
    Element(const Element& src) = delete;
    ~Element() = default;
    
    Impl::List<Attribute>& attribute() { return listAttr; }
    
    StringView8 getName() const { return name; }
    void setName(StringView8 name_) { name = name_; }
    
    Attribute& getFirstAttribute() { return listAttr.getFirst(); }
    Attribute& getLastAttribute() { return listAttr.getLast(); }
    Attribute& appendAttribute(Attribute& attr) { return listAttr.append(*this, attr); }
    Attribute& removeAttribute(Attribute& attr) { return listAttr.remove(attr); }
    
};

class Text : public Node {
    
private:
    
    using StringView8 = Cats::Corecat::StringView8;
    
private:
    
    StringView8 value;
    
public:
    
    Text() : Node(Type::Text), value() {}
    Text(StringView8 value_) : Node(Type::Text), value(value_) {}
    Text(const Text& src) = delete;
    ~Text() = default;
    
    StringView8 getValue() const { return value; }
    void setValue(StringView8 value_) { value = value_; }
    
};

class CDATA : public Node {
    
private:
    
    using StringView8 = Cats::Corecat::StringView8;
    
private:
    
    StringView8 value;
    
public:
    
    CDATA() : Node(Type::CDATA), value() {}
    CDATA(StringView8 value_) : Node(Type::CDATA), value(value_) {}
    CDATA(const CDATA& src) = delete;
    ~CDATA() = default;
    
    StringView8 getValue() const { return value; }
    void setValue(StringView8 value_) { value = value_; }
    
};

class Comment : public Node {
    
private:
    
    using StringView8 = Cats::Corecat::StringView8;
    
private:
    
    StringView8 value;
    
public:
    
    Comment() : Node(Type::Comment), value() {}
    Comment(StringView8 value_) : Node(Type::Comment), value(value_) {}
    Comment(const Comment& src) = delete;
    ~Comment() = default;
    
    StringView8 getValue() const { return value; }
    void setValue(StringView8 value_) { value = value_; }
    
};

class ProcessingInstruction : public Node {
    
private:
    
    using StringView8 = Cats::Corecat::StringView8;
    
private:
    
    StringView8 name;
    StringView8 value;
    
public:
    
    ProcessingInstruction() : Node(Type::ProcessingInstruction), name(), value() {};
    ProcessingInstruction(StringView8& name_, StringView8& value_) :
        Node(Type::ProcessingInstruction), name(name_), value(value_) {}
    ProcessingInstruction(const ProcessingInstruction& src) = delete;
    ~ProcessingInstruction() = default;
    
    StringView8 getName() const { return name; }
    void setName(StringView8 name_) { name = name_; }
    StringView8 getValue() const { return value; }
    void setValue(StringView8 value_) { value = value_; }
    
};

class Document : public Node {
    
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
    
    Document() : Node(Type::Document), memoryPool() {}
    Document(const Document& src) = delete;
    ~Document() = default;
    
    Element& createElement(StringView8 name) {
        
        return *new(memoryPool.allocate(sizeof(Element))) Element(name);
        
    }
    Attribute& createAttribute(StringView8 name, StringView8 value) {
        
        return *new(memoryPool.allocate(sizeof(Attribute))) Attribute(name, value);
        
    }
    Text& createText(StringView8 value) {
        
        return *new(memoryPool.allocate(sizeof(Text))) Text(value);
        
    }
    CDATA& createCDATA(StringView8 value) {
        
        return *new(memoryPool.allocate(sizeof(CDATA))) CDATA(value);
        
    }
    Comment& createComment(StringView8 value) {
        
        return *new(memoryPool.allocate(sizeof(Comment))) Comment(value);
        
    }
    ProcessingInstruction& createProcessingInstruction(StringView8 name, StringView8 value) {
        
        return *new(memoryPool.allocate(sizeof(ProcessingInstruction))) ProcessingInstruction(name, value);
        
    }
    
    void clear() {
        
        memoryPool.clear();
        
    }
    
    Element& getRootElement() {
        
        for(auto& node : child()) if(node.getType() == Type::Element) return static_cast<Element&>(node);
        throw Exception("root element not found");
        
    }
    
    template <Parser::Flag F = Parser::Flag::Default>
    void parse(char* data) {
        
        class Handler : public HandlerBase {
            
        private:
            
            Document* document;
            Node* cur;
            
        public:
            
            Handler(Document* document_) : document(document_), cur(nullptr) {}
            
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
                
                static_cast<Element*>(cur)->appendAttribute(document->createAttribute(name, value));
                
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
        Parser parser;
        Handler handler(this);
        parser.parse<F>(data, handler);
        
    }
    
    void serialize(OutputStream<char>& stream) {
        
        Serializer serializer(stream);
        serializer.startDocument();
        if(hasChildNodes()) {
            
            Node* cur = &getFirstChild();
            while(true) {
                
                switch(cur->getType()) {
                
                case Type::Element: {
                    
                    auto& element = static_cast<Element&>(*cur);
                    serializer.startElement(element.getName());
                    for(auto& attr : element.attribute()) {
                        
                        serializer.attribute(attr.getName(), attr.getValue());
                        
                    }
                    bool empty = !cur->hasChildNodes();
                    serializer.endAttributes(empty);
                    if(!empty) { cur = &cur->getFirstChild(); continue; }
                    break;
                    
                }
                case Type::Text: {
                    
                    auto& text = static_cast<Text&>(*cur);
                    serializer.text(text.getValue());
                    break;
                    
                }
                case Type::CDATA: {
                    
                    auto& cdata = static_cast<CDATA&>(*cur);
                    serializer.cdata(cdata.getValue());
                    break;
                    
                }
                case Type::Comment: {
                    
                    auto& comment = static_cast<Comment&>(*cur);
                    serializer.comment(comment.getValue());
                    break;
                    
                }
                case Type::ProcessingInstruction: {
                    
                    auto& pi = static_cast<ProcessingInstruction&>(*cur);
                    serializer.processingInstruction(pi.getName(), pi.getValue());
                    break;
                    
                }
                default: throw std::runtime_error("invalid type");
                
                }
                while(!cur->next) {
                    
                    cur = cur->parent;
                    if(cur == this) break;
                    auto name = static_cast<Element*>(cur)->getName();
                    serializer.endElement(name);
                    
                }
                if(cur == this) break;
                cur = cur->next;
                
            }
            
        }
        serializer.endDocument();
        
    }
    
};

inline std::ostream& operator <<(std::ostream& stream, Document& document) {
    
    auto wrapper = Corecat::Stream::createWrapperOutputStream(stream);
    document.serialize(wrapper);
    return stream;
    
}

}
}
}


#endif
