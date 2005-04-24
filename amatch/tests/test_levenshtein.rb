require 'test/unit'
require 'amatch'

class TC_Levenshtein < Test::Unit::TestCase
  D = 0.000001

  def setup
    @empty = Amatch.new('')
    @simple = Amatch.new('test')
  end

  def test_match
    assert_in_delta 4,     @simple.l_match(''), D
    assert_in_delta 0,     @simple.l_match('test'), D
    assert_in_delta 0,     @simple.l_match('test'), D
    assert_in_delta 1,     @simple.l_match('testa'), D
    assert_in_delta 1,     @simple.l_match('atest'), D
    assert_in_delta 1,     @simple.l_match('teast'), D
    assert_in_delta 1,     @simple.l_match('est'), D
    assert_in_delta 1,     @simple.l_match('tes'), D
    assert_in_delta 1,     @simple.l_match('tst'), D
    assert_in_delta 1,     @simple.l_match('best'), D
    assert_in_delta 1,     @simple.l_match('tost'), D
    assert_in_delta 1,     @simple.l_match('tesa'), D
    assert_in_delta 3,     @simple.l_match('taex'), D
    assert_in_delta 6,     @simple.l_match('aaatestbbb'), D
  end

  def test_search
    assert_in_delta 4,     @simple.l_search(''), D
    assert_in_delta 0,     @empty.l_search(''), D
    assert_in_delta 0,     @empty.l_search('test'), D
    assert_in_delta 0,     @simple.l_search('aaatestbbb'), D
    assert_in_delta 3,     @simple.l_search('aaataexbbb'), D
    assert_in_delta 4,     @simple.l_search('aaaaaaaaa'), D
  end

  def test_compare
    assert_in_delta -4,    @simple.l_compare(''), D
    assert_in_delta 3,     @simple.l_compare('taex'), D
    assert_in_delta 6,     @simple.l_compare('aaatestbbb'), D
    assert_in_delta 0,     @simple.l_compare('test'), D
    assert_in_delta -2,    @simple.l_compare('tex'), D
    assert_in_delta 4,     @simple.l_compare('wxyz'), D
    assert_raises(TypeError) { @simple.l_match(:foo) }
  end

  def assert_in_delta_array(left, right, delta = D)
    left.size.times do |i|
      assert_in_delta left[i], right[i], delta
    end
  end

  def test_array_result
  return
    assert_in_delta_array [2, 0],    @simple.l_match(["tets", "test"])
    assert_in_delta_array [2, 0],    @simple.l_compare(["tets", "test"])
    assert_in_delta_array [1, 0],    @simple.l_search(["tetsaaa", "testaaa"])
    assert_raises(TypeError) { @simple.l_match([:foo, "bar"]) }
  end

  def test_weights
    assert_in_delta 1, @simple.substitution, D
    assert_in_delta 1, @simple.insertion, D
    assert_in_delta 1, @simple.deletion, D
    @simple.substitution = 2
    assert_in_delta 2, @simple.substitution, D
    assert_in_delta 2, @simple.l_match('tast'), D
    @simple.substitution = 1
    assert_in_delta 1, @simple.l_match('tast'), D
    @simple.insertion = 2
    assert_in_delta 2, @simple.insertion, D
    assert_in_delta 2, @simple.l_match('teist'), D
    @simple.deletion = 2
    assert_in_delta 2, @simple.deletion, D
    assert_in_delta 2, @simple.l_match('tst'), D
    @simple.reset_weights
    assert_in_delta 1, @simple.substitution, D
    assert_in_delta 1, @simple.insertion, D
    assert_in_delta 1, @simple.deletion, D
    @simple.substitution = 0.5
    assert_in_delta 0.5, @simple.substitution, D
    assert_in_delta 0.5, @simple.l_match('tast'), D
    @simple.substitution = 1
    assert_in_delta 1, @simple.l_match('tast'), D
    @simple.insertion = 0.5 
    assert_in_delta 0.5, @simple.insertion, D
    assert_in_delta 0.5, @simple.l_match('teist'), D
    @simple.deletion = 0.5
    assert_in_delta 0.5, @simple.deletion, D
    assert_in_delta 0.5, @simple.l_match('tst'), D
  end


  def test_pattern_setting
    assert_raises(TypeError) { @simple.pattern = :something }
    @simple.pattern = 'test'
    assert_raises(TypeError) { @simple.substitution = :something }
    assert_raises(TypeError) { @simple.insertion = :something }
    assert_raises(TypeError) { @simple.deletion = :something }
    assert_in_delta 0, @simple.l_match('test'), D
  end
end
  # vim: set et sw=2 ts=2:
