require 'test/unit'
require 'amatch'

class TC_Levenshtein < Test::Unit::TestCase
  D = 0.000001

  def setup
    @matcher = Amatch.new('test')
  end

  def test_match
    assert_in_delta 4,     @matcher.l_match(''), D
    assert_in_delta 0,     @matcher.l_match('test'), D
    assert_in_delta 0,     @matcher.l_match('test'), D
    assert_in_delta 1,     @matcher.l_match('testa'), D
    assert_in_delta 1,     @matcher.l_match('atest'), D
    assert_in_delta 1,     @matcher.l_match('teast'), D
    assert_in_delta 1,     @matcher.l_match('est'), D
    assert_in_delta 1,     @matcher.l_match('tes'), D
    assert_in_delta 1,     @matcher.l_match('tst'), D
    assert_in_delta 1,     @matcher.l_match('best'), D
    assert_in_delta 1,     @matcher.l_match('tost'), D
    assert_in_delta 1,     @matcher.l_match('tesa'), D
    assert_in_delta 3,     @matcher.l_match('taex'), D
    assert_in_delta 6,     @matcher.l_match('aaatestbbb'), D
  end

  def test_search
    assert_in_delta 4,     @matcher.l_search(''), D
    assert_in_delta 0,     @matcher.l_search('aaatestbbb'), D
    assert_in_delta 3,     @matcher.l_search('aaataexbbb'), D
    assert_in_delta 4,     @matcher.l_search('aaaaaaaaa'), D
  end

  def test_compare
    assert_in_delta -4,    @matcher.l_compare(''), D
    assert_in_delta 3,     @matcher.l_compare('taex'), D
    assert_in_delta 6,     @matcher.l_compare('aaatestbbb'), D
    assert_in_delta 0,     @matcher.l_compare('test'), D
    assert_in_delta -2,    @matcher.l_compare('tex'), D
    assert_in_delta 4,     @matcher.l_compare('wxyz'), D
    assert_raises(TypeError) { @matcher.l_match(:foo) }
  end

  def assert_in_delta_array(left, right, delta = D)
    left.size.times do |i|
      assert_in_delta left[i], right[i], delta
    end
  end

  def test_array_result
    assert_in_delta_array [2, 0],    @matcher.l_match(["tets", "test"])
    assert_in_delta_array [2, 0],    @matcher.l_compare(["tets", "test"])
    assert_in_delta_array [1, 0],    @matcher.l_search(["tetsaaa", "testaaa"])
    assert_raises(TypeError) { @matcher.l_match([:foo, "bar"]) }
  end

  def test_weights
    assert_in_delta 1, @matcher.substitution, D
    assert_in_delta 1, @matcher.insertion, D
    assert_in_delta 1, @matcher.deletion, D
    @matcher.substitution = 2
    assert_in_delta 2, @matcher.substitution, D
    assert_in_delta 2, @matcher.l_match('tast'), D
    @matcher.substitution = 1
    assert_in_delta 1, @matcher.l_match('tast'), D
    @matcher.insertion = 2
    assert_in_delta 2, @matcher.insertion, D
    assert_in_delta 2, @matcher.l_match('teist'), D
    @matcher.deletion = 2
    assert_in_delta 2, @matcher.deletion, D
    assert_in_delta 2, @matcher.l_match('tst'), D
    @matcher.reset_weights
    assert_in_delta 1, @matcher.substitution, D
    assert_in_delta 1, @matcher.insertion, D
    assert_in_delta 1, @matcher.deletion, D
    @matcher.substitution = 0.5
    assert_in_delta 0.5, @matcher.substitution, D
    assert_in_delta 0.5, @matcher.l_match('tast'), D
    @matcher.substitution = 1
    assert_in_delta 1, @matcher.l_match('tast'), D
    @matcher.insertion = 0.5 
    assert_in_delta 0.5, @matcher.insertion, D
    assert_in_delta 0.5, @matcher.l_match('teist'), D
    @matcher.deletion = 0.5
    assert_in_delta 0.5, @matcher.deletion, D
    assert_in_delta 0.5, @matcher.l_match('tst'), D
  end


  def test_pattern_setting
    assert_raises(TypeError) { @matcher.pattern = :something }
    @matcher.pattern = 'test'
    assert_raises(TypeError) { @matcher.substitution = :something }
    assert_raises(TypeError) { @matcher.insertion = :something }
    assert_raises(TypeError) { @matcher.deletion = :something }
    assert_in_delta 0, @matcher.l_match('test'), D
  end
end
  # vim: set et sw=2 ts=2:
