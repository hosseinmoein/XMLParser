// Hossein Moein
// March 24, 2018
// Copyright (C) 2018-2019 Hossein Moein
// Distributed under the BSD Software License (see file License)

#ifndef _INCLUDED_XMLNVPair_h
#define _INCLUDED_XMLNVPair_h 0

#include <cstdlib>
#include <iostream>
#include <string>

#include <XMLString.h>

// ----------------------------------------------------------------------------

namespace hmxml
{

//
// The reason that we use "char *" instead of "std:string" here is for space
// and performance efficiency. "std:string" is a pretty good construct, however
// under extreme conditions, it still cannot beat "char *" in space and
// performace efficiency.
//

// ----------------------------------------------------------------------------

// This is just a dynamic name/value pair with an XML oriented dump() method.
// The reason that a std:pair wasn't used is to avoid using std:string.
//
class   XMLNVPair  {

    private:

        typedef char        CharType;
        typedef CharType *  StrType;

        StrType buffer_;

    public:

        typedef unsigned int            size_type;
        typedef const CharType *const   ConstStrType;

        inline XMLNVPair () throw () : buffer_ (NULL)  {   }
        inline XMLNVPair (ConstStrType name,
                              ConstStrType value) throw () : buffer_ (NULL)  {

            set_name_value (name, value);
        }
        inline XMLNVPair (const XMLNVPair &that) throw ()
            : buffer_ (NULL)  {

            *this = that;
        }
        inline ~XMLNVPair () throw ()  { delete[] buffer_; }

        inline XMLNVPair &operator = (const XMLNVPair &rhs) throw ()  {

            if (&rhs != this)
                set_name_value (rhs.get_name (), rhs.get_value ());

            return (*this);
        }

        inline void
        set_name_value (ConstStrType name, ConstStrType value) throw ()  {

            if (name != NULL && value != NULL)  {
                const   size_type   nlen = ::strlen (name);
                const   size_type   vlen = ::strlen (value);

                if (buffer_ == NULL || ::strlen (get_name ()) +
                                       ::strlen (get_value()) < nlen + vlen)  {
                    delete[] buffer_;
                    buffer_ = new CharType [nlen + vlen + 2];
                }

                ::strcpy (buffer_, name);
                ::strcpy (buffer_ + nlen + 1, value);
            }
            else  {
                delete[] buffer_;
                buffer_ = NULL;
            }

            return;
        }

        inline void set_name_value (const XMLCh *const name,
                                    const XMLCh *const value) throw ()  {

            typedef XERCES_CPP_NAMESPACE::XMLString XERCString;

            if (name && value)  {
                const   size_type   nlen = XMLString::charstar_len (name);
                const   size_type   vlen = XMLString::charstar_len (value);

                if (buffer_ == NULL ||
                    ::strlen (get_name ()) + ::strlen (get_value ()) <
                        nlen + vlen)  {
                    delete[] buffer_;
                    buffer_ = new CharType [nlen + vlen + 2];
                }

                XERCString::transcode (name, buffer_, nlen);
                XERCString::transcode (value, buffer_ + nlen + 1, vlen);
            }
            else  {
                delete[] buffer_;
                buffer_ = NULL;
            }

            return;
        }

        inline ConstStrType get_name () const throw ()  { return (buffer_); }
        inline ConstStrType get_value () const throw ()  {

            return (buffer_ ? buffer_ + ::strlen (buffer_) + 1 : buffer_);
        }

        inline std::ostream &
        dump (std::ostream &os, const char *const prefix = "") const  {

            os << prefix << get_name () << " = \"" << get_value () << "\"\n";

            return (os);
        }

        inline std::string &dump (std::string &str) const  {

            str += get_name ();
            str += "=\"";
            str += get_value ();
            str += "\" ";

            return (str);
        }

        inline bool operator == (const XMLNVPair &rhs) const throw ()  {

            return (! ::strcmp (get_name (), rhs.get_name ()));
        }

        inline void swap (XMLNVPair &other) throw ()  {

            std::swap (buffer_, other.buffer_);
            return;
        }
};

// ----------------------------------------------------------------------------

inline std::ostream &operator << (std::ostream &os, const XMLNVPair &nvp) {

    return (nvp.dump (os));
}

} // namespace hmxml

// ----------------------------------------------------------------------------

#undef _INCLUDED_XMLNVPair_h
#define _INCLUDED_XMLNVPair_h 1
#endif  // _INCLUDED_XMLNVPair_h

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
