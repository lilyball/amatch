require 'test/unit'
require 'amatch'

class TC_LCSLength < Test::Unit::TestCase
  D = 0.000001

  def setup
    @small   = Amatch.new('test')
    @empty   = Amatch.new('')
  end

  def test_empty_subsequence
    assert_in_delta 0, @empty.longest_subsequence(''), D
    assert_in_delta 0, @empty.longest_subsequence('a'), D
    assert_in_delta 0, @small.longest_subsequence(''), D
    assert_in_delta 0, @empty.longest_subsequence('not empty'), D
  end

  def test_empty_substring
    assert_in_delta 0, @empty.longest_substring(''), D
    assert_in_delta 0, @empty.longest_substring('a'), D
    assert_in_delta 0, @small.longest_substring(''), D
    assert_in_delta 0, @empty.longest_substring('not empty'), D
  end

  def test_small_subsequence
    assert_in_delta 4, @small.longest_subsequence('test'), D
    assert_in_delta 4, @small.longest_subsequence('testa'), D
    assert_in_delta 4, @small.longest_subsequence('atest'), D
    assert_in_delta 4, @small.longest_subsequence('teast'), D
    assert_in_delta 3, @small.longest_subsequence('est'), D
    assert_in_delta 3, @small.longest_subsequence('tes'), D
    assert_in_delta 3, @small.longest_subsequence('tst'), D
    assert_in_delta 3, @small.longest_subsequence('best'), D
    assert_in_delta 3, @small.longest_subsequence('tost'), D
    assert_in_delta 3, @small.longest_subsequence('tesa'), D
    assert_in_delta 2, @small.longest_subsequence('taex'), D
    assert_in_delta 1, @small.longest_subsequence('aaatbbb'), D
    assert_in_delta 1, @small.longest_subsequence('aaasbbb'), D
    assert_in_delta 4, @small.longest_subsequence('aaatestbbb'), D
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
