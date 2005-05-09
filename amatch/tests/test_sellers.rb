require 'test/unit'
require 'amatch'
require 'test_levenshtein'

class TC_Sellers < TC_Levenshtein
  include Amatch

  D = 0.000001

  def setup
    @empty = Sellers.new('')
    @simple = Sellers.new('test')
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
end
  # vim: set et sw=2 ts=2:
