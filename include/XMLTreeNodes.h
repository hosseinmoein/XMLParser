// Hossein Moein
// March 24, 2018
// Copyright (C) 2018-2019 Hossein Moein
// Distributed under the BSD Software License (see file License)

#ifndef _INCLUDED_XMLTreeNodes_h
#define _INCLUDED_XMLTreeNodes_h 0

#include <cstdlib>
#include <iostream>

#include <string>
#include <vector>
#include <utility>

#include <XMLString.h>
#include <XMLNVPair.h>

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

// This is a tree structure to represent an XML statement.
// It satisfies the following properties:
//
//   1) It has a list of zero or more attributes.
//   2) It has zero or one child of self type.
//   3) It has zero or one sibling of self type.
//   4) It provides iterator and const_iterator for its children, siblings,
//      and attributes. The iterators satisfy the following properties:
//
//        a) They have a STL conformant interface.
//        b) They are stateless.
//        c) They _appear_ as a const pointer to XMLTreeNodes.
//        d) They are cheap to create, copy around, and pass by value.
//
//   5) It has a dump_xml() method that reproduces a syntactically correct
//      XML statement, identical to the statement that was parsed to
//      build this tree.
//
class   XMLTreeNodes  {

    public:

        typedef std::vector<XMLNVPair>  attr_vector;

       // This is a const pointer to a const XMLNVPair pointer
       //
        typedef attr_vector::const_iterator attr_const_iterator;
        typedef attr_vector::iterator       attr_iterator;
        typedef unsigned int                size_type;

    private:

        char                *name_;
        XMLTreeNodes    *child_;
        XMLTreeNodes    *sibling_;
        attr_vector         &attr_list_;
        size_type           attr_starting_point_;
        size_type           attr_size_;

    public:

        inline XMLTreeNodes (attr_vector &attr_list) throw ()
            : name_ (NULL),
              child_ (NULL),
              sibling_ (NULL),
              attr_list_ (attr_list),
              attr_starting_point_ (attr_list.size ()),
              attr_size_ (0)  {    }
        inline XMLTreeNodes (XMLNVPair::ConstStrType name,
                                 size_type attr_size,
                                 attr_vector &attr_list) throw ()
            : child_ (NULL),
              sibling_ (NULL),
              name_ (NULL),
              attr_list_ (attr_list),
              attr_starting_point_ (attr_list.size ()),
              attr_size_ (attr_size)  {

            set_name (name);
        }
        inline XMLTreeNodes (const XMLCh *const name,
                                 size_type attr_size,
                                 attr_vector &attr_list) throw ()
            : child_ (NULL),
              sibling_ (NULL),
              name_ (NULL),
              attr_list_ (attr_list),
              attr_starting_point_ (attr_list.size ()),
              attr_size_ (attr_size)  {

            set_name (name);
        }
        inline ~XMLTreeNodes () throw ()  {

            typedef std::vector<XMLTreeNodes *> VecType;

            delete[] name_;
            delete child_;

            XMLTreeNodes::iterator itr = sibling_begin ();

            if (itr != child_sibling_end ())  {
                VecType vec;

                vec.reserve (1024);
                for ( ; itr != child_sibling_end (); ++itr)
                    vec.push_back (const_cast<XMLTreeNodes *>(&(*itr)));

                for (VecType::reverse_iterator ritr = vec.rbegin ();
                     ritr != vec.rend (); ++ritr)  {
                    delete (*ritr)->sibling_;
                    (*ritr)->sibling_ = NULL;
                }
                delete sibling_;
            }
        }

        inline void set_attr_size (size_type attr_size) throw ()  {

            attr_size_ = attr_size;
            return;
        }

       // Access methods to private members.
       //
        inline XMLNVPair::ConstStrType get_name () const throw ()  {

            return (name_);
        }
        inline void set_name (XMLNVPair::ConstStrType name_in) throw ()  {

            const   size_type   nilen = ::strlen (name_in);

            if (name_ == NULL || nilen > ::strlen (name_))  {
                delete[] name_;
                name_ = new char [nilen + 1];
            }

            ::strcpy (name_, name_in);
            return;
        }

        inline void set_name (const XMLCh *const name_in) throw ()  {

            delete[] name_;
            name_ = XERCES_CPP_NAMESPACE::XMLString::transcode (name_in);
            return;
        }

       // NOTE: If the user sets either child or sibling twice without
       //       deleting the first child or sibling, there will be a
       //       memory leak.
       //
        inline void set_child (XMLTreeNodes *child) throw ()  {

            child_ = child;
        }
        inline void set_sibling (XMLTreeNodes *sibling) throw ()  {

            sibling_ = sibling;
        }

        inline const XMLTreeNodes *get_child () const throw ()  {

            return (child_);
        }
        inline const XMLTreeNodes *get_sibling () const throw ()  {

            return (sibling_);
        }
        inline XMLTreeNodes *get_child () throw ()  { return (child_); }
        inline XMLTreeNodes *get_sibling () throw () { return (sibling_); }

        inline void add_attr (XMLNVPair::ConstStrType name,
                              XMLNVPair::ConstStrType value) throw ()  {

            attr_list_.push_back (XMLNVPair());
            attr_list_.back ().set_name_value (name, value);

            //
            // NOTE: In the name of speed, we are going to comment out the
            //       following block of code.
            //

#ifdef XMLSPEED_IS_NO_ISSUE
            if (attr_list_.size () > attr_starting_point_ + attr_size_)
                throw std::runtime_error ("XMLTreeNodes::add_attr(): "
                                         "Too many attributes.");
#endif // XMLSPEED_IS_NO_ISSUE

            return;
        } 
        inline void
        add_attr (const XMLCh *const name, const XMLCh *const value) throw () {

            attr_list_.push_back (XMLNVPair());
            attr_list_.back ().set_name_value (name, value);
            return;
        } 
        inline XMLNVPair::ConstStrType
        get_attr (XMLNVPair::ConstStrType name) const throw ()  {

            const   attr_const_iterator end_iter = attr_end ();

            for (attr_const_iterator itr = attr_begin ();
                 itr != end_iter; ++itr)
                if (! ::strcmp (itr->get_name (), name))
                    return (itr->get_value ());

            return (NULL);
        }

        inline XMLNVPair::ConstStrType
        get_attr (size_type index) const throw ()  {

            return (attr_list_ [attr_starting_point_ + index].get_value ());
        }

       // Currently the assumption is that 'prefix' is one or more
       // SPACE character(s).
       //
       // It must produce a syntactically correct XML statement that is
       // identical to the one that was parsed to build this tree.
       //
        inline std::ostream &
        dump_xml (std::ostream &os, const char *const prefix = "") const  {

            const   std::string pf = prefix;

            os << pf << "<" << name_ << "\n";

            const   std::string pf2 = pf + "    ";

            dump_attr (os, pf2.c_str ());

            if (child_ == NULL)
                os << pf << "/>\n";
            else  {
                os << pf << ">\n";

                const   std::string pf3 = pf + "  ";

                child_->dump_xml (os, pf3.c_str ());
                os << pf << "</" << name_ << ">\n";
            }

            if (sibling_ != NULL)
                 sibling_->dump_xml (os, pf.c_str ());

            return (os);
        }
        inline std::string &dump_xml (std::string &str) const  {

            str += "<";
            str += name_;
            str += " ";

            dump_attr (str);

            if (child_ == NULL)
                str += "/>\n";
            else  {
                str += ">\n";
                child_->dump_xml (str);
                str += "</";
                str += name_;
                str += ">\n";
            }

            if (sibling_ != NULL)
                 sibling_->dump_xml (str);

            return (str);
        }

        inline std::ostream &
        dump_attr (std::ostream &os, const char *const prefix = "") const  {

            for (attr_const_iterator itr = attr_begin ();
                 itr != attr_end (); ++itr)
                itr->dump (os, prefix);

            return (os);
        }
        inline std::string &dump_attr (std::string &str) const  {

            for (attr_const_iterator itr = attr_begin ();
                 itr != attr_end (); ++itr)
                itr->dump (str);

            return (str);
        }

    public:

       // This iterator contains only one pointer. Like STL iterators,
       // it is cheap to create and copy around.
       //
        class   iterator  {

            public:

               // NOTE: The constructor with no argument initializes
               //       the iterator to be the "end" iterator
               //
                inline iterator () throw ()
                    : node_ (XMLTreeNodes::our_end_node_ ())  {   }
                inline iterator (XMLTreeNodes *node) throw ()
                    : node_ (node ? node
                                  : XMLTreeNodes::our_end_node_ ())  {   }

                inline bool operator == (const iterator &rhs) const throw ()  {

                    return (node_ == rhs.node_);
                }
                inline bool operator != (const iterator &rhs) const throw ()  {

                    return (node_ != rhs.node_);
                }

               // Following STL style, this iterator appears as a pointer
               // to XMLTreeNodes.
               //
                inline XMLTreeNodes *operator -> () const throw ()  {

                    return (node_);
                }
                inline XMLTreeNodes &operator * () const throw ()  {

                    return (*node_);
                }

               // We are following STL style iterator interface.
               //
                inline iterator &operator ++ () throw ()  {    // ++Prefix

                    if (node_->sibling_ == NULL)
                        node_ = XMLTreeNodes::our_end_node_ ();
                    else
                        node_ = node_->sibling_;

                    return (*this);
                }
                inline iterator operator ++ (int) throw ()  {  // Postfix++

                    XMLTreeNodes   *ret_node = node_;

                    if (node_->sibling_ == NULL)
                        node_ = XMLTreeNodes::our_end_node_ ();
                    else
                        node_ = node_->sibling_;

                    return (ret_node);
                }

            private:

                XMLTreeNodes    *node_;
        };

       // Same as above, only it is const
       //
        class   const_iterator  {

            public:

               // NOTE: The constructor with no argument initializes
               //       the iterator to be the "end" iterator
               //
                inline const_iterator () throw ()
                    : node_ (XMLTreeNodes::our_const_end_node ())  {   }
                inline const_iterator (XMLTreeNodes const *node) throw ()
                    : node_ (node ? node
                                  : XMLTreeNodes::our_const_end_node ())  {
                }

                inline const_iterator (const iterator &itr) throw ()
                    : node_ (NULL)  {

                    *this = itr;
                }

                inline const_iterator &
                operator = (const iterator &rhs) throw ()  {

                    node_ = &(*rhs);
                    return (*this);
                }

                inline bool
                operator == (const const_iterator &rhs) const throw ()  {

                    return (node_ == rhs.node_);
                }
                inline bool
                operator != (const const_iterator &rhs) const throw ()  {

                    return (node_ != rhs.node_);
                }

               // Following STL style, this iterator appears as a pointer
               // to XMLTreeNodes.
               //
                inline const XMLTreeNodes *operator -> () const throw ()  {

                    return (node_);
                }
                inline const XMLTreeNodes &operator * () const throw ()  {

                    return (*node_);
                }

               // We are following STL style iterator interface.
               //
                inline const_iterator &operator ++ () throw ()  { // ++Prefix

                    if (node_->sibling_ == NULL)
                        node_ = XMLTreeNodes::our_const_end_node ();
                    else
                        node_ = node_->sibling_;

                    return (*this);
                }

               // Postfix++
               //
                inline const_iterator operator ++ (int) throw ()  {

                    XMLTreeNodes   const   *ret_node = node_;

                    if (node_->sibling_ == NULL)
                        node_ = XMLTreeNodes::our_const_end_node ();
                    else
                        node_ = node_->sibling_;

                    return (ret_node);
                }

            private:

                XMLTreeNodes    const   *node_;
        };

       // Iterator related interface.
       //
        inline const_iterator child_begin () const throw ()  {

            return (child_);
        }
        inline const_iterator sibling_begin () const throw ()  {

            return (sibling_);
        }
        inline const_iterator child_sibling_end () const throw ()  {

            return (our_const_end_node ());
        }

        inline attr_const_iterator attr_begin () const throw ()  {

            return (attr_list_.begin () + attr_starting_point_);
        }
        inline attr_const_iterator attr_end () const throw ()  {

            return (attr_begin () + attr_size_);
        }

        inline iterator child_begin () throw ()  { return (child_); }
        inline iterator sibling_begin () throw ()  { return (sibling_); }
        inline iterator child_sibling_end () throw ()  {

            return (our_end_node_ ());
        }

        inline attr_iterator attr_begin () throw ()  {

            return (attr_list_.begin () + attr_starting_point_);
        }
        inline attr_iterator attr_end () throw ()  {

            return (attr_begin () + attr_size_);
        }

    private:

       // Inspired by agent 99 in "Get Smart".
       //
        inline static XMLTreeNodes const *our_const_end_node () throw ()  {

            return (reinterpret_cast<XMLTreeNodes const *>(-99));
        }

        inline static XMLTreeNodes *our_end_node_ () throw ()  {

            return (reinterpret_cast<XMLTreeNodes *>(-99));
        }

        friend  class   const_iterator;
        friend  class   iterator;

    private:

       // These are not implemented and therefore prohibited
       //
        XMLTreeNodes (const XMLTreeNodes &);
        XMLTreeNodes &operator = (const XMLTreeNodes &);
};

// ----------------------------------------------------------------------------

typedef std::vector<const XMLTreeNodes *>   XMLhildrenVector;
typedef std::vector<const XMLTreeNodes *>   XMLiblingsVector;

// ----------------------------------------------------------------------------

// A convenient function to get a conditional list of children
//
template <class xml_FUNC>
inline XMLhildrenVector &
XMLget_children_if (const XMLTreeNodes &the_tree,
                        xml_FUNC functor,
                        XMLhildrenVector &vec) throw ()  {

    XMLhildrenVector    tmp_vec;

    for (XMLTreeNodes::const_iterator itr = the_tree.child_begin ();
         itr != the_tree.child_sibling_end (); ++itr)
        if (functor (*itr))
            tmp_vec.push_back (&(*itr));

    tmp_vec.swap (vec);

    return (vec);
}

// ----------------------------------------------------------------------------

// A convenient function to get a conditional list of siblings
//
template <class xml_FUNC>
inline XMLhildrenVector &
XMLget_siblings_if (const XMLTreeNodes &the_tree,
                        xml_FUNC functor,
                        XMLiblingsVector &vec) throw ()  {

    XMLiblingsVector    tmp_vec;

    for (XMLTreeNodes::const_iterator itr = the_tree.sibling_begin ();
         itr != the_tree.child_sibling_end (); ++itr)
        if (functor (*itr))
            tmp_vec.push_back (&(*itr));

    tmp_vec.swap (vec);

    return (vec);
}

// ----------------------------------------------------------------------------

inline std::ostream &
operator << (std::ostream &os, const XMLTreeNodes &pt)  {

    return (pt.dump_xml (os));
}

} // namespace hmxml

// ----------------------------------------------------------------------------

#undef _INCLUDED_XMLTreeNodes_h
#define _INCLUDED_XMLTreeNodes_h 1
#endif  // _INCLUDED_XMLTreeNodes_h

// Local Variables:
// mode:C++
// tab-width:4
// c-basic-offset:4
// End:
