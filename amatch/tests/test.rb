require 'test/unit'
require 'amatch'

class TC_Levenshtein < Test::Unit::TestCase
  def setup
    @matcher = Amatch.new('test')
  end

  def test_match
    assert_equal 4,     @matcher.match('')
    assert_equal 0,     @matcher.match('test')
    assert_equal 0,     @matcher.match('test')
    assert_equal 1,     @matcher.match('testa')
    assert_equal 1,     @matcher.match('atest')
    assert_equal 1,     @matcher.match('teast')
    assert_equal 1,     @matcher.match('est')
    assert_equal 1,     @matcher.match('tes')
    assert_equal 1,     @matcher.match('tst')
    assert_equal 1,     @matcher.match('best')
    assert_equal 1,     @matcher.match('tost')
    assert_equal 1,     @matcher.match('tesa')
    assert_equal 3,     @matcher.match('taex')
    assert_equal 1.0,   @matcher.matchr('')
    assert_equal 0.25,  @matcher.matchr('tesa')
    assert_equal 6,     @matcher.match('aaatestbbb')
  end

  def test_search
    assert_equal 4,     @matcher.search('')
    assert_equal 1.0,   @matcher.searchr('')
    assert_equal 0,     @matcher.search('aaatestbbb')
    assert_equal 3,     @matcher.search('aaataexbbb')
    assert_equal 0.75,  @matcher.searchr('aaataexbbb')
    assert_equal 4,     @matcher.search('aaaaaaaaa')
    assert_equal 1.0,   @matcher.searchr('aaaaaaaaa')
  end

  def test_compare
    assert_equal -4,    @matcher.compare('')
    assert_equal -1.0,  @matcher.comparer('')
    assert_equal 3,     @matcher.compare('taex')
    assert_equal 0.25,  @matcher.comparer('tesa')
    assert_equal 6,     @matcher.compare('aaatestbbb')
    assert_equal 0,     @matcher.compare('test')
    assert_equal -2,    @matcher.compare('tex')
    assert_equal -0.5,  @matcher.comparer('tsa')
    assert_equal 4,     @matcher.compare('wxyz')
    assert_equal 1.0,   @matcher.comparer('wxyz')
    assert_raises(TypeError) { @matcher.match(:foo) }
  end

  def test_array_result
    assert_equal [2, 0],    @matcher.match("tets", "test");
    assert_equal [0.5, 0],  @matcher.matchr("tets", "test");
    assert_equal [2, 0],    @matcher.compare("tets", "test");
    assert_equal [0.5, 0],  @matcher.comparer("tets", "test");
    assert_equal [1, 0],    @matcher.search("tetsaaa", "testaaa");
    assert_equal [0.25, 0], @matcher.searchr("tetsaaa", "testaaa");
    assert_raises(TypeError) { @matcher.match(:foo, "bar") }
  end

  def test_weights
    assert_equal 1, @matcher.subw
    assert_equal 1, @matcher.insw
    assert_equal 1, @matcher.delw
    @matcher.subw = 2
    assert_equal 2, @matcher.subw
    assert_equal 2, @matcher.match('tast')
    @matcher.subw = 1
    assert_equal 1, @matcher.match('tast')
    @matcher.insw = 2
    assert_equal 2, @matcher.insw
    assert_equal 2, @matcher.match('teist')
    @matcher.delw = 2
    assert_equal 2, @matcher.delw
    assert_equal 2, @matcher.match('tst')
    @matcher.resetw  
    assert_equal 1, @matcher.subw
    assert_equal 1, @matcher.insw
    assert_equal 1, @matcher.delw
    assert_raises(TypeError) { @matcher.pattern = :something }
    @matcher.pattern = 'test'
    assert_raises(TypeError) { @matcher.subw = :something }
    @matcher.subw = 1
    assert_raises(TypeError) { @matcher.insw = :something }
    @matcher.insw = 1
    assert_raises(TypeError) { @matcher.delw = :something }
    @matcher.delw = 1
    assert_equal 0, @matcher.match('test')
  end
end
  # vim: set et sw=2 ts=2:
