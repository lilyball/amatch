Gem::Specification.new do |s|
  s.name = 'amatch'
  s.version = "0.2.4"
  s.summary = "Approximate String Matching library"
  s.description = <<EOF
Amatch is a library for approximate string matching and searching in strings.
Several algorithms can be used to do this, and it's also possible to compute a
similarity metric number between 0.0 and 1.0 for two given strings.
EOF

  s.files = ["AUTHORS", "bin", "bin/agrep.rb", "CHANGES", "ext", "ext/amatch.bundle", "ext/amatch.c", "ext/amatch.o", "ext/extconf.rb", "ext/Makefile", "ext/MANIFEST", "ext/pair.c", "ext/pair.h", "ext/pair.o", "GPL", "install.rb", "Rakefile", "README.en", "tests", "tests/runner.rb", "tests/test_hamming.rb", "tests/test_jaro.rb", "tests/test_jaro_winkler.rb", "tests/test_levenshtein.rb", "tests/test_longest_subsequence.rb", "tests/test_longest_substring.rb", "tests/test_pair_distance.rb", "tests/test_sellers.rb", "VERSION"]

  s.extensions << "ext/extconf.rb"

  s.require_path = 'ext'

  s.bindir = "bin"
  s.executables = ["agrep.rb"]
  s.default_executable = "agrep.rb"

  s.has_rdoc = true
  s.rdoc_options <<
    '--title' <<  'Amatch -- Approximate Matching' <<
    '--main' << 'Amatch' <<
    '--line-numbers'
  s.test_files << 'tests/runner.rb'

  s.author = "Florian Frank"
  s.email = "flori@ping.de"
  s.homepage = "http://amatch.rubyforge.org"
  s.rubyforge_project = "amatch"
end
