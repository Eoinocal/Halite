# This ruby script ... has very little in the way of error checking ;-)

# require "iconv"
require "yaml"

exit if Object.const_defined?(:Ocra)

def compile_dlls(arg)

	puts " - - Wrote new language file"
	
	puts "\n - - Compiling resource file\n\n"
	
	system "call rc /I \"..\\src\" \"..\\res\\"+arg+".rc\""

	system "call cvtres /MACHINE:X86 \"..\\res\\"+arg+".res\""
	system "call link /NOENTRY /DLL /MACHINE:X86 /OUT:\".\\bin\\"+arg+".dll\" \"..\\res\\"+arg+".obj\""

	puts " - - Cleaning temp files\n"
	
	system "del \"..\\res\\"+arg+".rc\""
	system "del \"..\\res\\"+arg+".res\""
	system "del \"..\\res\\"+arg+".obj\""
	
	puts " - Done\n"	
end

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
		#	resource_original_file = Iconv.iconv('UTF-8', 'UTF-16LE', 
		#		resource_original_file).join
				
			resource_original_file.encode!('UTF-8', 'UTF-16LE')
				
			puts " - loaded base resource file."	
		rescue
			puts " ! Unicode conversion issue."
		end	

		resource_original_file.force_encoding("ASCII-8BIT")
		resource_file = resource_original_file.clone
		puts resource_original_file.encoding
		
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

		# Collect all strings in the resource file
		resource_file.scan(/\".*?\"/) do |text_string|
			text_string.force_encoding("utf-8")
			if (not ignore_array.include?(text_string))				
				
				if text_string.include?(';')
					# Split semicolon delimited strings
					text_string.delete('"').split(';').each {|s| resource_array.push('"'+s+'"') }					
				else
					resource_array.push(text_string)					
				#	puts text_string
				end
				
			end
		end
		
		resource_array.uniq!
	end

rescue
	puts " ! Problem reading base resource file."
	exit
end	

#	puts resource_array
		
ARGV.each do |arg| 

	# Parse any translated strings already present.	
	
	lang_filename = arg+".txt"
	lang_map = Hash.new
	
	puts "\n + "+lang_filename
	
	File.open(lang_filename, 'r+:utf-8') do |language_file|
		while lang_line = language_file.gets
			lang_line.force_encoding("ASCII-8BIT")
			lang_line.scan(/(\".*?\")\s*--->\s*(\".*?\")/) do |original, trans| 
				
				if original.include?(';')
					# Split semicolon delimited strings
					
					o = original.delete('"').split(';')
					t = trans.delete('"').split(';')
					 
					o.each_index {|i| lang_map['"'+o[i]+'"'] = '"'+t[i]+'"' }				
				else
					lang_map[original] = trans
				end
			end
		end
	end
	
	puts " - - Scanned"	
	
#	puts lang_map
	
	begin	
		if res_lang = resource_original_file.clone
		
			puts " - - Building map..."
		
			res_lang.gsub!(/\".*?\"/) do |text_string|

				if text_string.include?(';')
					# Rebuild semicolon delimited strings
				
					tmp = ''
					
					text_string.delete('"').split(';').each do |s| 
					
						if lang_map.has_key?('"'+s+'"')
							tmp = tmp+ lang_map['"'+s+'"'].delete('"') +';'
						else		
							tmp = tmp+s+';' 
						end				
					end
					
					tmp = '"'+tmp.chomp!(';')+'"'
										
				else
					if lang_map.has_key?(text_string)
						lang_map[text_string].force_encoding("ASCII-8BIT")
					else
						text_string
					end
				end
			end
			
			puts " - - Map built"				
			
			begin
				local_file = File.new(resource_dir+arg+'.rc', "w+b")
				res_lang.encode!('UTF-16LE', 'UTF-8')
				local_file.print(res_lang)
				local_file.close
				puts " - - Constructed localized resource file"		
			rescue
				puts " ! Problem writing localized resource file."
				next 
			end	
		end
		
	rescue Exception => e
		puts "#{ e } (#{ e.class })!"
		puts " ! Problem reading/creating existing localized resource file."
		
		next 
	end		
	
	#puts resource_array
	
	begin	
		lang_file = File.new(lang_filename, 'w+:utf-8')
		
		# Special case for 'English' and 'English.rtf' to bring them to the top of list.
		
		resource_array.delete('"English"')
		if lang_map.has_key?('"English"')
			lang_file.print('"English"' + " ---> " + lang_map['"English"'].force_encoding("UTF-8") + "\n")
		else
			lang_file.print('"English"' + " --->  ??? \n")
		end
			
		resource_array.delete('"English.rtf"')
		if lang_map.has_key?('"English.rtf"')
			lang_file.print('"English.rtf"' + " ---> " + lang_map['"English.rtf"'].force_encoding("UTF-8") + "\n")
		else
			lang_file.print('"English.rtf"' + " --->  ??? \n")
		end	
		
		# Process rest alphabetically [sic].

		resource_array.sort.each do |value| 
			if lang_map.has_key?(value)	
				lang_file.print(value + " ---> " + lang_map[value].force_encoding("UTF-8") + "\n")
			else	
				lang_file.print(value + " --->  ??? \n")
			end
		end	
	rescue
		puts " ! Problem writing new language file."
		next 
	end	
	
	compile_dlls(arg)
end

puts " - Completed successfully"	