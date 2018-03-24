// Hossein Moein
// March 24, 2018
// Copyright (C) 2018-2019 Hossein Moein
// Distributed under the BSD Software License (see file License)

#include <cstdio>
#include <assert.h>

#include <xercesc/sax/AttributeList.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>

#include <DMScu_FixedSizeString.h>

#include <XMLParser.h>
#include <XMLString.h>

// ----------------------------------------------------------------------------

namespace hmxml
{

const   XMLParser::PP_Initializer   XMLParser::pp_initializer_;
XMLParser::ParserVector             XMLParser::parser_cache_;

// ----------------------------------------------------------------------------

// static intialization of SAX parser
//
XMLParser::PP_Initializer::PP_Initializer ()  {

    try  {
        XERCES_CPP_NAMESPACE::XMLPlatformUtils::Initialize ();
    }
    catch (const XERCES_CPP_NAMESPACE::XMLException &ex)  {
        DMScu_FixedSizeString<1023> err;

        err.printf ("XMLParser::PP_Initializer::PP_Initializer(): "
                    "ERROR during XML utilities initialization. "
                    "Message: '%s'\n",
                    XMLString::to_stdstring (ex.getMessage ()).c_str ());

        throw std::runtime_error (err.c_str ());
    }
}

// ----------------------------------------------------------------------------

XMLParser::PP_Initializer::~PP_Initializer ()  {

    XERCES_CPP_NAMESPACE::XMLPlatformUtils::Terminate ();
}

// ----------------------------------------------------------------------------

// Class static
//
inline XMLParser::ParserStrap &XMLParser::
get_available_parser_ () throw ()  {

    for (ParserVector::iterator itr = parser_cache_.begin ();
         itr != parser_cache_.end (); ++itr)
        if (! (*itr)->busy)  {
            (*itr)->busy = true;
            return (**itr);
        }

    ParserStrap *ps = new ParserStrap (new SAXParser, true);

    parser_cache_.push_back (ps);

    return (*ps);
}

// ----------------------------------------------------------------------------

// Release the sax parser, so it can be reused by another XMLParser.
//
XMLParser::~XMLParser () throw ()  {

    my_parser_strap_.busy = false;
}

// ----------------------------------------------------------------------------

XMLParser::XMLParser (XMLTreeNodes &i_n,
                              XMLTreeNodes::attr_vector &attr_vector,
                              SAXParser::ValSchemes vs,
                              bool do_namespace)
    : has_problem_ (false),
      just_opened_element_ (NULL),
      just_closed_element_ (NULL),
      started_ (false),
      initial_node_ (i_n),
      attr_vector_ (attr_vector),
      my_parser_strap_ (get_available_parser_ ())  {

    try  {
        my_parser_strap_.parser->setValidationScheme (vs);
        my_parser_strap_.parser->setDoNamespaces (do_namespace);
        my_parser_strap_.parser->setDocumentHandler (this);
        my_parser_strap_.parser->setErrorHandler (this);
    }
    catch (const XERCES_CPP_NAMESPACE::SAXException &ex)  {
        DMScu_FixedSizeString<1023> err;

        err.printf ("XMLParser::XMLParser(): "
                    "ERROR during SAX Parser initialization. "
                    "Message: '%s'\n",
                    XMLString::to_stdstring (ex.getMessage ()).c_str ());

        throw std::runtime_error (err.c_str ());
    }
}

// ----------------------------------------------------------------------------

#ifdef XMLSPEED_IS_NO_ISSUE
template <class xml_TYPE>
class   xml_crude_auto_array_ptr  {

    public:

        inline xml_crude_auto_array_ptr () throw () : ptr (NULL)  {   }
        inline ~xml_crude_auto_array_ptr () throw ()   { delete[] ptr; }

        xml_TYPE    *ptr;
};
#endif // XMLSPEED_IS_NO_ISSUE

// ----------------------------------------------------------------------------

void XMLParser::
startElement (const XMLCh *const name, AttributeList &attr) throw ()  {

//    std::cout << "--> XMLParser::startElement for "
//              << XMLString::to_stdstring (name)
//              << " -->" << std::endl;

    const   size_type   attr_size = attr.getLength ();
    XMLTreeNodes    *pt_ptr = NULL;

    if (! started_)  {
        started_ = true;
        initial_node_.set_name (name);
        initial_node_.set_attr_size (attr_size);
        pt_ptr = &initial_node_;
    }
    else
        pt_ptr = new XMLTreeNodes (name, attr_size, attr_vector_);

   // Set all the attributes for this node.
   //
    for (size_type idx = 0; idx < attr_size; ++idx)
        pt_ptr->add_attr (attr.getName (idx), attr.getValue (idx));

    //
    // At this point there are only 3 possible events that may have
    // happened prior to the call to this method:
    //
    // 1) This is the very first node of an XML statement.
    // 2) An element has just been opened.
    // 3) An element was just closed.
    //

   // If there is an element that is still open, then we are a child
   // of that element.
   //
    if (just_opened_element_)
        just_opened_element_->set_child (pt_ptr);

   // Otherwise, if we just closed an element, then we are a sibling of
   // that element.
   //
    else if (just_closed_element_)
        just_closed_element_->set_sibling (pt_ptr);

   // The else part means this is the very first element.
   //
    // else {   }

    astack_.push (pt_ptr);

    just_opened_element_ = pt_ptr;
    just_closed_element_ = NULL;

    return;
}

// ----------------------------------------------------------------------------

void XMLParser::endElement (const XMLCh *const name)  {

//    std::cout << "--> XMLParser::endElement for "
//                  << XMLString::to_stdstring(name);

    if (astack_.empty ())  {
        DMScu_FixedSizeString<1023> err;

        err.printf ("XMLParser::endElement(): "
                    "ERROR during XML utilities initialization. "
                    "Empty stack encountered when it shouldn't be empty.");

        throw std::runtime_error (err.c_str ());
    }

    //
    // Refer to STL documentation for std:stack, for an explanation of, why
    // we do a top() and pop() in two stages.
    //

    XMLTreeNodes    *pt_ptr = astack_.top ();

    //
    // NOTE: In the name of speed, we are going to comment out the following
    //       block of code. Damn the XMLString::to_charstar().
    //

#ifdef XMLSPEED_IS_NO_ISSUE
   // If we have an inconsistent stack, we are screwed and there is nothing
   // we can do to recover.
   //
    xml_crude_auto_array_ptr<const char>  nar_name;

    XMLString::to_charstar (nar_name.ptr, name);
    if (::strcmp (pt_ptr->get_name (), nar_name.ptr))  {
        DMScu_FixedSizeString<1023> err;

        err.printf ("XMLParser::endElement(): "
                    "Unbalanced stack encountered.\n%s != %s",
            I       pt_ptr->get_name (),
                    XMLString::to_stdstring (name).c_str ());

        throw std::runtime_error (err.c_str ());
    }
#endif // XMLSPEED_IS_NO_ISSUE

    astack_.pop ();  // Don't forget to pop()

    just_closed_element_ = pt_ptr;
    just_opened_element_ = NULL;

    return;
}

// ----------------------------------------------------------------------------

void XMLParser::warning (const SAXParseException &e) throw ()  {

    DMScu_FixedSizeString<1023> err;

    err.printf ("WARNING: (System ID: %s) -- line: %d, char: %d\n"
                "         Message: '%s'",
                XMLString::to_stdstring (e.getSystemId ()).c_str (),
                e.getLineNumber (),
                e.getColumnNumber (),
                XMLString::to_stdstring (e.getMessage ()).c_str ());

    warning_msgs_.push_back (err.c_str ());
    return;
}

// ----------------------------------------------------------------------------

void XMLParser::error (const SAXParseException &e) throw ()  {

    has_problem_ = true;

    DMScu_FixedSizeString<1023> err;

    err.printf ("WARNING: (System ID: %s) -- line: %d, char: %d\n"
                "         Message: '%s'",
                XMLString::to_stdstring (e.getSystemId ()).c_str (),
                e.getLineNumber (),
                e.getColumnNumber (),
                XMLString::to_stdstring (e.getMessage ()).c_str ());

    error_msgs_.push_back (err.c_str ());
    return;
}

// ----------------------------------------------------------------------------

void XMLParser::fatalError (const SAXParseException &e) throw ()  {

    has_problem_ = true;

    DMScu_FixedSizeString<1023> err;

    err.printf ("FATAL ERROR: (System ID: %s) -- line: %d, char: %d\n"
                "         Message: '%s'",
                XMLString::to_stdstring (e.getSystemId ()).c_str (),
                e.getLineNumber (),
                e.getColumnNumber (),
                XMLString::to_stdstring (e.getMessage ()).c_str ());

    fatal_error_ = err.c_str ();
    return;
}

// ----------------------------------------------------------------------------

bool XMLParser::parse_string (const char *const xml,
                                  size_type xml_len,
                                  const char *const sys_id)  {

    typedef XERCES_CPP_NAMESPACE::MemBufInputSource XmlBuffer;

    const   XmlBuffer   mem_buf (reinterpret_cast<const XMLByte *const>(xml),
                                 xml_len,
                                 sys_id,
                                 false);  // Don't adopt the input buffer

    try   {
        my_parser_strap_.parser->parse (mem_buf);
        // my_parser_strap_.parser->parse (xml);
    }
    catch (const XERCES_CPP_NAMESPACE::SAXException &ex)  {
        DMScu_FixedSizeString<1023> err;

        err.printf ("XMLParser::parse_string(): SAX exception thrown. "
                    "Message: '%s'\n",
                    XMLString::to_stdstring (ex.getMessage ()).c_str ());

        has_problem_ = true;
        throw std::runtime_error (err.c_str ());
    }
    catch (const std::exception &ex)  {
        has_problem_ = true;
        throw;
    }
    catch (...)  {
        DMScu_FixedSizeString<1023> err;

        err.printf ("XMLParser::parse_string(): An unknown exception was "
                    "thrown during parsing.\n"
                    "No further information is available.");

        has_problem_ = true;
        throw std::runtime_error (err.c_str ());
    }

    return (! has_problem_);
}

// ----------------------------------------------------------------------------

bool XMLParser::parse_file (const char *const filename)  {

    try  {
        my_parser_strap_.parser->parse (filename);
    }
    catch (const XERCES_CPP_NAMESPACE::SAXException &ex)  {
        DMScu_FixedSizeString<1023> err;

        err.printf ("XMLParser::parse_file(): SAX exception thrown. "
                    "Message: '%s'\n",
                    XMLString::to_stdstring (ex.getMessage ()).c_str ());

        has_problem_ = true;
        throw std::runtime_error (err.c_str ());
    }
    catch (const std::exception &ex)  {
        has_problem_ = true;
        throw;
    }
    catch (...)  {
        DMScu_FixedSizeString<1023> err;

        err.printf ("XMLParser::parse_file(): An unknown exception was "
                    "thrown during parsing.\n"
                    "No further information is available.");

        has_problem_ = true;
        throw std::runtime_error (err.c_str ());
    }

    return (! has_problem_);
}

// ----------------------------------------------------------------------------

std::ostream &XMLParser::dumpForm (std::ostream &os) const  {

    return (initial_node_.dump_xml (os) << std::endl);
}

} // namespace hmxml

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
