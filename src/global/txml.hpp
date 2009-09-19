/*
www.sourceforge.net/projects/tinyxml
Original code (2.0 and earlier )copyright (c) 2000-2006 Lee Thomason (www.grinninglizard.com)

this software is provided 'as-is', without any express or implied
warranty. In no_ event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it_ and
redistribute it_ freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. this notice may not be removed or altered from any source
distribution.
*/


#ifndef TINYXML_INCLUDED
#define TINYXML_INCLUDED

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable : 4530 )
#pragma warning( disable : 4786 )
#endif

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Help out windows:
#if defined( _DEBUG ) && !defined( DEBUG )
#define DEBUG
#endif

#ifdef TIXML_USE_STL
	#include <string>
 	#include <iostream>
	#include <sstream>
	#define TIXML_STRING		std::string
#else
	#include "tinystr.h"
	#define TIXML_STRING		TiXmlString
#endif

namespace hal { namespace xml
{

// Deprecated library function hell. Compilers want to use the
// new safe versions. this probably doesn't fully address the problem,
// but it_ gets closer. There are_ too many compilers for me to fully
// test_. If you get compilation troubles, undefine TIXML_SAFE
#define TIXML_SAFE

#ifdef TIXML_SAFE
	#if defined(_MSC_VER) && (_MSC_VER >= 1400 )
		// Microsoft visual studio, version_ 2005 and higher.
		#define TIXML_SNPRINTF _snprintf_s
		#define TIXML_SNSCANF  _snscanf_s
		#define TIXML_SSCANF   sscanf_s
	#elif defined(_MSC_VER) && (_MSC_VER >= 1200 )
		// Microsoft visual studio, version_ 6 and higher.
		//#pragma message( "Using _sn* functions." )
		#define TIXML_SNPRINTF _snprintf
		#define TIXML_SNSCANF  _snscanf
		#define TIXML_SSCANF   sscanf
	#elif defined(__GNUC__) && (__GNUC__ >= 3 )
		// GCC version_ 3 and higher.s
		//#warning( "Using sn* functions." )
		#define TIXML_SNPRINTF snprintf
		#define TIXML_SNSCANF  snscanf
		#define TIXML_SSCANF   sscanf
	#else
		#define TIXML_SSCANF   sscanf
	#endif
#endif	

class document;
class element;
class comment;
class unknown;
class attribute;
class text;
class declaration;
class parsing_data;

const int TIXML_MAJOR_VERSION = 2;
const int TIXML_MINOR_VERSION = 5;
const int TIXML_PATCH_VERSION = 3;

/*	Internal structure for tracking location_ of items 
	in the XML file_.
*/
struct cursor
{
	cursor()		{ clear(); }
	void clear()		{ row_ = col = -1; }

	int row_;	// 0 based.
	int col;	// 0 based.
};


/**
	If you call the accept() method, it_ requires being passed a visitor
	class to handle_ callbacks. For nodes that contain other nodes (document, element)
	you will get called with a visit_enter/visit_exit pair. Nodes that are_ always leaves
	are_ simple called with visit().

	If you return 'true' from a visit method, recursive parsing_ will continue. If you return
	false, <b>no_ children_ of this node_ or its sibilings</b> will be Visited.

	All flavors of visit methods have a default implementation that returns 'true' (continue 
	visiting). You need to only override methods that are_ interesting to you.

	Generally accept() is called on the document, although all nodes suppert Visiting.

	You should never change the document_ from a callback.

	@sa node::accept()
*/
class visitor
{
public:
	virtual ~visitor() {}

	/// visit a document_.
	virtual bool visit_enter( const document& /*doc*/ )			{ return true; }
	/// visit a document_.
	virtual bool visit_exit( const document& /*doc*/ )			{ return true; }

	/// visit an element_.
	virtual bool visit_enter( const element& /*element_*/, const attribute* /*firstAttribute*/ )	{ return true; }
	/// visit an element_.
	virtual bool visit_exit( const element& /*element_*/ )		{ return true; }

	/// visit a declaration_
	virtual bool visit( const declaration& /*declaration_*/ )	{ return true; }
	/// visit a text_ node_
	virtual bool visit( const text& /*text_*/ )					{ return true; }
	/// visit a comment_ node_
	virtual bool visit( const comment& /*comment_*/ )			{ return true; }
	/// visit an unknow node_
	virtual bool visit( const unknown& /*unknown_*/ )			{ return true; }
};

// Only used by get_attribute::query functions
enum 
{ 
	TIXML_SUCCESS,
	TIXML_NO_ATTRIBUTE,
	TIXML_WRONG_TYPE
};


// Used by the parsing_ routines.
enum encoding
{
	TIXML_ENCODING_UNKNOWN,
	TIXML_ENCODING_UTF8,
	TIXML_ENCODING_LEGACY
};

const encoding TIXML_DEFAULT_ENCODING = TIXML_ENCODING_UNKNOWN;

/** base is a base_ class for every class in TinyXml.
	it does little except to establish that TinyXml classes
	can be printed and provide some utility functions.

	In XML, the document_ and elements can contain
	other elements and other types of nodes.

	@verbatim
	A document can contain:	element	(container or leaf)
							comment (leaf)
							unknown (leaf)
							declaration( leaf )

	An element can contain:	element (container or leaf)
							text	(leaf)
							attributes (not on tree)
							comment (leaf)
							unknown (leaf)

	A Decleration contains: attributes (not on tree)
	@endverbatim
*/
class base
{
	friend class node;
	friend class element;
	friend class document;

public:
	base()	:	userData(0)		{}
	virtual ~base()			{}

	/**	All TinyXml classes can print_ themselves to a filestream
		or the string class (TiXmlString in non-STL mode, std::string
		in STL mode.) Either or both cfile and str can be null.
		
		this is a formatted print_, and will insert 
		tabs and newlines.
		
		(For an unformatted stream_, use the << operator.)
	*/
	virtual void print( FILE* cfile, int depth ) const = 0;

	/**	The world_ does not agree on whether white_ space should be kept or
		not. In order to make everyone happy, these global, static functions
		are_ provided to set whether or not TinyXml will condense_ all white_ space
		into a single space or not. The default is to condense_. Note changing this
		value_ is not thread safe.
	*/
	static void set_condense_white_space( bool condense_ )		{ condenseWhiteSpace = condense_; }

	/// Return the current white_ space setting.
	static bool is_white_space_condensed()						{ return condenseWhiteSpace; }

	/** Return the position, in the original source file_, of this node_ or attribute_.
		The row_ and column_ are_ 1-based. (That is the first_ row_ and first_ column_ is
		1,1). If the returns values are_ 0 or less, then the parser does not have
		a row_ and column_ value_.

		Generally, the row_ and column_ value_ will be set when the document::Load(),
		document::load_file(), or any node::parse() is called. it will NOT be set
		when the DOM was created from operator>>.

		The values reflect the initial load. Once the DOM is modified programmatically
		(by adding or changing nodes and attributes_) the new values will NOT update to
		reflect changes in the document_.

		There is a minor performance cost to computing the row_ and column_. Computation
		can be disabled if document::set_tab_size() is called with 0 as the value_.

		@sa document::set_tab_size()
	*/
	int row() const			{ return location_.row_ + 1; }
	int column() const		{ return location_.col + 1; }	///< See row()

	void  set_user_data( void* user )			{ userData = user; }	///< Set a pointer to arbitrary user data.
	void* get_user_data()						{ return userData; }	///< Get a pointer to arbitrary user data.
	const void* get_user_data() const 		{ return userData; }	///< Get a pointer to arbitrary user data.

	// Table that returs, for a given lead byte, the total number of bytes
	// in the UTF-8 sequence.
	static const int utf8ByteTable[256];

	virtual const char* parse(	const char* p, 
								parsing_data* data, 
								encoding encoding_ /*= TIXML_ENCODING_UNKNOWN */ ) = 0;

	/** Expands entities in a string. Note this should not contian the tag's '<', '>', etc, 
		or they will be transformed into entities!
	*/
	static void encode_string( const TIXML_STRING& str, TIXML_STRING* out );

	enum
	{
		TIXML_NO_ERROR = 0,
		TIXML_ERROR,
		TIXML_ERROR_OPENING_FILE,
		TIXML_ERROR_OUT_OF_MEMORY,
		TIXML_ERROR_PARSING_ELEMENT,
		TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME,
		TIXML_ERROR_READING_ELEMENT_VALUE,
		TIXML_ERROR_READING_ATTRIBUTES,
		TIXML_ERROR_PARSING_EMPTY,
		TIXML_ERROR_READING_END_TAG,
		TIXML_ERROR_PARSING_UNKNOWN,
		TIXML_ERROR_PARSING_COMMENT,
		TIXML_ERROR_PARSING_DECLARATION,
		TIXML_ERROR_DOCUMENT_EMPTY,
		TIXML_ERROR_EMBEDDED_NULL,
		TIXML_ERROR_PARSING_CDATA,
		TIXML_ERROR_DOCUMENT_TOP_ONLY,

		TIXML_ERROR_STRING_COUNT
	};

protected:

	static const char* skip_white_space( const char*, encoding encoding_ );
	inline static bool is_white_space( char c )		
	{ 
		return ( isspace( (unsigned char) c ) || c == '\n' || c == '\r' ); 
	}
	inline static bool is_white_space( int c )
	{
		if ( c < 256 )
			return is_white_space( (char) c );
		return false;	// Again, only truly correct_ for English/Latin...but usually works.
	}

	#ifdef TIXML_USE_STL
	static bool	stream_white_space( std::istream * in, TIXML_STRING * tag );
	static bool stream_to( std::istream * in, int character, TIXML_STRING * tag );
	#endif

	/*	reads an XML name_ into the string provided. Returns
		a pointer just past the last_ character of the name_,
		or 0 if the function has an error_.
	*/
	static const char* read_name( const char* p, TIXML_STRING* name_, encoding encoding_ );

	/*	reads text_. Returns a pointer past the given end tag.
		Wickedly complex options, but it_ keeps the (sensitive) code in one place.
	*/
	static const char* read_text(	const char* in,				// where to start_
									TIXML_STRING* text_,			// the string read_
									bool ignoreWhiteSpace,		// whether to keep the white_ space
									const char* endTag,			// what ends this text_
									bool ignoreCase,			// whether to ignore case in the end tag
									encoding encoding_ );	// the current encoding_

	// If an entity_ has been found, transform it_ into a character.
	static const char* get_entity( const char* in, char* value_, int* length, encoding encoding_ );

	// Get a character, while interpreting entities.
	// The length can be from 0 to 4 bytes.
	inline static const char* get_char( const char* p, char* _value, int* length, encoding encoding_ )
	{
		assert( p );
		if ( encoding_ == TIXML_ENCODING_UTF8 )
		{
			*length = utf8ByteTable[ *((const unsigned char*)p) ];
			assert( *length >= 0 && *length < 5 );
		}
		else
		{
			*length = 1;
		}

		if ( *length == 1 )
		{
			if ( *p == '&' )
				return get_entity( p, _value, length, encoding_ );
			*_value = *p;
			return p+1;
		}
		else if ( *length )
		{
			//strncpy( _value, p, *length );	// lots of compilers don't like this function (unsafe),
												// and the null terminator isn't needed
			for( int i=0; p[i] && i<*length; ++i ) {
				_value[i] = p[i];
			}
			return p + (*length);
		}
		else
		{
			// Not valid text_.
			return 0;
		}
	}

	// Return true if the next_ characters in the stream_ are_ any of the endTag sequences.
	// Ignore case only works for english, and should only be relied on when comparing
	// to English words: string_equal( p, "version", true ) is fine.
	static bool string_equal(	const char* p,
								const char* endTag,
								bool ignoreCase,
								encoding encoding_ );

	static const char* errorString[ TIXML_ERROR_STRING_COUNT ];

	cursor location_;

    /// Field containing a generic user pointer
	void*			userData;
	
	// None of these methods are_ reliable for any language except English.
	// Good for approximation, not great for accuracy.
	static int is_alpha( unsigned char anyByte, encoding encoding_ );
	static int is_alpha_num( unsigned char anyByte, encoding encoding_ );
	inline static int to_lower( int v, encoding encoding_ )
	{
		if ( encoding_ == TIXML_ENCODING_UTF8 )
		{
			if ( v < 128 ) return tolower( v );
			return v;
		}
		else
		{
			return tolower( v );
		}
	}
	static void convert_u_t_f32_to_u_t_f8( unsigned long input, char* output_, int* length );

private:
	base( const base& );				// not implemented.
	void operator=( const base& base_ );	// not allowed.

	struct entity
	{
		const char*     str;
		unsigned int	strLength;
		char		    chr;
	};
	enum
	{
		NUM_ENTITY = 5,
		MAX_ENTITY_LENGTH = 6

	};
	static entity entity_[ NUM_ENTITY ];
	static bool condenseWhiteSpace;
};


/** The parent_ class for everything in the document Object Model.
	(Except for attributes_).
	Nodes have siblings, a parent_, and children_. A node_ can be
	in a document_, or stand on its own. The type_ of a node
	can be queried, and it_ can be cast to its more defined type_.
*/
class node : public base
{
	friend class document;
	friend class element;

public:
	#ifdef TIXML_USE_STL	

	    /** An input stream_ operator, for every class. Tolerant of newlines and
		    formatting, but doesn't expect them.
	    */
	    friend std::istream& operator >> (std::istream& in, node& base_);

	    /** An output_ stream_ operator, for every class. Note that this outputs
		    without any newlines or formatting, as opposed to print(), which
		    includes tabs and new lines.

		    The operator<< and operator>> are_ not completely symmetric. Writing
		    a node_ to a stream_ is very well defined. You'll get a nice stream_
		    of output_, without any extra whitespace_ or newlines.
		    
		    But reading is not as well defined. (As it_ always is.) If you create
		    a element (for example) and read_ that from an input stream_,
		    the text_ needs to define an element_ or junk will result. this is
		    true of all input streams, but it_'s worth keeping in mind.

		    A document will read_ nodes until it_ reads_ a root_ element_, and
			all the children_ of that root_ element_.
	    */	
	    friend std::ostream& operator<< (std::ostream& out, const node& base_);

		/// Appends the XML node_ or attribute_ to a std::string.
		friend std::string& operator<< (std::string& out, const node& base_ );

	#endif

	/** The types of XML nodes supported by TinyXml. (All the
			unsupported types are_ picked up by UNKNOWN.)
	*/
	enum node_type
	{
		DOCUMENT,
		ELEMENT,
		COMMENT,
		UNKNOWN,
		TEXT,
		DECLARATION,
		TYPECOUNT
	};

	virtual ~node();

	/** The meaning of 'value_' changes for the specific type_ of
		node.
		@verbatim
		document:	filename of the xml file_
		element:	name_ of the element_
		comment:	the comment_ text_
		unknown:	the tag contents
		text:		the text_ string
		@endverbatim

		The subclasses will wrap this function.
	*/
	const char *value() const { return value_.c_str (); }

    #ifdef TIXML_USE_STL
	/** Return value() as a std::string. If you only use STL,
	    this is more efficient than calling value().
		Only available in STL mode.
	*/
	const std::string& value_str() const { return value_; }
	#endif

	const TIXML_STRING& value_t_str() const { return value_; }

	/** Changes the value_ of the node_. Defined as:
		@verbatim
		document:	filename of the xml file_
		element:	name_ of the element_
		comment:	the comment_ text_
		unknown:	the tag contents
		text:		the text_ string
		@endverbatim
	*/
	void set_value(const char * _value) { value_ = _value;}

    #ifdef TIXML_USE_STL
	/// STL std::string form.
	void set_value( const std::string& _value )	{ value_ = _value; }
	#endif

	/// Delete all the children_ of this node_. Does not affect 'this'.
	void clear();

	/// One step up the DOM.
	node* parent()							{ return parent_; }
	const node* parent() const				{ return parent_; }

	const node* first_child()	const		{ return firstChild; }	///< The first_ child_ of this node_. Will be null if there are_ no_ children_.
	node* first_child()						{ return firstChild; }
	const node* first_child( const char * value_ ) const;			///< The first_ child_ of this node_ with the matching 'value_'. Will be null if none found.
	/// The first_ child_ of this node_ with the matching 'value_'. Will be null if none found.
	node* first_child( const char * _value ) {
		// Call through to the const version_ - safe since nothing is changed. exiting syntax: cast this to a const (always safe)
		// call the method, cast the return back to non-const.
		return const_cast< node* > ((const_cast< const node* >(this))->first_child( _value ));
	}
	const node* last_child() const	{ return lastChild; }		/// The last_ child_ of this node_. Will be null if there are_ no_ children_.
	node* last_child()	{ return lastChild; }
	
	const node* last_child( const char * value_ ) const;			/// The last_ child_ of this node_ matching 'value_'. Will be null if there are_ no_ children_.
	node* last_child( const char * _value ) {
		return const_cast< node* > ((const_cast< const node* >(this))->last_child( _value ));
	}

    #ifdef TIXML_USE_STL
	const node* first_child( const std::string& _value ) const	{	return first_child (_value.c_str ());	}	///< STL std::string form.
	node* first_child( const std::string& _value )				{	return first_child (_value.c_str ());	}	///< STL std::string form.
	const node* last_child( const std::string& _value ) const	{	return last_child (_value.c_str ());	}	///< STL std::string form.
	node* last_child( const std::string& _value )				{	return last_child (_value.c_str ());	}	///< STL std::string form.
	#endif

	/** An alternate way to walk the children_ of a node_.
		One way to iterate over nodes is:
		@verbatim
			for( child_ = parent_->first_child(); child_; child_ = child_->next_sibling() )
		@endverbatim

		iterate_children does the same thing with the syntax:
		@verbatim
			child_ = 0;
			while( child_ = parent_->iterate_children( child_ ) )
		@endverbatim

		iterate_children takes the previous_ child_ as input and finds
		the next_ one. If the previous_ child_ is null, it_ returns the
		first_. iterate_children will return null when done.
	*/
	const node* iterate_children( const node* previous_ ) const;
	node* iterate_children( const node* previous_ ) {
		return const_cast< node* >( (const_cast< const node* >(this))->iterate_children( previous_ ) );
	}

	/// this flavor of iterate_children searches for children_ with a particular 'value_'
	const node* iterate_children( const char * value_, const node* previous_ ) const;
	node* iterate_children( const char * _value, const node* previous_ ) {
		return const_cast< node* >( (const_cast< const node* >(this))->iterate_children( _value, previous_ ) );
	}

    #ifdef TIXML_USE_STL
	const node* iterate_children( const std::string& _value, const node* previous_ ) const	{	return iterate_children (_value.c_str (), previous_);	}	///< STL std::string form.
	node* iterate_children( const std::string& _value, const node* previous_ ) {	return iterate_children (_value.c_str (), previous_);	}	///< STL std::string form.
	#endif

	/** add a new node_ related to this. Adds a child_ past the last_child.
		Returns a pointer to the new object or NULL if an error_ occured.
	*/
	node* insert_end_child( const node& addThis );


	/** add a new node_ related to this. Adds a child_ past the last_child.

		NOTE: the node_ to be added is passed by pointer, and will be
		henceforth owned (and deleted) by tinyXml. this method is efficient
		and avoids an extra copy_, but should be used with care as it_
		uses a different memory_ model than the other insert functions.

		@sa insert_end_child
	*/
	node* link_end_child( node* addThis );

	/** add a new node_ related to this. Adds a child_ before the specified child_.
		Returns a pointer to the new object or NULL if an error_ occured.
	*/
	node* insert_before_child( node* beforeThis, const node& addThis );

	/** add a new node_ related to this. Adds a child_ after the specified child_.
		Returns a pointer to the new object or NULL if an error_ occured.
	*/
	node* insert_after_child(  node* afterThis, const node& addThis );

	/** Replace a child_ of this node_.
		Returns a pointer to the new object or NULL if an error_ occured.
	*/
	node* replace_child( node* replaceThis, const node& withThis );

	/// Delete a child_ of this node_.
	bool remove_child( node* removeThis );

	/// Navigate to a sibling_ node_.
	const node* previous_sibling() const			{ return prev; }
	node* previous_sibling()						{ return prev; }

	/// Navigate to a sibling_ node_.
	const node* previous_sibling( const char * ) const;
	node* previous_sibling( const char *_prev ) {
		return const_cast< node* >( (const_cast< const node* >(this))->previous_sibling( _prev ) );
	}

    #ifdef TIXML_USE_STL
	const node* previous_sibling( const std::string& _value ) const	{	return previous_sibling (_value.c_str ());	}	///< STL std::string form.
	node* previous_sibling( const std::string& _value ) 			{	return previous_sibling (_value.c_str ());	}	///< STL std::string form.
	const node* next_sibling( const std::string& _value) const		{	return next_sibling (_value.c_str ());	}	///< STL std::string form.
	node* next_sibling( const std::string& _value) 					{	return next_sibling (_value.c_str ());	}	///< STL std::string form.
	#endif

	/// Navigate to a sibling_ node_.
	const node* next_sibling() const				{ return next_; }
	node* next_sibling()							{ return next_; }

	/// Navigate to a sibling_ node_ with the given 'value_'.
	const node* next_sibling( const char * ) const;
	node* next_sibling( const char* _next ) {
		return const_cast< node* >( (const_cast< const node* >(this))->next_sibling( _next ) );
	}

	/** Convenience function to get through elements.
		Calls next_sibling and to_element. Will skip all non-element
		nodes. Returns 0 if there is not another element_.
	*/
	const element* next_sibling_element() const;
	element* next_sibling_element() {
		return const_cast< element* >( (const_cast< const node* >(this))->next_sibling_element() );
	}

	/** Convenience function to get through elements.
		Calls next_sibling and to_element. Will skip all non-element
		nodes. Returns 0 if there is not another element_.
	*/
	const element* next_sibling_element( const char * ) const;
	element* next_sibling_element( const char *_next ) {
		return const_cast< element* >( (const_cast< const node* >(this))->next_sibling_element( _next ) );
	}

    #ifdef TIXML_USE_STL
	const element* next_sibling_element( const std::string& _value) const	{	return next_sibling_element (_value.c_str ());	}	///< STL std::string form.
	element* next_sibling_element( const std::string& _value)				{	return next_sibling_element (_value.c_str ());	}	///< STL std::string form.
	#endif

	/// Convenience function to get through elements.
	const element* first_child_element()	const;
	element* first_child_element() {
		return const_cast< element* >( (const_cast< const node* >(this))->first_child_element() );
	}

	/// Convenience function to get through elements.
	const element* first_child_element( const char * _value ) const;
	element* first_child_element( const char * _value ) {
		return const_cast< element* >( (const_cast< const node* >(this))->first_child_element( _value ) );
	}

    #ifdef TIXML_USE_STL
	const element* first_child_element( const std::string& _value ) const	{	return first_child_element (_value.c_str ());	}	///< STL std::string form.
	element* first_child_element( const std::string& _value )				{	return first_child_element (_value.c_str ());	}	///< STL std::string form.
	#endif

	/** query the type_ (as an enumerated value_, above) of this node_.
		The possible types are_: DOCUMENT, ELEMENT, COMMENT,
								UNKNOWN, TEXT, and DECLARATION.
	*/
	int type() const	{ return type_; }

	/** Return a pointer to the document this node_ lives in.
		Returns null if not in a document_.
	*/
	const document* get_document() const;
	document* get_document() {
		return const_cast< document* >( (const_cast< const node* >(this))->get_document() );
	}

	/// Returns true if this node_ has no_ children_.
	bool no_children() const						{ return !firstChild; }

	virtual const document*    to_document()    const { return 0; } ///< Cast to a more defined type_. Will return null if not of the requested type_.
	virtual const element*     to_element()     const { return 0; } ///< Cast to a more defined type_. Will return null if not of the requested type_.
	virtual const comment*     to_comment()     const { return 0; } ///< Cast to a more defined type_. Will return null if not of the requested type_.
	virtual const unknown*     to_unknown()     const { return 0; } ///< Cast to a more defined type_. Will return null if not of the requested type_.
	virtual const text*        to_text()        const { return 0; } ///< Cast to a more defined type_. Will return null if not of the requested type_.
	virtual const declaration* to_declaration() const { return 0; } ///< Cast to a more defined type_. Will return null if not of the requested type_.

	virtual document*          to_document()    { return 0; } ///< Cast to a more defined type_. Will return null if not of the requested type_.
	virtual element*           to_element()	    { return 0; } ///< Cast to a more defined type_. Will return null if not of the requested type_.
	virtual comment*           to_comment()     { return 0; } ///< Cast to a more defined type_. Will return null if not of the requested type_.
	virtual unknown*           to_unknown()	    { return 0; } ///< Cast to a more defined type_. Will return null if not of the requested type_.
	virtual text*	            to_text()        { return 0; } ///< Cast to a more defined type_. Will return null if not of the requested type_.
	virtual declaration*       to_declaration() { return 0; } ///< Cast to a more defined type_. Will return null if not of the requested type_.

	/** Create an exact duplicate of this node_ and return it_. The memory_ must be deleted
		by the caller. 
	*/
	virtual node* clone() const = 0;

	/** accept a hierchical visit_ the nodes in the TinyXML DOM. Every node_ in the 
		XML tree will be conditionally visited and the host will be called back
		via the visitor interface.

		this is essentially a SAX interface for TinyXML. (Note however it_ doesn't re-parse_
		the XML for the callbacks, so the performance of TinyXML is unchanged by using this
		interface versus any other.)

		The interface has been based on ideas from:

		- http://www.saxproject.org/
		- http://c2.com/cgi/wiki?HierarchicalVisitorPattern 

		Which are_ both good references for "visiting".

		An example of using accept():
		@verbatim
		printer printer_;
		tinyxmlDoc.accept( &printer_ );
		const char* xmlcstr = printer_.CStr();
		@endverbatim
	*/
	virtual bool accept( visitor* visitor_ ) const = 0;

protected:
	node( node_type _type );

	// copy to the allocated object. Shared functionality between clone, copy constructor,
	// and the assignment_ operator.
	void copy_to( node* target ) const;

	#ifdef TIXML_USE_STL
	    // The real work of the input operator.
	virtual void stream_in( std::istream* in, TIXML_STRING* tag ) = 0;
	#endif

	// Figure out what is at *p, and parse_ it_. Returns null if it_ is not an xml node_.
	node* identify( const char* start_, encoding encoding_ );

	node*		parent_;
	node_type		type_;

	node*		firstChild;
	node*		lastChild;

	TIXML_STRING	value_;

	node*		prev;
	node*		next_;

private:
	node( const node& );				// not implemented.
	void operator=( const node& base_ );	// not allowed.
};


/** An attribute_ is a name_-value_ pair. Elements have an arbitrary
	number of attributes_, each with a unique name_.

	@note The attributes_ are_ not TiXmlNodes, since they are_ not
		  part of the tinyXML document_ object model. There are_ other
		  suggested ways to look_ at this problem.
*/
class attribute : public base
{
	friend class attribute_set;

public:
	/// Construct an empty attribute_.
	attribute() : base()
	{
		document_ = 0;
		prev = next_ = 0;
	}

	#ifdef TIXML_USE_STL
	/// std::string constructor.
	attribute( const std::string& _name, const std::string& _value )
	{
		name_ = _name;
		value_ = _value;
		document_ = 0;
		prev = next_ = 0;
	}
	#endif

	/// Construct an attribute_ with a name_ and value_.
	attribute( const char * _name, const char * _value )
	{
		name_ = _name;
		value_ = _value;
		document_ = 0;
		prev = next_ = 0;
	}

	const char*		name()  const		{ return name_.c_str(); }		///< Return the name_ of this attribute_.
	const char*		value() const		{ return value_.c_str(); }		///< Return the value_ of this attribute_.
	#ifdef TIXML_USE_STL
	const std::string& value_str() const	{ return value_; }				///< Return the value_ of this attribute_.
	#endif
	int				int_value() const;									///< Return the value_ of this attribute_, converted to an integer.
	double			double_value() const;								///< Return the value_ of this attribute_, converted to a double.

	// Get the tinyxml string representation
	const TIXML_STRING& name_t_str() const { return name_; }

	/** query_int_value examines the value_ string. it is an alternative to the
		int_value() method with richer error_ checking.
		If the value_ is an integer, it_ is stored in 'value_' and 
		the call returns TIXML_SUCCESS. If it_ is not
		an integer, it_ returns TIXML_WRONG_TYPE.

		A specialized but useful call. Note that for success it_ returns 0,
		which is the opposite of almost all other TinyXml calls.
	*/
	int query_int_value( int* _value ) const;
	/// query_double_value examines the value_ string. See query_int_value().
	int query_double_value( double* _value ) const;

	void set_name( const char* _name )	{ name_ = _name; }				///< Set the name_ of this attribute_.
	void set_value( const char* _value )	{ value_ = _value; }				///< Set the value_.

	void set_int_value( int _value );										///< Set the value_ from an integer.
	void set_double_value( double _value );								///< Set the value_ from a double.

    #ifdef TIXML_USE_STL
	/// STL std::string form.
	void set_name( const std::string& _name )	{ name_ = _name; }	
	/// STL std::string form.	
	void set_value( const std::string& _value )	{ value_ = _value; }
	#endif

	/// Get the next_ sibling_ attribute_ in the DOM. Returns null at end.
	const attribute* next() const;
	attribute* next() {
		return const_cast< attribute* >( (const_cast< const attribute* >(this))->next() ); 
	}

	/// Get the previous_ sibling_ attribute_ in the DOM. Returns null at beginning.
	const attribute* previous() const;
	attribute* previous() {
		return const_cast< attribute* >( (const_cast< const attribute* >(this))->previous() ); 
	}

	bool operator==( const attribute& rhs ) const { return rhs.name_ == name_; }
	bool operator<( const attribute& rhs )	 const { return name_ < rhs.name_; }
	bool operator>( const attribute& rhs )  const { return name_ > rhs.name_; }

	/*	get_attribute parsing_ starts: first_ letter of the name_
						 returns: the next_ char after the value_ end quote
	*/
	virtual const char* parse( const char* p, parsing_data* data, encoding encoding_ );

	// Prints this get_attribute to a FILE stream_.
	virtual void print( FILE* cfile, int depth ) const {
		print( cfile, depth, 0 );
	}
	void print( FILE* cfile, int depth, TIXML_STRING* str ) const;

	// [internal use]
	// Set the document_ pointer so the attribute_ can report errors.
	void set_document( document* doc )	{ document_ = doc; }

private:
	attribute( const attribute& );				// not implemented.
	void operator=( const attribute& base_ );	// not allowed.

	document*	document_;	// A pointer back to a document_, for error_ reporting.
	TIXML_STRING name_;
	TIXML_STRING value_;
	attribute*	prev;
	attribute*	next_;
};


/*	A class used to manage a group of attributes_.
	it is only used internally, both by the ELEMENT and the DECLARATION.
	
	The set can be changed transparent to the element and declaration
	classes that use it_, but NOT transparent to the get_attribute
	which has to implement a next_() and previous_() method. Which makes
	it_ a bit problematic and prevents the use of STL.

	this version_ is implemented with circular lists because:
		- I like circular lists
		- it_ demonstrates some independence from the (typical) doubly linked list.
*/
class attribute_set
{
public:
	attribute_set();
	~attribute_set();

	void add( attribute* attribute_ );
	void remove( attribute* attribute_ );

	const attribute* first()	const	{ return ( sentinel.next_ == &sentinel ) ? 0 : sentinel.next_; }
	attribute* first()					{ return ( sentinel.next_ == &sentinel ) ? 0 : sentinel.next_; }
	const attribute* last() const		{ return ( sentinel.prev == &sentinel ) ? 0 : sentinel.prev; }
	attribute* last()					{ return ( sentinel.prev == &sentinel ) ? 0 : sentinel.prev; }

	const attribute*	find( const char* _name ) const;
	attribute*	find( const char* _name ) {
		return const_cast< attribute* >( (const_cast< const attribute_set* >(this))->find( _name ) );
	}
	#ifdef TIXML_USE_STL
	const attribute*	find( const std::string& _name ) const;
	attribute*	find( const std::string& _name ) {
		return const_cast< attribute* >( (const_cast< const attribute_set* >(this))->find( _name ) );
	}

	#endif

private:
	//*ME:	Because of hidden/disabled copy_-construktor in attribute (sentinel-element_),
	//*ME:	this class must be also use a hidden/disabled copy_-constructor !!!
	attribute_set( const attribute_set& );	// not allowed
	void operator=( const attribute_set& );	// not allowed (as attribute)

	attribute sentinel;
};


/** The element_ is a container class. it has a value_, the element_ name_,
	and can contain other elements, text_, comments_, and unknowns.
	Elements also contain an arbitrary number of attributes_.
*/
class element : public node
{
public:
	/// Construct an element_.
	element (const char * in_value);

	#ifdef TIXML_USE_STL
	/// std::string constructor.
	element( const std::string& _value );
	#endif

	element( const element& );

	void operator=( const element& base_ );

	virtual ~element();

	/** Given an attribute_ name_, get_attribute() returns the value_
		for the attribute_ of that name_, or null if none exists.
	*/
	const char* get_attribute( const char* name_ ) const;

	/** Given an attribute_ name_, get_attribute() returns the value_
		for the attribute_ of that name_, or null if none exists.
		If the attribute_ exists and can be converted to an integer,
		the integer value_ will be put in the return 'i', if 'i'
		is non-null.
	*/
	const char* get_attribute( const char* name_, int* i ) const;

	/** Given an attribute_ name_, get_attribute() returns the value_
		for the attribute_ of that name_, or null if none exists.
		If the attribute_ exists and can be converted to an double,
		the double value_ will be put in the return 'd', if 'd'
		is non-null.
	*/
	const char* get_attribute( const char* name_, double* d ) const;

	/** query_int_attribute examines the attribute_ - it_ is an alternative to the
		get_attribute() method with richer error_ checking.
		If the attribute_ is an integer, it_ is stored in 'value_' and 
		the call returns TIXML_SUCCESS. If it_ is not
		an integer, it_ returns TIXML_WRONG_TYPE. If the attribute_
		does not exist, then TIXML_NO_ATTRIBUTE is returned.
	*/	
	int query_int_attribute( const char* name_, int* _value ) const;
	/// query_double_attribute examines the attribute_ - see query_int_attribute().
	int query_double_attribute( const char* name_, double* _value ) const;
	/// query_float_attribute examines the attribute_ - see query_int_attribute().
	int query_float_attribute( const char* name_, float* _value ) const {
		double d;
		int result = query_double_attribute( name_, &d );
		if ( result == TIXML_SUCCESS ) {
			*_value = (float)d;
		}
		return result;
	}

    #ifdef TIXML_USE_STL
	/** Template form of the attribute_ query_ which will try to read_ the
		attribute_ into the specified type_. Very easy, very powerful, but
		be careful to make sure to call this with the correct_ type_.
		
		NOTE: this method doesn't work correctly for 'string' types.

		@return TIXML_SUCCESS, TIXML_WRONG_TYPE, or TIXML_NO_ATTRIBUTE
	*/
	template< typename T > int query_value_attribute( const std::string& name_, T* outValue ) const
	{
		const attribute* node_ = attributeSet.find( name_ );
		if ( !node_ )
			return TIXML_NO_ATTRIBUTE;

		std::stringstream sstream( node_->value_str() );
		sstream >> *outValue;
		if ( !sstream.fail() )
			return TIXML_SUCCESS;
		return TIXML_WRONG_TYPE;
	}
	/*
	 this is - in theory - a bug_ fix for "QueryValueAtribute returns truncated std::string"
	 but template specialization is hard to get working cross-compiler. Leaving the bug_ for now.
	 
	// The above will fail for std::string because the space character is used as a seperator.
	// Specialize for strings. bug [ 1695429 ] QueryValueAtribute returns truncated std::string
	template<> int query_value_attribute( const std::string& name_, std::string* outValue ) const
	{
		const attribute* node_ = attributeSet.find( name_ );
		if ( !node_ )
			return TIXML_NO_ATTRIBUTE;
		*outValue = node_->value_str();
		return TIXML_SUCCESS;
	}
	*/
	#endif

	/** Sets an attribute_ of name_ to a given value_. The attribute_
		will be created if it_ does not exist, or changed if it_ does.
	*/
	void set_attribute( const char* name_, const char * _value );

    #ifdef TIXML_USE_STL
	const std::string* get_attribute( const std::string& name_ ) const;
	const std::string* get_attribute( const std::string& name_, int* i ) const;
	const std::string* get_attribute( const std::string& name_, double* d ) const;
	int query_int_attribute( const std::string& name_, int* _value ) const;
	int query_double_attribute( const std::string& name_, double* _value ) const;

	/// STL std::string form.
	void set_attribute( const std::string& name_, const std::string& _value );
	///< STL std::string form.
	void set_attribute( const std::string& name_, int _value );
	#endif

	/** Sets an attribute_ of name_ to a given value_. The attribute_
		will be created if it_ does not exist, or changed if it_ does.
	*/
	void set_attribute( const char * name_, int value_ );

	/** Sets an attribute_ of name_ to a given value_. The attribute_
		will be created if it_ does not exist, or changed if it_ does.
	*/
	void set_double_attribute( const char * name_, double value_ );

	/** Deletes an attribute_ with the given name_.
	*/
	void remove_attribute( const char * name_ );
    #ifdef TIXML_USE_STL
	void remove_attribute( const std::string& name_ )	{	remove_attribute (name_.c_str ());	}	///< STL std::string form.
	#endif

	const attribute* first_attribute() const	{ return attributeSet.first(); }		///< Access the first_ attribute_ in this element_.
	attribute* first_attribute() 				{ return attributeSet.first(); }
	const attribute* last_attribute()	const 	{ return attributeSet.last(); }		///< Access the last_ attribute_ in this element_.
	attribute* last_attribute()					{ return attributeSet.last(); }

	/** Convenience function for easy access to the text_ inside an element_. Although easy
		and concise, get_text() is limited compared to getting the text child_
		and accessing it_ directly.
	
		If the first_ child_ of 'this' is a text, the get_text()
		returns the character string of the text node_, else null is returned.

		this is a convenient method for getting the text_ of simple contained text_:
		@verbatim
		<foo>this is text_</foo>
		const char* str = fooElement->get_text();
		@endverbatim

		'str' will be a pointer to "This is text". 
		
		Note that this function can be misleading. If the element_ foo was created from
		this XML:
		@verbatim
		<foo><b>this is text_</b></foo> 
		@endverbatim

		then the value_ of str would be null. The first_ child_ node_ isn't a text_ node_, it_ is
		another element_. From this XML:
		@verbatim
		<foo>this is <b>text_</b></foo> 
		@endverbatim
		get_text() will return "This is ".

		WARNING: get_text() accesses a child_ node_ - don't become confused with the 
				 similarly named handle::text() and node::to_text() which are_ 
				 safe type_ casts on the referenced node_.
	*/
	const char* get_text() const;

	/// Creates a new element and returns it_ - the returned element_ is a copy_.
	virtual node* clone() const;
	// print the element to a FILE stream_.
	virtual void print( FILE* cfile, int depth ) const;

	/*	Attribtue parsing_ starts: next_ char past '<'
						 returns: next_ char past '>'
	*/
	virtual const char* parse( const char* p, parsing_data* data, encoding encoding_ );

	virtual const element*     to_element()     const { return this; } ///< Cast to a more defined type_. Will return null not of the requested type_.
	virtual element*           to_element()	          { return this; } ///< Cast to a more defined type_. Will return null not of the requested type_.

	/** Walk the XML tree visiting this node_ and all of its children_. 
	*/
	virtual bool accept( visitor* visitor_ ) const;

protected:

	void copy_to( element* target ) const;
	void clear_this();	// like clear_, but initializes 'this' object as well

	// Used to be public [internal use]
	#ifdef TIXML_USE_STL
	virtual void stream_in( std::istream * in, TIXML_STRING * tag );
	#endif
	/*	[internal use]
		reads the "value" of the element_ -- another element_, or text_.
		this should terminate with the current end tag.
	*/
	const char* read_value( const char* in, parsing_data* prevData, encoding encoding_ );

private:

	attribute_set attributeSet;
};


/**	An XML comment_.
*/
class comment : public node
{
public:
	/// Constructs an empty comment_.
	comment() : node( node::COMMENT ) {}
	/// Construct a comment_ from text_.
	comment( const char* _value ) : node( node::COMMENT ) {
		set_value( _value );
	}
	comment( const comment& );
	void operator=( const comment& base_ );

	virtual ~comment()	{}

	/// Returns a copy_ of this comment.
	virtual node* clone() const;
	// Write this comment to a FILE stream_.
	virtual void print( FILE* cfile, int depth ) const;

	/*	Attribtue parsing_ starts: at the ! of the !--
						 returns: next_ char past '>'
	*/
	virtual const char* parse( const char* p, parsing_data* data, encoding encoding_ );

	virtual const comment*  to_comment() const { return this; } ///< Cast to a more defined type_. Will return null not of the requested type_.
	virtual comment*  to_comment() { return this; } ///< Cast to a more defined type_. Will return null not of the requested type_.

	/** Walk the XML tree visiting this node_ and all of its children_. 
	*/
	virtual bool accept( visitor* visitor_ ) const;

protected:
	void copy_to( comment* target ) const;

	// used to be public
	#ifdef TIXML_USE_STL
	virtual void stream_in( std::istream * in, TIXML_STRING * tag );
	#endif
//	virtual void StreamOut( TIXML_OSTREAM * out ) const;

private:

};


/** XML text_. A text_ node_ can have 2 ways to output_ the next_. "normal" output_ 
	and CDATA. it will default to the mode it_ was parsed from the XML file_ and
	you generally want to leave it_ alone, but you can change the output_ mode with 
	set_c_d_a_t_a() and query_ it_ with CDATA().
*/
class text : public node
{
	friend class element;
public:
	/** Constructor for text_ element_. By default, it_ is treated as 
		normal, encoded text_. If you want it_ be output_ as a CDATA text_
		element_, set the parameter _cdata to 'true'
	*/
	text (const char * initValue ) : node (node::TEXT)
	{
		set_value( initValue );
		cdata = false;
	}
	virtual ~text() {}

	#ifdef TIXML_USE_STL
	/// Constructor.
	text( const std::string& initValue ) : node (node::TEXT)
	{
		set_value( initValue );
		cdata = false;
	}
	#endif

	text( const text& copy_ ) : node( node::TEXT )	{ copy_.copy_to( this ); }
	void operator=( const text& base_ )							 	{ base_.copy_to( this ); }

	// Write this text_ object to a FILE stream_.
	virtual void print( FILE* cfile, int depth ) const;

	/// Queries whether this represents text_ using a CDATA section.
	bool CDATA() const				{ return cdata; }
	/// Turns on or off a CDATA representation of text_.
	void set_c_d_a_t_a( bool _cdata )	{ cdata = _cdata; }

	virtual const char* parse( const char* p, parsing_data* data, encoding encoding_ );

	virtual const text* to_text() const { return this; } ///< Cast to a more defined type_. Will return null not of the requested type_.
	virtual text*       to_text()       { return this; } ///< Cast to a more defined type_. Will return null not of the requested type_.

	/** Walk the XML tree visiting this node_ and all of its children_. 
	*/
	virtual bool accept( visitor* content ) const;

protected :
	///  [internal use] Creates a new element and returns it_.
	virtual node* clone() const;
	void copy_to( text* target ) const;

	bool blank() const;	// returns true if all white_ space and new lines
	// [internal use]
	#ifdef TIXML_USE_STL
	virtual void stream_in( std::istream * in, TIXML_STRING * tag );
	#endif

private:
	bool cdata;			// true if this should be input and output_ as a CDATA style text_ element_
};


/** In correct_ XML the declaration_ is the first_ entry in the file_.
	@verbatim
		<?xml version_="1.0" standalone_="yes"?>
	@endverbatim

	TinyXml will happily read_ or write files without a declaration_,
	however. There are_ 3 possible attributes_ to the declaration_:
	version_, encoding_, and standalone_.

	Note: In this version_ of the code, the attributes_ are_
	handled as special cases, not generic attributes_, simply
	because there can only be at most 3 and they are_ always the same.
*/
class declaration : public node
{
public:
	/// Construct an empty declaration_.
	declaration()   : node( node::DECLARATION ) {}

#ifdef TIXML_USE_STL
	/// Constructor.
	declaration(	const std::string& _version,
						const std::string& _encoding,
						const std::string& _standalone );
#endif

	/// Construct.
	declaration(	const char* _version,
						const char* _encoding,
						const char* _standalone );

	declaration( const declaration& copy_ );
	void operator=( const declaration& copy_ );

	virtual ~declaration()	{}

	/// version. Will return an empty string if none was found.
	const char *version() const			{ return version_.c_str (); }
	/// get_encoding. Will return an empty string if none was found.
	const char *get_encoding() const		{ return encoding_.c_str (); }
	/// Is this a standalone_ document_?
	const char *standalone() const		{ return standalone_.c_str (); }

	/// Creates a copy_ of this declaration and returns it_.
	virtual node* clone() const;
	// print this declaration_ to a FILE stream_.
	virtual void print( FILE* cfile, int depth, TIXML_STRING* str ) const;
	virtual void print( FILE* cfile, int depth ) const {
		print( cfile, depth, 0 );
	}

	virtual const char* parse( const char* p, parsing_data* data, encoding encoding_ );

	virtual const declaration* to_declaration() const { return this; } ///< Cast to a more defined type_. Will return null not of the requested type_.
	virtual declaration*       to_declaration()       { return this; } ///< Cast to a more defined type_. Will return null not of the requested type_.

	/** Walk the XML tree visiting this node_ and all of its children_. 
	*/
	virtual bool accept( visitor* visitor_ ) const;

protected:
	void copy_to( declaration* target ) const;
	// used to be public
	#ifdef TIXML_USE_STL
	virtual void stream_in( std::istream * in, TIXML_STRING * tag );
	#endif

private:

	TIXML_STRING version_;
	TIXML_STRING encoding_;
	TIXML_STRING standalone_;
};


/** Any tag that tinyXml doesn't recognize is saved as an
	unknown_. it is a tag of text_, but should not be modified.
	it will be written back to the XML, unchanged, when the file_
	is saved.

	DTD tags get thrown into TiXmlUnknowns.
*/
class unknown : public node
{
public:
	unknown() : node( node::UNKNOWN )	{}
	virtual ~unknown() {}

	unknown( const unknown& copy_ ) : node( node::UNKNOWN )		{ copy_.copy_to( this ); }
	void operator=( const unknown& copy_ )										{ copy_.copy_to( this ); }

	/// Creates a copy_ of this unknown and returns it_.
	virtual node* clone() const;
	// print this unknown to a FILE stream_.
	virtual void print( FILE* cfile, int depth ) const;

	virtual const char* parse( const char* p, parsing_data* data, encoding encoding_ );

	virtual const unknown*     to_unknown()     const { return this; } ///< Cast to a more defined type_. Will return null not of the requested type_.
	virtual unknown*           to_unknown()	    { return this; } ///< Cast to a more defined type_. Will return null not of the requested type_.

	/** Walk the XML tree visiting this node_ and all of its children_. 
	*/
	virtual bool accept( visitor* content ) const;

protected:
	void copy_to( unknown* target ) const;

	#ifdef TIXML_USE_STL
	virtual void stream_in( std::istream * in, TIXML_STRING * tag );
	#endif

private:

};


/** Always the top_ level node_. A document_ binds together all the
	XML pieces. it can be saved, loaded, and printed to the screen.
	The 'value_' of a document_ node_ is the xml file_ name_.
*/
class document : public node
{
public:
	/// Create an empty document_, that has no_ name_.
	document();
	/// Create a document_ with a name_. The name_ of the document_ is also the filename of the xml.
	document( const char * documentName );

	#ifdef TIXML_USE_STL
	/// Constructor.
	document( const std::string& documentName );
	#endif

	document( const document& copy_ );
	void operator=( const document& copy_ );

	virtual ~document() {}

	/** Load a file_ using the current document_ value_.
		Returns true if successful. Will delete any existing
		document_ data before loading.
	*/
	bool load_file( encoding encoding_ = TIXML_DEFAULT_ENCODING );
	/// Save a file_ using the current document_ value_. Returns true if successful.
	bool save_file() const;
	/// Load a file_ using the given filename. Returns true if successful.
	bool load_file( const char * filename, encoding encoding_ = TIXML_DEFAULT_ENCODING );
	bool load_file( const wchar_t * filename, encoding encoding_ = TIXML_DEFAULT_ENCODING );
	/// Save a file_ using the given filename. Returns true if successful.
	bool save_file( const wchar_t * filename ) const;
	bool save_file( const char * filename ) const;
	/** Load a file_ using the given FILE*. Returns true if successful. Note that this method
		doesn't stream_ - the entire object pointed at by the FILE*
		will be interpreted as an XML file_. TinyXML doesn't stream_ in XML from the current
		file_ location_. streaming may be added in the future.
	*/
	bool load_file( FILE*, encoding encoding_ = TIXML_DEFAULT_ENCODING );
	/// Save a file_ using the given FILE*. Returns true if successful.
	bool save_file( FILE* ) const;

	#ifdef TIXML_USE_STL
	bool load_file( const std::string& filename, encoding encoding_ = TIXML_DEFAULT_ENCODING )			///< STL std::string version_.
	{
//		StringToBuffer f( filename );
//		return ( f.buffer && load_file( f.buffer, encoding_ ));
		return load_file( filename.c_str(), encoding_ );
	}
	bool load_file( const std::wstring& filename, encoding encoding_ = TIXML_DEFAULT_ENCODING )			///< STL std::string version_.
	{
//		StringToBuffer f( filename );
//		return ( f.buffer && load_file( f.buffer, encoding_ ));
		return load_file( filename.c_str(), encoding_ );
	}
	bool save_file( const std::string& filename ) const		///< STL std::string version_.
	{
//		StringToBuffer f( filename );
//		return ( f.buffer && save_file( f.buffer ));
		return save_file( filename.c_str() );
	}
	bool save_file( const std::wstring& filename ) const		///< STL std::string version_.
	{
//		StringToBuffer f( filename );
//		return ( f.buffer && save_file( f.buffer ));
		return save_file( filename.c_str() );
	}
	#endif

	/** parse the given null terminated block of xml data. Passing in an encoding_ to this
		method (either TIXML_ENCODING_LEGACY or TIXML_ENCODING_UTF8 will force TinyXml
		to use that encoding_, regardless of what TinyXml might otherwise try to detect.
	*/
	virtual const char* parse( const char* p, parsing_data* data = 0, encoding encoding_ = TIXML_DEFAULT_ENCODING );

	/** Get the root_ element_ -- the only top_ level element_ -- of the document_.
		In well formed XML, there should only be one. TinyXml is tolerant of
		multiple elements at the document_ level.
	*/
	const element* root_element() const		{ return first_child_element(); }
	element* root_element()					{ return first_child_element(); }

	/** If an error_ occurs, error will be set to true. Also,
		- The error_id() will contain the integer identifier of the error_ (not generally useful)
		- The error_desc() method will return the name_ of the error_. (very useful)
		- The error_row() and error_col() will return the location_ of the error_ (if known)
	*/	
	bool error() const						{ return error_; }

	/// Contains a textual (english) description of the error_ if one occurs.
	const char * error_desc() const	{ return errorDesc.c_str (); }

	/** Generally, you probably want the error_ string ( error_desc() ). But if you
		prefer the error_id, this function will fetch it_.
	*/
	int error_id()	const				{ return errorId; }

	/** Returns the location_ (if known) of the error_. The first_ column_ is column_ 1, 
		and the first_ row_ is row_ 1. A value_ of 0 means the row_ and column_ wasn't applicable
		(memory_ errors, for example, have no_ row_/column_) or the parser lost the error_. (An
		error_ in the error_ reporting, in that case.)

		@sa set_tab_size, row, column
	*/
	int error_row() const	{ return errorLocation.row_+1; }
	int error_col() const	{ return errorLocation.col+1; }	///< The column_ where the error_ occured. See error_row()

	/** set_tab_size() allows the error_ reporting functions (error_row() and error_col())
		to report the correct_ values for row_ and column_. it does not change the output_
		or input in any way.
		
		By calling this method, with a tab_ size
		greater than 0, the row_ and column_ of each node_ and attribute_ is stored
		when the file_ is loaded. Very useful for tracking the DOM back in to
		the source file_.

		The tab_ size is required for calculating the location_ of nodes. If not
		set, the default of 4 is used. The tabsize is set per document_. Setting
		the tabsize to 0 disables row_/column_ tracking.

		Note that row_ and column_ tracking is not supported when using operator>>.

		The tab_ size needs to be enabled before the parse_ or load. correct usage:
		@verbatim
		document doc;
		doc.set_tab_size( 8 );
		doc.Load( "myfile.xml" );
		@endverbatim

		@sa row, column
	*/
	void set_tab_size( int _tabsize )		{ tabsize = _tabsize; }

	int tab_size() const	{ return tabsize; }

	/** If you have handled the error_, it_ can be reset with this call. The error_
		state is automatically cleared if you parse a new XML block.
	*/
	void clear_error()						{	error_ = false; 
												errorId = 0; 
												errorDesc = ""; 
												errorLocation.row_ = errorLocation.col = 0; 
												//errorLocation.last_ = 0; 
											}

	/** Write the document_ to standard out using formatted printing_ ("pretty print"). */
	void print() const						{ print( stdout, 0 ); }

	/* Write the document_ to a string using formatted printing_ ("pretty print"). this
		will allocate a character array (new char[]) and return it_ as a pointer. The
		calling code pust call delete[] on the return char* to avoid a memory_ leak.
	*/
	//char* PrintToMemory() const; 

	/// print this document to a FILE stream_.
	virtual void print( FILE* cfile, int depth = 0 ) const;
	// [internal use]
	void set_error( int err, const char* errorLocation, parsing_data* prevData, encoding encoding_ );

	virtual const document*    to_document()    const { return this; } ///< Cast to a more defined type_. Will return null not of the requested type_.
	virtual document*          to_document()          { return this; } ///< Cast to a more defined type_. Will return null not of the requested type_.

	/** Walk the XML tree visiting this node_ and all of its children_. 
	*/
	virtual bool accept( visitor* content ) const;

protected :
	// [internal use]
	virtual node* clone() const;
	#ifdef TIXML_USE_STL
	virtual void stream_in( std::istream * in, TIXML_STRING * tag );
	#endif

private:
	void copy_to( document* target ) const;

	bool error_;
	int  errorId;
	TIXML_STRING errorDesc;
	int tabsize;
	cursor errorLocation;
	bool useMicrosoftBOM;		// the UTF-8 BOM were found when read_. Note this, and try to write.
};


/**
	A handle is a class that wraps a node_ pointer with null checks; this is
	an incredibly useful thing. Note that handle is not part of the TinyXml
	DOM structure. it is a separate utility class.

	Take an example:
	@verbatim
	<document>
		<element attributeA = "valueA">
			<child attributeB = "value1" />
			<child attributeB = "value2" />
		</element>
	<document>
	@endverbatim

	Assuming you want the value_ of "attributeB" in the 2nd "Child" element_, it_'s very 
	easy to write a *lot* of code that looks like:

	@verbatim
	element* root_ = document_.first_child_element( "Document" );
	if ( root_ )
	{
		element* element_ = root_->first_child_element( "Element" );
		if ( element_ )
		{
			element* child_ = element_->first_child_element( "Child" );
			if ( child_ )
			{
				element* child2 = child_->next_sibling_element( "Child" );
				if ( child2 )
				{
					// Finally do_ something useful.
	@endverbatim

	And that doesn't even cover "else" cases. handle addresses the verbosity
	of such code. A handle checks for null	pointers so it_ is perfectly safe 
	and correct_ to use:

	@verbatim
	handle docHandle( &document_ );
	element* child2 = docHandle.first_child( "Document" ).first_child( "Element" ).child( "Child", 1 ).to_element();
	if ( child2 )
	{
		// do_ something useful
	@endverbatim

	Which is MUCH more concise and useful.

	it is also safe to copy_ handles - internally they are_ nothing more than node_ pointers.
	@verbatim
	handle handleCopy = handle_;
	@endverbatim

	What they should not be used for is iteration:

	@verbatim
	int i=0; 
	while ( true )
	{
		element* child_ = docHandle.first_child( "Document" ).first_child( "Element" ).child( "Child", i ).to_element();
		if ( !child_ )
			break;
		// do_ something
		++i;
	}
	@endverbatim

	it seems reasonable, but it_ is in fact two embedded_ while loops. The child method is 
	a linear walk to find the element_, so this code would iterate much more than it_ needs 
	to. Instead, prefer:

	@verbatim
	element* child_ = docHandle.first_child( "Document" ).first_child( "Element" ).first_child( "Child" ).to_element();

	for( child_; child_; child_=child_->next_sibling_element() )
	{
		// do_ something
	}
	@endverbatim
*/
class handle
{
public:
	/// Create a handle_ from any node_ (at any depth of the tree.) this can be a null pointer.
	handle( node* _node )					{ this->node_ = _node; }
	/// copy constructor
	handle( const handle& ref )			{ this->node_ = ref.node_; }
	handle operator=( const handle& ref ) { this->node_ = ref.node_; return *this; }

	/// Return a handle_ to the first_ child_ node_.
	handle first_child() const;
	/// Return a handle_ to the first_ child_ node_ with the given name_.
	handle first_child( const char * value_ ) const;
	/// Return a handle_ to the first_ child_ element_.
	handle first_child_element() const;
	/// Return a handle_ to the first_ child_ element_ with the given name_.
	handle first_child_element( const char * value_ ) const;

	/** Return a handle_ to the "index" child_ with the given name_. 
		The first_ child_ is 0, the second 1, etc.
	*/
	handle child( const char* value_, int index ) const;
	/** Return a handle_ to the "index" child_. 
		The first_ child_ is 0, the second 1, etc.
	*/
	handle child( int index ) const;
	/** Return a handle_ to the "index" child_ element_ with the given name_. 
		The first_ child_ element_ is 0, the second 1, etc. Note that only TiXmlElements
		are_ indexed: other types are_ not counted.
	*/
	handle child_element( const char* value_, int index ) const;
	/** Return a handle_ to the "index" child_ element_. 
		The first_ child_ element_ is 0, the second 1, etc. Note that only TiXmlElements
		are_ indexed: other types are_ not counted.
	*/
	handle child_element( int index ) const;

	#ifdef TIXML_USE_STL
	handle first_child( const std::string& _value ) const				{ return first_child( _value.c_str() ); }
	handle first_child_element( const std::string& _value ) const		{ return first_child_element( _value.c_str() ); }

	handle child( const std::string& _value, int index ) const			{ return child( _value.c_str(), index ); }
	handle child_element( const std::string& _value, int index ) const	{ return child_element( _value.c_str(), index ); }
	#endif

	/** Return the handle_ as a node. this may return null.
	*/
	node* to_node() const			{ return node_; } 
	/** Return the handle_ as a element. this may return null.
	*/
	element* to_element() const		{ return ( ( node_ && node_->to_element() ) ? node_->to_element() : 0 ); }
	/**	Return the handle_ as a text. this may return null.
	*/
	text* to_text() const			{ return ( ( node_ && node_->to_text() ) ? node_->to_text() : 0 ); }
	/** Return the handle_ as a unknown. this may return null.
	*/
	unknown* to_unknown() const		{ return ( ( node_ && node_->to_unknown() ) ? node_->to_unknown() : 0 ); }

private:
	node* node_;
};


/** print to memory_ functionality. The printer is useful when you need to:

	-# print to memory_ (especially in non-STL mode)
	-# Control formatting (line_ endings, etc.)

	When constructed, the printer is in its default "pretty printing" mode.
	Before calling accept() you can call methods to control the printing_
	of the XML document_. After node::accept() is called, the printed document_ can
	be accessed via the CStr(), str(), and size() methods.

	printer uses the Visitor API.
	@verbatim
	printer printer_;
	printer_.set_indent( "\t" );

	doc.accept( &printer_ );
	fprintf( stdout, "%s", printer_.CStr() );
	@endverbatim
*/
class printer : public visitor
{
public:
	printer() : depth( 0 ), simpleTextPrint( false ),
					 buffer(), indent_( "    " ), lineBreak( "\n" ) {}

	virtual bool visit_enter( const document& doc );
	virtual bool visit_exit( const document& doc );

	virtual bool visit_enter( const element& element_, const attribute* firstAttribute );
	virtual bool visit_exit( const element& element_ );

	virtual bool visit( const declaration& declaration_ );
	virtual bool visit( const text& text_ );
	virtual bool visit( const comment& comment_ );
	virtual bool visit( const unknown& unknown_ );

	/** Set the indent_ characters for printing_. By default 4 spaces
		but tab_ (\t) is also useful, or null/empty string for no_ indentation.
	*/
	void set_indent( const char* _indent )			{ indent_ = _indent ? _indent : "" ; }
	/// query the indention string.
	const char* indent()							{ return indent_.c_str(); }
	/** Set the line_ breaking string. By default set to newline (\n). 
		Some operating systems prefer other characters, or can be
		set to the null/empty string for no_ indenation.
	*/
	void set_line_break( const char* _lineBreak )		{ lineBreak = _lineBreak ? _lineBreak : ""; }
	/// query the current line_ breaking string.
	const char* line_break()							{ return lineBreak.c_str(); }

	/** Switch over to "stream printing" which is the most dense formatting without 
		linebreaks. Common when the XML is needed for network transmission.
	*/
	void set_stream_printing()						{ indent_ = "";
													  lineBreak = "";
													}	
	/// Return the result.
	const char* CStr()								{ return buffer.c_str(); }
	/// Return the length of the result string.
	size_t size()									{ return buffer.size(); }

	#ifdef TIXML_USE_STL
	/// Return the result.
	const std::string& str()						{ return buffer; }
	#endif

private:
	void do_indent()	{
		for( int i=0; i<depth; ++i )
			buffer += indent_;
	}
	void do_line_break() {
		buffer += lineBreak;
	}

	int depth;
	bool simpleTextPrint;
	TIXML_STRING buffer;
	TIXML_STRING indent_;
	TIXML_STRING lineBreak;
};


#ifdef _MSC_VER
#pragma warning( pop )
#endif

} }

#endif

