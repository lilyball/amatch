require 'test/unit'
require 'amatch'

class TC_Levenshtein < Test::Unit::TestCase
  D = 0.000001

  def setup
    @empty = Amatch.new('')
    @simple = Amatch.new('test')
  end

  def test_match
    assert_in_delta 4,     @simple.levenshtein_match(''), D
    assert_in_delta 0,     @simple.levenshtein_match('test'), D
    assert_in_delta 0,     @simple.levenshtein_match('test'), D
    assert_in_delta 1,     @simple.levenshtein_match('testa'), D
    assert_in_delta 1,     @simple.levenshtein_match('atest'), D
    assert_in_delta 1,     @simple.levenshtein_match('teast'), D
    assert_in_delta 1,     @simple.levenshtein_match('est'), D
    assert_in_delta 1,     @simple.levenshtein_match('tes'), D
    assert_in_delta 1,     @simple.levenshtein_match('tst'), D
    assert_in_delta 1,     @simple.levenshtein_match('best'), D
    assert_in_delta 1,     @simple.levenshtein_match('tost'), D
    assert_in_delta 1,     @simple.levenshtein_match('tesa'), D
    assert_in_delta 3,     @simple.levenshtein_match('taex'), D
    assert_in_delta 6,     @simple.levenshtein_match('aaatestbbb'), D
  end

  def test_search
    assert_in_delta 4,     @simple.levenshtein_search(''), D
    assert_in_delta 0,     @empty.levenshtein_search(''), D
    assert_in_delta 0,     @empty.levenshtein_search('test'), D
    assert_in_delta 0,     @simple.levenshtein_search('aaatestbbb'), D
    assert_in_delta 3,     @simple.levenshtein_search('aaataexbbb'), D
    assert_in_delta 4,     @simple.levenshtein_search('aaaaaaaaa'), D
  end

  def assert_in_delta_array(left, right, delta = D)
    left.size.times do |i|
      assert_in_delta left[i], right[i], delta
    end
  end

  def test_array_result
  return
    assert_in_delta_array [2, 0],    @simple.levenshtein_match(["tets", "test"])
    assert_in_delta_array [1, 0],    @simple.levenshtein_search(["tetsaaa", "testaaa"])
    assert_raises(TypeError) { @simple.levenshtein_match([:foo, "bar"]) }
  end

  def test_weights
    assert_in_delta 1, @simple.substitution, D
    assert_in_delta 1, @simple.insertion, D
    assert_in_delta 1, @simple.deletion, D
    @simple.insertion = 1
    @simple.substitution = @simple.deletion = 1000
    assert_in_delta 1, @simple.levenshtein_match('tst'), D
    assert_in_delta 1, @simple.levenshtein_search('bbbtstccc'), D
    @simple.deletion = 1
    @simple.substitution = @simple.insertion = 1000
    assert_in_delta 1, @simple.levenshtein_match('tedst'), D
    assert_in_delta 1, @simple.levenshtein_search('bbbtedstccc'), D
    @simple.substitution = 1
    @simple.deletion = @simple.insertion = 1000
    assert_in_delta 1, @simple.levenshtein_match('tast'), D
    assert_in_delta 1, @simple.levenshtein_search('bbbtastccc'), D
    @simple.insertion = 0.5
    @simple.substitution = @simple.deletion = 1000
    assert_in_delta 0.5, @simple.levenshtein_match('tst'), D
    assert_in_delta 0.5, @simple.levenshtein_search('bbbtstccc'), D
    @simple.deletion = 0.5
    @simple.substitution = @simple.insertion = 1000
    assert_in_delta 0.5, @simple.levenshtein_match('tedst'), D
    assert_in_delta 0.5, @simple.levenshtein_search('bbbtedstccc'), D
    @simple.substitution = 0.5
    @simple.deletion = @simple.insertion = 1000
    assert_in_delta 0.5, @simple.levenshtein_match('tast'), D
    assert_in_delta 0.5, @simple.levenshtein_search('bbbtastccc'), D
    @simple.reset_weights
    assert_in_delta 1, @simple.substitution, D
    assert_in_delta 1, @simple.insertion, D
    assert_in_delta 1, @simple.deletion, D
  end


  def test_pattern_setting
    assert_raises(TypeError) { @simple.pattern = :something }
    @simple.pattern = 'test'
    assert_raises(TypeError) { @simple.substitution = :something }
    assert_raises(TypeError) { @simple.insertion = :something }
    assert_raises(TypeError) { @simple.deletion = :something }
    assert_in_delta 0, @simple.levenshtein_match('test'), D
  end
end
  # vim: set et sw=2 ts=2:
