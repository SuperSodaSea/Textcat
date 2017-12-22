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

#ifndef CATS_TEXTCAT_XML_PARSER_HPP
#define CATS_TEXTCAT_XML_PARSER_HPP


#include <cassert>
#include <cstdint>

#include <algorithm>
#include <exception>
#include <limits>

#include "Cats/Corecat/Sequence.hpp"


namespace Cats {
namespace Textcat{
namespace XML {

namespace Impl {

template <typename T, T... V>
struct Include {
    
    static constexpr bool get(T t) { using namespace Cats::Corecat::Sequence; return ContainSequence<Sequence<T, V...>>::get(t); }
    
};

template <typename T, T... V>
struct Exclude {
    
    static constexpr bool get(T t) { using namespace Cats::Corecat::Sequence; return !ContainSequence<Sequence<T, V...>>::get(t); }
    
};


template <typename Cond, typename = void>
struct Skipper {
    
    static size_t skip(char*& p) {
        
        using namespace Cats::Corecat::Sequence;
        
        auto t = p;
        while(SequenceTable<MapperSequence<Cond, IndexSequence<unsigned char, 0, 255>>>::get(*t)) ++t;
        const size_t length = t - p;
        p = t;
        return length;
        
    }
    
};


using Space = Include<unsigned char, '\t', '\n', '\r', ' '>;
using Name = Exclude<unsigned char, 0, '\t', '\n', '\r', ' ', '/', '>', '?'>;
using AttributeName = Exclude<unsigned char, 0, '\t', '\n', '\r', ' ', '!', '/', '<', '=', '>', '?'>;
using AttributeValue1 = Exclude<unsigned char, 0, '"'>;
using AttributeValueNoRef1 = Exclude<unsigned char, 0, '"', '&'>;
using AttributeValue2 = Exclude<unsigned char, 0, '\''>;
using AttributeValueNoRef2 = Exclude<unsigned char, 0, '&', '\''>;
using Text = Exclude<unsigned char, 0, '<'>;
using TextNoSpace = Exclude<unsigned char, 0, '\t', '\n', '\r', ' ', '<'>;
using TextNoRef = Exclude<unsigned char, 0, '&', '<'>;
using TextNoSpaceRef = Exclude<unsigned char, 0, '\t', '\n', '\r', ' ', '&', '<'>;

struct Decimal {
    
    static constexpr unsigned char get(unsigned char t) {
        
        return (t >= '0' && t <= '9') ? (t - '0') : 255;
        
    }
    
};

struct Hexadecimal {
    
    static constexpr unsigned char get(unsigned char t) {
        
        return (t >= '0' && t <= '9') ? (t - '0')
            : ((t >= 'A' && t <= 'F') ? (t - 'A' + 10)
                : ((t >= 'a' && t <= 'f') ? (t - 'a' + 10) : 255));
        
    }
    
};

}


class XMLParser {
    
private:
    
    using StringView8 = Corecat::Text::StringView8;
    
public:
    
    enum class Flag : std::uint32_t {
        
        None = 0x00000000,
        TrimSpace = 0x00000001,
        NormalizeSpace = 0x00000002,
        EntityTranslation = 0x00000004,
        ClosingTagValidate = 0x00000008,
        
        Default = TrimSpace | EntityTranslation,
        
    };
    friend constexpr bool operator &(Flag a, Flag b) {
        
        return static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b);
        
    }
    friend constexpr Flag operator |(Flag a, Flag b) {
        
        return static_cast<Flag>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
        
    }
    
    class Exception : public std::exception {
        
    private:
        
        std::size_t pos;
        const char* str;
        
    public:
        
        Exception(std::size_t pos_, const char* str_) : pos(pos_), str(str_) {}
        Exception(const Exception& src) : pos(src.pos), str(src.str) {}
        
        const char* what() const noexcept final { return str; }
        
    };
    
private:
    
    char* s;
    char* p;
    
private:
    
    template <Flag F>
    void parseReference(char*& q) {
        
        using namespace Corecat::Sequence;
        
        switch(p[1]) {
        
        case 0: throw Exception(p - s, "unexpected end");
        case '#': {
            
            if(p[2] == 'x') {
                
                p += 3;
                if(*p == ';') throw Exception(p - s, "unexpected ;");
                std::uint32_t code = 0;
                for(unsigned char t; (t = SequenceTable<MapperSequence<Impl::Hexadecimal, IndexSequence<unsigned char, 0, 255>>>::get(*p)) != 255; code = code * 16 + t, ++p);
                if(*p != ';') throw Exception(p - s, "expected ;");
                ++p;
                // TODO: Code conversion
                *q = code;
                ++q;
                
            } else {
                
                p += 2;
                if(*p == ';') throw Exception(p - s, "unexpected ;");
                std::uint32_t code = 0;
                for(unsigned char t; (t = SequenceTable<MapperSequence<Impl::Decimal, IndexSequence<unsigned char, 0, 255>>>::get(*p)) != 255; code = code * 10 + t, ++p);
                if(*p != ';') throw Exception(p - s, "expected ;");
                ++p;
                // TODO: Code conversion
                *q = code;
                ++q;
                
            }
            return;
            
        }
        case 'a': {
            
            if(p[2] == 'm' && p[3] == 'p' && p[4] == ';') {
                
                // amp
                p += 5;
                *q = '&';
                ++q;
                return;
                
            }
            if(p[2] == 'p' && p[3] == 'o' && p[4] == 's' && p[5] == ';') {
                
                // apos
                p += 6;
                *q = '\'';
                ++q;
                return;
                
            }
            break;
            
        }
        case 'g': {
            
            if(p[2] == 't' && p[3] == ';') {
                
                // gt
                p += 4;
                *q = '>';
                ++q;
                return;
                
            }
            break;
            
        }
        case 'l': {
            
            if(p[2] == 't' && p[3] == ';') {
                
                // lt
                p += 4;
                *q = '<';
                ++q;
                return;
                
            }
            break;
            
        }
        case 'q': {
            
            if(p[2] == 'u' && p[3] == 'o' && p[4] == 't' && p[5] == ';') {
                
                // quot
                p += 6;
                *q = '"';
                ++q;
                return;
                
            }
            break;
            
        }
        default: {
            
            break;
            
        }
        
        }
        throw Exception(p - s, "unexpected reference");
        
    }
    template <Flag F, typename H>
    void parseXMLDeclaration(H& /*handler*/) {
        
        using namespace Corecat::Sequence;
        
        Impl::Skipper<Impl::Space>::skip(p);
        
        // Parse "version"
        if(p[0] != 'v' || p[1] != 'e' || p[2] != 'r' || p[3] != 's' || p[4] != 'i' || p[5] != 'o' || p[6] != 'n')
            throw Exception(p - s, "expected version");
        p += 7;
        Impl::Skipper<Impl::Space>::skip(p);
        if(*p != '=') throw Exception(p - s, "expected =");
        ++p;
        Impl::Skipper<Impl::Space>::skip(p);
        if(*p == '"') {
            
            ++p;
            Impl::Skipper<Impl::AttributeValue1>::skip(p);
            if(*p != '"') throw Exception(p - s, "expected \"");

        } else if(*p == '\'') {
            
            ++p;
            Impl::Skipper<Impl::AttributeValue2>::skip(p);
            if(*p != '\'') throw Exception(p - s, "expected '");
            
        } else throw Exception(p - s, "expected \" or '");
        ++p;
        
        if(*p != '?' && !SequenceTable<MapperSequence<Impl::Space, IndexSequence<unsigned char, 0, 255>>>::get(*p))
            throw Exception(p - s, "unexpected character");
        Impl::Skipper<Impl::Space>::skip(p);
        
        // Parse "encoding"
        if(p[0] == 'e' && p[1] == 'n' && p[2] == 'c' && p[3] == 'o' && p[4] == 'd' && p[5] == 'i' && p[6] == 'n' && p[7] == 'g') {
            
            p += 8;
            Impl::Skipper<Impl::Space>::skip(p);
            if(*p != '=') throw Exception(p - s, "expected =");
            ++p;
            Impl::Skipper<Impl::Space>::skip(p);
            if(*p == '"') {
                
                ++p;
                Impl::Skipper<Impl::AttributeValue1>::skip(p);
                if(*p != '"') throw Exception(p - s, "expected \"");
    
            } else if(*p == '\'') {
                
                ++p;
                Impl::Skipper<Impl::AttributeValue2>::skip(p);
                if(*p != '\'') throw Exception(p - s, "expected '");
                
            } else throw Exception(p - s, "expected \" or '");
            ++p;
            
        }
        
        if(*p != '?' && !SequenceTable<MapperSequence<Impl::Space, IndexSequence<unsigned char, 0, 255>>>::get(*p))
            throw Exception(p - s, "unexpected character");
        Impl::Skipper<Impl::Space>::skip(p);
        
        // Parse "standalone"
        if(p[0] == 's' && p[1] == 't' && p[2] == 'a' && p[3] == 'n' && p[4] == 'd' && p[5] == 'a' && p[6] == 'l' && p[7] == 'o' && p[8] == 'n' && p[9] == 'e') {
            
            p += 10;
            Impl::Skipper<Impl::Space>::skip(p);
            if(*p != '=') throw Exception(p - s, "expected =");
            ++p;
            Impl::Skipper<Impl::Space>::skip(p);
            if(*p == '"') {
                
                ++p;
                Impl::Skipper<Impl::AttributeValue1>::skip(p);
                if(*p != '"') throw Exception(p - s, "expected \"");
    
            } else if(*p == '\'') {
                
                ++p;
                Impl::Skipper<Impl::AttributeValue2>::skip(p);
                if(*p != '\'') throw Exception(p - s, "expected '");
                
            } else throw Exception(p - s, "expected \" or '");
            ++p;
            
        }
        
        Impl::Skipper<Impl::Space>::skip(p);
        if(p[0] != '?' || p[1] != '>') throw Exception(p - s, "expected ?>");
        p += 2;
        
    }
    template <Flag F, typename H>
    void parseDoctype(H& /*handler*/) {
        
        throw Exception(p - s, "not implemented");
        
    }
    template <Flag F, typename H>
    void parseComment(H& handler) {
        
        StringView8 comment(p, 0);
        // Until "-->"
        while(*p && (p[0] != '-' || p[1] != '-' || p[2] != '>')) ++p;
        if(!*p) throw Exception(p - s, "unexpected end");
        comment.setLength(p - comment.getData());
        p += 3;
        handler.comment(comment);
        
    }
    template <Flag F, typename H>
    void parseProcessingInstruction(H& handler) {
        
        StringView8 target(p, 0);
        target.setLength(Impl::Skipper<Impl::Name>::skip(p));
        if(!target.getLength()) throw Exception(p - s, "expected PI target");
        if((p[0] != '?' || p[1] != '>') && !Impl::Skipper<Impl::Space>::skip(p))
            throw Exception(p - s, "expected space");
        
        StringView8 content(p, 0);
        // Until "?>"
        while(*p && (p[0] != '?' || p[1] != '>')) ++p;
        if(!*p) throw Exception(p - s, "unexpected end");
        content.setLength(p - content.getData());
        p += 2;
        
        handler.processingInstruction(target, content);
        
    }
    template <Flag F, typename H>
    void parseCDATA(H& handler) {
        
        StringView8 text(p, 0);
        // Until "]]>"
        while(*p && (p[0] != ']' || p[1] != ']' || p[2] != '>')) ++p;
        if(!*p) throw Exception(p - s, "unexpected end");
        text.setLength(p - text.getData());
        p += 3;
        handler.cdata(text);
        
    }
    template <Flag F, typename H>
    void parseElement(H& handler) {
        
        using namespace Corecat::Sequence;
        
        // Parse element type
        StringView8 name(p, 0);
        name.setLength(Impl::Skipper<Impl::Name>::skip(p));
        if(!name.getLength()) throw Exception(p - s, "expected element type");
        bool empty = false;
        if(*p == '>') {
            
            ++p;
            handler.startElement(name);
            
        } else if(*p == '/') {
            
            if(p[1] != '>') throw Exception(p + 1 - s, "expected >");
            p += 2;
            handler.startElement(name);
            empty = true;
            
        } else {
            
            ++p;
            handler.startElement(name);
            Impl::Skipper<Impl::Space>::skip(p);
            while(SequenceTable<MapperSequence<Impl::AttributeName, IndexSequence<unsigned char, 0, 255>>>::get(*p)) {
                
                // Parse attribute name
                StringView8 name(p, 0);
                name.setLength(Impl::Skipper<Impl::AttributeName>::skip(p));
                if(!name.getLength()) throw Exception(p - s, "expected attribute name");
                Impl::Skipper<Impl::Space>::skip(p);
                if(*p != '=') throw Exception(p - s, "expected =");
                ++p;
                Impl::Skipper<Impl::Space>::skip(p);
                
                // Parse attribute value
                StringView8 value;
                if(*p == '"') {
                    
                    ++p;
                    value.setData(p, 0);
                    if(F & Flag::EntityTranslation) {
                        
                        auto q = p;
                        while(true) {
                            
                            auto len = Impl::Skipper<Impl::AttributeValueNoRef1>::skip(p);
                            if(*p == 0) throw Exception(p - s, "unexpected end");
                            if(p != q + len) std::copy(q, q + len, p - len);
                            q += len;
                            if(*p == '&') parseReference<F>(q);
                            else break;
                            
                        }
                        value.setLength(q - value.getData());
                        
                    } else {
                        
                        value.setLength(Impl::Skipper<Impl::AttributeValue1>::skip(p));
                        if(*p == 0) throw Exception(p - s, "unexpected end");
                        
                    }
                    ++p;
                    
                } else if(*p == '\'') {
                    
                    ++p;
                    value.setData(p, 0);
                    if(F & Flag::EntityTranslation) {
                        
                        auto q = p;
                        while(true) {
                            
                            auto len = Impl::Skipper<Impl::AttributeValueNoRef2>::skip(p);
                            if(*p == 0) throw Exception(p - s, "unexpected end");
                            if(p != q + len) std::copy(q, q + len, p - len);
                            q += len;
                            if(*p == '&') parseReference<F>(q);
                            else break;
                            
                        }
                        value.setLength(q - value.getData());
                        
                    } else {
                        
                        value.setLength(Impl::Skipper<Impl::AttributeValue2>::skip(p));
                        if(*p == 0) throw Exception(p - s, "unexpected end");
                        
                    }
                    ++p;
                    
                } else throw Exception(p - s, "expected \" or '");
                handler.attribute(name, value);
                Impl::Skipper<Impl::Space>::skip(p);
                
            }
            if(*p == '>') {
                
                ++p;
                
            } else if(*p == '/') {
                
                if(p[1] != '>') throw Exception(p + 1 - s, "expected >");
                p += 2;
                empty = true;
                
            } else throw Exception(p + 1 - s, "unexpected character");
            
        }
        handler.endAttributes(empty);
        if(!empty) {
            
            bool c = true;
            do {
                
                // Parse text
                if(F & Flag::TrimSpace) Impl::Skipper<Impl::Space>::skip(p);
                if(*p != '<') {
                    
                    if(F & Flag::EntityTranslation) {
                        
                        if(F & Flag::NormalizeSpace) {
                            
                            StringView8 text(p, 0);
                            auto q = p;
                            while(true) {
                                
                                auto len = Impl::Skipper<Impl::TextNoSpaceRef>::skip(p);
                                if(*p == 0) throw Exception(p - s, "unexpected end");
                                if(p != q + len) std::copy(p - len, p, q);
                                q += len;
                                if(*p == '&') parseReference<F>(q);
                                else if(*p != '<') { Impl::Skipper<Impl::Space>::skip(p); *(q++) = ' '; }
                                else break;
                                
                            }
                            if(F & Flag::TrimSpace && q[-1] == ' ') --q;
                            text.setLength(q - text.getData());
                            handler.text(text);
                            
                        } else {
                            
                            StringView8 text(p, 0);
                            auto q = p;
                            while(true) {
                                
                                auto len = Impl::Skipper<Impl::TextNoRef>::skip(p);
                                if(*p == 0) throw Exception(p - s, "unexpected end");
                                if(p != q + len) std::copy(p - len, p, q);
                                q += len;
                                if(*p == '&') parseReference<F>(q);
                                else break;
                                
                            }
                            --q;
                            if(F & Flag::TrimSpace)
                                for(; SequenceTable<MapperSequence<Impl::Space, IndexSequence<unsigned char, 0, 255>>>::get(*q); --q);
                            ++q;
                            text.setLength(q - text.getData());
                            handler.text(text);
                            
                        }
                        
                    } else {
                        
                        if(F & Flag::NormalizeSpace) {
                            
                            StringView8 text(p, 0);
                            auto q = p;
                            while(true) {
                                
                                auto len = Impl::Skipper<Impl::TextNoSpace>::skip(p);
                                if(*p == 0) throw Exception(p - s, "unexpected end");
                                if(p != q + len) std::copy(p - len, p, q);
                                q += len;
                                if(*p != '<') { Impl::Skipper<Impl::Space>::skip(p); *(q++) = ' '; }
                                else break;
                                
                            }
                            --q;
                            if(F & Flag::TrimSpace)
                                for(; SequenceTable<MapperSequence<Impl::Space, IndexSequence<unsigned char, 0, 255>>>::get(*q); --q);
                            ++q;
                            text.setLength(q - text.getData());
                            handler.text(text);
                            
                        } else {
                            
                            StringView8 text(p, 0);
                            Impl::Skipper<Impl::Text>::skip(p);
                            if(*p == 0) throw Exception(p - s, "unexpected end");
                            auto q = p - 1;
                            if(F & Flag::TrimSpace)
                                for(; SequenceTable<MapperSequence<Impl::Space, IndexSequence<unsigned char, 0, 255>>>::get(*q); --q);
                            ++q;
                            text.setLength(q - text.getData());
                            handler.text(text);
                            
                        }
                        
                    }
                    
                }
                
                ++p;
                switch(*p) {
                    
                case '!': {
                    
                    ++p;
                    if(p[0] == '-' && p[1] == '-') {
                        
                        p += 2;
                        parseComment<F>(handler);
                        
                    } else if(p[0] == '[' && p[1] == 'C' && p[2] == 'D' && p[3] == 'A' && p[4] == 'T' && p[5] == 'A' && p[6] == '[') {
                        
                        // "[CDATA["
                        p += 7;
                        parseCDATA<F>(handler);
                        
                    } else throw Exception(p - s, "unexpected character");
                    break;
                    
                }
                case '/': {
                    
                    ++p;
                    if(F & Flag::ClosingTagValidate) {
                    
                        StringView8 endName(p, 0);
                        Impl::Skipper<Impl::Name>::skip(p);
                        endName.setLength(p - endName.getData());
                        Impl::Skipper<Impl::Space>::skip(p);
                        if(*p != '>') throw Exception(p - s, "expected >");
                        ++p;
                        handler.endElement(endName);
                        
                    } else {
                        
                        StringView8 endName(p, name.getLength());
                        if(endName != name) throw Exception(p - s, "unmatch element type");
                        p += name.getLength();
                        Impl::Skipper<Impl::Space>::skip(p);
                        if(*p != '>') throw Exception(p - s, "expected >");
                        ++p;
                        handler.endElement(endName);
                        
                    }
                    c = false;
                    break;
                    
                }
                case '?': {
                    
                    ++p;
                    parseProcessingInstruction<F>(handler);
                    break;
                    
                }
                default: {
                    
                    parseElement<F>(handler);
                    break;
                    
                }
                
                }
                
            } while(c);
            
        }
        
    }
    
public:
    
    XMLParser() = default;
    
    template <Flag F = Flag::Default, typename H>
    void parse(char* data, H& handler) {
        
        using namespace Corecat::Sequence;
        
        assert(data);
        
        s = data;
        p = data;
        handler.startDocument();
        
        // Parse BOM
        if(static_cast<unsigned char>(p[0]) == 0xEF &&
            static_cast<unsigned char>(p[1]) == 0xBB &&
            static_cast<unsigned char>(p[2]) == 0xBF) {
            
            p += 3;
            
        }
        
        // Parse XML declaration
        if(p[0] == '<' && p[1] == '?' && p[2] == 'x' && p[3] == 'm' && p[4] == 'l' && SequenceTable<MapperSequence<Impl::Space, IndexSequence<unsigned char, 0, 255>>>::get(p[5])) {
            
            // "<?xml "
            p += 6;
            parseXMLDeclaration<F>(handler);
            
        }
        while(true) {
            
            Impl::Skipper<Impl::Space>::skip(p);
            if(!*p) break;
            else if(*p == '<') {
                
                ++p;
                if(*p == '!') {
                    
                    ++p;
                    if(p[0] == '-' && p[1] == '-') {
                        
                        p += 2;
                        parseComment<F>(handler);
                        
                    } else if(p[0] == 'D' && p[1] == 'O' && p[2] == 'C' && p[3] == 'T' && p[4] == 'Y' && p[5] == 'P' && p[6] == 'E') {
                        
                        // "DOCTYPE"
                        p += 7;
                        parseDoctype<F>(handler);
                        
                    } else throw Exception(p - s, "unexpected character");
                    
                } else if(*p == '?') {
                    
                    ++p;
                    parseProcessingInstruction<F>(handler);
                    
                } else {
                    
                    parseElement<F>(handler);
                    
                }
                
            } else throw Exception(p - s, "expected <");
            
        }
        
        handler.endDocument();
        
    }
    
};

}
}
}


#endif
