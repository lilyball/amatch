require 'test/unit'
require 'amatch'

class TC_Levenshtein < Test::Unit::TestCase
  include Amatch

  D = 0.000001

  def setup
    @empty = Levenshtein.new('')
    @simple = Levenshtein.new('test')
  end

  def test_match
    assert_in_delta 4,     @simple.match(''), D
    assert_in_delta 0,     @simple.match('test'), D
    assert_in_delta 0,     @simple.match('test'), D
    assert_in_delta 1,     @simple.match('testa'), D
    assert_in_delta 1,     @simple.match('atest'), D
    assert_in_delta 1,     @simple.match('teast'), D
    assert_in_delta 1,     @simple.match('est'), D
    assert_in_delta 1,     @simple.match('tes'), D
    assert_in_delta 1,     @simple.match('tst'), D
    assert_in_delta 1,     @simple.match('best'), D
    assert_in_delta 1,     @simple.match('tost'), D
    assert_in_delta 1,     @simple.match('tesa'), D
    assert_in_delta 3,     @simple.match('taex'), D
    assert_in_delta 6,     @simple.match('aaatestbbb'), D
  end

  def test_search
    assert_in_delta 4,     @simple.search(''), D
    assert_in_delta 0,     @empty.search(''), D
    assert_in_delta 0,     @empty.search('test'), D
    assert_in_delta 0,     @simple.search('aaatestbbb'), D
    assert_in_delta 3,     @simple.search('aaataexbbb'), D
    assert_in_delta 4,     @simple.search('aaaaaaaaa'), D
  end

  def assert_in_delta_array(left, right, delta = D)
    left.size.times do |i|
      assert_in_delta left[i], right[i], delta
    end
  end

  def test_array_result
  return
    assert_in_delta_array [2, 0],    @simple.match(["tets", "test"])
    assert_in_delta_array [1, 0],    @simple.search(["tetsaaa", "testaaa"])
    assert_raises(TypeError) { @simple.match([:foo, "bar"]) }
  end

  def test_pattern_setting
    assert_raises(TypeError) { @simple.pattern = :something }
    assert_in_delta 0, @simple.match('test'), D
    @simple.pattern = ''
    assert_in_delta 4, @simple.match('test'), D
    @simple.pattern = 'test'
    assert_in_delta 0, @simple.match('test'), D
  end
end
  # vim: set et sw=2 ts=2:
