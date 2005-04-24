require 'test/unit'
require 'amatch'

class TC_LCSLength < Test::Unit::TestCase
  D = 0.000001

  def setup
    @small   = Amatch.new('test')
    @empty   = Amatch.new('')
  end

  def test_empty
    assert_in_delta 0, @empty.lc_subsequence(''), D
    assert_in_delta 0, @empty.lc_subsequence('a'), D
    assert_in_delta 0, @small.lc_subsequence(''), D
    assert_in_delta 0, @empty.lc_subsequence('not empty'), D
  end

  def test_small
    assert_in_delta 4, @small.lc_subsequence('test'), D
    assert_in_delta 4, @small.lc_subsequence('testa'), D
    assert_in_delta 4, @small.lc_subsequence('atest'), D
    assert_in_delta 4, @small.lc_subsequence('teast'), D
    assert_in_delta 3, @small.lc_subsequence('est'), D
    assert_in_delta 3, @small.lc_subsequence('tes'), D
    assert_in_delta 3, @small.lc_subsequence('tst'), D
    assert_in_delta 3, @small.lc_subsequence('best'), D
    assert_in_delta 3, @small.lc_subsequence('tost'), D
    assert_in_delta 3, @small.lc_subsequence('tesa'), D
    assert_in_delta 2, @small.lc_subsequence('taex'), D
    assert_in_delta 1, @small.lc_subsequence('aaatbbb'), D
    assert_in_delta 1, @small.lc_subsequence('aaasbbb'), D
    assert_in_delta 4, @small.lc_subsequence('aaatestbbb'), D
  end
end
  # vim: set et sw=2 ts=2:
