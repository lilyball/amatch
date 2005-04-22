#
## $Id$
#

require 'mkmf'
require 'rbconfig'
if CONFIG['CC'] = 'gcc'
  CONFIG['CC'] = 'gcc -Wall '
end
create_makefile 'amatch' 
  # vim: set et sw=2 ts=2:
