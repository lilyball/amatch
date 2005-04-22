require 'test/unit'
require 'amatch'

class TC_PairDistance < Test::Unit::TestCase
  DELTA = 0.000001

  def setup
    @matcher = Amatch.new('test')
    @empty = Amatch.new('')
    @france = Amatch.new('republic of france')
    @germany = Amatch.new('federal republic of germany')
  end

  def test_countries
    assert_in_delta 1.0,       @empty.pair_distance(''), DELTA
    assert_in_delta 0.5555555, @france.pair_distance('france'), DELTA
    assert_in_delta 0.1052631, @france.pair_distance('germany'), DELTA
    assert_in_delta 0.4615384, @germany.pair_distance('germany'), DELTA
  end

  def test_matcher
    assert_in_delta 0,          @matcher.pair_distance(''), DELTA
    assert_in_delta 1,          @matcher.pair_distance('test'), DELTA
    assert_in_delta 0.8571428,  @matcher.pair_distance('testa'), DELTA
    assert_in_delta 0.8571428,  @matcher.pair_distance('atest'), DELTA
    assert_in_delta 0.5714285,  @matcher.pair_distance('teast'), DELTA
    assert_in_delta 0.8,        @matcher.pair_distance('est'), DELTA
    assert_in_delta 0.8,        @matcher.pair_distance('tes'), DELTA
    assert_in_delta 0.4,        @matcher.pair_distance('tst'), DELTA
    assert_in_delta 0.6666666,  @matcher.pair_distance('best'), DELTA
    assert_in_delta 0.3333333,  @matcher.pair_distance('tost'), DELTA
    assert_in_delta 0.6666666,  @matcher.pair_distance('tesa'), DELTA
    assert_in_delta 0.0,        @matcher.pair_distance('taex'), DELTA
    assert_in_delta 0.5,        @matcher.pair_distance('aaatestbbb'), DELTA
  end
end
  # vim: set et sw=2 ts=2:
