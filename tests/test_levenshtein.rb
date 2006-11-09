require 'test/unit'
require 'amatch'

class TC_Levenshtein < Test::Unit::TestCase
  include Amatch

  D = 0.000001
  
  def setup
    @empty    = Levenshtein.new('')
    @simple   = Levenshtein.new('test')
    @long     = Levenshtein.new('A' * 160)
  end

  def test_long
    a = "lost this fantasy,  this fantasy,  this  fantasy,  this fantasy,  this fantasy,  this fantasy\r\n\r\nGood love  Neat work\r\n\r\nSuper job Fancy work\r\n\r\nPants job Cool work"
    b = "lost\r\n\r\nGood love Neat work\r\n\r\nSuper  job Fancy work\r\n\r\nPants job Cool work"
    p a.levenshtein_similar b 
  end

  def test_match
    assert_equal 4,     @simple.match('')
    assert_equal 0,     @simple.match('test')
    assert_equal 0,     @simple.match('test')
    assert_equal 1,     @simple.match('testa')
    assert_equal 1,     @simple.match('atest')
    assert_equal 1,     @simple.match('teast')
    assert_equal 1,     @simple.match('est')
    assert_equal 1,     @simple.match('tes')
    assert_equal 1,     @simple.match('tst')
    assert_equal 1,     @simple.match('best')
    assert_equal 1,     @simple.match('tost')
    assert_equal 1,     @simple.match('tesa')
    assert_equal 3,     @simple.match('taex')
    assert_equal 6,     @simple.match('aaatestbbb')
  end

  def test_search
    assert_equal 4,     @simple.search('')
    assert_equal 0,     @empty.search('')
    assert_equal 0,     @empty.search('test')
    assert_equal 0,     @simple.search('aaatestbbb')
    assert_equal 3,     @simple.search('aaataexbbb')
    assert_equal 4,     @simple.search('aaaaaaaaa')
  end

  def test_array_result
    assert_equal [2, 0],    @simple.match(["tets", "test"])
    assert_equal [1, 0],    @simple.search(["tetsaaa", "testaaa"])
    assert_raises(TypeError) { @simple.match([:foo, "bar"]) }
  end

  def test_pattern_setting
    assert_raises(TypeError) { @simple.pattern = :something }
    assert_equal 0, @simple.match('test')
    @simple.pattern = ''
    assert_equal 4, @simple.match('test')
    @simple.pattern = 'test'
    assert_equal 0, @simple.match('test')
  end

  def test_similar
    assert_in_delta 1, @empty.similar(''), D
    assert_in_delta 0, @empty.similar('not empty'), D
    assert_in_delta 0.0, @simple.similar(''), D
    assert_in_delta 1.0, @simple.similar('test'), D
    assert_in_delta 0.8, @simple.similar('testa'), D
    assert_in_delta 0.8, @simple.similar('atest'), D
    assert_in_delta 0.8, @simple.similar('teast'), D
    assert_in_delta 0.75, @simple.similar('est'), D
    assert_in_delta 0.75, @simple.similar('tes'), D
    assert_in_delta 0.75, @simple.similar('tst'), D
    assert_in_delta 0.75, @simple.similar('best'), D
    assert_in_delta 0.75, @simple.similar('tost'), D
    assert_in_delta 0.75, @simple.similar('tesa'), D
    assert_in_delta 0.25, @simple.similar('taex'), D
    assert_in_delta 0.4, @simple.similar('aaatestbbb'), D
    assert_in_delta 0.75, @simple.pattern.levenshtein_similar('est'), D
  end

  def test_long
    assert_in_delta 1.0, @long.similar(@long.pattern), D
  end
end
  # vim: set et sw=2 ts=2:
