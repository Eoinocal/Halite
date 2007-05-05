# This ruby script ...

if ARGV.size == 0 
	puts "usage RcTextExt resource_file languages" 
	exit
end

resource_file_name = ARGV.shift
resource_array = Array.new
resource_file = File.open(resource_file_name).gets(nil)

puts "Working with "+resource_file_name

# First we strip comments and lines starting with '#' so they don't confuse the map
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
	
	lang_file_name = "../lang/"+arg+'.txt'
	puts "Processing "+lang_file_name
	
	lang_file = File.open(lang_file_name, File::CREAT).gets(nil)
	lang_map = Hash.new
	
	if lang_file 
		lang_file.scan(/(\".*?\")\s*--->\s*(\".*?\")/) {|original, trans| lang_map[original] = trans}
	end
	
	resource_lang_file = File.open(resource_file_name).gets(nil)
	
    resource_lang_file.gsub!(/\".*?\"/) do |text_string|
        if lang_map.has_key?(text_string)
            lang_map[text_string]
        else
            text_string
        end
    end

    File.new(arg+'.rc', "w").print(resource_lang_file)
	
	lang_file = File.new(lang_file_name, File::CREAT|File::TRUNC|File::RDWR)
	
	resource_array.each do |value| 	
		if lang_map.has_key?(value)
            lang_file.print(value + " ---> " + lang_map[value] + "\n")
        else
            lang_file.print(value + " --->  ??? \n")
        end
	end	
end
