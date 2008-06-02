/*
	Copyright (c) 2007, Cory Nelson
	All rights reserved.
	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
	
		* Redistributions of source code must retain the above copyright
		  notice, this list of conditions and the following disclaimer.
		* Redistributions in binary form must reproduce the above copyright
		  notice, this list of conditions and the following disclaimer in the
		  documentation and/or other materials provided with the distribution.
		* Neither the name of Int64.org nor the names of its contributors may
		  be used to endorse or promote products derived from this software
		  without specific prior written permission.

	 THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND ANY
	 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	 DISCLAIMED. IN NO EVENT SHALL THE REGENTS AND CONTRIBUTORS BE LIABLE FOR ANY
	 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	Contact Info:
		email:   phrosty@gmail.com
		website: http://www.int64.org
*/

#ifndef UNICODE_HPP
#define UNICODE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
#pragma once
#endif

#include <cstddef>
#include <iterator>
#include <algorithm>
#include <stdexcept>
#include <boost/cstdint.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/mpl/sizeof.hpp>

namespace unicode {
	typedef boost::uint8_t utf8_t;
	typedef boost::uint16_t utf16_t;
	typedef boost::uint32_t utf32_t;

	class unicode_error : public std::exception {
	private:
		const char* const m_msg;

	public:
		unicode_error(const char *msg) : m_msg(msg) {}
		const char* what() const throw() { return m_msg; }
	};

	class utf8 {
	private:
		template<typename InputIterator>
		static utf32_t decode_part(InputIterator &iter, InputIterator &end) {
			if(iter == end) {
				throw unicode_error("incomplete utf-8 sequence");
			}

			utf32_t part = static_cast<utf8_t>(*iter);

			if((part & 0xC0) != 0x80) {
				throw unicode_error("invalid utf-8 octet");
			}

			++iter;
			return part & 0x3F;
		}

	public:
		typedef utf8_t char_type;
		static const unsigned int max_encodelen = 4;

		template<typename InputIterator>
		static utf32_t decode(InputIterator &iter, InputIterator &end) {
			utf32_t part, val = 0;

			if(iter == end) {
				throw unicode_error("empty iterator range");
			}

			part = static_cast<utf8_t>(*iter);
			if(!(part & 0x80)) { // 1 octet
				++iter;
				val = part;
			}
			else if((part & 0xE0) == 0xC0) { // 2 octets
				++iter;
				val = (part & 0x1F) << 6;
				val |= decode_part(iter, end);

				if(val < 0x80) {
					throw unicode_error("overlong utf-8 sequence");
				}
			}
			else if((part & 0xF0) == 0xE0) { // 3 octets
				++iter;
				val = (part & 0xF) << 12;
				val |= decode_part(iter, end) << 6;
				val |= decode_part(iter, end);

				if(val < 0x800) {
					throw unicode_error("overlong utf-8 sequence");
				}
			}
			else if((part & 0xF8) == 0xF0) { // 4 octets
				++iter;
				val = (part & 0x7) << 18;
				val |= decode_part(iter, end) << 12;
				val |= decode_part(iter, end) << 6;
				val |= decode_part(iter, end);

				if(val < 0x10000) {
					throw unicode_error("overlong utf-8 sequence");
				}
			}
			else {
				throw unicode_error("invalid utf-8 octet");
			}

			if(val > 0x10FFFF) {
				throw unicode_error("invalid unicode character");
			}

			return val;
		}

		template<typename OutputIterator>
		static OutputIterator encode(utf32_t val, OutputIterator iter) {
			if(val <= 0x7F) {
				*iter = static_cast<utf8_t>(val); ++iter;
			}
			else if(val <= 0x7FF) {
				*iter = static_cast<utf8_t>(0xC0 | ((val & 0x7C0) >> 6)); ++iter;
				*iter = static_cast<utf8_t>(0x80 | (val & 0x3F)); ++iter;
			}
			else if(val <= 0xFFFF) {
				*iter = static_cast<utf8_t>(0xE0 | ((val & 0xF000) >> 12)); ++iter;
				*iter = static_cast<utf8_t>(0x80 | ((val & 0xFC0) >> 6)); ++iter;
				*iter = static_cast<utf8_t>(0x80 | (val & 0x3F)); ++iter;
			}
			else if(val <= 0x10FFFF) {
				*iter = static_cast<utf8_t>(0xF0 | ((val & 0x1C0000) >> 18)); ++iter;
				*iter = static_cast<utf8_t>(0x80 | ((val & 0x3F000) >> 12)); ++iter;
				*iter = static_cast<utf8_t>(0x80 | ((val & 0xFC0) >> 6)); ++iter;
				*iter = static_cast<utf8_t>(0x80 | (val & 0x3F)); ++iter;
			}
			else {
				throw unicode_error("invalid unicode character");
			}

			return iter;
		}
	};

	struct host_endian {
		static utf16_t swap(utf16_t i) { return i; }
		static utf32_t swap(utf32_t i) { return i; }
	};

	struct swap_endian {
		static utf16_t swap(utf16_t i) { return (i >> 8) | (i << 8); }

		static utf32_t swap(utf32_t i) {
			return (
				(i >> 24) |
				((i & 0x00FF0000) >> 8) |
				((i & 0x0000FF00) << 8) |
				(i << 24)
			);
		}
	};

#ifdef BIG_ENDIAN
	typedef swap_endian little_endian;
	typedef host_endian big_endian;
#else
	typedef host_endian little_endian;
	typedef swap_endian big_endian;
#endif

	template<typename Endian>
	class utf16 {
	public:
		typedef utf16_t char_type;
		static const unsigned int max_encodelen = 2;

		template<typename InputIterator>
		static utf32_t decode(InputIterator &iter, InputIterator &end) {
			utf32_t part, val = 0;

			if(iter == end) {
				throw unicode_error("empty iterator range");
			}

			part = Endian::swap(static_cast<utf16_t>(*iter));
			if(part < 0xD800 || part > 0xDFFF) {
				val = part;
			}
			else {
				val = (part & 0x3FF) << 10;

				if(++iter == end) {
					throw unicode_error("incomplete utf-16 surrogate pair");
				}

				part = Endian::swap(static_cast<utf16_t>(*iter));
				val |= (part & 0x3FF);
			}
			
			++iter;

			if(val > 0x10FFFF) {
				throw unicode_error("invalid unicode character");
			}

			return val;
		}

		template<typename OutputIterator>
		static OutputIterator encode(utf32_t val, OutputIterator iter) {
			if(val <= 0xFFFF) {
				*iter = Endian::swap(static_cast<utf16_t>(val)); ++iter;
			}
			else if(val <= 0x10FFFF) {
				val -= 0x10000;

				*iter = Endian::swap(static_cast<utf16_t>(0xD800 | (val >> 10))); ++iter;
				*iter = Endian::swap(static_cast<utf16_t>(0xDC00 | (val & 0x3FF))); ++iter;
			}
			else {
				throw unicode_error("invalid unicode character");
			}

			return iter;
		}
	};

	typedef utf16<little_endian> utf16le;
	typedef utf16<big_endian> utf16be;

	template<typename Endian>
	class utf32 {
	public:
		typedef utf32_t char_type;
		static const unsigned int max_encodelen = 1;

		template<typename InputIterator>
		static utf32_t decode(InputIterator &iter, InputIterator &end) {
			if(iter == end) {
				throw unicode_error("empty iterator range");
			}

			utf32_t val = Endian::swap(static_cast<utf32_t>(*iter)); ++iter;

			if(val > 0x10FFFF) {
				throw unicode_error("invalid unicode character");
			}

			return val;
		}

		template<typename OutputIterator>
		static OutputIterator encode(utf32_t val, OutputIterator iter) {
			if(val > 0x10FFFF) {
				throw unicode_error("invalid unicode character");
			}

			*iter = Endian::swap(val);
			return ++iter;
		}
	};

	typedef utf32<little_endian> utf32le;
	typedef utf32<big_endian> utf32be;

	// wchar_encoding is a typedef for the proper encoding for your platform.  assumes wchar_t is UTF-16 or UTF-32.
	typedef boost::mpl::if_<
		boost::mpl::equal_to<boost::mpl::sizeof_<wchar_t>, boost::mpl::sizeof_<utf16_t> >,
		utf16<host_endian>,
		utf32<host_endian>
	>::type wchar_encoding;

	template<typename InEnc, typename OutEnc, typename InputIterator>
	class transcode_iterator : public std::iterator<std::input_iterator_tag, typename OutEnc::char_type> {
	public:
		typedef InEnc input_encoding;
		typedef OutEnc output_encoding;
		typedef InputIterator iterator_type;
		
		typedef std::iterator<std::input_iterator_tag, typename OutEnc::char_type> base_type;
		typedef transcode_iterator<input_encoding, output_encoding, iterator_type> this_type;

	private:
		iterator_type m_iter, m_end;
		typename base_type::value_type m_out[OutEnc::max_encodelen];
		unsigned int m_outpos, m_outlen;
		typename base_type::value_type m_ch;
		bool m_done;

		void getval() {
			if(!m_done) {
				if(m_outlen) {
					m_ch = m_out[m_outpos];

					if(++m_outpos == m_outlen) {
						m_outlen = 0;
					}
				}
				else {
					if(m_iter != m_end) {
						utf32_t ch = InEnc::decode(m_iter, m_end);

						typename base_type::value_type *end = OutEnc::encode(ch, m_out);

						if(OutEnc::max_encodelen > 1) {
							unsigned int len = static_cast<unsigned int>(std::distance(m_out, end));

							if(len > 1) {
								m_outpos = 1;
								m_outlen = len;
							}
						}

						m_ch = m_out[0];
					}
					else {
						m_done = true;
					}
				}
			}
		}

	public:
		transcode_iterator() : m_done(true) {}
		transcode_iterator(iterator_type iter, iterator_type end) : m_iter(iter),m_end(end),m_outpos(0),m_outlen(0),m_done(false) {
			getval();
		}

		this_type& operator++() {
			getval();
			return *this;
		}

		this_type operator++(int) {
			this_type ret = *this;
			++*this;
			return ret;
		}

		bool operator==(const this_type &iter) const { return m_done == iter.m_done; }
		bool operator!=(const this_type &iter) const { return m_done != iter.m_done; }

		const typename base_type::value_type operator*() const { return m_ch; }
	};

	template<typename InEnc, typename OutEnc, typename InputIterator, typename OutputIterator>
	OutputIterator transcode(InputIterator iter, InputIterator end, OutputIterator out) {
		while(iter != end) {
			utf32_t ch = InEnc::decode(iter, end);
			out = OutEnc::encode(ch, out);
		}

		return out;
	}

	namespace detail {
		template<typename LengthT, typename CharT>
		class length_iterator : public std::iterator<std::output_iterator_tag, CharT> {
		public:
      typedef std::iterator<std::output_iterator_tag, CharT> base_type;
			typedef length_iterator<LengthT, CharT> this_type;

		private:
			LengthT m_len;
			CharT m_ch;

		public:
			length_iterator() : m_len(0) {}

			this_type& operator++() { return *this; }

			this_type operator++(int) {
				this_type ret = *this;
				++*this;
				return ret;
			}

			typename base_type::value_type& operator*() { return m_ch; }

			LengthT length() const { return m_len; }
		};
	}

	template<typename InEnc, typename OutEnc, typename InputIterator>
	typename std::iterator_traits<InputIterator>::difference_type transcode_length(InputIterator iter, InputIterator end) {
		typedef typename std::iterator_traits<InputIterator>::difference_type length_type;
		typedef detail::length_iterator<length_type, typename OutEnc::char_type> length_iterator;

		return transcode<InEnc, OutEnc, InputIterator>(iter, end, length_iterator()).length();
	}

	template<typename InEnc, typename InputIterator>
	typename std::iterator_traits<InputIterator>::difference_type length(InputIterator iter, InputIterator end) {
		typename std::iterator_traits<InputIterator>::difference_type len = 0;

		while(iter != end) {
			InEnc::decode(iter, end);
			++len;
		}

		return len;
	}
}

#endif
