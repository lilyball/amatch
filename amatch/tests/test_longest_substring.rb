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
    assert_in_delta 0, @empty.match(''), D
    assert_in_delta 0, @empty.match('a'), D
    assert_in_delta 0, @small.match(''), D
    assert_in_delta 0, @empty.match('not empty'), D
  end

  def test_small_substring
    assert_in_delta 4, @small.match('test'), D
    assert_in_delta 4, @small.match('testa'), D
    assert_in_delta 4, @small.match('atest'), D
    assert_in_delta 2, @small.match('teast'), D
    assert_in_delta 3, @small.match('est'), D
    assert_in_delta 3, @small.match('tes'), D
    assert_in_delta 2, @small.match('tst'), D
    assert_in_delta 3, @small.match('best'), D
    assert_in_delta 2, @small.match('tost'), D
    assert_in_delta 3, @small.match('tesa'), D
    assert_in_delta 1, @small.match('taex'), D
    assert_in_delta 1, @small.match('aaatbbb'), D
    assert_in_delta 1, @small.match('aaasbbb'), D
    assert_in_delta 4, @small.match('aaatestbbb'), D
  end
end
  # vim: set et sw=2 ts=2:
