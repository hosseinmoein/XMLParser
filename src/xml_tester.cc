// Hossein Moein
// March 24, 2018
// Copyright (C) 2018-2019 Hossein Moein
// Distributed under the BSD Software License (see file License)

#include <XMLParser.h>
#include <XMLWriter.h>
#include <XMLString.h>

using namespace hmxml;

// ---------------------------------------------------------------------------

void usage ()  {

    std::cout << "\nUsage:\n"
                 "    xml_test [options] <XML file>\n\n"
                 "Options:\n"
                 "    -v=xxx      Validation scheme [always | never | auto*]\n"
                 "    -n          Enable namespace processing. "
                 "Defaults to off.\n\n"
                 "This program prints the number of elements, attributes,\n"
                 "white spaces and other non-white space characters in the "
                 "input file.\n\n"
                 "  * = Default if not provided explicitly\n"
              << std::endl;
};

// ---------------------------------------------------------------------------

class   ptxml_test_eq_name
    : public std::unary_function <const XMLTreeNodes, bool>  {

    public :

        ptxml_test_eq_name ()  {   }

        bool operator () (const XMLTreeNodes &node) const  {

            return (! ::strcmp (node.get_name (), "DMS_DATA_REQUEST"));
        }
};

// ---------------------------------------------------------------------------

int main (int argC, char* argV[])  {


    XMLStreamWriter<std::ostream>   pw (std::cout);

    pw.write_open_tag ("First_name");
    pw.write_open_tag ("Second_name");
    pw.write_open_tag ("Third_name");
    pw.write_open_tag ("Forth_name");
    pw.write_close_tag ();
    pw.write_close_tag ();
    pw.write_close_tag ();
    pw.write_close_tag ();

    std::cout << std::endl << std::endl << std::endl;

   // Check command line and extract arguments.
   //
    if (argC < 2)  {
        usage ();
        return (EXIT_FAILURE);
    }

    const   char                                *xmlFile = NULL;
    XERCES_CPP_NAMESPACE::SAXParser::ValSchemes valScheme =
        XERCES_CPP_NAMESPACE::SAXParser::Val_Auto;
    bool                                        doNamespaces = false;

    // See if non validating dom parser configuration is requested.
    //
    if ((argC == 2) && ! ::strcmp (argV [1], "-?"))  {
        usage ();
        return (2);
    }

    int argInd;
    for (argInd = 1; argInd < argC; argInd++)  {
       // Break out on first non-dash parameter
       //
        if (argV [argInd] [0] != '-')
            break;

        if (! ::strncmp (argV [argInd], "-v=", 3) ||
            ! ::strncmp (argV [argInd], "-V=", 3))  {
            const   char    *const  parm = &argV[argInd][3];

            if (! ::strcmp (parm, "never"))
                valScheme = XERCES_CPP_NAMESPACE::SAXParser::Val_Never;
            else if (! ::strcmp (parm, "auto"))
                valScheme = XERCES_CPP_NAMESPACE::SAXParser::Val_Auto;
            else if (! ::strcmp (parm, "always"))
                valScheme = XERCES_CPP_NAMESPACE::SAXParser::Val_Always;
            else  {
                std::cerr << "Unknown -v= value: " << parm << std::endl;
                return (2);
            }
        }
        else if (! ::strcmp (argV [argInd], "-n") ||
                 ! ::strcmp (argV [argInd], "-N"))  {
            doNamespaces = true;
        }
        else
            std::cerr << "Unknown option '"
                      << argV[argInd]
                      << "', ignoring it\n"
                      << std::endl;
    }

   //
   //  There should be only one and only one parameter left, and that
   //  should be the file name.
   //
    if (argInd != argC - 1)  {
        usage ();
        return (1);
    }
    xmlFile = argV[argInd];

    for (int i = 0; i < 1; ++i)  {
        try  {
            XMLTreeNodes::attr_vector   attr_vector;
            bool                            error = false;

            attr_vector.reserve (8192);

            XMLTreeNodes    pn (attr_vector);
            XMLParser       parser (pn, attr_vector, valScheme,
                                        doNamespaces);

            parser.parse_file (xmlFile);

            if (parser.has_warning ())  {
                std::cout << "WARNINGS:" << std::endl;

                const   std::vector<std::string>    &vs = parser.warnings ();

                for (int i = 0; i < vs.size (); ++i)
                    std::cout << "\t" << vs [i].c_str () << std::endl;
            }
            if (parser.has_error ())  {
                std::cout << "ERRORS:" << std::endl;

                const   std::vector<std::string>    &vs = parser.errors ();

                for (int i = 0; i < vs.size (); ++i)
                    std::cout << "\t" << vs [i].c_str () << std::endl;
                error = true;
            }
            if (parser.has_fatal_error ())  {
                std::cout << "FATAL ERROR:\n"
                          << parser.fatal_error ().c_str () << std::endl;
                error = true;
            }

            if (i == 0 && ! error)  {

               // Testing the dump_xml method.
               //
                std::cout << pn << std::endl;

                std::cout << "Now testing dump via a string\n" << std::endl;

                std::string    str;

                std::cout << pn.dump_xml (str) << std::endl;

               // Testing the iterators.
               //
                for (XMLTreeNodes::const_iterator itr = pn.child_begin ();
                     itr != pn.child_sibling_end (); ++itr)
                    std::cout << "Name: " << itr->get_name () << std::endl;

                std::cout << std::endl << std::endl;

                if (pn.get_child ())  {
                    for (XMLTreeNodes::const_iterator itr =
                             pn.get_child ()->sibling_begin ();
                         itr != pn.get_child ()->child_sibling_end (); ++itr)
                        std::cout << "Name: " << itr->get_name () << std::endl;

                    std::cout << std::endl << std::endl;
                }

                XMLhildrenVector    cld_vec;

                XMLget_children_if (pn, ptxml_test_eq_name (), cld_vec);
                for (XMLhildrenVector::const_iterator itr =
                         cld_vec.begin ();
                     itr != cld_vec.end (); ++itr)
                    std::cout << "Name: " << (*itr)->get_name () << std::endl;

                std::cout << std::endl << std::endl;
            }

           // Measure the performance of the XMLTreeNodes destructor
           //
	}
        catch (const XERCES_CPP_NAMESPACE::XMLException& e)  {
            std::cerr << "\nError during parsing: '"
                      << xmlFile
                      << "'\n"
                      << "Exception message is:  \n"
                      << XMLString::to_stdstring (e.getMessage()) << "\n"
                      << std::endl;
        }
        catch (const std::exception &e)  {
            std::cerr << "\nError during parsing: '"
                      << xmlFile
                      << "'\n"
                      << "Exception message is:\n"
                      << e.what ()
                      << std::endl;
        }
        catch (...)  {
            std::cerr << "\nUnexpected exception during parsing: '"
                      << xmlFile
                      << "'\n";
        }

    }

    return (EXIT_SUCCESS);
}

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
