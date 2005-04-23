require 'test/unit'
require 'amatch'

class TC_PairDistance < Test::Unit::TestCase
  D = 0.000001

  def setup
    @single   = Amatch.new('test')
    @empty    = Amatch.new('')
    @france   = Amatch.new('republic of france')
    @germany  = Amatch.new('federal republic of germany')
    @csv      = Amatch.new('foo,bar,baz')
  end

  def test_empty
    assert_in_delta 1, @empty.pair_distance(''), D
    assert_in_delta 0, @empty.pair_distance('not empty'), D
    assert_in_delta 0, @single.pair_distance(''), D
  end

  def test_countries
    assert_in_delta 0.5555555,  @france.pair_distance('france'), D
    assert_in_delta 0.1052631,  @france.pair_distance('germany'), D
    assert_in_delta 0.4615384,  @germany.pair_distance('germany'), D
    assert_in_delta 0.16,       @germany.pair_distance('france'), D
    assert_in_delta 0.6829268,
      @germany.pair_distance('german democratic republic'), D
    assert_in_delta 0.72,
      @france.pair_distance('french republic'), D
    assert_in_delta 0.4375,
      @germany.pair_distance('french republic'), D
    assert_in_delta 0.5294117,
      @france.pair_distance('german democratic republic'), D
  end

  def test_single
    assert_in_delta 0,          @single.pair_distance(''), D
    assert_in_delta 1,          @single.pair_distance('test'), D
    assert_in_delta 0.8571428,  @single.pair_distance('testa'), D
    assert_in_delta 0.8571428,  @single.pair_distance('atest'), D
    assert_in_delta 0.5714285,  @single.pair_distance('teast'), D
    assert_in_delta 0.8,        @single.pair_distance('est'), D
    assert_in_delta 0.8,        @single.pair_distance('tes'), D
    assert_in_delta 0.4,        @single.pair_distance('tst'), D
    assert_in_delta 0.6666666,  @single.pair_distance('best'), D
    assert_in_delta 0.3333333,  @single.pair_distance('tost'), D
    assert_in_delta 0.6666666,  @single.pair_distance('tesa'), D
    assert_in_delta 0.0,        @single.pair_distance('taex'), D
    assert_in_delta 0.5,        @single.pair_distance('aaatestbbb'), D
    assert_in_delta 0.6,        @single.pair_distance('aaa test bbb'), D
    assert_in_delta 0.6,        @single.pair_distance('test aaa bbb'), D
    assert_in_delta 0.6,        @single.pair_distance('bbb aaa test'), D
  end

  def test_csv
    assert_in_delta 0,          @csv.pair_distance('', /,/), D
    assert_in_delta 0.5,        @csv.pair_distance('foo', /,/), D
    assert_in_delta 0.5,        @csv.pair_distance('bar', /,/), D
    assert_in_delta 0.5,        @csv.pair_distance('baz', /,/), D
    assert_in_delta 0.8,        @csv.pair_distance('foo,bar', /,/), D
    assert_in_delta 0.8,        @csv.pair_distance('bar,foo', /,/), D
    assert_in_delta 0.8,        @csv.pair_distance('bar,baz', /,/), D
    assert_in_delta 0.8,        @csv.pair_distance('baz,bar', /,/), D
    assert_in_delta 0.8,        @csv.pair_distance('foo,baz', /,/), D
    assert_in_delta 0.8,        @csv.pair_distance('baz,foo', /,/), D
    assert_in_delta 1,          @csv.pair_distance('foo,bar,baz', /,/), D
    assert_in_delta 1,          @csv.pair_distance('foo,baz,bar', /,/), D
    assert_in_delta 1,          @csv.pair_distance('baz,foo,bar', /,/), D
    assert_in_delta 1,          @csv.pair_distance('baz,bar,foo', /,/), D
    assert_in_delta 1,          @csv.pair_distance('bar,foo,baz', /,/), D
    assert_in_delta 1,          @csv.pair_distance('bar,baz,foo', /,/), D
  end
end
  # vim: set et sw=2 ts=2:
