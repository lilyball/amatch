# vim: set et sw=2 ts=2:
require 'rake/clean'
require 'rake/testtask'
require 'rake/rdoctask'
begin
  require 'rake/gempackagetask'
rescue LoadError
end
require 'rbconfig'
include Config

PKG_VERSION = File.read('VERSION').chomp
PKG_FILES   = FileList['**/*']
PKG_FILES.exclude(/CVS/)
PKG_FILES.exclude(/^pkg/)
PKG_FILES.exclude(/^doc/)
PKG_FILES.exclude(/^amatch.gemspec$/)

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

if defined? Gem
  # ugly ugly hack to generate the amatch.gemspec file for github
  gemspec = <<GEMSPEC
Gem::Specification.new do |s|
  s.name = 'amatch'
  s.version = #{PKG_VERSION.inspect}
  s.summary = "Approximate String Matching library"
  s.description = <<EOF
Amatch is a library for approximate string matching and searching in strings.
Several algorithms can be used to do this, and it's also possible to compute a
similarity metric number between 0.0 and 1.0 for two given strings.
EOF

  s.files = #{PKG_FILES.inspect}

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
GEMSPEC

  spec = eval(gemspec)

  Rake::GemPackageTask.new(spec) do |pkg|
    pkg.need_tar      = true
    pkg.package_files += PKG_FILES
  end

  task :gemspec do
    File.open("amatch.gemspec", "w") { |f| f.write(gemspec) }
  end
end

task :release => [ :clean, :package ]
