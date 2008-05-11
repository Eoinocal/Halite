# This ruby script ... has very little in the way of error checking ;-)

require "iconv"
require "yaml"
require "rubyscript2exe"

exit if RUBYSCRIPT2EXE.is_compiling?

resource_dir = ARGV.shift
resource_name = ARGV.shift

resource_array = Array.new
resource_filename = resource_dir+resource_name+'.rc'
resource_original_file = ""

ignore_filename = "ignore.yaml"
ignore_array = Array.new

begin
	puts " + "+resource_filename
	res_language_file = File.open(resource_filename, "r+b")

	if resource_original_file = res_language_file.gets(nil)
		
		begin
			resource_original_file = Iconv.iconv('UTF-8', 'UTF-16LE', 
				resource_original_file).join
				
			puts " - loaded base resource file."	
		rescue
			puts " ! Uincode conversion issue."
		end	

		resource_file = resource_original_file.clone
		
		begin	# Parse the translation file.	
			File.open(ignore_filename, File::CREAT) do |ignore_yaml_file|
				ignore_array = YAML::load( ignore_yaml_file ) 
			end
		rescue
			puts " ! No ignore file."
		end
		
		# First we strip comments so they don't confuse the map and also lines starting with '#'
		resource_file.gsub!(/\/\/.*/, "\n")
		resource_file.gsub!(/#.*/, "\n")
		resource_file.gsub!(Regexp.new('\/\*.*?\*\/', Regexp::MULTILINE), "\n")

		# Collect all string in the resource file
		resource_file.scan(/\".*?\"/) do |text_string|
			if (not resource_array.include?(text_string)) and
					(not ignore_array.include?(text_string))
				resource_array.push(text_string)
			end
		end
	end

rescue
	puts " ! Problem reading base resource file."
	exit
end	
		
ARGV.each do |arg| 

	# Parse any translated strings already present.	
	
	lang_filename = arg+".txt"
	lang_map = Hash.new
	
	puts "\n + "+lang_filename
	
	File.open(lang_filename, 'r+') do |language_file|
	
		while lang_line = language_file.gets
			lang_line.scan(/(\".*?\")\s*--->\s*(\".*?\")/) {|original, trans| lang_map[original] = trans}
		end
	end
	
	puts " - - Scanned"	
	
	begin
		#res_language_file = File.open(resource_filename, "r+")
	
		if res_lang = resource_original_file.clone
		
			res_lang.gsub!(/\".*?\"/) do |text_string|
				if lang_map.has_key?(text_string)
					lang_map[text_string]
				else
					text_string
				end
			end
			
			begin
				local_file = File.new(resource_dir+arg+'.rc', "w+b")
				local_file.print(Iconv.iconv('UTF-16LE', 'UTF-8', res_lang))
				local_file.close
				puts " - - Constructed localized resource file"		
			rescue
				puts " ! Problem writing localized resource file."
				next 
			end	
		end
		
	rescue
		puts " ! Problem reading/creating existing localized resource file."
		next 
	end		
	
	begin	
		lang_file = File.new(lang_filename, 'w+')

		resource_array.sort.each do |value| 	
			if lang_map.has_key?(value)
				lang_file.print(value + " ---> " + lang_map[value] + "\n")
			else
				lang_file.print(value + " --->  ??? \n")
			end
		end	
	rescue
		puts " ! Problem writing new language file."
		next 
	end	
	
	puts " - - Wrote new language file"
	
	puts "\n - Compiling resource file\n\n"
	
	system "call tools\\rc \"..\\res\\"+arg+".rc\""

	system "call tools\\cvtres /MACHINE:X86 \"..\\res\\"+arg+".res\""
	system "call tools\\link /NOENTRY /DLL /MACHINE:X86 /OUT:\".\\bin\\x86\\"+arg+".dll\" \"..\\res\\"+arg+".obj\""

	system "call tools\\cvtres /MACHINE:AMD64 \"..\\res\\"+arg+".res\""
	system "call tools\\link /NOENTRY /DLL /MACHINE:AMD64 /OUT:\".\\bin\\x64\\"+arg+".dll\" \"..\\res\\"+arg+".obj\""

	system "del \"..\\res\\"+arg+".rc\""
	system "del \"..\\res\\"+arg+".res\""
	system "del \"..\\res\\"+arg+".obj\""
	
	puts " - Done\n\n"	
end

puts " - Completed sucessfully"	
