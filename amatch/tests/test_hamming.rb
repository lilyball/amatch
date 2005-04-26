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
    assert_in_delta 0, @empty.match(''), D
    assert_in_delta 9, @empty.match('not empty'), D
  end

  def test_small
    assert_in_delta 4,          @small.match(''), D
    assert_in_delta 0,          @small.match('test'), D
    assert_in_delta 1,          @small.match('testa'), D
    assert_in_delta 5,          @small.match('atest'), D
    assert_in_delta 3,          @small.match('teast'), D
    assert_in_delta 4,          @small.match('est'), D
    assert_in_delta 1,          @small.match('tes'), D
    assert_in_delta 3,          @small.match('tst'), D
    assert_in_delta 1,          @small.match('best'), D
    assert_in_delta 1,          @small.match('tost'), D
    assert_in_delta 1,          @small.match('tesa'), D
    assert_in_delta 3,          @small.match('taex'), D
    assert_in_delta 9,          @small.match('aaatestbbb'), D
  end
end
  # vim: set et sw=2 ts=2:
