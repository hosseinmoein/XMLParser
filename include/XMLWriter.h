// Hossein Moein
// March 24, 2018
// Copyright (C) 2018-2019 Hossein Moein
// Distributed under the BSD Software License (see file License)

#ifndef _INCLUDED_XMLWriter_h
#define _INCLUDED_XMLWriter_h 0

// ----------------------------------------------------------------------------

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

// ----------------------------------------------------------------------------

namespace hmxml
{

class   XMLWriter  {

    public:

        typedef unsigned int    size_type;

        inline XMLWriter () throw ()  { needs_indent = true; }

        void write_open_tag (const char *name);
        void write_close_tag ();

        void write_attribute (const char *name, const char *value,
                              bool encode_content = true);
        void write_content (const char *content);
        void write_cdata (const char *cdata);
        void write_comment (const char *const comment);

        static bool decode_comment (const std::string &encoded_comment,
                                    std::string &decoded_comment);

        static void encode_literal (const char *const input,
                                    std::string &output) throw ();

        bool    needs_indent;

    protected:

        virtual void write (const char *str) = 0;

        class   Element  {

            public:

                Element (XMLWriter *writer, const char *name) throw ()
                    : writer_ (writer),
                      name_ (name),
                      state_ (es_initial)  {   }
                Element () throw ()
                    : writer_ (NULL), name_ (""), state_ (es_initial)  {   }


                void write_open_tag ();
                void write_close_tag ();
                void write_attribute (const char *name, const char *value,
                                      bool encode_content);
                void write_content (const char *content);
                void write_cdata (const char *cdata);

                void notify_child_inserted ();

                inline void write (const char *s)  { writer_->write (s); }

                inline void indent ()  { writer_->indent (); }
                inline void indent (size_type indent_level)  {

                    writer_->indent (indent_level);
                    return;
                }

            private:

                enum State  { es_initial, es_tagSet, es_contentSet,
                              es_closed };

                inline Element *get_parent_element ()  {

                     return (writer_->get_current_element ());
                }

                std::string     name_;
                XMLWriter   *writer_;
                State           state_;

            public:

                Element (const Element &that) throw ()
                    : writer_ (NULL), name_ (""), state_ (es_initial)  {
                    (*this) = that;
                }
                Element *operator = (const Element &rhs) throw ()  {

                    if (this != &rhs)  {
                        name_ = rhs.name_;
                        writer_ = rhs.writer_;
                        state_ = rhs.state_;
                    }

                    return (this);
                }

        };

        void indent ();
        void indent (size_type indent_level);

        inline size_type depth () const throw ()  { return (stack_.size ()); }

        std::vector<Element>    stack_;

        inline Element *get_current_element () throw ()  {

            return (stack_.empty () ? NULL : &(stack_.back ()));
        }

    friend class Element;
};

// ----------------------------------------------------------------------------

template <class xml_STREAM>
class   XMLStreamWriter : public XMLWriter  {

    public:

        inline XMLStreamWriter (xml_STREAM &str) throw ()
            : stream_like_obj_ (str)  {   }

    protected:

        inline void write (const char *str)  {

            stream_like_obj_ << str;
            return;
        }

    private:

        xml_STREAM  &stream_like_obj_;

       // These are prohibited
       //
        XMLStreamWriter ();
        XMLStreamWriter (const XMLStreamWriter &that);
        XMLStreamWriter &operator = (const XMLStreamWriter &rhs);
};

} // namespace hmxml

// ----------------------------------------------------------------------------

#undef _INCLUDED_XMLWriter_h
#define _INCLUDED_XMLWriter_h 1
#endif    // _INCLUDED_XMLWriter_h

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
