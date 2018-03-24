// Hossein Moein
// March 24, 2018
// Copyright (C) 2018-2019 Hossein Moein
// Distributed under the BSD Software License (see file License)

#ifndef _INCLUDED_XMLString_h
#define _INCLUDED_XMLString_h 0

// The SAX documentation implies that you're supposed to explicitly include the
// "util" subdirectory in your include-path.  I think that's a dumb idea.

#include <stdexcept>

#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/XMLUniDefs.hpp>

// ----------------------------------------------------------------------------

namespace hmxml
{

// This is a std::string-like object (in the vaguest sense...it's really
// rudimentary) that is a container for an array of "XMLCh" objects.  The
// reason I'm implementing this the way I do is that the XMLString class in the
// Xerces library uses a character type ("XMLCh") that has changed from release
// to release and from architeecture to architecture.  Therefore, we must
// abstract it away.
//
// I'm considering calling this a XercesString since it's really specific
// to Xerces.

class   XMLString  {

    private:

        typedef XERCES_CPP_NAMESPACE::XMLString XERCString;

    public:

        typedef XMLCh               value_type;
        typedef unsigned int        size_type;
	    // typedef ptrdiff_t           difference_type;
        typedef value_type &        reference;
        typedef const value_type &  const_reference;
        typedef value_type *        pointer;
        typedef const value_type *  const_pointer;

        static  const   size_type   npos = static_cast<size_type>(-1);
        static  const   value_type  null_character = 0;

        inline XMLString () throw () : data_ (NULL)  {   }
        inline XMLString (const XMLString &that) throw ()
            : data_ (that.data_ ? XERCString::replicate (that.data_) : NULL)  {
        }

        XMLString (const_pointer s, size_type n) throw ();
        inline XMLString (const_pointer s) throw ()
            : data_ (XERCString::replicate (s))  {   }
        XMLString (size_type n, value_type c) throw ();

       // NTBS-based constructors
       //
        inline XMLString (const std::string &rhs) throw ()
            : data_ (XERCString::transcode (rhs.c_str ()))  {   }
        inline XMLString (const char *rhs) throw ()
            : data_ (XERCString::transcode (rhs))  {   }

        inline ~XMLString () throw ()  { delete[] data_; }

        XMLString &operator = (const_pointer s) throw ();
        inline XMLString &operator = (const XMLString &rhs) throw ()  {

            return (*this = rhs.data_);
        }
        XMLString &operator = (value_type c) throw ();
        XMLString &operator = (const char *rhs) throw ();
        inline XMLString &operator = (const std::string &rhs) throw ()  {

            return (*this = rhs.c_str ());
        }

        // iterator begin();
        // const_iterator begin() const;

        // iterator end();
        // const_iterator end() const;

        inline size_type size () const throw ()  {

            return (data_ ? XERCString::stringLen (data_) : 0);
        }

        inline size_type length () const throw ()  { return (size ()); }

        // No max_size, resize, capacity, reserve methods

        inline void clear () throw ()  {

            if (data_ != NULL)
                *data_ = null_character;
        }
        inline bool empty () const throw ()  {

            return (data_ == NULL || *data_ == null_character);
        }

        inline const_reference operator [] (size_type pos) const throw ()  {

            return (data_ [pos]);
        }
        inline reference operator [] (size_type pos) throw ()  {

            return (data_ [pos]);
        }

        void swap (XMLString &rhs) throw ();

        inline const_pointer c_str () const throw ()  { return (data_); }

        static inline size_type
        charstar_len (const value_type *const str) throw ()  {

            size_type   len = 0;

            for (const value_type *iter = str; *iter; ++iter)
                switch (*iter)  {

                    default:
                        len += 1;
                        break;

                    case XERCES_CPP_NAMESPACE::chHTab:         // '\t'
                    case XERCES_CPP_NAMESPACE::chOpenAngle:    // '<'
                    case XERCES_CPP_NAMESPACE::chCloseAngle:   // '>'
                        len += 4;
                        break;

                    case XERCES_CPP_NAMESPACE::chAmpersand:    // '&'
                    case XERCES_CPP_NAMESPACE::chSemiColon:    // ';'
                    case XERCES_CPP_NAMESPACE::chCR:           // '\r' or CR
                    case XERCES_CPP_NAMESPACE::chLF:           // '\n' or LF
                        len += 5;
                        break;

                    case XERCES_CPP_NAMESPACE::chDoubleQuote:  // '"'
                    case XERCES_CPP_NAMESPACE::chSingleQuote:  // '''
                        len += 6;
                        break;
                }

            return (len);
        }

        std::string to_stdstring ();

       // NOTE: In this version, the caller is responsible to delete[] result.
       //
        static bool to_charstar (const char *&str, const_pointer rhs);

        static void
        to_stdstring (std::string &result, XMLString::const_pointer rhs);
        static std::string to_stdstring (const_pointer rhs);

    private:

       // This array is null-terminated.
       //
        pointer data_;
};

} // namespace hmxml

// ----------------------------------------------------------------------------

#undef _INCLUDED_XMLString_h
#define _INCLUDED_XMLString_h 1
#endif // _INCLUDED_XMLString_h

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
