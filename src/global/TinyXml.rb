# This ruby script undoes the CamelCase in TinyXML's public interface. It also
# drops the "TiXml" prefixes. As of v 2.5.2 you will need to remove the lines
# 1666 - 1682 (the deprecated functions) from tinyxml.h and also wrap the code
# in a "namespace tinyxml {}" manually.

map = Hash.new

# We'll go through each file to generate the map prior to making any changes

ARGV.each do |arg|
    whole_file = File.open(arg).gets(nil)

    # First we strip comments so they don't confuse the map
    r = Regexp.new('\/\*.*?\*\/', Regexp::MULTILINE)

    whole_file.gsub!(/\/\/.*/, '')
    whole_file.gsub!(r, '')
#   puts whole_file

    whole_file.gsub(/\w+/) do |word|

        if word =~ /(\b([A-Z][a-z]+)+\w*)/

            newWord = word.gsub(/\bTiXml/, '')

            newWord.gsub!(/\b([A-Z])/) { $&.downcase }
            newWord.gsub!(/[A-Z]/) { '_' + $&.downcase }

            map[word] = newWord
        end
    end
end

# Special case since we won't use TiXmlString anyway.
map["TiXmlString"] = "TiXmlString"
map["string"] = "string"

# Stop some unwanted changes
map["size"] = "size"
map["str"] = "str"
map["find"] = "find"
map["fail"] = "fail"
map["this"] = "this"
map["empty"] = "empty"
map["assign"] = "assign"

# Correct duplicates
map["Attribute"] = "get_attribute"
map["Encoding"] = "get_encoding"
map["Cursor"] = "get_cursor"

map.each { |key, value| puts key + ' -> '+ value }

# Now go through the files again this time making the substitutions everywhere
# EXCEPT within quoted text.

ARGV.each do |arg|
    whole_file = File.open(arg).gets(nil)

    cycle = true

    whole_file.gsub!(/\\"/) { "escaped-single-quote" }
    whole_file.gsub!(/""/) { "empty-double-quote" }

    r = Regexp.new('[^"]+', Regexp::MULTILINE)

    whole_file.gsub!(r) do |non_quote|

        if (cycle)
            non_quote.gsub!(/\w+/) do |word|

                if map.has_key?(word)
                    map[word]
                elsif map.has_value?(word)
                    word+'_'
                else
                    word
                end
            end
        end
        cycle = !cycle

        non_quote
    end

    whole_file.gsub!(/escaped-single-quote/) { '\"' }
    whole_file.gsub!(/empty-double-quote/) { '""' }

    File.new('./out/' +arg, "w").print(whole_file)
end

