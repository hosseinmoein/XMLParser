// Hossein Moein
// March 24, 2018
// Copyright (C) 2018-2019 Hossein Moein
// Distributed under the BSD Software License (see file License)

#ifndef _INCLUDED_XMLParser_h
#define _INCLUDED_XMLParser_h 0

// ----------------------------------------------------------------------------

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <stack>

#include <DMScu_PtrVector.h>

#include <XMLTreeNodes.h>

#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/util/PlatformUtils.hpp>

// ----------------------------------------------------------------------------

namespace hmxml
{

class   XMLParser : public XERCES_CPP_NAMESPACE::HandlerBase  {

    private:

        typedef XERCES_CPP_NAMESPACE::SAXParser         SAXParser;
        typedef XERCES_CPP_NAMESPACE::SAXParseException SAXParseException;
        typedef XERCES_CPP_NAMESPACE::AttributeList     AttributeList;

    public:

        typedef unsigned int    size_type;

        XMLParser (XMLTreeNodes &i_n,
                       XMLTreeNodes::attr_vector &attr_vector,
                       SAXParser::ValSchemes vs = SAXParser::Val_Never,
                       bool do_namespace = false);
        ~XMLParser () throw ();

    protected:

       // SAX DocumentHandler interface
       //
        void startElement (const XMLCh *const name,
                           AttributeList &attributes) throw ();
        void endElement (const XMLCh *const name);

       // SAX ErrorHandler interface
       //
        void warning (const SAXParseException &exception) throw ();
        void error (const SAXParseException &exception) throw ();
        void fatalError (const SAXParseException &exception) throw ();

    public:

        std::ostream &dumpForm (std::ostream &os = std::cout) const;

        typedef std::vector<std::string>    XmlErrorVector;

        inline const XMLTreeNodes &get_form () const throw ()  {

            return (initial_node_);
        }
        inline bool has_warning () const throw ()  {

            return (! warning_msgs_.empty ());
        }
        inline bool has_error () const throw ()  {

            return (! error_msgs_.empty ());
        }
        inline bool has_fatal_error () const throw ()  {

            return (has_problem_);
        }
        inline const XmlErrorVector &warnings () const throw ()  {

            return (warning_msgs_);
        }
        inline const XmlErrorVector &errors () const throw ()  {

            return (error_msgs_);
        }
        inline const std::string fatal_error () const throw ()  {

            return (fatal_error_);
        }

    private:

        std::stack<XMLTreeNodes *,
                   std::vector<XMLTreeNodes *> >    astack_;

        bool                            has_problem_;
        bool                            started_;

        XMLTreeNodes                *just_closed_element_;
        XMLTreeNodes                *just_opened_element_;
        XMLTreeNodes                &initial_node_;
        XMLTreeNodes::attr_vector   &attr_vector_;

        XmlErrorVector                  warning_msgs_;
        XmlErrorVector                  error_msgs_;
        std::string                     fatal_error_;

    public:

        bool parse_string (const char *const xml,
                           size_type xml_len,
                           const char *const sys_id);
        inline bool parse_string (const char *const xml, size_type xml_len)  {

            return (parse_string (xml, xml_len, "default"));
        }
        bool parse_file (const char *const file);

    private:

       // Creating and destroying SAXParser objects on the stack
       // has proven expensive, therefore we cache a bunch of them here.
       //
        class   ParserStrap  {

            public:

                inline ParserStrap (SAXParser *p, bool b) throw ()
                    : parser (p), busy (b)  {   }
                inline ~ParserStrap () throw ()  { delete parser; }

                inline bool
                operator == (const ParserStrap &rhs) const throw ()  {

                    return (parser == rhs.parser);
                }

                SAXParser   *parser;
                bool        busy;

            private:

               // Not implemented
               //
                ParserStrap ();
        };

       // this is a std::vector<ParserStrap *> that takes ownership of
       // the pointers it contains.
       //
        typedef DMScu_PtrVector<ParserStrap>    ParserVector;

        static  ParserVector    parser_cache_;

       // If there is a SAX parser available, return it. Otherwise
       // create a new sax parser object and add it to the cache.
       //
        inline static ParserStrap &get_available_parser_ () throw ();

       // The parser inside this strap will be used by a particular instance
       // of XMLParser object.
       //
        ParserStrap   &my_parser_strap_;

       // Initializes the static SAX parser stuff
       //
        class   PP_Initializer  {

            public:

                PP_Initializer ();
                ~PP_Initializer ();
        };

        static  const   PP_Initializer  pp_initializer_;
};

} // namespace hmxml

// ----------------------------------------------------------------------------

#undef _INCLUDED_XMLParser_h
#define _INCLUDED_XMLParser_h 1
#endif    // _INCLUDED_XMLParser_h

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
