/*
www.sourceforge.net/projects/tinyxml
Original code (2.0 and earlier )copyright (c) 2000-2006 Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied
warranty. In no_ event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#include <ctype.h>
#include <wchar.h>

#ifdef TIXML_USE_STL
#include <sstream>
#include <iostream>
#endif

#include "tinyxml.hpp"


namespace tinyxml {

bool base::condenseWhiteSpace = true;

void base::put_string( const TIXML_STRING& str, TIXML_STRING* outString )
{
	int i=0;

	while( i<(int)str.length() )
	{
		unsigned char c = (unsigned char) str[i];

		if (    c == '&' 
		     && i < ( (int)str.length() - 2 )
			 && str[i+1] == '#'
			 && str[i+2] == 'x' )
		{
			// Hexadecimal character reference.
			// Pass through unchanged.
			// &#xA9;	-- copyright symbol, for example.
			//
			// The -1 is a bug fix from Rob Laveaux. It keeps
			// an overflow from happening if there is no_ ';'.
			// There are actually 2 ways to exit this loop -
			// while fails (error_ case) and break (semicolon found).
			// However, there is no_ mechanism (currently) for
			// this function to return an error_.
			while ( i<(int)str.length()-1 )
			{
				outString->append( str.c_str() + i, 1 );
				++i;
				if ( str[i] == ';' )
					break;
			}
		}
		else if ( c == '&' )
		{
			outString->append( entity_[0].str, entity_[0].strLength );
			++i;
		}
		else if ( c == '<' )
		{
			outString->append( entity_[1].str, entity_[1].strLength );
			++i;
		}
		else if ( c == '>' )
		{
			outString->append( entity_[2].str, entity_[2].strLength );
			++i;
		}
		else if ( c == '\"' )
		{
			outString->append( entity_[3].str, entity_[3].strLength );
			++i;
		}
		else if ( c == '\'' )
		{
			outString->append( entity_[4].str, entity_[4].strLength );
			++i;
		}
		else if ( c < 32 )
		{
			// Easy pass at non-alpha/numeric/symbol
			// Below 32 is symbolic.
			char buf[ 32 ];
			
			#if defined(TIXML_SNPRINTF)		
				TIXML_SNPRINTF( buf, sizeof(buf), "&#x%02X;", (unsigned) ( c & 0xff ) );
			#else
				sprintf( buf, "&#x%02X;", (unsigned) ( c & 0xff ) );
			#endif		

			//*ME:	warning C4267: convert 'size_t' to 'int'
			//*ME:	Int-Cast to make compiler happy ...
			outString->append( buf, (int)strlen( buf ) );
			++i;
		}
		else
		{
			//char realc = (char) c;
			//outString->append( &realc, 1 );
			*outString += (char) c;	// somewhat more efficient function call.
			++i;
		}
	}
}


node::node( node_type _type ) : base()
{
	parent_ = 0;
	type_ = _type;
	firstChild = 0;
	lastChild = 0;
	prev = 0;
	next_ = 0;
}


node::~node()
{
	node* node_ = firstChild;
	node* temp = 0;

	while ( node_ )
	{
		temp = node_;
		node_ = node_->next_;
		delete temp;
	}	
}


void node::copy_to( node* target ) const
{
	target->set_value (value_.c_str() );
	target->userData = userData; 
}


void node::clear()
{
	node* node_ = firstChild;
	node* temp = 0;

	while ( node_ )
	{
		temp = node_;
		node_ = node_->next_;
		delete temp;
	}	

	firstChild = 0;
	lastChild = 0;
}


node* node::link_end_child( node* node_ )
{
	assert( node_->parent_ == 0 || node_->parent_ == this );
	assert( node_->get_document() == 0 || node_->get_document() == this->get_document() );

	if ( node_->type() == node::DOCUMENT )
	{
		delete node_;
		if ( get_document() ) get_document()->set_error( TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return 0;
	}

	node_->parent_ = this;

	node_->prev = lastChild;
	node_->next_ = 0;

	if ( lastChild )
		lastChild->next_ = node_;
	else
		firstChild = node_;			// it was an empty list.

	lastChild = node_;
	return node_;
}


node* node::insert_end_child( const node& addThis )
{
	if ( addThis.type() == node::DOCUMENT )
	{
		if ( get_document() ) get_document()->set_error( TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return 0;
	}
	node* node_ = addThis.clone();
	if ( !node_ )
		return 0;

	return link_end_child( node_ );
}


node* node::insert_before_child( node* beforeThis, const node& addThis )
{	
	if ( !beforeThis || beforeThis->parent_ != this ) {
		return 0;
	}
	if ( addThis.type() == node::DOCUMENT )
	{
		if ( get_document() ) get_document()->set_error( TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return 0;
	}

	node* node_ = addThis.clone();
	if ( !node_ )
		return 0;
	node_->parent_ = this;

	node_->next_ = beforeThis;
	node_->prev = beforeThis->prev;
	if ( beforeThis->prev )
	{
		beforeThis->prev->next_ = node_;
	}
	else
	{
		assert( firstChild == beforeThis );
		firstChild = node_;
	}
	beforeThis->prev = node_;
	return node_;
}


node* node::insert_after_child( node* afterThis, const node& addThis )
{
	if ( !afterThis || afterThis->parent_ != this ) {
		return 0;
	}
	if ( addThis.type() == node::DOCUMENT )
	{
		if ( get_document() ) get_document()->set_error( TIXML_ERROR_DOCUMENT_TOP_ONLY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return 0;
	}

	node* node_ = addThis.clone();
	if ( !node_ )
		return 0;
	node_->parent_ = this;

	node_->prev = afterThis;
	node_->next_ = afterThis->next_;
	if ( afterThis->next_ )
	{
		afterThis->next_->prev = node_;
	}
	else
	{
		assert( lastChild == afterThis );
		lastChild = node_;
	}
	afterThis->next_ = node_;
	return node_;
}


node* node::replace_child( node* replaceThis, const node& withThis )
{
	if ( replaceThis->parent_ != this )
		return 0;

	node* node_ = withThis.clone();
	if ( !node_ )
		return 0;

	node_->next_ = replaceThis->next_;
	node_->prev = replaceThis->prev;

	if ( replaceThis->next_ )
		replaceThis->next_->prev = node_;
	else
		lastChild = node_;

	if ( replaceThis->prev )
		replaceThis->prev->next_ = node_;
	else
		firstChild = node_;

	delete replaceThis;
	node_->parent_ = this;
	return node_;
}


bool node::remove_child( node* removeThis )
{
	if ( removeThis->parent_ != this )
	{	
		assert( 0 );
		return false;
	}

	if ( removeThis->next_ )
		removeThis->next_->prev = removeThis->prev;
	else
		lastChild = removeThis->prev;

	if ( removeThis->prev )
		removeThis->prev->next_ = removeThis->next_;
	else
		firstChild = removeThis->next_;

	delete removeThis;
	return true;
}

const node* node::first_child( const char * _value ) const
{
	const node* node_;
	for ( node_ = firstChild; node_; node_ = node_->next_ )
	{
		if ( strcmp( node_->value(), _value ) == 0 )
			return node_;
	}
	return 0;
}


const node* node::last_child( const char * _value ) const
{
	const node* node_;
	for ( node_ = lastChild; node_; node_ = node_->prev )
	{
		if ( strcmp( node_->value(), _value ) == 0 )
			return node_;
	}
	return 0;
}


const node* node::iterate_children( const node* previous_ ) const
{
	if ( !previous_ )
	{
		return first_child();
	}
	else
	{
		assert( previous_->parent_ == this );
		return previous_->next_sibling();
	}
}


const node* node::iterate_children( const char * val, const node* previous_ ) const
{
	if ( !previous_ )
	{
		return first_child( val );
	}
	else
	{
		assert( previous_->parent_ == this );
		return previous_->next_sibling( val );
	}
}


const node* node::next_sibling( const char * _value ) const 
{
	const node* node_;
	for ( node_ = next_; node_; node_ = node_->next_ )
	{
		if ( strcmp( node_->value(), _value ) == 0 )
			return node_;
	}
	return 0;
}


const node* node::previous_sibling( const char * _value ) const
{
	const node* node_;
	for ( node_ = prev; node_; node_ = node_->prev )
	{
		if ( strcmp( node_->value(), _value ) == 0 )
			return node_;
	}
	return 0;
}


void element::remove_attribute( const char * name_ )
{
    #ifdef TIXML_USE_STL
	TIXML_STRING str( name_ );
	attribute* node_ = attributeSet.find( str );
	#else
	attribute* node_ = attributeSet.find( name_ );
	#endif
	if ( node_ )
	{
		attributeSet.remove( node_ );
		delete node_;
	}
}

const element* node::first_child_element() const
{
	const node* node_;

	for (	node_ = first_child();
			node_;
			node_ = node_->next_sibling() )
	{
		if ( node_->to_element() )
			return node_->to_element();
	}
	return 0;
}


const element* node::first_child_element( const char * _value ) const
{
	const node* node_;

	for (	node_ = first_child( _value );
			node_;
			node_ = node_->next_sibling( _value ) )
	{
		if ( node_->to_element() )
			return node_->to_element();
	}
	return 0;
}


const element* node::next_sibling_element() const
{
	const node* node_;

	for (	node_ = next_sibling();
			node_;
			node_ = node_->next_sibling() )
	{
		if ( node_->to_element() )
			return node_->to_element();
	}
	return 0;
}


const element* node::next_sibling_element( const char * _value ) const
{
	const node* node_;

	for (	node_ = next_sibling( _value );
			node_;
			node_ = node_->next_sibling( _value ) )
	{
		if ( node_->to_element() )
			return node_->to_element();
	}
	return 0;
}


const document* node::get_document() const
{
	const node* node_;

	for( node_ = this; node_; node_ = node_->parent_ )
	{
		if ( node_->to_document() )
			return node_->to_document();
	}
	return 0;
}


element::element (const char * _value)
	: node( node::ELEMENT )
{
	firstChild = lastChild = 0;
	value_ = _value;
}


#ifdef TIXML_USE_STL
element::element( const std::string& _value ) 
	: node( node::ELEMENT )
{
	firstChild = lastChild = 0;
	value_ = _value;
}
#endif


element::element( const element& copy)
	: node( node::ELEMENT )
{
	firstChild = lastChild = 0;
	copy.copy_to( this );	
}


void element::operator=( const element& base_ )
{
	clear_this();
	base_.copy_to( this );
}


element::~element()
{
	clear_this();
}


void element::clear_this()
{
	clear();
	while( attributeSet.first() )
	{
		attribute* node_ = attributeSet.first();
		attributeSet.remove( node_ );
		delete node_;
	}
}


const char* element::get_attribute( const char* name_ ) const
{
	const attribute* node_ = attributeSet.find( name_ );
	if ( node_ )
		return node_->value();
	return 0;
}


#ifdef TIXML_USE_STL
const std::string* element::get_attribute( const std::string& name_ ) const
{
	const attribute* node_ = attributeSet.find( name_ );
	if ( node_ )
		return &node_->value_str();
	return 0;
}
#endif


const char* element::get_attribute( const char* name_, int* i ) const
{
	const char* s = get_attribute( name_ );
	if ( i )
	{
		if ( s ) {
			*i = atoi( s );
		}
		else {
			*i = 0;
		}
	}
	return s;
}


#ifdef TIXML_USE_STL
const std::string* element::get_attribute( const std::string& name_, int* i ) const
{
	const std::string* s = get_attribute( name_ );
	if ( i )
	{
		if ( s ) {
			*i = atoi( s->c_str() );
		}
		else {
			*i = 0;
		}
	}
	return s;
}
#endif


const char* element::get_attribute( const char* name_, double* d ) const
{
	const char* s = get_attribute( name_ );
	if ( d )
	{
		if ( s ) {
			*d = atof( s );
		}
		else {
			*d = 0;
		}
	}
	return s;
}


#ifdef TIXML_USE_STL
const std::string* element::get_attribute( const std::string& name_, double* d ) const
{
	const std::string* s = get_attribute( name_ );
	if ( d )
	{
		if ( s ) {
			*d = atof( s->c_str() );
		}
		else {
			*d = 0;
		}
	}
	return s;
}
#endif


int element::query_int_attribute( const char* name_, int* ival ) const
{
	const attribute* node_ = attributeSet.find( name_ );
	if ( !node_ )
		return TIXML_NO_ATTRIBUTE;
	return node_->query_int_value( ival );
}


#ifdef TIXML_USE_STL
int element::query_int_attribute( const std::string& name_, int* ival ) const
{
	const attribute* node_ = attributeSet.find( name_ );
	if ( !node_ )
		return TIXML_NO_ATTRIBUTE;
	return node_->query_int_value( ival );
}
#endif


int element::query_double_attribute( const char* name_, double* dval ) const
{
	const attribute* node_ = attributeSet.find( name_ );
	if ( !node_ )
		return TIXML_NO_ATTRIBUTE;
	return node_->query_double_value( dval );
}


#ifdef TIXML_USE_STL
int element::query_double_attribute( const std::string& name_, double* dval ) const
{
	const attribute* node_ = attributeSet.find( name_ );
	if ( !node_ )
		return TIXML_NO_ATTRIBUTE;
	return node_->query_double_value( dval );
}
#endif


void element::set_attribute( const char * name_, int val )
{	
	char buf[64];
	#if defined(TIXML_SNPRINTF)		
		TIXML_SNPRINTF( buf, sizeof(buf), "%d", val );
	#else
		sprintf( buf, "%d", val );
	#endif
	set_attribute( name_, buf );
}


#ifdef TIXML_USE_STL
void element::set_attribute( const std::string& name_, int val )
{	
   std::ostringstream oss;
   oss << val;
   set_attribute( name_, oss.str() );
}
#endif


void element::set_double_attribute( const char * name_, double val )
{	
	char buf[256];
	#if defined(TIXML_SNPRINTF)		
		TIXML_SNPRINTF( buf, sizeof(buf), "%f", val );
	#else
		sprintf( buf, "%f", val );
	#endif
	set_attribute( name_, buf );
}


void element::set_attribute( const char * cname, const char * cvalue )
{
    #ifdef TIXML_USE_STL
	TIXML_STRING _name( cname );
	TIXML_STRING _value( cvalue );
	#else
	const char* _name = cname;
	const char* _value = cvalue;
	#endif

	attribute* node_ = attributeSet.find( _name );
	if ( node_ )
	{
		node_->set_value( _value );
		return;
	}

	attribute* attrib = new attribute( cname, cvalue );
	if ( attrib )
	{
		attributeSet.add( attrib );
	}
	else
	{
		document* document_ = get_document();
		if ( document_ ) document_->set_error( TIXML_ERROR_OUT_OF_MEMORY, 0, 0, TIXML_ENCODING_UNKNOWN );
	}
}


#ifdef TIXML_USE_STL
void element::set_attribute( const std::string& name_, const std::string& _value )
{
	attribute* node_ = attributeSet.find( name_ );
	if ( node_ )
	{
		node_->set_value( _value );
		return;
	}

	attribute* attrib = new attribute( name_, _value );
	if ( attrib )
	{
		attributeSet.add( attrib );
	}
	else
	{
		document* document_ = get_document();
		if ( document_ ) document_->set_error( TIXML_ERROR_OUT_OF_MEMORY, 0, 0, TIXML_ENCODING_UNKNOWN );
	}
}
#endif


void element::print( FILE* cfile, int depth ) const
{
	int i;
	assert( cfile );
	for ( i=0; i<depth; i++ ) {
		fprintf( cfile, "    " );
	}

	fprintf( cfile, "<%s", value_.c_str() );

	const attribute* attrib;
	for ( attrib = attributeSet.first(); attrib; attrib = attrib->next() )
	{
		fprintf( cfile, " " );
		attrib->print( cfile, depth );
	}

	// There are 3 different formatting approaches:
	// 1) An element_ without children is printed as a <foo /> node_
	// 2) An element_ with only a text_ child_ is printed as <foo> text_ </foo>
	// 3) An element_ with children is printed on multiple lines.
	node* node_;
	if ( !firstChild )
	{
		#if defined (HAL_BOOST_SERIALIZATION_COMPAT)
		fprintf( cfile, "></%s>", value_.c_str() );		
		#else
		fprintf( cfile, " />" );
		#endif
	}
	else if ( firstChild == lastChild && firstChild->to_text() )
	{
		fprintf( cfile, ">" );
		firstChild->print( cfile, depth + 1 );
		fprintf( cfile, "</%s>", value_.c_str() );
	}
	else
	{
		fprintf( cfile, ">" );

		for ( node_ = firstChild; node_; node_=node_->next_sibling() )
		{
			if ( !node_->to_text() )
			{
				fprintf( cfile, "\n" );
			}
			node_->print( cfile, depth+1 );
		}
		fprintf( cfile, "\n" );
		for( i=0; i<depth; ++i ) {
			fprintf( cfile, "    " );
		}
		fprintf( cfile, "</%s>", value_.c_str() );
	}
}


void element::copy_to( element* target ) const
{
	// superclass:
	node::copy_to( target );

	// element class: 
	// clone the attributes_, then clone_ the children.
	const attribute* attribute_ = 0;
	for(	attribute_ = attributeSet.first();
	attribute_;
	attribute_ = attribute_->next() )
	{
		target->set_attribute( attribute_->name(), attribute_->value() );
	}

	node* node_ = 0;
	for ( node_ = firstChild; node_; node_ = node_->next_sibling() )
	{
		target->link_end_child( node_->clone() );
	}
}

bool element::accept( visitor* visitor_ ) const
{
	if ( visitor_->visit_enter( *this, attributeSet.first() ) ) 
	{
		for ( const node* node_=first_child(); node_; node_=node_->next_sibling() )
		{
			if ( !node_->accept( visitor_ ) )
				break;
		}
	}
	return visitor_->visit_exit( *this );
}


node* element::clone() const
{
	element* clone_ = new element( value() );
	if ( !clone_ )
		return 0;

	copy_to( clone_ );
	return clone_;
}


const char* element::get_text() const
{
	const node* child_ = this->first_child();
	if ( child_ ) {
		const text* childText = child_->to_text();
		if ( childText ) {
			return childText->value();
		}
	}
	return 0;
}


document::document() : node( node::DOCUMENT )
{
	tabsize = 4;
	useMicrosoftBOM = false;
	clear_error();
}

document::document( const char * documentName ) : node( node::DOCUMENT )
{
	tabsize = 4;
	useMicrosoftBOM = false;
	value_ = documentName;
	clear_error();
}


#ifdef TIXML_USE_STL
document::document( const std::string& documentName ) : node( node::DOCUMENT )
{
	tabsize = 4;
	useMicrosoftBOM = false;
    value_ = documentName;
	clear_error();
}
#endif


document::document( const document& copy ) : node( node::DOCUMENT )
{
	copy.copy_to( this );
}


void document::operator=( const document& copy )
{
	clear();
	copy.copy_to( this );
}


bool document::load_file( encoding encoding_ )
{
	// See STL_STRING_BUG below.
	//StringToBuffer buf( value_ );

	return load_file( value(), encoding_ );
}


bool document::save_file() const
{
	// See STL_STRING_BUG below.
//	StringToBuffer buf( value_ );
//
//	if ( buf.buffer && save_file( buf.buffer ) )
//		return true;
//
//	return false;
	return save_file( value() );
}

bool document::load_file( const char* _filename, encoding encoding_ )
{
	// There was a really terrifying little bug here. The code:
	//		value_ = filename
	// in the STL case, cause the assignment method of the std::string to
	// be called. What is strange, is that the std::string had the same
	// address as it's c_str() method, and so bad things happen. Looks
	// like a bug in the Microsoft STL implementation.
	// add an extra string to avoid the crash.
	TIXML_STRING filename( _filename );
	value_ = filename;

	// reading in binary mode so that tinyxml can normalize the EOL
	FILE* file = fopen( value_.c_str (), "rb" );	

	if ( file )
	{
		bool result = load_file( file, encoding_ );
		fclose( file );
		return result;
	}
	else
	{
		set_error( TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN );
		return false;
	}
}

bool document::load_file( const wchar_t* _filename, encoding encoding_ )
{
	// reading in binary mode so that tinyxml can normalize the EOL
	FILE* file = _wfopen( _filename, L"rb" );	

	if ( file )
	{
		bool result = load_file( file, encoding_ );
		fclose( file );
		return result;
	}
	else
	{
		set_error( TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN );
		return false;
	}
}

bool document::load_file( FILE* file, encoding encoding_ )
{
	if ( !file ) 
	{
		set_error( TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN );
		return false;
	}

	// Delete the existing data:
	clear();
	location.clear();

	// Get the file size, so we can pre-allocate the string. HUGE speed impact.
	long length = 0;
	fseek( file, 0, SEEK_END );
	length = ftell( file );
	fseek( file, 0, SEEK_SET );

	// Strange case, but good to handle_ up front.
	if ( length == 0 )
	{
		set_error( TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return false;
	}

	// If we have a file, assume it is all one big XML file, and read it in.
	// The document_ parser may decide the document_ ends sooner than the entire file, however.
	TIXML_STRING data;
	data.reserve( length );

	// Subtle bug here. TinyXml did use fgets. But from the XML spec:
	// 2.11 End-of-Line Handling
	// <snip>
	// <quote>
	// ...the XML processor MUST behave as if it normalized all line breaks in external 
	// parsed entities (including the document_ entity_) on input, before parsing, by translating 
	// both the two-character sequence #xD #xA and any #xD that is not followed by #xA to 
	// a single #xA character.
	// </quote>
	//
	// It is not clear_ fgets does that, and certainly isn't clear_ it works cross platform. 
	// Generally, you expect fgets to translate from the convention of the OS to the c/unix
	// convention, and not work generally.

	/*
	while( fgets( buf, sizeof(buf), file ) )
	{
		data += buf;
	}
	*/

	char* buf = new char[ length+1 ];
	buf[0] = 0;

	if ( fread( buf, length, 1, file ) != 1 ) {
		delete [] buf;
		set_error( TIXML_ERROR_OPENING_FILE, 0, 0, TIXML_ENCODING_UNKNOWN );
		return false;
	}

	const char* lastPos = buf;
	const char* p = buf;

	buf[length] = 0;
	while( *p ) {
		assert( p < (buf+length) );
		if ( *p == 0xa ) {
			// Newline character. no special rules for this. Append all the characters
			// since the last_ string, and include the newline.
			data.append( lastPos, (p-lastPos+1) );	// append, include the newline
			++p;									// move past the newline
			lastPos = p;							// and point to the new buffer (may be 0)
			assert( p <= (buf+length) );
		}
		else if ( *p == 0xd ) {
			// Carriage return. Append what we have so far, then
			// handle_ moving forward in the buffer.
			if ( (p-lastPos) > 0 ) {
				data.append( lastPos, p-lastPos );	// do not add_ the CR
			}
			data += (char)0xa;						// a proper newline

			if ( *(p+1) == 0xa ) {
				// Carriage return - new line sequence
				p += 2;
				lastPos = p;
				assert( p <= (buf+length) );
			}
			else {
				// it was followed by something else...that is presumably characters again.
				++p;
				lastPos = p;
				assert( p <= (buf+length) );
			}
		}
		else {
			++p;
		}
	}
	// Handle any left over characters.
	if ( p-lastPos ) {
		data.append( lastPos, p-lastPos );
	}		
	delete [] buf;
	buf = 0;

	parse( data.c_str(), 0, encoding_ );

	if (  error() )
        return false;
    else
		return true;
}


bool document::save_file( const char * filename ) const
{
	// The old c stuff lives on...
	FILE* fp = fopen( filename, "w" );
	if ( fp )
	{
		bool result = save_file( fp );
		fclose( fp );
		return result;
	}
	return false;
}

bool document::save_file( const wchar_t * filename ) const
{
	// The old c stuff lives on...
	FILE* fp = _wfopen( filename, L"w" );
	if ( fp )
	{
		bool result = save_file( fp );
		fclose( fp );
		return result;
	}
	return false;
}

bool document::save_file( FILE* fp ) const
{
	if ( useMicrosoftBOM ) 
	{
		const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
		const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
		const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;

		fputc( TIXML_UTF_LEAD_0, fp );
		fputc( TIXML_UTF_LEAD_1, fp );
		fputc( TIXML_UTF_LEAD_2, fp );
	}
	print( fp, 0 );
	return (ferror(fp) == 0);
}


void document::copy_to( document* target ) const
{
	node::copy_to( target );

	target->error_ = error_;
	target->errorDesc = errorDesc.c_str ();

	node* node_ = 0;
	for ( node_ = firstChild; node_; node_ = node_->next_sibling() )
	{
		target->link_end_child( node_->clone() );
	}	
}


node* document::clone() const
{
	document* clone_ = new document();
	if ( !clone_ )
		return 0;

	copy_to( clone_ );
	return clone_;
}


void document::print( FILE* cfile, int depth ) const
{
	assert( cfile );
	for ( const node* node_=first_child(); node_; node_=node_->next_sibling() )
	{
		node_->print( cfile, depth );
		fprintf( cfile, "\n" );
	}
}


bool document::accept( visitor* visitor_ ) const
{
	if ( visitor_->visit_enter( *this ) )
	{
		for ( const node* node_=first_child(); node_; node_=node_->next_sibling() )
		{
			if ( !node_->accept( visitor_ ) )
				break;
		}
	}
	return visitor_->visit_exit( *this );
}


const attribute* attribute::next() const
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value_ or name_.
	if ( next_->value_.empty() && next_->name_.empty() )
		return 0;
	return next_;
}

/*
attribute* attribute::next()
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value_ or name_.
	if ( next_->value_.empty() && next_->name_.empty() )
		return 0;
	return next_;
}
*/

const attribute* attribute::previous() const
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value_ or name_.
	if ( prev->value_.empty() && prev->name_.empty() )
		return 0;
	return prev;
}

/*
attribute* attribute::previous()
{
	// We are using knowledge of the sentinel. The sentinel
	// have a value_ or name_.
	if ( prev->value_.empty() && prev->name_.empty() )
		return 0;
	return prev;
}
*/

void attribute::print( FILE* cfile, int /*depth*/, TIXML_STRING* str ) const
{
	TIXML_STRING n, v;

	put_string( name_, &n );
	put_string( value_, &v );

	if (value_.find ('\"') == TIXML_STRING::npos) {
		if ( cfile ) {
		fprintf (cfile, "%s=\"%s\"", n.c_str(), v.c_str() );
		}
		if ( str ) {
			(*str) += n; (*str) += "=\""; (*str) += v; (*str) += "\"";
		}
	}
	else {
		if ( cfile ) {
		fprintf (cfile, "%s='%s'", n.c_str(), v.c_str() );
		}
		if ( str ) {
			(*str) += n; (*str) += "='"; (*str) += v; (*str) += "'";
		}
	}
}


int attribute::query_int_value( int* ival ) const
{
	if ( sscanf( value_.c_str(), "%d", ival ) == 1 )
		return TIXML_SUCCESS;
	return TIXML_WRONG_TYPE;
}

int attribute::query_double_value( double* dval ) const
{
	if ( sscanf( value_.c_str(), "%lf", dval ) == 1 )
		return TIXML_SUCCESS;
	return TIXML_WRONG_TYPE;
}

void attribute::set_int_value( int _value )
{
	char buf [64];
	#if defined(TIXML_SNPRINTF)		
		TIXML_SNPRINTF(buf, sizeof(buf), "%d", _value);
	#else
		sprintf (buf, "%d", _value);
	#endif
	set_value (buf);
}

void attribute::set_double_value( double _value )
{
	char buf [256];
	#if defined(TIXML_SNPRINTF)		
		TIXML_SNPRINTF( buf, sizeof(buf), "%lf", _value);
	#else
		sprintf (buf, "%lf", _value);
	#endif
	set_value (buf);
}

int attribute::int_value() const
{
	return atoi (value_.c_str ());
}

double  attribute::double_value() const
{
	return atof (value_.c_str ());
}


comment::comment( const comment& copy ) : node( node::COMMENT )
{
	copy.copy_to( this );
}


void comment::operator=( const comment& base_ )
{
	clear();
	base_.copy_to( this );
}


void comment::print( FILE* cfile, int depth ) const
{
	assert( cfile );
	for ( int i=0; i<depth; i++ )
	{
		fprintf( cfile,  "    " );
	}
	fprintf( cfile, "<!--%s-->", value_.c_str() );
}


void comment::copy_to( comment* target ) const
{
	node::copy_to( target );
}


bool comment::accept( visitor* visitor_ ) const
{
	return visitor_->visit( *this );
}


node* comment::clone() const
{
	comment* clone_ = new comment();

	if ( !clone_ )
		return 0;

	copy_to( clone_ );
	return clone_;
}


void text::print( FILE* cfile, int depth ) const
{
	assert( cfile );
	if ( cdata )
	{
		int i;
		fprintf( cfile, "\n" );
		for ( i=0; i<depth; i++ ) {
			fprintf( cfile, "    " );
		}
		fprintf( cfile, "<![CDATA[%s]]>\n", value_.c_str() );	// unformatted output
	}
	else
	{
		TIXML_STRING buffer;
		put_string( value_, &buffer );
		fprintf( cfile, "%s", buffer.c_str() );
	}
}


void text::copy_to( text* target ) const
{
	node::copy_to( target );
	target->cdata = cdata;
}


bool text::accept( visitor* visitor_ ) const
{
	return visitor_->visit( *this );
}


node* text::clone() const
{	
	text* clone_ = 0;
	clone_ = new text( "" );

	if ( !clone_ )
		return 0;

	copy_to( clone_ );
	return clone_;
}


declaration::declaration( const char * _version,
									const char * _encoding,
									const char * _standalone )
	: node( node::DECLARATION )
{
	version_ = _version;
	encoding_ = _encoding;
	standalone_ = _standalone;
}


#ifdef TIXML_USE_STL
declaration::declaration(	const std::string& _version,
									const std::string& _encoding,
									const std::string& _standalone )
	: node( node::DECLARATION )
{
	version_ = _version;
	encoding_ = _encoding;
	standalone_ = _standalone;
}
#endif


declaration::declaration( const declaration& copy )
	: node( node::DECLARATION )
{
	copy.copy_to( this );	
}


void declaration::operator=( const declaration& copy )
{
	clear();
	copy.copy_to( this );
}


void declaration::print( FILE* cfile, int /*depth*/, TIXML_STRING* str ) const
{
	if ( cfile ) fprintf( cfile, "<?xml " );
	if ( str )	 (*str) += "<?xml ";

	if ( !version_.empty() ) {
		if ( cfile ) fprintf (cfile, "version=\"%s\" ", version_.c_str ());
		if ( str ) { (*str) += "version=\""; (*str) += version_; (*str) += "\" "; }
	}
	if ( !encoding_.empty() ) {
		if ( cfile ) fprintf (cfile, "encoding=\"%s\" ", encoding_.c_str ());
		if ( str ) { (*str) += "encoding=\""; (*str) += encoding_; (*str) += "\" "; }
	}
	if ( !standalone_.empty() ) {
		if ( cfile ) fprintf (cfile, "standalone=\"%s\" ", standalone_.c_str ());
		if ( str ) { (*str) += "standalone=\""; (*str) += standalone_; (*str) += "\" "; }
	}
	if ( cfile ) fprintf( cfile, "?>" );
	if ( str )	 (*str) += "?>";
}


void declaration::copy_to( declaration* target ) const
{
	node::copy_to( target );

	target->version_ = version_;
	target->encoding_ = encoding_;
	target->standalone_ = standalone_;
}


bool declaration::accept( visitor* visitor_ ) const
{
	return visitor_->visit( *this );
}


node* declaration::clone() const
{	
	declaration* clone_ = new declaration();

	if ( !clone_ )
		return 0;

	copy_to( clone_ );
	return clone_;
}


void unknown::print( FILE* cfile, int depth ) const
{
	for ( int i=0; i<depth; i++ )
		fprintf( cfile, "    " );
	fprintf( cfile, "<%s>", value_.c_str() );
}


void unknown::copy_to( unknown* target ) const
{
	node::copy_to( target );
}


bool unknown::accept( visitor* visitor_ ) const
{
	return visitor_->visit( *this );
}


node* unknown::clone() const
{
	unknown* clone_ = new unknown();

	if ( !clone_ )
		return 0;

	copy_to( clone_ );
	return clone_;
}


attribute_set::attribute_set()
{
	sentinel.next_ = &sentinel;
	sentinel.prev = &sentinel;
}


attribute_set::~attribute_set()
{
	assert( sentinel.next_ == &sentinel );
	assert( sentinel.prev == &sentinel );
}


void attribute_set::add( attribute* addMe )
{
    #ifdef TIXML_USE_STL
	assert( !find( TIXML_STRING( addMe->name() ) ) );	// Shouldn't be multiply adding to the set.
	#else
	assert( !find( addMe->name() ) );	// Shouldn't be multiply adding to the set.
	#endif

	addMe->next_ = &sentinel;
	addMe->prev = sentinel.prev;

	sentinel.prev->next_ = addMe;
	sentinel.prev      = addMe;
}

void attribute_set::remove( attribute* removeMe )
{
	attribute* node_;

	for( node_ = sentinel.next_; node_ != &sentinel; node_ = node_->next_ )
	{
		if ( node_ == removeMe )
		{
			node_->prev->next_ = node_->next_;
			node_->next_->prev = node_->prev;
			node_->next_ = 0;
			node_->prev = 0;
			return;
		}
	}
	assert( 0 );		// we tried to remove_ a non-linked attribute_.
}


#ifdef TIXML_USE_STL
const attribute* attribute_set::find( const std::string& name_ ) const
{
	for( const attribute* node_ = sentinel.next_; node_ != &sentinel; node_ = node_->next_ )
	{
		if ( node_->name_ == name_ )
			return node_;
	}
	return 0;
}

/*
attribute*	attribute_set::find( const std::string& name_ )
{
	for( attribute* node_ = sentinel.next_; node_ != &sentinel; node_ = node_->next_ )
	{
		if ( node_->name_ == name_ )
			return node_;
	}
	return 0;
}
*/
#endif


const attribute* attribute_set::find( const char* name_ ) const
{
	for( const attribute* node_ = sentinel.next_; node_ != &sentinel; node_ = node_->next_ )
	{
		if ( strcmp( node_->name_.c_str(), name_ ) == 0 )
			return node_;
	}
	return 0;
}

/*
attribute*	attribute_set::find( const char* name_ )
{
	for( attribute* node_ = sentinel.next_; node_ != &sentinel; node_ = node_->next_ )
	{
		if ( strcmp( node_->name_.c_str(), name_ ) == 0 )
			return node_;
	}
	return 0;
}
*/

#ifdef TIXML_USE_STL	
std::istream& operator>> (std::istream & in, node & base_)
{
	TIXML_STRING tag;
	tag.reserve( 8 * 1000 );
	base_.stream_in( &in, &tag );

	base_.parse( tag.c_str(), 0, TIXML_DEFAULT_ENCODING );
	return in;
}
#endif


#ifdef TIXML_USE_STL	
std::ostream& operator<< (std::ostream & out, const node & base_)
{
	printer printer_;
	printer_.set_stream_printing();
	base_.accept( &printer_ );
	out << printer_.str();

	return out;
}


std::string& operator<< (std::string& out, const node& base_ )
{
	printer printer_;
	printer_.set_stream_printing();
	base_.accept( &printer_ );
	out.append( printer_.str() );

	return out;
}
#endif


handle handle::first_child() const
{
	if ( node_ )
	{
		node* child_ = node_->first_child();
		if ( child_ )
			return handle( child_ );
	}
	return handle( 0 );
}


handle handle::first_child( const char * value_ ) const
{
	if ( node_ )
	{
		node* child_ = node_->first_child( value_ );
		if ( child_ )
			return handle( child_ );
	}
	return handle( 0 );
}


handle handle::first_child_element() const
{
	if ( node_ )
	{
		element* child_ = node_->first_child_element();
		if ( child_ )
			return handle( child_ );
	}
	return handle( 0 );
}


handle handle::first_child_element( const char * value_ ) const
{
	if ( node_ )
	{
		element* child_ = node_->first_child_element( value_ );
		if ( child_ )
			return handle( child_ );
	}
	return handle( 0 );
}


handle handle::child( int count ) const
{
	if ( node_ )
	{
		int i;
		node* child_ = node_->first_child();
		for (	i=0;
				child_ && i<count;
				child_ = child_->next_sibling(), ++i )
		{
			// nothing
		}
		if ( child_ )
			return handle( child_ );
	}
	return handle( 0 );
}


handle handle::child( const char* value_, int count ) const
{
	if ( node_ )
	{
		int i;
		node* child_ = node_->first_child( value_ );
		for (	i=0;
				child_ && i<count;
				child_ = child_->next_sibling( value_ ), ++i )
		{
			// nothing
		}
		if ( child_ )
			return handle( child_ );
	}
	return handle( 0 );
}


handle handle::child_element( int count ) const
{
	if ( node_ )
	{
		int i;
		element* child_ = node_->first_child_element();
		for (	i=0;
				child_ && i<count;
				child_ = child_->next_sibling_element(), ++i )
		{
			// nothing
		}
		if ( child_ )
			return handle( child_ );
	}
	return handle( 0 );
}


handle handle::child_element( const char* value_, int count ) const
{
	if ( node_ )
	{
		int i;
		element* child_ = node_->first_child_element( value_ );
		for (	i=0;
				child_ && i<count;
				child_ = child_->next_sibling_element( value_ ), ++i )
		{
			// nothing
		}
		if ( child_ )
			return handle( child_ );
	}
	return handle( 0 );
}


bool printer::visit_enter( const document& )
{
	return true;
}

bool printer::visit_exit( const document& )
{
	return true;
}

bool printer::visit_enter( const element& element_, const attribute* firstAttribute )
{
	do_indent();
	buffer += "<";
	buffer += element_.value();

	for( const attribute* attrib = firstAttribute; attrib; attrib = attrib->next() )
	{
		buffer += " ";
		attrib->print( 0, 0, &buffer );
	}

	if ( !element_.first_child() ) 
	{		
		#if defined (HAL_BOOST_SERIALIZATION_COMPAT)
		buffer += ">";	
		#else
		buffer += " />";
		do_line_break();
		#endif
	}
	else 
	{
		buffer += ">";
		if (    element_.first_child()->to_text()
			  && element_.last_child() == element_.first_child()
			  && element_.first_child()->to_text()->CDATA() == false )
		{
			simpleTextPrint = true;
			// no_ do_line_break()!
		}
		else
		{
			do_line_break();
		}
	}
	++depth;	
	return true;
}


bool printer::visit_exit( const element& element_ )
{
	--depth;
	if ( !element_.first_child() ) 
	{		
		#if defined (HAL_BOOST_SERIALIZATION_COMPAT)
		buffer += "</";
		buffer += element_.value();
		buffer += ">";
		do_line_break();
		#else
		// nothing
		#endif
	}
	else 
	{
		if ( simpleTextPrint )
		{
			simpleTextPrint = false;
		}
		else
		{
			do_indent();
		}
		buffer += "</";
		buffer += element_.value();
		buffer += ">";
		do_line_break();
	}
	return true;
}


bool printer::visit( const text& text_ )
{
	if ( text_.CDATA() )
	{
		do_indent();
		buffer += "<![CDATA[";
		buffer += text_.value();
		buffer += "]]>";
		do_line_break();
	}
	else if ( simpleTextPrint )
	{
		buffer += text_.value();
	}
	else
	{
		do_indent();
		buffer += text_.value();
		do_line_break();
	}
	return true;
}


bool printer::visit( const declaration& declaration_ )
{
	do_indent();
	declaration_.print( 0, 0, &buffer );
	do_line_break();
	return true;
}


bool printer::visit( const comment& comment_ )
{
	do_indent();
	buffer += "<!--";
	buffer += comment_.value();
	buffer += "-->";
	do_line_break();
	return true;
}


bool printer::visit( const unknown& unknown_ )
{
	do_indent();
	buffer += "<";
	buffer += unknown_.value();
	buffer += ">";
	do_line_break();
	return true;
}

}
