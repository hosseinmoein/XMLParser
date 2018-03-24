// Hossein Moein
// March 24, 2018
// Copyright (C) 2018-2019 Hossein Moein
// Distributed under the BSD Software License (see file License)

#include <algorithm>
#include <sstream>

#include <XMLString.h>

// ----------------------------------------------------------------------------

namespace hmxml
{

XMLString::XMLString (const_pointer s, size_type n) throw ()  {

    data_ = new XMLCh [n + 1];
    XERCString::copyNString (data_, s, n);
}

// ----------------------------------------------------------------------------

XMLString::XMLString (size_type n, value_type c) throw ()  {

    data_ = new XMLCh [n + 1];

    std::fill (data_, &data_ [n], c);

    data_ [n] = null_character;
}

// ----------------------------------------------------------------------------

XMLString &XMLString::operator = (const_pointer s) throw ()  {

    if (data_ != s)
        if (data_ == NULL ||
            XERCString::stringLen (s) > XERCString::stringLen (data_))  {
            const   pointer old_data = data_;

            data_ = XERCString::replicate (s);
            delete[] old_data;
        }
        else
            XERCString::copyString (data_, s);

    return (*this);
}

// ----------------------------------------------------------------------------

XMLString &XMLString::operator = (value_type c) throw ()  {

    if (data_ == NULL ||
        XERCString::stringLen (data_) < 2)  {
        delete[] data_;
        data_ = new value_type [2];
    }

    *data_ = c;
    data_ [1] = null_character;

    return (*this);
}

// ----------------------------------------------------------------------------

XMLString &XMLString::operator = (const char *rhs) throw ()  {

    const   size_type   slen = XERCString::stringLen (rhs);

    if (data_ == NULL || XERCString::stringLen (data_) < slen)  {
        const   pointer old_data = data_;

        data_ = XERCString::transcode (rhs);

        delete[] old_data;
    }
    else
        XERCString::transcode (rhs, data_, slen);

    return (*this);
}

// ----------------------------------------------------------------------------

void XMLString::swap (XMLString &rhs) throw ()  {

    pointer const   tmp_holder = data_;

    data_ = rhs.data_;
    rhs.data_ = tmp_holder;
    return;
}

// ----------------------------------------------------------------------------

static std::string
hokey_transcode (const XMLString &input_string) throw ()  {

    std::ostringstream  out_strm;

    const   XMLString::const_pointer    end_of_input =
        input_string.c_str () + input_string.size ();

    for (XMLString::const_pointer iter = input_string.c_str ();
         iter != end_of_input; ++iter)  {
        std::string::const_reference    the_character = *iter;

        // Narrow the character into an unsigned int if possible.

        out_strm << static_cast<unsigned char>(the_character);
    }
    out_strm << std::ends;

    return (out_strm.str ());
}

// ----------------------------------------------------------------------------

static std::string
to_human_readable (const std::string &input_string) throw ()  {

    std::ostringstream  out_strm;

    for (std::string::const_iterator iter = input_string.begin ();
         iter != input_string.end (); ++iter)  {
        std::string::const_reference    the_char = *iter;

        out_strm << (isprint (the_char) ? the_char : '.');
    }
    out_strm << std::ends;

    return (out_strm.str ());
}

// ----------------------------------------------------------------------------

static std::string
truncate_string (const std::string &input_string,
                 std::string::size_type max_length,
                 const std::string &continuation_suffix = "...") throw ()  {

    if (input_string.length () <= max_length)
        return (input_string);
    else  {
        const   std::string::size_type  cont_suffix_len =
            continuation_suffix.length ();

        if (max_length >= cont_suffix_len)  {
            const   std::string::size_type  substr_length =
                max_length - cont_suffix_len;

            return (input_string.substr (0, substr_length) +
                    continuation_suffix);
        }
        else
            return (continuation_suffix.substr (cont_suffix_len - max_length,
                                                std::string::npos));
    }
}

// ----------------------------------------------------------------------------

// class-static
//
bool XMLString::to_charstar (const char *&str, const_pointer rhs)  {

    if (! rhs)
        throw std::invalid_argument ("XMLString::to_charstar(): "
                                     "Null pointer passed "
                                     "as input XMLString.");

    str = XERCString::transcode (rhs);

    return (str != NULL);
}

// ----------------------------------------------------------------------------

// class-static
//
void XMLString::to_stdstring (std::string &result, const_pointer rhs)  {

    if (! rhs)
        throw std::invalid_argument ("XMLString::to_stdstring(): "
                                     "Null pointer passed "
                                     "as input XMLString.");

    char    *const  tmp_array = XERCString::transcode (rhs);

    if (! tmp_array)  {
        if (XERCString::stringLen (rhs) == 0)  {
            result = "";
            return;
        }

        std::ostringstream      error_strm;
        const std::string       hokey_transcoded_string = hokey_transcode(rhs);
        const std::string       hokey_human_readable_string =
            to_human_readable (hokey_transcoded_string);
        const std::string       hokey_truncated_string =
            truncate_string (hokey_human_readable_string, 70);
        static  const   char    function_name[] =
            "XMLString::to_stdstring";


        error_strm << "XMLString::to_stdstring(): "
                   << "when processing incoming string '"
                   << hokey_truncated_string
                   << "', XERCES_CPP_NAMESPACE::XERCString::transcode() "
                      "returned a NULL pointer."
                   << std::ends;

        throw std::runtime_error (error_strm.str ());
    }

    result.assign (tmp_array);
    delete[] tmp_array;

    return;
}

// ----------------------------------------------------------------------------

// class-static
//
std::string XMLString::to_stdstring (const_pointer rhs)  {

    std::string result;

    to_stdstring (result, rhs);

    return (result);
}

// ----------------------------------------------------------------------------

std::string XMLString::to_stdstring ()  {

    std::string result;

    if (data_ != NULL)  {
        char    *const  tmp_array = XERCString::transcode (data_);

        result.assign (tmp_array);
        delete[] tmp_array;
    }

    return (result);
}

} // namespace hmxml

// ----------------------------------------------------------------------------

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
