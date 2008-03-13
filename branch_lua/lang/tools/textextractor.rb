# This ruby script ... has very little in the way of error checking ;-)
require "rubyscript2exe"
exit if RUBYSCRIPT2EXE.is_compiling?

resource_name = ARGV.shift

# The conversion from utf-16 to utf-8 is now done by the batch file.
# system(".\\from_utf16.bat " + resource_name+'.rc ' + resource_name+'.utf8.rc')

resource_array = Array.new
resource_filename = resource_name+'.in'
resource_file = File.open(resource_filename).gets(nil)

# First we strip comments so they don't confuse the map and also lines starting with '#'
resource_file.gsub!(/\/\/.*/, '')
resource_file.gsub!(/#.*/, '')
resource_file.gsub!(Regexp.new('\/\*.*?\*\/', Regexp::MULTILINE), '')

# Collect all string in the resource file
resource_file.scan(/\".*?\"/) do |text_string|
	if not resource_array.include?(text_string)
		resource_array.push(text_string)
	end
end

ARGV.each do |arg| 
	# Parse any translated strings already present.
	
	lang_filename = arg+".txt"
	lang_file = File.open(lang_filename, File::CREAT).gets(nil)
	lang_map = Hash.new
	
	if lang_file 
		lang_file.scan(/(\".*?\")\s*--->\s*(\".*?\")/) {|original, trans| lang_map[original] = trans}
	end
	
	resource_lang_file = File.open(resource_filename).gets(nil)
	
    resource_lang_file.gsub!(/\".*?\"/) do |text_string|
        if lang_map.has_key?(text_string)
            lang_map[text_string]
        else
            text_string
        end
    end

    File.new(arg+'.out', "w").print(resource_lang_file)
	
	lang_file = File.new(lang_filename, File::CREAT|File::TRUNC|File::RDWR)
	
	resource_array.each do |value| 	
		if lang_map.has_key?(value)
            lang_file.print(value + " ---> " + lang_map[value] + "\n")
        else
            lang_file.print(value + " --->  ??? \n")
        end
	end	
	
#	system(".\\to_utf16.bat " + arg+'.utf8.rc ' + arg+'.rc')
end
