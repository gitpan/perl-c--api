
package example;

require Exporter;

$VERSION = 1.0;

@ISA = qw(Exporter);

@EXPORT = qw();
@EXPORT_OK = qw($X $Y %H);

use strict;
use vars qw($X $Y %H);

$X = 0;
$Y = 0;
%H = ();

1;
