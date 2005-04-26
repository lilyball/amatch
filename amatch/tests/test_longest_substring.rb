require 'test/unit'
require 'amatch'

class TC_LongestSubstring < Test::Unit::TestCase
  include Amatch

  D = 0.000001

  def setup
    @small   = LongestSubstring.new('test')
    @empty   = LongestSubstring.new('')
  end

  def test_empty_substring
    assert_in_delta 0, @empty.longest_substring(''), D
    assert_in_delta 0, @empty.longest_substring('a'), D
    assert_in_delta 0, @small.longest_substring(''), D
    assert_in_delta 0, @empty.longest_substring('not empty'), D
  end

  def test_small_substring
    assert_in_delta 4, @small.longest_substring('test'), D
    assert_in_delta 4, @small.longest_substring('testa'), D
    assert_in_delta 4, @small.longest_substring('atest'), D
    assert_in_delta 2, @small.longest_substring('teast'), D
    assert_in_delta 3, @small.longest_substring('est'), D
    assert_in_delta 3, @small.longest_substring('tes'), D
    assert_in_delta 2, @small.longest_substring('tst'), D
    assert_in_delta 3, @small.longest_substring('best'), D
    assert_in_delta 2, @small.longest_substring('tost'), D
    assert_in_delta 3, @small.longest_substring('tesa'), D
    assert_in_delta 1, @small.longest_substring('taex'), D
    assert_in_delta 1, @small.longest_substring('aaatbbb'), D
    assert_in_delta 1, @small.longest_substring('aaasbbb'), D
    assert_in_delta 4, @small.longest_substring('aaatestbbb'), D
  end
end
  # vim: set et sw=2 ts=2:
