
sub add {
   my($a, $b) = @_;
   return $a + $b;
}

$x = 1;
$y = 2;

$r = 0;

$r = add($x, $y);

print "r = $r\n";
