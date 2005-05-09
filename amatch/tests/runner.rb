#!/usr/bin/env ruby

require 'test/unit/ui/console/testrunner'
require 'test/unit/testsuite'
$:.unshift File.expand_path(File.dirname($0))
require 'test_levenshtein'
require 'test_sellers'
require 'test_pair_distance'
require 'test_hamming'
require 'test_longest_subsequence'
require 'test_longest_substring'

class TS_AllTests
  def self.suite
    suite = Test::Unit::TestSuite.new 'All tests'
    suite << TC_Levenshtein.suite
    suite << TC_Sellers.suite
    suite << TC_PairDistance.suite
    suite << TC_Hamming.suite
    suite << TC_LongestSubsequence.suite
    suite << TC_LongestSubstring.suite
    suite
  end
end
Test::Unit::UI::Console::TestRunner.run(TS_AllTests)
  # vim: set et sw=2 ts=2:
