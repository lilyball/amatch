require 'test/unit'
require 'amatch'

class TC_Hamming < Test::Unit::TestCase
  include Amatch

  D = 0.000001

  def setup
    @small   = Hamming.new('test')
    @empty   = Hamming.new('')
  end

  def test_empty
    assert_in_delta 0, @empty.hamming(''), D
    assert_in_delta 9, @empty.hamming('not empty'), D
  end

  def test_small
    assert_in_delta 4,          @small.hamming(''), D
    assert_in_delta 0,          @small.hamming('test'), D
    assert_in_delta 1,          @small.hamming('testa'), D
    assert_in_delta 5,          @small.hamming('atest'), D
    assert_in_delta 3,          @small.hamming('teast'), D
    assert_in_delta 4,          @small.hamming('est'), D
    assert_in_delta 1,          @small.hamming('tes'), D
    assert_in_delta 3,          @small.hamming('tst'), D
    assert_in_delta 1,          @small.hamming('best'), D
    assert_in_delta 1,          @small.hamming('tost'), D
    assert_in_delta 1,          @small.hamming('tesa'), D
    assert_in_delta 3,          @small.hamming('taex'), D
    assert_in_delta 9,          @small.hamming('aaatestbbb'), D
  end
end
  # vim: set et sw=2 ts=2:
