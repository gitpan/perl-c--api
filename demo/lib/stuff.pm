
package stuff;

require Exporter;

$VERSION = 1.0;

@ISA = qw(Exporter);

@EXPORT = qw();
@EXPORT_OK = qw($c f);

use strict;
use vars qw($c);

$c = 10;

sub f {
    return 20;
}

sub g {
    return 40;
}

1;
