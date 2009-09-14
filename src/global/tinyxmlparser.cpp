/*
www.sourceforge.net/projects/tinyxml
Original code (2.0 and earlier )copyright (c) 2000-2002 Lee Thomason (www.grinninglizard.com)

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

#include <ctype.h>
#include <stddef.h>

#include "tinyxml.hpp"

namespace xml
{

//#define DEBUG_PARSER
#if defined( DEBUG_PARSER )
#	if defined( DEBUG ) && defined( _MSC_VER )
#		include <windows.h>
#		define TIXML_LOG output_debug_string
#	else
#		define TIXML_LOG printf
#	endif
#endif

// Note tha "PutString" hardcodes the same list. this
// is less flexible than it_ appears. Changing the entries
// or order will break putstring.	
base::entity base::entity_[ NUM_ENTITY ] = 
{
	{ "&amp;",  5, '&' },
	{ "&lt;",   4, '<' },
	{ "&gt;",   4, '>' },
	{ "&quot;", 6, '\"' },
	{ "&apos;", 6, '\'' }
};

// Bunch of unicode info at:
//		http://www.unicode.org/faq/utf_bom.html
// Including the basic_ of this table, which determines the #bytes in the
// sequence from the lead byte. 1 placed for invalid sequences --
// although the result will be junk, pass it_ through as much as possible.
// Beware of the non-characters in UTF-8:	
//				ef bb bf (Microsoft "lead bytes")
//				ef bf be
//				ef bf bf 

const unsigned char TIXML_UTF_LEAD_0 = 0xefU;
const unsigned char TIXML_UTF_LEAD_1 = 0xbbU;
const unsigned char TIXML_UTF_LEAD_2 = 0xbfU;

const int base::utf8ByteTable[256] = 
{
	//	0	1	2	3	4	5	6	7	8	9	a	b	c	d	e	f
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x00
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x10
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x20
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x30
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x40
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x50
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x60
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x70	End of ASCII range
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x80 0x80 to 0xc1 invalid
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0x90 
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0xa0 
		1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	// 0xb0 
		1,	1,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	// 0xc0 0xc2 to 0xdf 2 byte
		2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	2,	// 0xd0
		3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	3,	// 0xe0 0xe0 to 0xef 3 byte
		4,	4,	4,	4,	4,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1,	1	// 0xf0 0xf0 to 0xf4 4 byte, 0xf5 and higher invalid
};


void base::convert_u_t_f32_to_u_t_f8( unsigned long input, char* output_, int* length )
{
	const unsigned long BYTE_MASK = 0xBF;
	const unsigned long BYTE_MARK = 0x80;
	const unsigned long FIRST_BYTE_MARK[7] = { 0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

	if (input < 0x80) 
		*length = 1;
	else if ( input < 0x800 )
		*length = 2;
	else if ( input < 0x10000 )
		*length = 3;
	else if ( input < 0x200000 )
		*length = 4;
	else
		{ *length = 0; return; }	// this code won't covert this correctly anyway.

	output_ += *length;

	// Scary scary fall throughs.
	switch (*length) 
	{
		case 4:
			--output_; 
			*output_ = (char)((input | BYTE_MARK) & BYTE_MASK); 
			input >>= 6;
		case 3:
			--output_; 
			*output_ = (char)((input | BYTE_MARK) & BYTE_MASK); 
			input >>= 6;
		case 2:
			--output_; 
			*output_ = (char)((input | BYTE_MARK) & BYTE_MASK); 
			input >>= 6;
		case 1:
			--output_; 
			*output_ = (char)(input | FIRST_BYTE_MARK[*length]);
	}
}


/*static*/ int base::is_alpha( unsigned char anyByte, encoding /*encoding_*/ )
{
	// this will only work for low_-ascii, everything else is assumed to be a valid
	// letter. I'm not sure this is the best approach, but it_ is quite tricky trying
	// to figure out alhabetical vs. not across encoding_. So take a very 
	// conservative approach.

//	if ( encoding_ == TIXML_ENCODING_UTF8 )
//	{
		if ( anyByte < 127 )
			return isalpha( anyByte );
		else
			return 1;	// What else to do_? The unicode set is huge...get the english ones right.
//	}
//	else
//	{
//		return isalpha( anyByte );
//	}
}


/*static*/ int base::is_alpha_num( unsigned char anyByte, encoding /*encoding_*/ )
{
	// this will only work for low_-ascii, everything else is assumed to be a valid
	// letter. I'm not sure this is the best approach, but it_ is quite tricky trying
	// to figure out alhabetical vs. not across encoding_. So take a very 
	// conservative approach.

//	if ( encoding_ == TIXML_ENCODING_UTF8 )
//	{
		if ( anyByte < 127 )
			return isalnum( anyByte );
		else
			return 1;	// What else to do_? The unicode set is huge...get the english ones right.
//	}
//	else
//	{
//		return isalnum( anyByte );
//	}
}


class parsing_data
{
	friend class document;
  public:
	void stamp( const char* now, encoding encoding_ );

	const cursor& get_cursor()	{ return cursor_; }

  private:
	// Only used by the document_!
	parsing_data( const char* start_, int _tabsize, int row_, int col )
	{
		assert( start_ );
		stamp_ = start_;
		tabsize = _tabsize;
		cursor_.row_ = row_;
		cursor_.col = col;
	}

	cursor		cursor_;
	const char*		stamp_;
	int				tabsize;
};


void parsing_data::stamp( const char* now, encoding encoding_ )
{
	assert( now );

	// do nothing if the tabsize is 0.
	if ( tabsize < 1 )
	{
		return;
	}

	// Get the current row_, column_.
	int row_ = cursor_.row_;
	int col = cursor_.col;
	const char* p = stamp_;
	assert( p );

	while ( p < now )
	{
		// Treat p as unsigned, so we have a happy compiler.
		const unsigned char* pU = (const unsigned char*)p;

		// Code contributed by Fletcher Dunn: (modified by lee)
		switch (*pU) {
			case 0:
				// We *should* never get here, but in case we do_, don't
				// advance past the terminating null character, ever
				return;

			case '\r':
				// bump down to the next_ line_
				++row_;
				col = 0;				
				// Eat the character
				++p;

				// Check for \r\n sequence, and treat this as a single character
				if (*p == '\n') {
					++p;
				}
				break;

			case '\n':
				// bump down to the next_ line_
				++row_;
				col = 0;

				// Eat the character
				++p;

				// Check for \n\r sequence, and treat this as a single
				// character.  (Yes, this bizarre thing does occur still
				// on some arcane platforms...)
				if (*p == '\r') {
					++p;
				}
				break;

			case '\t':
				// Eat the character
				++p;

				// Skip to next_ tab_ stop
				col = (col / tabsize + 1) * tabsize;
				break;

			case TIXML_UTF_LEAD_0:
				if ( encoding_ == TIXML_ENCODING_UTF8 )
				{
					if ( *(p+1) && *(p+2) )
					{
						// In these cases, don't advance the column_. These are_
						// 0-width spaces.
						if ( *(pU+1)==TIXML_UTF_LEAD_1 && *(pU+2)==TIXML_UTF_LEAD_2 )
							p += 3;	
						else if ( *(pU+1)==0xbfU && *(pU+2)==0xbeU )
							p += 3;	
						else if ( *(pU+1)==0xbfU && *(pU+2)==0xbfU )
							p += 3;	
						else
							{ p +=3; ++col; }	// A normal character.
					}
				}
				else
				{
					++p;
					++col;
				}
				break;

			default:
				if ( encoding_ == TIXML_ENCODING_UTF8 )
				{
					// Eat the 1 to 4 byte utf8 character.
					int step = base::utf8ByteTable[*((const unsigned char*)p)];
					if ( step == 0 )
						step = 1;		// error case from bad encoding_, but handle_ gracefully.
					p += step;

					// Just advance one column_, of course.
					++col;
				}
				else
				{
					++p;
					++col;
				}
				break;
		}
	}
	cursor_.row_ = row_;
	cursor_.col = col;
	assert( cursor_.row_ >= -1 );
	assert( cursor_.col >= -1 );
	stamp_ = p;
	assert( stamp_ );
}


const char* base::skip_white_space( const char* p, encoding encoding_ )
{
	if ( !p || !*p )
	{
		return 0;
	}
	if ( encoding_ == TIXML_ENCODING_UTF8 )
	{
		while ( *p )
		{
			const unsigned char* pU = (const unsigned char*)p;
			
			// Skip the stupid Microsoft UTF-8 Byte order marks
			if (	*(pU+0)==TIXML_UTF_LEAD_0
				 && *(pU+1)==TIXML_UTF_LEAD_1 
				 && *(pU+2)==TIXML_UTF_LEAD_2 )
			{
				p += 3;
				continue;
			}
			else if(*(pU+0)==TIXML_UTF_LEAD_0
				 && *(pU+1)==0xbfU
				 && *(pU+2)==0xbeU )
			{
				p += 3;
				continue;
			}
			else if(*(pU+0)==TIXML_UTF_LEAD_0
				 && *(pU+1)==0xbfU
				 && *(pU+2)==0xbfU )
			{
				p += 3;
				continue;
			}

			if ( is_white_space( *p ) || *p == '\n' || *p =='\r' )		// Still using old rules for white_ space.
				++p;
			else
				break;
		}
	}
	else
	{
		while ( *p && is_white_space( *p ) || *p == '\n' || *p =='\r' )
			++p;
	}

	return p;
}

#ifdef TIXML_USE_STL
/*static*/ bool base::stream_white_space( std::istream * in, TIXML_STRING * tag )
{
	for( ;; )
	{
		if ( !in->good() ) return false;

		int c = in->peek();
		// At this scope, we can't get to a document_. So fail silently.
		if ( !is_white_space( c ) || c <= 0 )
			return true;

		*tag += (char) in->get();
	}
}

/*static*/ bool base::stream_to( std::istream * in, int character, TIXML_STRING * tag )
{
	//assert( character > 0 && character < 128 );	// else it_ won't work in utf-8
	while ( in->good() )
	{
		int c = in->peek();
		if ( c == character )
			return true;
		if ( c <= 0 )		// Silent failure: can't get document_ at this scope
			return false;

		in->get();
		*tag += (char) c;
	}
	return false;
}
#endif

// One of TinyXML's more performance demanding functions. Try to keep the memory_ overhead down. The
// "assign" optimization removes over 10% of the execution time.
//
const char* base::read_name( const char* p, TIXML_STRING * name_, encoding encoding_ )
{
	// Oddly, not supported on some comilers,
	//name_->clear_();
	// So use this:
	*name_ = "";
	assert( p );

	// Names start_ with letters or underscores.
	// Of course, in unicode, tinyxml has no_ idea what a letter *is*. The
	// algorithm is generous.
	//
	// After that, they can be letters, underscores, numbers,
	// hyphens, or colons. (Colons are_ valid ony for namespaces,
	// but tinyxml can't tell namespaces from names.)
	if (    p && *p 
		 && ( is_alpha( (unsigned char) *p, encoding_ ) || *p == '_' ) )
	{
		const char* start_ = p;
		while(		p && *p
				&&	(		is_alpha_num( (unsigned char ) *p, encoding_ ) 
						 || *p == '_'
						 || *p == '-'
						 || *p == '.'
						 || *p == ':' ) )
		{
			//(*name_) += *p; // expensive
			++p;
		}
		if ( p-start_ > 0 ) {
			name_->assign( start_, p-start_ );
		}
		return p;
	}
	return 0;
}

const char* base::get_entity( const char* p, char* value_, int* length, encoding encoding_ )
{
	// Presume an entity_, and pull it_ out.
    TIXML_STRING ent;
	int i;
	*length = 0;

	if ( *(p+1) && *(p+1) == '#' && *(p+2) )
	{
		unsigned long ucs = 0;
		ptrdiff_t delta = 0;
		unsigned mult = 1;

		if ( *(p+2) == 'x' )
		{
			// Hexadecimal.
			if ( !*(p+3) ) return 0;

			const char* q = p+3;
			q = strchr( q, ';' );

			if ( !q || !*q ) return 0;

			delta = q-p;
			--q;

			while ( *q != 'x' )
			{
				if ( *q >= '0' && *q <= '9' )
					ucs += mult * (*q - '0');
				else if ( *q >= 'a' && *q <= 'f' )
					ucs += mult * (*q - 'a' + 10);
				else if ( *q >= 'A' && *q <= 'F' )
					ucs += mult * (*q - 'A' + 10 );
				else 
					return 0;
				mult *= 16;
				--q;
			}
		}
		else
		{
			// Decimal.
			if ( !*(p+2) ) return 0;

			const char* q = p+2;
			q = strchr( q, ';' );

			if ( !q || !*q ) return 0;

			delta = q-p;
			--q;

			while ( *q != '#' )
			{
				if ( *q >= '0' && *q <= '9' )
					ucs += mult * (*q - '0');
				else 
					return 0;
				mult *= 10;
				--q;
			}
		}
		if ( encoding_ == TIXML_ENCODING_UTF8 )
		{
			// convert the UCS to UTF-8
			convert_u_t_f32_to_u_t_f8( ucs, value_, length );
		}
		else
		{
			*value_ = (char)ucs;
			*length = 1;
		}
		return p + delta + 1;
	}

	// Now try to match it_.
	for( i=0; i<NUM_ENTITY; ++i )
	{
		if ( strncmp( entity_[i].str, p, entity_[i].strLength ) == 0 )
		{
			assert( strlen( entity_[i].str ) == entity_[i].strLength );
			*value_ = entity_[i].chr;
			*length = 1;
			return ( p + entity_[i].strLength );
		}
	}

	// So it_ wasn't an entity_, its unrecognized, or something like that.
	*value_ = *p;	// Don't put back the last_ one, since we return it_!
	//*length = 1;	// Leave unrecognized entities - this doesn't really work.
					// Just writes strange XML.
	return p+1;
}


bool base::string_equal( const char* p,
							 const char* tag,
							 bool ignoreCase,
							 encoding encoding_ )
{
	assert( p );
	assert( tag );
	if ( !p || !*p )
	{
		assert( 0 );
		return false;
	}

	const char* q = p;

	if ( ignoreCase )
	{
		while ( *q && *tag && to_lower( *q, encoding_ ) == to_lower( *tag, encoding_ ) )
		{
			++q;
			++tag;
		}

		if ( *tag == 0 )
			return true;
	}
	else
	{
		while ( *q && *tag && *q == *tag )
		{
			++q;
			++tag;
		}

		if ( *tag == 0 )		// Have we found the end of the tag, and everything equal?
			return true;
	}
	return false;
}

const char* base::read_text(	const char* p, 
									TIXML_STRING * text_, 
									bool trimWhiteSpace, 
									const char* endTag, 
									bool caseInsensitive,
									encoding encoding_ )
{
    *text_ = "";
	if (    !trimWhiteSpace			// certain tags always keep whitespace_
		 || !condenseWhiteSpace )	// if true, whitespace_ is always kept
	{
		// Keep all the white_ space.
		while (	   p && *p
				&& !string_equal( p, endTag, caseInsensitive, encoding_ )
			  )
		{
			int len;
			char cArr[4] = { 0, 0, 0, 0 };
			p = get_char( p, cArr, &len, encoding_ );
			text_->append( cArr, len );
		}
	}
	else
	{
		bool whitespace_ = false;

		// remove leading white_ space:
		p = skip_white_space( p, encoding_ );
		while (	   p && *p
				&& !string_equal( p, endTag, caseInsensitive, encoding_ ) )
		{
			if ( *p == '\r' || *p == '\n' )
			{
				whitespace_ = true;
				++p;
			}
			else if ( is_white_space( *p ) )
			{
				whitespace_ = true;
				++p;
			}
			else
			{
				// If we've found whitespace_, add_ it_ before the
				// new character. Any whitespace_ just becomes a space.
				if ( whitespace_ )
				{
					(*text_) += ' ';
					whitespace_ = false;
				}
				int len;
				char cArr[4] = { 0, 0, 0, 0 };
				p = get_char( p, cArr, &len, encoding_ );
				if ( len == 1 )
					(*text_) += cArr[0];	// more efficient
				else
					text_->append( cArr, len );
			}
		}
	}
	if ( p ) 
		p += strlen( endTag );
	return p;
}

#ifdef TIXML_USE_STL

void document::stream_in( std::istream * in, TIXML_STRING * tag )
{
	// The basic_ issue with a document_ is that we don't know what we're
	// streaming_. read something presumed to be a tag (and hope), then
	// identify_ it_, and call the appropriate stream_ method on the tag.
	//
	// this "pre-streaming" will never read_ the closing ">" so the
	// sub-tag can orient itself.

	if ( !stream_to( in, '<', tag ) ) 
	{
		set_error( TIXML_ERROR_PARSING_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return;
	}

	while ( in->good() )
	{
		int tagIndex = (int) tag->length();
		while ( in->good() && in->peek() != '>' )
		{
			int c = in->get();
			if ( c <= 0 )
			{
				set_error( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
				break;
			}
			(*tag) += (char) c;
		}

		if ( in->good() )
		{
			// We now have something we presume to be a node_ of 
			// some sort. identify it_, and call the node_ to
			// continue streaming_.
			node* node_ = identify( tag->c_str() + tagIndex, TIXML_DEFAULT_ENCODING );

			if ( node_ )
			{
				node_->stream_in( in, tag );
				bool isElement = node_->to_element() != 0;
				delete node_;
				node_ = 0;

				// If this is the root_ element_, we're done. parsing will be
				// done by the >> operator.
				if ( isElement )
				{
					return;
				}
			}
			else
			{
				set_error( TIXML_ERROR, 0, 0, TIXML_ENCODING_UNKNOWN );
				return;
			}
		}
	}
	// We should have returned sooner.
	set_error( TIXML_ERROR, 0, 0, TIXML_ENCODING_UNKNOWN );
}

#endif

const char* document::parse( const char* p, parsing_data* prevData, encoding encoding_ )
{
	clear_error();

	// parse away, at the document_ level. Since a document_
	// contains nothing but other tags, most of what happens
	// here is skipping white_ space.
	if ( !p || !*p )
	{
		set_error( TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return 0;
	}

	// Note that, for a document_, this needs to come
	// before the while space skip, so that parsing_
	// starts from the pointer we are_ given.
	location_.clear();
	if ( prevData )
	{
		location_.row_ = prevData->cursor_.row_;
		location_.col = prevData->cursor_.col;
	}
	else
	{
		location_.row_ = 0;
		location_.col = 0;
	}
	parsing_data data( p, tab_size(), location_.row_, location_.col );
	location_ = data.get_cursor();

	if ( encoding_ == TIXML_ENCODING_UNKNOWN )
	{
		// Check for the Microsoft UTF-8 lead bytes.
		const unsigned char* pU = (const unsigned char*)p;
		if (	*(pU+0) && *(pU+0) == TIXML_UTF_LEAD_0
			 && *(pU+1) && *(pU+1) == TIXML_UTF_LEAD_1
			 && *(pU+2) && *(pU+2) == TIXML_UTF_LEAD_2 )
		{
			encoding_ = TIXML_ENCODING_UTF8;
			useMicrosoftBOM = true;
		}
	}

    p = skip_white_space( p, encoding_ );
	if ( !p )
	{
		set_error( TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, TIXML_ENCODING_UNKNOWN );
		return 0;
	}

	while ( p && *p )
	{
		node* node_ = identify( p, encoding_ );
		if ( node_ )
		{
			p = node_->parse( p, &data, encoding_ );
			link_end_child( node_ );
		}
		else
		{
			break;
		}

		// Did we get encoding_ info?
		if (    encoding_ == TIXML_ENCODING_UNKNOWN
			 && node_->to_declaration() )
		{
			declaration* dec = node_->to_declaration();
			const char* enc = dec->get_encoding();
			assert( enc );

			if ( *enc == 0 )
				encoding_ = TIXML_ENCODING_UTF8;
			else if ( string_equal( enc, "UTF-8", true, TIXML_ENCODING_UNKNOWN ) )
				encoding_ = TIXML_ENCODING_UTF8;
			else if ( string_equal( enc, "UTF8", true, TIXML_ENCODING_UNKNOWN ) )
				encoding_ = TIXML_ENCODING_UTF8;	// incorrect, but be nice
			else 
				encoding_ = TIXML_ENCODING_LEGACY;
		}

		p = skip_white_space( p, encoding_ );
	}

	// Was this empty?
	if ( !firstChild ) {
		set_error( TIXML_ERROR_DOCUMENT_EMPTY, 0, 0, encoding_ );
		return 0;
	}

	// All is well.
	return p;
}

void document::set_error( int err, const char* pError, parsing_data* data, encoding encoding_ )
{	
	// The first_ error_ in a chain is more accurate - don't set again!
	if ( error_ )
		return;

	assert( err > 0 && err < TIXML_ERROR_STRING_COUNT );
	error_   = true;
	errorId = err;
	errorDesc = errorString[ errorId ];

	errorLocation.clear();
	if ( pError && data )
	{
		data->stamp( pError, encoding_ );
		errorLocation = data->get_cursor();
	}
}


node* node::identify( const char* p, encoding encoding_ )
{
	node* returnNode = 0;

	p = skip_white_space( p, encoding_ );
	if( !p || !*p || *p != '<' )
	{
		return 0;
	}

	document* doc = get_document();
	p = skip_white_space( p, encoding_ );

	if ( !p || !*p )
	{
		return 0;
	}

	// What is this thing? 
	// - Elements start_ with a letter or underscore, but xml is reserved.
	// - comments: <!--
	// - Decleration: <?xml
	// - Everthing else is unknown_ to tinyxml.
	//

	const char* xmlHeader = { "<?xml" };
	const char* commentHeader = { "<!--" };
	const char* dtdHeader = { "<!" };
	const char* cdataHeader = { "<![CDATA[" };

	if ( string_equal( p, xmlHeader, true, encoding_ ) )
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Declaration\n" );
		#endif
		returnNode = new declaration();
	}
	else if ( string_equal( p, commentHeader, false, encoding_ ) )
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Comment\n" );
		#endif
		returnNode = new comment();
	}
	else if ( string_equal( p, cdataHeader, false, encoding_ ) )
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing CDATA\n" );
		#endif
		text* text_ = new text( "" );
		text_->set_c_d_a_t_a( true );
		returnNode = text_;
	}
	else if ( string_equal( p, dtdHeader, false, encoding_ ) )
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Unknown(1)\n" );
		#endif
		returnNode = new unknown();
	}
	else if (    is_alpha( *(p+1), encoding_ )
			  || *(p+1) == '_' )
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Element\n" );
		#endif
		returnNode = new element( "" );
	}
	else
	{
		#ifdef DEBUG_PARSER
			TIXML_LOG( "XML parsing Unknown(2)\n" );
		#endif
		returnNode = new unknown();
	}

	if ( returnNode )
	{
		// Set the parent_, so it_ can report errors
		returnNode->parent_ = this;
	}
	else
	{
		if ( doc )
			doc->set_error( TIXML_ERROR_OUT_OF_MEMORY, 0, 0, TIXML_ENCODING_UNKNOWN );
	}
	return returnNode;
}

#ifdef TIXML_USE_STL

void element::stream_in (std::istream * in, TIXML_STRING * tag)
{
	// We're called with some amount of pre-parsing_. That is, some of "this"
	// element_ is in "tag". go ahead and stream_ to the closing ">"
	while( in->good() )
	{
		int c = in->get();
		if ( c <= 0 )
		{
			document* document_ = get_document();
			if ( document_ )
				document_->set_error( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
			return;
		}
		(*tag) += (char) c ;
		
		if ( c == '>' )
			break;
	}

	if ( tag->length() < 3 ) return;

	// Okay...if we are_ a "/>" tag, then we're done. We've read_ a complete_ tag.
	// If not, identify_ and stream_.

	if (    tag->at( tag->length() - 1 ) == '>' 
		 && tag->at( tag->length() - 2 ) == '/' )
	{
		// All good!
		return;
	}
	else if ( tag->at( tag->length() - 1 ) == '>' )
	{
		// There is more. could be:
		//		text_
		//		cdata text_ (which looks like another node_)
		//		closing tag
		//		another node_.
		for ( ;; )
		{
			stream_white_space( in, tag );

			// do we have text_?
			if ( in->good() && in->peek() != '<' ) 
			{
				// Yep, text_.
				text text_( "" );
				text_.stream_in( in, tag );

				// What follows text_ is a closing tag or another node_.
				// go around again and figure it_ out.
				continue;
			}

			// We now have either a closing tag...or another node_.
			// We should be at a "<", regardless.
			if ( !in->good() ) return;
			assert( in->peek() == '<' );
			int tagIndex = (int) tag->length();

			bool closingTag = false;
			bool firstCharFound = false;

			for( ;; )
			{
				if ( !in->good() )
					return;

				int c = in->peek();
				if ( c <= 0 )
				{
					document* document_ = get_document();
					if ( document_ )
						document_->set_error( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
					return;
				}
				
				if ( c == '>' )
					break;

				*tag += (char) c;
				in->get();

				// Early out if we find the CDATA id.
				if ( c == '[' && tag->size() >= 9 )
				{
					size_t len = tag->size();
					const char* start_ = tag->c_str() + len - 9;
					if ( strcmp( start_, "<![CDATA[" ) == 0 ) {
						assert( !closingTag );
						break;
					}
				}

				if ( !firstCharFound && c != '<' && !is_white_space( c ) )
				{
					firstCharFound = true;
					if ( c == '/' )
						closingTag = true;
				}
			}
			// If it_ was a closing tag, then read_ in the closing '>' to clean up the input stream_.
			// If it_ was not, the streaming_ will be done by the tag.
			if ( closingTag )
			{
				if ( !in->good() )
					return;

				int c = in->get();
				if ( c <= 0 )
				{
					document* document_ = get_document();
					if ( document_ )
						document_->set_error( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
					return;
				}
				assert( c == '>' );
				*tag += (char) c;

				// We are_ done, once we've found our_ closing tag.
				return;
			}
			else
			{
				// If not a closing tag, id it_, and stream_.
				const char* tagloc = tag->c_str() + tagIndex;
				node* node_ = identify( tagloc, TIXML_DEFAULT_ENCODING );
				if ( !node_ )
					return;
				node_->stream_in( in, tag );
				delete node_;
				node_ = 0;

				// no return: go_ around from the beginning: text_, closing tag, or node_.
			}
		}
	}
}
#endif

const char* element::parse( const char* p, parsing_data* data, encoding encoding_ )
{
	p = skip_white_space( p, encoding_ );
	document* document_ = get_document();

	if ( !p || !*p )
	{
		if ( document_ ) document_->set_error( TIXML_ERROR_PARSING_ELEMENT, 0, 0, encoding_ );
		return 0;
	}

	if ( data )
	{
		data->stamp( p, encoding_ );
		location_ = data->get_cursor();
	}

	if ( *p != '<' )
	{
		if ( document_ ) document_->set_error( TIXML_ERROR_PARSING_ELEMENT, p, data, encoding_ );
		return 0;
	}

	p = skip_white_space( p+1, encoding_ );

	// read the name_.
	const char* pErr = p;

    p = read_name( p, &value_, encoding_ );
	if ( !p || !*p )
	{
		if ( document_ )	document_->set_error( TIXML_ERROR_FAILED_TO_READ_ELEMENT_NAME, pErr, data, encoding_ );
		return 0;
	}

    TIXML_STRING endTag ("</");
	endTag += value_;
	endTag += ">";

	// Check for and read_ attributes_. Also look_ for an empty
	// tag or an end tag.
	while ( p && *p )
	{
		pErr = p;
		p = skip_white_space( p, encoding_ );
		if ( !p || !*p )
		{
			if ( document_ ) document_->set_error( TIXML_ERROR_READING_ATTRIBUTES, pErr, data, encoding_ );
			return 0;
		}
		if ( *p == '/' )
		{
			++p;
			// empty tag.
			if ( *p  != '>' )
			{
				if ( document_ ) document_->set_error( TIXML_ERROR_PARSING_EMPTY, p, data, encoding_ );		
				return 0;
			}
			return (p+1);
		}
		else if ( *p == '>' )
		{
			// Done with attributes_ (if there were any.)
			// read the value_ -- which can include other
			// elements -- read_ the end tag, and return.
			++p;
			p = read_value( p, data, encoding_ );		// Note this is an element method, and will set the error_ if one happens.
			if ( !p || !*p ) {
				// We were looking for the end tag, but found nothing.
				// Fix for [ 1663758 ] Failure to report error_ on bad XML
				if ( document_ ) document_->set_error( TIXML_ERROR_READING_END_TAG, p, data, encoding_ );
				return 0;
			}

			// We should find the end tag now
			if ( string_equal( p, endTag.c_str(), false, encoding_ ) )
			{
				p += endTag.length();
				return p;
			}
			else
			{
				if ( document_ ) document_->set_error( TIXML_ERROR_READING_END_TAG, p, data, encoding_ );
				return 0;
			}
		}
		else
		{
			// Try to read_ an attribute_:
			attribute* attrib = new attribute();
			if ( !attrib )
			{
				if ( document_ ) document_->set_error( TIXML_ERROR_OUT_OF_MEMORY, pErr, data, encoding_ );
				return 0;
			}

			attrib->set_document( document_ );
			pErr = p;
			p = attrib->parse( p, data, encoding_ );

			if ( !p || !*p )
			{
				if ( document_ ) document_->set_error( TIXML_ERROR_PARSING_ELEMENT, pErr, data, encoding_ );
				delete attrib;
				return 0;
			}

			// Handle the strange case of double attributes_:
			#ifdef TIXML_USE_STL
			attribute* node_ = attributeSet.find( attrib->name_t_str() );
			#else
			attribute* node_ = attributeSet.find( attrib->name() );
			#endif
			if ( node_ )
			{
				node_->set_value( attrib->value() );
				delete attrib;
				return 0;
			}

			attributeSet.add( attrib );
		}
	}
	return p;
}


const char* element::read_value( const char* p, parsing_data* data, encoding encoding_ )
{
	document* document_ = get_document();

	// read in text_ and elements in any order.
	const char* pWithWhiteSpace = p;
	p = skip_white_space( p, encoding_ );

	while ( p && *p )
	{
		if ( *p != '<' )
		{
			// Take what we have, make a text_ element_.
			text* textNode = new text( "" );

			if ( !textNode )
			{
				if ( document_ ) document_->set_error( TIXML_ERROR_OUT_OF_MEMORY, 0, 0, encoding_ );
				    return 0;
			}

			if ( base::is_white_space_condensed() )
			{
				p = textNode->parse( p, data, encoding_ );
			}
			else
			{
				// Special case: we want to keep the white_ space
				// so that leading spaces aren't removed.
				p = textNode->parse( pWithWhiteSpace, data, encoding_ );
			}

			if ( !textNode->blank() )
				link_end_child( textNode );
			else
				delete textNode;
		} 
		else 
		{
			// We hit a '<'
			// Have we hit a new element_ or an end tag? this could_ also be
			// a text in the "CDATA" style.
			if ( string_equal( p, "</", false, encoding_ ) )
			{
				return p;
			}
			else
			{
				node* node_ = identify( p, encoding_ );
				if ( node_ )
				{
					p = node_->parse( p, data, encoding_ );
					link_end_child( node_ );
				}				
				else
				{
					return 0;
				}
			}
		}
		pWithWhiteSpace = p;
		p = skip_white_space( p, encoding_ );
	}

	if ( !p )
	{
		if ( document_ ) document_->set_error( TIXML_ERROR_READING_ELEMENT_VALUE, 0, 0, encoding_ );
	}	
	return p;
}


#ifdef TIXML_USE_STL
void unknown::stream_in( std::istream * in, TIXML_STRING * tag )
{
	while ( in->good() )
	{
		int c = in->get();	
		if ( c <= 0 )
		{
			document* document_ = get_document();
			if ( document_ )
				document_->set_error( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
			return;
		}
		(*tag) += (char) c;

		if ( c == '>' )
		{
			// All is well.
			return;		
		}
	}
}
#endif


const char* unknown::parse( const char* p, parsing_data* data, encoding encoding_ )
{
	document* document_ = get_document();
	p = skip_white_space( p, encoding_ );

	if ( data )
	{
		data->stamp( p, encoding_ );
		location_ = data->get_cursor();
	}
	if ( !p || !*p || *p != '<' )
	{
		if ( document_ ) document_->set_error( TIXML_ERROR_PARSING_UNKNOWN, p, data, encoding_ );
		return 0;
	}
	++p;
    value_ = "";

	while ( p && *p && *p != '>' )
	{
		value_ += *p;
		++p;
	}

	if ( !p )
	{
		if ( document_ )	document_->set_error( TIXML_ERROR_PARSING_UNKNOWN, 0, 0, encoding_ );
	}
	if ( *p == '>' )
		return p+1;
	return p;
}

#ifdef TIXML_USE_STL
void comment::stream_in( std::istream * in, TIXML_STRING * tag )
{
	while ( in->good() )
	{
		int c = in->get();	
		if ( c <= 0 )
		{
			document* document_ = get_document();
			if ( document_ )
				document_->set_error( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
			return;
		}

		(*tag) += (char) c;

		if ( c == '>' 
			 && tag->at( tag->length() - 2 ) == '-'
			 && tag->at( tag->length() - 3 ) == '-' )
		{
			// All is well.
			return;		
		}
	}
}
#endif


const char* comment::parse( const char* p, parsing_data* data, encoding encoding_ )
{
	document* document_ = get_document();
	value_ = "";

	p = skip_white_space( p, encoding_ );

	if ( data )
	{
		data->stamp( p, encoding_ );
		location_ = data->get_cursor();
	}
	const char* startTag = "<!--";
	const char* endTag   = "-->";

	if ( !string_equal( p, startTag, false, encoding_ ) )
	{
		document_->set_error( TIXML_ERROR_PARSING_COMMENT, p, data, encoding_ );
		return 0;
	}
	p += strlen( startTag );

	// [ 1475201 ] TinyXML parses entities in comments_
	// Oops - read_text doesn't work, because we don't want to parse_ the entities.
	// p = read_text( p, &value_, false, endTag, false, encoding_ );
	//
	// from the XML spec:
	/*
	 [Definition: comments may appear anywhere in a document_ outside other markup; in addition, 
	              they may appear within the document_ type_ declaration_ at places allowed by the grammar. 
				  They are_ not part of the document_'s character data; an XML processor MAY, but need not, 
				  make it_ possible for an application to retrieve the text_ of comments_. For compatibility, 
				  the string "--" (double-hyphen) MUST NOT occur within comments_.] Parameter entity_ 
				  references MUST NOT be recognized within comments_.

				  An example of a comment_:

				  <!-- declarations for <head> & <body> -->
	*/

    value_ = "";
	// Keep all the white_ space.
	while (	p && *p && !string_equal( p, endTag, false, encoding_ ) )
	{
		value_.append( p, 1 );
		++p;
	}
	if ( p ) 
		p += strlen( endTag );

	return p;
}


const char* attribute::parse( const char* p, parsing_data* data, encoding encoding_ )
{
	p = skip_white_space( p, encoding_ );
	if ( !p || !*p ) return 0;

//	int tabsize = 4;
//	if ( document_ )
//		tabsize = document_->tab_size();

	if ( data )
	{
		data->stamp( p, encoding_ );
		location_ = data->get_cursor();
	}
	// read the name_, the '=' and the value_.
	const char* pErr = p;
	p = read_name( p, &name_, encoding_ );
	if ( !p || !*p )
	{
		if ( document_ ) document_->set_error( TIXML_ERROR_READING_ATTRIBUTES, pErr, data, encoding_ );
		return 0;
	}
	p = skip_white_space( p, encoding_ );
	if ( !p || !*p || *p != '=' )
	{
		if ( document_ ) document_->set_error( TIXML_ERROR_READING_ATTRIBUTES, p, data, encoding_ );
		return 0;
	}

	++p;	// skip '='
	p = skip_white_space( p, encoding_ );
	if ( !p || !*p )
	{
		if ( document_ ) document_->set_error( TIXML_ERROR_READING_ATTRIBUTES, p, data, encoding_ );
		return 0;
	}
	
	const char* end;
	const char SINGLE_QUOTE = '\'';
	const char DOUBLE_QUOTE = '\"';

	if ( *p == SINGLE_QUOTE )
	{
		++p;
		end = "\'";		// single quote in string
		p = read_text( p, &value_, false, end, false, encoding_ );
	}
	else if ( *p == DOUBLE_QUOTE )
	{
		++p;
		end = "\"";		// double quote in string
		p = read_text( p, &value_, false, end, false, encoding_ );
	}
	else
	{
		// All attribute_ values should be in single or double quotes.
		// But this is such a common error_ that the parser will try
		// its best, even without them.
		value_ = "";
		while (    p && *p											// existence
				&& !is_white_space( *p ) && *p != '\n' && *p != '\r'	// whitespace_
				&& *p != '/' && *p != '>' )							// tag end
		{
			if ( *p == SINGLE_QUOTE || *p == DOUBLE_QUOTE ) {
				// [ 1451649 ] get_attribute values with trailing quotes not handled correctly
				// We did not have an opening quote but seem to have a 
				// closing one. Give up and throw_ an error_.
				if ( document_ ) document_->set_error( TIXML_ERROR_READING_ATTRIBUTES, p, data, encoding_ );
				return 0;
			}
			value_ += *p;
			++p;
		}
	}
	return p;
}

#ifdef TIXML_USE_STL
void text::stream_in( std::istream * in, TIXML_STRING * tag )
{
	while ( in->good() )
	{
		int c = in->peek();	
		if ( !cdata && (c == '<' ) ) 
		{
			return;
		}
		if ( c <= 0 )
		{
			document* document_ = get_document();
			if ( document_ )
				document_->set_error( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
			return;
		}

		(*tag) += (char) c;
		in->get();	// "commits" the peek made above

		if ( cdata && c == '>' && tag->size() >= 3 ) {
			size_t len = tag->size();
			if ( (*tag)[len-2] == ']' && (*tag)[len-3] == ']' ) {
				// terminator of cdata.
				return;
			}
		}    
	}
}
#endif

const char* text::parse( const char* p, parsing_data* data, encoding encoding_ )
{
	value_ = "";
	document* document_ = get_document();

	if ( data )
	{
		data->stamp( p, encoding_ );
		location_ = data->get_cursor();
	}

	const char* const startTag = "<![CDATA[";
	const char* const endTag   = "]]>";

	if ( cdata || string_equal( p, startTag, false, encoding_ ) )
	{
		cdata = true;

		if ( !string_equal( p, startTag, false, encoding_ ) )
		{
			document_->set_error( TIXML_ERROR_PARSING_CDATA, p, data, encoding_ );
			return 0;
		}
		p += strlen( startTag );

		// Keep all the white_ space, ignore the encoding_, etc.
		while (	   p && *p
				&& !string_equal( p, endTag, false, encoding_ )
			  )
		{
			value_ += *p;
			++p;
		}

		TIXML_STRING dummy; 
		p = read_text( p, &dummy, false, endTag, false, encoding_ );
		return p;
	}
	else
	{
		bool ignoreWhite = true;

		const char* end = "<";
		p = read_text( p, &value_, ignoreWhite, end, false, encoding_ );
		if ( p )
			return p-1;	// don't truncate the '<'
		return 0;
	}
}

#ifdef TIXML_USE_STL
void declaration::stream_in( std::istream * in, TIXML_STRING * tag )
{
	while ( in->good() )
	{
		int c = in->get();
		if ( c <= 0 )
		{
			document* document_ = get_document();
			if ( document_ )
				document_->set_error( TIXML_ERROR_EMBEDDED_NULL, 0, 0, TIXML_ENCODING_UNKNOWN );
			return;
		}
		(*tag) += (char) c;

		if ( c == '>' )
		{
			// All is well.
			return;
		}
	}
}
#endif

const char* declaration::parse( const char* p, parsing_data* data, encoding _encoding )
{
	p = skip_white_space( p, _encoding );
	// find the beginning, find the end, and look_ for
	// the stuff in-between.
	document* document_ = get_document();
	if ( !p || !*p || !string_equal( p, "<?xml", true, _encoding ) )
	{
		if ( document_ ) document_->set_error( TIXML_ERROR_PARSING_DECLARATION, 0, 0, _encoding );
		return 0;
	}
	if ( data )
	{
		data->stamp( p, _encoding );
		location_ = data->get_cursor();
	}
	p += 5;

	version_ = "";
	encoding_ = "";
	standalone_ = "";

	while ( p && *p )
	{
		if ( *p == '>' )
		{
			++p;
			return p;
		}

		p = skip_white_space( p, _encoding );
		if ( string_equal( p, "version", true, _encoding ) )
		{
			attribute attrib;
			p = attrib.parse( p, data, _encoding );		
			version_ = attrib.value();
		}
		else if ( string_equal( p, "encoding", true, _encoding ) )
		{
			attribute attrib;
			p = attrib.parse( p, data, _encoding );		
			encoding_ = attrib.value();
		}
		else if ( string_equal( p, "standalone", true, _encoding ) )
		{
			attribute attrib;
			p = attrib.parse( p, data, _encoding );		
			standalone_ = attrib.value();
		}
		else
		{
			// read over whatever it_ is.
			while( p && *p && *p != '>' && !is_white_space( *p ) )
				++p;
		}
	}
	return 0;
}

bool text::blank() const
{
	for ( unsigned i=0; i<value_.length(); i++ )
		if ( !is_white_space( value_[i] ) )
			return false;
	return true;
}

} // namespace tinyxml
