require 'test/unit'
require 'amatch'

class TC_LCSLength < Test::Unit::TestCase
  D = 0.000001

  def setup
    @small   = Amatch.new('test')
    @empty   = Amatch.new('')
  end

  def test_empty
    assert_in_delta 0, @empty.lcs_length(''), D
    assert_in_delta 0, @empty.lcs_length('a'), D
    assert_in_delta 0, @small.lcs_length(''), D
    assert_in_delta 0, @empty.lcs_length('not empty'), D
  end

  def test_small
    assert_in_delta 4, @small.lcs_length('test'), D
    assert_in_delta 4, @small.lcs_length('testa'), D
    assert_in_delta 4, @small.lcs_length('atest'), D
    assert_in_delta 4, @small.lcs_length('teast'), D
    assert_in_delta 3, @small.lcs_length('est'), D
    assert_in_delta 3, @small.lcs_length('tes'), D
    assert_in_delta 3, @small.lcs_length('tst'), D
    assert_in_delta 3, @small.lcs_length('best'), D
    assert_in_delta 3, @small.lcs_length('tost'), D
    assert_in_delta 3, @small.lcs_length('tesa'), D
    assert_in_delta 2, @small.lcs_length('taex'), D
    assert_in_delta 1, @small.lcs_length('aaatbbb'), D
    assert_in_delta 1, @small.lcs_length('aaasbbb'), D
    assert_in_delta 4, @small.lcs_length('aaatestbbb'), D
  end
end
  # vim: set et sw=2 ts=2:
