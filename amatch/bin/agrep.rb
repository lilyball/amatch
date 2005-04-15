#! /usr/bin/env ruby
#
## $Id$
#

require 'amatch'
require 'getoptlong'

def usage(msg, options)
  print msg, "\nUsage: #{File.basename($0)} pattern [FILE ...]\n\n"
  options.each { |o|
    print "  " + o[1] + ", " + o[0] + " " +
      (o[2] == GetoptLong::REQUIRED_ARGUMENT ? 'ARGUMENT' : '') + "\n"
  }
  print "\nReport bugs to <flori@ping.de>.\n"
  exit 0
end

$distance = 1
begin
  parser = GetoptLong.new
  options = [
    [ '--distance',  '-d',  GetoptLong::REQUIRED_ARGUMENT ],
    [ '--relative',  '-r',  GetoptLong::NO_ARGUMENT ],
    [ '--verbose',  '-v',  GetoptLong::NO_ARGUMENT ],
    [ '--help',    '-h',  GetoptLong::NO_ARGUMENT ],
  ]
  parser.set_options(*options)
  parser.each_option { |name, arg|
    name = name.sub(/^--/, '')
    case name
    when 'distance'
      $distance = arg.to_f
    when 'relative'
      $relative = 1
    when 'verbose'
      $verbose = 1
    when 'help'
      usage('You\'ve asked for it!', options)
    end
  }
rescue
  exit 1
end
pattern = ARGV.shift or usage('Pattern needed!', options)

matcher = Amatch.new(pattern)
size = 0
start = Time.new
if ARGV.size > 0 then
  ARGV.each { |filename|
    File.stat(filename).file? or next
    size += File.size(filename)
    begin
      File.open(filename, 'r').each_line { |line|
        print "#{filename}:#{line}" if
          ($relative ?  matcher.searchr(line) :
                  matcher.search(line)) <= $distance
      }
    rescue
      $stderr.print "Failure at #{filename}: #{$!} => Skipping!\n"
    end
  }
else
  $stdin.each_line { |line|
    size += line.size
    print line if ($relative ?  matcher.searchr(line) : matcher.search(line)) <= $distance
  }
end
time = Time.new - start
$verbose and $stderr.printf "%.3f secs running, scanned %.3f KB/s.\n",
  time, size / time / 1024
exit 0
  # vim: set et sw=2 ts=2:
