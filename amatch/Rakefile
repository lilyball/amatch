# vim: set et sw=2 ts=2:
require 'rake/clean'
require 'rake/testtask'
require 'rake/gempackagetask'
require 'rake/rdoctask'
require 'rbconfig'

include Config

PKG_VERSION = File.read('VERSION').chomp
PKG_FILES   = FileList['**/*']
PKG_FILES.exclude(/CVS/)
PKG_FILES.exclude(/^pkg/)
PKG_FILES.exclude(/^doc/)

task :default => :test

desc "Run unit tests"
task :test => :compile do
  cd 'tests' do
    ruby %{-I../ext runner.rb}
  end
end

desc "Compiling library"
task :compile do
  cd 'ext'  do
    ruby %{extconf.rb}
    sh "make"
  end
end

desc "Installing library"
task :install => :test do
  src, = Dir['ext/amatch.*'].reject { |x| /\.[co]$/.match x }
  filename = File.basename(src)
  dst = File.join(CONFIG["sitelibdir"], filename)
  install(src, dst, :verbose => true)
end

desc "Removing generated files"
task :clean do
  rm_rf 'doc'
  cd 'ext'  do
    ruby 'extconf.rb'
    sh "make distclean" if File.exist?('Makefile')
  end
end

Rake::RDocTask.new do |rd|
  rd.main = 'Amatch'
  rd.rdoc_files.include("ext/amatch.c")
  rd.rdoc_dir = 'doc'
end

spec = Gem::Specification.new do |s|
  #### Basic information.

  s.name = 'amatch'
  s.version = PKG_VERSION
  s.summary = "Approximate String Matching library"
  s.description = <<EOF
Amatch is a library for approximate string matching and searching in strings.
Several algorithms can be used to do this, and it's also possible to compute a
similarity metric number between 0.0 and 1.0 for two given strings.
EOF

  #### Dependencies and requirements.

  #s.add_dependency('log4r', '> 1.0.4')
  #s.requirements << ""

  s.files = PKG_FILES

  #### C code extensions.

  s.extensions << "ext/extconf.rb"

  #### Load-time details: library and application (you will need one or both).

  s.require_path = 'ext'                         # Use these for libraries.
  s.autorequire = 'amatch'

  s.bindir = "bin"                               # Use these for applications.
  s.executables = ["agrep.rb"]
  s.default_executable = "agrep.rb"

  #### Documentation and testing.

  s.has_rdoc = true
  #s.extra_rdoc_files = FileList['ext/amatch.c']
  s.rdoc_options <<
    '--title' <<  'Amatch -- Approximate Matching' <<
    '--main' << 'Amatch' <<
    '--line-numbers'
  s.test_files << 'tests/runner.rb'

  #### Author and project details.

  s.author = "Florian Frank"
  s.email = "flori@ping.de"
  s.homepage = "http://amatch.rubyforge.org"
  s.rubyforge_project = "amatch"
end

Rake::GemPackageTask.new(spec) do |pkg|
  pkg.need_tar      = true
  pkg.package_files += PKG_FILES
end

task :release => [ :clean, :package ]
