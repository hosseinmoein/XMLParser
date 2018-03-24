// Hossein Moein
// March 24, 2018
// Copyright (C) 2018-2019 Hossein Moein
// Distributed under the BSD Software License (see file License)

#include <cstdio>
#include <string.h>
#include <assert.h>
#include <stdexcept>
#include <sstream>

#include <XMLWriter.h>

// ----------------------------------------------------------------------------

namespace hmxml
{

void XMLWriter::Element::write_open_tag ()  {

    if (state_ == es_initial)  {
        state_ = es_tagSet;

        XMLWriter::Element  *parent = get_parent_element ();

        if (NULL != parent)  {
            if (parent->state_ == es_tagSet)
                write (">");
            indent ();
        }

        write ("<");
        write (name_.c_str ());
    }
    else
        throw std::runtime_error ("XMLWriter::Element::write_open_tag(): "
                                  "Cannot write open tag; element state ");

    return;
}

// ----------------------------------------------------------------------------

void XMLWriter::Element::write_close_tag ()  {

    if (state_ == es_tagSet)  {
        state_ = es_closed;
        write ("/>");
    }
    else if (state_ == es_contentSet)  {
        state_ = es_closed;

        const   XMLWriter::size_type  indentation_level = writer_->depth();

        assert (indentation_level > 0);
        indent (indentation_level - 1);
        write ("</");
        write (name_.c_str ());
        write (">");
    }
    else
        throw std::runtime_error ("XMLWriter::Element::write_close_tag(): "
                                  "Cannot close open tag; element state ");

    return;
}

// ----------------------------------------------------------------------------

void XMLWriter::Element::
write_attribute(const char *name, const char *the_value, bool encode_content) {

    if (state_ == es_tagSet)  {
        write (" ");
        write (name);
        write ("=\"");

        if (encode_content)  {
            std::string value_as_literal;

            encode_literal (the_value, value_as_literal);
            write (value_as_literal.c_str ());
        }
        else
            write (the_value);

        write ("\"");
    }
    else
        throw std::runtime_error ("XMLWriter::Element::write_attribute(): "
                                   "Cannot write attribute; element state ");

    return;
}

// ----------------------------------------------------------------------------

void XMLWriter::Element::write_content (const char *content)  {

    if (state_ == es_tagSet)  {
        state_ = es_contentSet;
        write (content);
    }
    else
        throw std::runtime_error ("XMLWriter::Element::write_content(): "
                                  "Cannot write content; element state ");

    return;
}

// ----------------------------------------------------------------------------

void XMLWriter::Element::write_cdata (const char *cdata)  {

    if (state_ == es_tagSet)  {
        state_ = es_contentSet;
        write ("<");
        write (cdata);
        write (">");
    }
    else
        throw std::runtime_error ("XMLWriter::Element::write_cdata(): "
                                  "Cannot write cdata; element state ");

    return;
}

// ----------------------------------------------------------------------------

void XMLWriter::Element::notify_child_inserted () {

    if ((state_ == es_tagSet) || (state_ == es_contentSet))  {
        state_ = es_contentSet;
        return;
    }
    else
        throw std::runtime_error ("XMLWriter::Element::"
                                  "notify_child_inserted(): "
                                  "Cannot notify child; element state ");

    return;
}

// ----------------------------------------------------------------------------

// class-static
//
void XMLWriter::
encode_literal (const char *const input, std::string &output) throw ()  {

    // In theory, this is what's supposed to happen: I take "the string", which
    // is assumed to be a Unicode string -- some version of Unicode, dunno
    // which -- encoded as UTF-8.  Then we read it in, Unicode character by
    // Unicode character, and generate an output string that is suitable for
    // usage as an XML string literal.  That is, the following characters are
    // converted to their respective escape sequences:
    //
    //            MANDATORY FOR ALL XML
    //                '<' --> "&lt;"
    //                '&' --> "&amp;"
    //            MANDATORY FOR STRING LITERALS
    //                '>' --> "&gt;"
    //                '\' --> "&apos;"
    //                '\"' --> "&quot;"
    //            ALSO...
    //                Go further and escape all characters that are
    //                nonprintable in the computer's native character set.  For
    //                example, I'd disallow character values from 0 (ASCII NUL)
    //                to ASCII 31 (ASCII US) and 127 (ASCII DEL).
    //                ASCII TAB (a/k/a HT) --> "&#9;"
    //                ASCII CR --> "&#13;"
    //                ASCII LF --> "&#12;"
    //            ALSO...
    //                I'd consider doing this one too: [but I don't ]
    //                ";" --> "&#59;"
    //
    // INPUT: an XML string (that is, the XML subset of the Unicode character
    //        set) -- encoded as UTF-8.
    //
    // OUTPUT: an XML string (that is, the XML subset of the Unicode character
    //         set) -- encoded as a XMLString, whatever _that_ encoding
    //         that happens to imply.
    //
    // The thing is, this routine really isn't doing it right at the moment.
    // In particular, we have no real "UTF-8 <--> UTF-16" converter at present.
    // We'll cross that bridge when we come to it.  Right now the best I can do
    // is to read in the input string as a plain old byte stream and examine it
    // byte by byte; in other words, I'm assuming a trivial UTF-8 subset which
    // never escapes into multicharacter representations.  I assume that the
    // output string is the same way.  I'm also assuming that the OS uses some
    // superset of ASCII for its characters.

    std::string out_stream;

    out_stream.reserve (strlen (input));
    for (const char *iter = input; *iter != 0; ++iter)  {
        typedef unsigned char   unsigned_char;

        const   int the_char_as_int = unsigned_char (*iter);

        if (! ::isprint (the_char_as_int))  {
            char   buffer [16];

            ::sprintf (buffer, "%d", the_char_as_int);
            out_stream += "&#";
            out_stream += buffer;
            out_stream += ';';
        }
        else  {
            switch (*iter)  {
                case '<':
                    out_stream += "&lt;";
                    break;

                case '&':
                    out_stream += "&amp;";
                    break;

                case '>':
                    out_stream += "&gt;";
                    break;

                case '\'':
                    out_stream += "&apos;";
                    break;

                case '\"':
                    out_stream += "&quot;";
                    break;

               // implementation detail -- gotta do this because the internal
               // stream deals with the NULL character in a peculiar way.
               // Sorry.
               //
                case '\0':
                    out_stream += "&#0;";
                    break;

                default:
                    out_stream += *iter;
                    break;
            }
        }
    }

    output = out_stream;
    return;
}

// ----------------------------------------------------------------------------

void XMLWriter::write_open_tag (const char *name)  {

    XMLWriter::Element  *parent = get_current_element ();
    XMLWriter::Element  e (this, name);

    e.write_open_tag ();

    if (parent != NULL)
        parent->notify_child_inserted ();

    stack_.push_back (e);
    return;
}

// ----------------------------------------------------------------------------

void XMLWriter::write_close_tag ()  {

    XMLWriter::Element  *e = get_current_element ();

    if (NULL == e)
        throw std::runtime_error ("XMLWriter::write_close_tag(): "
                                  "Cannot write close tag: no current tag");

    e->write_close_tag ();

    stack_.pop_back ();
    return;
}

// ----------------------------------------------------------------------------

void XMLWriter::
write_attribute (const char *name, const char *value, bool encode_content) {

    XMLWriter::Element  *e = get_current_element ();

    if (e == NULL)
        throw std::runtime_error ("XMLWriter::write_attribute(): "
                                  "Cannot write attribute: no current tag");

    e->write_attribute (name, value, encode_content);
    return;
}

// ----------------------------------------------------------------------------

void XMLWriter::write_content (const char *content)  {

    XMLWriter::Element  *e = get_current_element ();

    if (e == NULL)
        throw std::runtime_error ("XMLWriter::write_content(): "
                                  "Cannot write content: no current tag");

    e->write_content (content);
    return;
}

// ----------------------------------------------------------------------------

void XMLWriter::write_cdata (const char *cdata)  {

    XMLWriter::Element  *e = get_current_element ();

    if (e == NULL)
        throw std::runtime_error ("XMLWriter::write_cdata(): "
                                  "Cannot write cdata: no current tag");

    e->write_cdata (cdata);
    return;
}

// ----------------------------------------------------------------------------

void XMLWriter::write_comment (const char *const comment)  {

    XMLWriter::Element  *const  e = get_current_element ();

    // Honestly, I'm not sure where one can and cannot write a comment in XML.
    // The XML 1.0 standard ("Extensible Markup Language (XML) 1.0 (Second
    // Edition)") says
    //
    //     Comments may appear anywhere in a document outside other
    //     markup; in addition, they may appear within the document
    //     type declaration at places allowed by the grammar.
    //
    // In XML 1.0, a comment can contain any Chars (that is, "any Unicode
    // character, excluding the surrogate blocks, FFFE, and FFFF") with the
    // constraints that (1) the character sequence "--" must not appear and (2)
    // there must not be a trailing hyphen.  I believe that this also implies
    // that you cannot have an empty comment.  See URL
    // "http://www.w3.org/TR/REC-xml#sec-comments" for more information.
    //

    // NOTE: as of this writing, this thing isn't null-character clean.

    // Let's assume that we're using a subset of Unicode that is representable,
    // without escape sequences, by CHAR_BIT-bit bytes in whatever character
    // set is locally associated with "std::string::value_type".  (Also: no
    // null characters!  We're not set up for that just yet.)  Then, if we were
    // to encode comments, we must provide escape sequences to encode "--", a
    // trailing hyphen, and of course the escape code itself.
    //
    //     empty-comment-indicator: '\\' '$'
    //     double-hyphen-indicator: '\\' '2'
    //     trailing-hyphen-indicator: '\\' '_'
    //     backslash-indicator: '\\' '\\'

    static  const   char    escape_code = '\\';
    static  const   char    empty_comment_indicator = '$';
    static  const   char    single_hyphen_indicator = '-';
    static  const   char    double_hyphen_indicator = '2';
    static  const   char    trailing_hyphen_indicator = '_';

    std::ostringstream      encoded_strm;

    encoded_strm << "<!--";

    if (*comment == 0)  {
        encoded_strm << escape_code << empty_comment_indicator;
    }
    else  {
        enum State { NO_HYPHEN_YET, ENCOUNTERED_A_HYPHEN }

        state = NO_HYPHEN_YET;

        for (const char *iter = comment; *iter != 0; ++iter)  {
            switch (state)  {
                case NO_HYPHEN_YET:
                    switch (*iter)  {
                        case escape_code:
                            encoded_strm << escape_code << escape_code;
                            break;

                        case '-':
                            state = ENCOUNTERED_A_HYPHEN;
                            break;

                        default:
                            encoded_strm << *iter;
                            break;
                    }
                    break;

                case ENCOUNTERED_A_HYPHEN:
                    switch (*iter)  {
                        case '-':
                            encoded_strm << escape_code
                                         << double_hyphen_indicator;
                            break;

                        case escape_code:
                            encoded_strm << single_hyphen_indicator
                                         << escape_code
                                         << escape_code;
                            break;

                        default:
                            encoded_strm << single_hyphen_indicator << *iter;
                            break;
                    }
                    state = NO_HYPHEN_YET;
                    break;

                default:
                    std::ostringstream  ex_stream;

                    ex_stream << "XMLWriter::write_comment: "
                              << "Invalid stream state (integer value "
                              << state
                              << ") encountered."
                              << std::ends;

                    throw std::logic_error (ex_stream.str ());
                    break;
            }
        }

        if (state == ENCOUNTERED_A_HYPHEN)
            encoded_strm << escape_code << trailing_hyphen_indicator;
    }

    encoded_strm << "-->" << std::ends;

    indent ();
    write (encoded_strm.str ().c_str ());
    return;
}

// ----------------------------------------------------------------------------

// This routine takes a comment encoded in the manner of the
// "XMLWriter::encode_comment" member function.  Note that the leading
// "<!--" and trailing "-->" are _not_ considered to be part of the comment.

// class-static
bool XMLWriter::decode_comment (const std::string &encoded_comment,
                                    std::string &decoded_comment)  {

    // empty-comment-indicator: '\\' '$'
    // double-hyphen-indicator: '\\' '2'
    // trailing-hyphen-indicator: '\\' '_'
    // backslash-indicator: '\\' '\\'

    static  const   char    escape_code = '\\';
    static  const   char    empty_comment_indicator = '$';
    static  const   char    single_hyphen_indicator = '-';
    static  const   char    double_hyphen_indicator = '2';
    static  const   char    trailing_hyphen_indicator = '_';

    std::ostringstream      decoded_strm;

    enum State { AT_BEGINNING, ESCAPED_AT_BEGINNING, NO_ESCAPE_YET,
                 ENCOUNTERED_A_HYPHEN, ESCAPED, ENCOUNTERED_TRAILING_HYPHEN }

    state = AT_BEGINNING;

    for (std::string::const_iterator iter = encoded_comment.begin();
         iter != encoded_comment.end(); ++iter)  {
        const   char    &the_char = *iter;

        switch (state)  {
            case AT_BEGINNING:
                switch (the_char)  {
                    case escape_code:
                        state = ESCAPED_AT_BEGINNING;
                        break;

                    case '-':
                        state = ENCOUNTERED_A_HYPHEN;
                        break;

                    default:
                        decoded_strm << the_char;
                        break;
                }
                break;

            case ESCAPED_AT_BEGINNING:
                switch (the_char)  {
                    case escape_code:
                        decoded_strm << escape_code;
                        state = NO_ESCAPE_YET;
                        break;

                    case empty_comment_indicator:
                       // That's easy!
                       //
                        decoded_comment.clear();
                        return (true);

                    case double_hyphen_indicator:
                        decoded_strm << "--";
                        state = NO_ESCAPE_YET;
                        break;

                    case trailing_hyphen_indicator:
                        decoded_strm << single_hyphen_indicator;
                        state = ENCOUNTERED_TRAILING_HYPHEN;
                        break;

                    case '-':
                    default:
                       // Invalid.
                       //
                        return (false);
                }
                break;

            case NO_ESCAPE_YET:
                switch (the_char)  {
                    case escape_code:
                        state = ESCAPED;
                        break;

                    case '-':
                        state = ENCOUNTERED_A_HYPHEN;
                        break;

                    default:
                        decoded_strm << the_char;
                        break;
                }
                break;

            case ENCOUNTERED_A_HYPHEN:
                switch (the_char)  {
                    case '-':
                        decoded_strm << escape_code << double_hyphen_indicator;
                        break;

                    case escape_code:
                        decoded_strm << single_hyphen_indicator << escape_code
                                     << escape_code;
                        break;

                    default:
                        decoded_strm << single_hyphen_indicator << the_char;
                        break;
                }
                state = NO_ESCAPE_YET;
                break;

            case ESCAPED:
                switch (the_char)  {
                    case escape_code:
                        decoded_strm << escape_code;
                        state = NO_ESCAPE_YET;
                        break;

                    case double_hyphen_indicator:
                        decoded_strm << "--";
                        state = NO_ESCAPE_YET;
                        break;

                    case trailing_hyphen_indicator:
                        decoded_strm << single_hyphen_indicator;
                        state = ENCOUNTERED_TRAILING_HYPHEN;
                        break;

                    case empty_comment_indicator:
                    case '-':
                    default:
                       // Invalid.
                       //
                        return (false);
                }
                break;

            case ENCOUNTERED_TRAILING_HYPHEN:
               // A trailing-hyphen-sequence is admissible only at the end of
               // the incoming sequence of character, so this is inadmissible.
               //
                return (false);

            default:
                std::ostringstream  ex_stream;

                ex_stream << "XMLWriter::decode_comment: "
                          << "Invalid stream state (integer value "
                          << state
                          << ") encountered."
                          << std::ends;

                throw std::logic_error (ex_stream.str ());
                break;
        }
    }

    switch (state)  {
        case AT_BEGINNING:              // no empty comments!  Bad XML
        case ESCAPED_AT_BEGINNING:      // invalid
        case ENCOUNTERED_A_HYPHEN:      // no trailing hyphens!  Bad XML
        case ESCAPED:                   // invalid
            return (false);

        case NO_ESCAPE_YET:
           // ok
           //
            break;

        case ENCOUNTERED_TRAILING_HYPHEN:
            decoded_strm << single_hyphen_indicator;
            break;

        default:
            std::ostringstream  ex_stream;

            ex_stream << "XMLWriter::decode_comment: "
                      << "Invalid stream state (integer value "
                      << state
                      << ") encountered."
                      << std::ends;

            throw std::logic_error (ex_stream.str ());
            break;
    }

    decoded_strm << std::ends;
    decoded_comment = decoded_strm.str ();

    return (true);
}

// ----------------------------------------------------------------------------

void XMLWriter::indent ()  {

    if (! needs_indent)
        return;

    write ("\n");

   // This is for indentation
   //
    for (int x = 0; x < depth (); x++)  { write ("   "); }
}

// ----------------------------------------------------------------------------

void XMLWriter::indent (XMLWriter::size_type indent_level)  {

    if (! needs_indent)
        return;

    write ("\n");

    for (size_type x = 0; x < indent_level; ++x)  { write ("   "); }
}

} // namespace hmxml

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
