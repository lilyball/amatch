require 'test/unit'
require 'amatch'

class TC_Levenshtein < Test::Unit::TestCase
  D = 0.000001

  def setup
    @matcher = Amatch.new('test')
  end

  def test_match
    assert_in_delta 4,     @matcher.match(''), D
    assert_in_delta 0,     @matcher.match('test'), D
    assert_in_delta 0,     @matcher.match('test'), D
    assert_in_delta 1,     @matcher.match('testa'), D
    assert_in_delta 1,     @matcher.match('atest'), D
    assert_in_delta 1,     @matcher.match('teast'), D
    assert_in_delta 1,     @matcher.match('est'), D
    assert_in_delta 1,     @matcher.match('tes'), D
    assert_in_delta 1,     @matcher.match('tst'), D
    assert_in_delta 1,     @matcher.match('best'), D
    assert_in_delta 1,     @matcher.match('tost'), D
    assert_in_delta 1,     @matcher.match('tesa'), D
    assert_in_delta 3,     @matcher.match('taex'), D
    assert_in_delta 1,     @matcher.matchr(''), D
    assert_in_delta 0.25,  @matcher.matchr('tesa'), D
    assert_in_delta 6,     @matcher.match('aaatestbbb'), D
  end

  def test_search
    assert_in_delta 4,     @matcher.search(''), D
    assert_in_delta 1,     @matcher.searchr(''), D
    assert_in_delta 0,     @matcher.search('aaatestbbb'), D
    assert_in_delta 3,     @matcher.search('aaataexbbb'), D
    assert_in_delta 0.75,  @matcher.searchr('aaataexbbb'), D
    assert_in_delta 4,     @matcher.search('aaaaaaaaa'), D
    assert_in_delta 1,     @matcher.searchr('aaaaaaaaa'), D
  end

  def test_compare
    assert_in_delta -4,    @matcher.compare(''), D
    assert_in_delta -1,    @matcher.comparer(''), D
    assert_in_delta 3,     @matcher.compare('taex'), D
    assert_in_delta 0.25,  @matcher.comparer('tesa'), D
    assert_in_delta 6,     @matcher.compare('aaatestbbb'), D
    assert_in_delta 0,     @matcher.compare('test'), D
    assert_in_delta -2,    @matcher.compare('tex'), D
    assert_in_delta -0.5,  @matcher.comparer('tsa'), D
    assert_in_delta 4,     @matcher.compare('wxyz'), D
    assert_in_delta 1,     @matcher.comparer('wxyz'), D
    assert_raises(TypeError) { @matcher.match(:foo) }
  end

  def assert_in_delta_array(left, right, delta = D)
    left.size.times do |i|
      assert_in_delta left[i], right[i], delta
    end
  end

  def test_array_result
    assert_in_delta_array [2, 0],    @matcher.match(["tets", "test"])
    assert_in_delta_array [0.5, 0],  @matcher.matchr(["tets", "test"])
    assert_in_delta_array [2, 0],    @matcher.compare(["tets", "test"])
    assert_in_delta_array [0.5, 0],  @matcher.comparer(["tets", "test"])
    assert_in_delta_array [1, 0],    @matcher.search(["tetsaaa", "testaaa"])
    assert_in_delta_array [0.25, 0], @matcher.searchr(["tetsaaa", "testaaa"])
    assert_raises(TypeError) { @matcher.match([:foo, "bar"]) }
  end

  def test_weights
    assert_in_delta 1, @matcher.substitution, D
    assert_in_delta 1, @matcher.insertion, D
    assert_in_delta 1, @matcher.deletion, D
    @matcher.substitution = 2
    assert_in_delta 2, @matcher.substitution, D
    assert_in_delta 2, @matcher.match('tast'), D
    @matcher.substitution = 1
    assert_in_delta 1, @matcher.match('tast'), D
    @matcher.insertion = 2
    assert_in_delta 2, @matcher.insertion, D
    assert_in_delta 2, @matcher.match('teist'), D
    @matcher.deletion = 2
    assert_in_delta 2, @matcher.deletion, D
    assert_in_delta 2, @matcher.match('tst'), D
    @matcher.reset_weights
    assert_in_delta 1, @matcher.substitution, D
    assert_in_delta 1, @matcher.insertion, D
    assert_in_delta 1, @matcher.deletion, D
    @matcher.substitution = 0.5
    assert_in_delta 0.5, @matcher.substitution, D
    assert_in_delta 0.5, @matcher.match('tast'), D
    @matcher.substitution = 1
    assert_in_delta 1, @matcher.match('tast'), D
    @matcher.insertion = 0.5 
    assert_in_delta 0.5, @matcher.insertion, D
    assert_in_delta 0.5, @matcher.match('teist'), D
    @matcher.deletion = 0.5
    assert_in_delta 0.5, @matcher.deletion, D
    assert_in_delta 0.5, @matcher.match('tst'), D
  end


  def test_pattern_setting
    assert_raises(TypeError) { @matcher.pattern = :something }
    @matcher.pattern = 'test'
    assert_raises(TypeError) { @matcher.substitution = :something }
    assert_raises(TypeError) { @matcher.insertion = :something }
    assert_raises(TypeError) { @matcher.deletion = :something }
    assert_in_delta 0, @matcher.match('test'), D
  end
end
  # vim: set et sw=2 ts=2:
