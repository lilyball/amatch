require 'test/unit'
require 'amatch'
require 'test_levenshtein'

class TC_Sellers < TC_Levenshtein
  include Amatch

  D = 0.000001

  def setup
    @empty    = Sellers.new('')
    @simple   = Sellers.new('test')
    @long     = Sellers.new('A' * 160)
  end

  def test_weights
    assert_in_delta 1, @simple.substitution, D
    assert_in_delta 1, @simple.insertion, D
    assert_in_delta 1, @simple.deletion, D
    @simple.insertion = 1
    @simple.substitution = @simple.deletion = 1000
    assert_in_delta 1, @simple.match('tst'), D
    assert_in_delta 1, @simple.search('bbbtstccc'), D
    @simple.deletion = 1
    @simple.substitution = @simple.insertion = 1000
    assert_in_delta 1, @simple.match('tedst'), D
    assert_in_delta 1, @simple.search('bbbtedstccc'), D
    @simple.substitution = 1
    @simple.deletion = @simple.insertion = 1000
    assert_in_delta 1, @simple.match('tast'), D
    assert_in_delta 1, @simple.search('bbbtastccc'), D
    @simple.insertion = 0.5
    @simple.substitution = @simple.deletion = 1000
    assert_in_delta 0.5, @simple.match('tst'), D
    assert_in_delta 0.5, @simple.search('bbbtstccc'), D
    @simple.deletion = 0.5
    @simple.substitution = @simple.insertion = 1000
    assert_in_delta 0.5, @simple.match('tedst'), D
    assert_in_delta 0.5, @simple.search('bbbtedstccc'), D
    @simple.substitution = 0.5
    @simple.deletion = @simple.insertion = 1000
    assert_in_delta 0.5, @simple.match('tast'), D
    assert_in_delta 0.5, @simple.search('bbbtastccc'), D
    @simple.reset_weights
    assert_in_delta 1, @simple.substitution, D
    assert_in_delta 1, @simple.insertion, D
    assert_in_delta 1, @simple.deletion, D
  end

  def test_weight_exceptions
    assert_raises(TypeError) { @simple.substitution = :something }
    assert_raises(TypeError) { @simple.insertion = :something }
    assert_raises(TypeError) { @simple.deletion = :something }
  end

  def test_similar
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
    @simple.insertion = 1
    @simple.substitution = @simple.deletion = 2
    assert_in_delta 0.875, @simple.similar('tst'), D
  end

  def test_long
    assert_in_delta 1.0, @long.similar(@long.pattern), D
  end
end
  # vim: set et sw=2 ts=2:
