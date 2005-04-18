require 'test/unit'
require 'amatch'

class TC_PairDistance < Test::Unit::TestCase
  def setup
    @matcher = Amatch.new('test')
  end

  def test_match
    assert_equal 4,     @matcher.pair_distance(//, '')
    assert_equal 0,     @matcher.pair_distance(//, 'test')
    assert_equal 0,     @matcher.pair_distance(//, 'test')
    assert_equal 1,     @matcher.pair_distance(//, 'testa')
    assert_equal 1,     @matcher.pair_distance(//, 'atest')
    assert_equal 1,     @matcher.pair_distance(//, 'teast')
    assert_equal 1,     @matcher.pair_distance(//, 'est')
    assert_equal 1,     @matcher.pair_distance(//, 'tes')
    assert_equal 1,     @matcher.pair_distance(//, 'tst')
    assert_equal 1,     @matcher.pair_distance(//, 'best')
    assert_equal 1,     @matcher.pair_distance(//, 'tost')
    assert_equal 1,     @matcher.pair_distance(//, 'tesa')
    assert_equal 3,     @matcher.pair_distance(//, 'taex')
    assert_equal 6,     @matcher.pair_distance(//, 'aaatestbbb')
  end
end
  # vim: set et sw=2 ts=2:
