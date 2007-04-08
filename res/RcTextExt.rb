# This ruby script ...

resource_array = Array.new
resource_file = File.open('Halite.rc').gets(nil)

# First we strip comments so they don't confuse the map and also lines starting with '#'
r = Regexp.new('\/\*.*?\*\/', Regexp::MULTILINE)
resource_file.gsub!(/\/\/.*/, '')
resource_file.gsub!(/#.*/, '')
resource_file.gsub!(r, '')

# Collect all string in the resource file
resource_file.scan(/\".*?\"/) do |text_string|
	if not resource_array.include?(text_string)
		resource_array.push(text_string)
	end
end

ARGV.each do |arg| 
	# Parse any translated strings already present.
	
	lang_file = File.open(arg+'.txt', File::CREAT).gets(nil)
	lang_map = Hash.new
	
	if lang_file 
		lang_file.scan(/(\".*?\")\s*--->\s*(\".*?\")/) {|original, trans| lang_map[original] = trans}
	end
	
	resource_lang_file = File.open('Halite.rc').gets(nil)
	
    resource_lang_file.gsub!(/\".*?\"/) do |text_string|
        if lang_map.has_key?(text_string)
            lang_map[text_string]
        else
            text_string
        end
    end

    File.new(arg+'.rc', "w").print(resource_lang_file)
	
	lang_file = File.new(arg+".txt", File::CREAT|File::TRUNC|File::RDWR)
	
	resource_array.each do |value| 	
		if lang_map.has_key?(value)
            lang_file.print(value + " ---> " + lang_map[value] + "\n")
        else
            lang_file.print(value + " --->  ??? \n")
        end
	end	
end
